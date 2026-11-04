#include "window.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSlider>
#include <QVector2D>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
struct Vertex {
    float x;
    float y;
};

void addTriangle(std::vector<Vertex>& vertices, QPointF a, QPointF b, QPointF c)
{
    for (const auto& p : {a, b, c})
        vertices.push_back({static_cast<float>(p.x()), static_cast<float>(p.y())});
}

void addKochEdge(std::vector<Vertex>& vertices, QPointF a, QPointF b, int depth)
{
    if (depth == 0)
        return;

    const auto third = (b - a) / 3.0;
    const auto p = a + third;
    const auto q = b - third;
    const auto peak = p + QPointF(0.5 * third.x() + std::sqrt(3.0) / 2.0 * third.y(),
                                  -std::sqrt(3.0) / 2.0 * third.x() + 0.5 * third.y());
    addTriangle(vertices, p, peak, q);
    addKochEdge(vertices, a, p, depth - 1);
    addKochEdge(vertices, p, peak, depth - 1);
    addKochEdge(vertices, peak, q, depth - 1);
    addKochEdge(vertices, q, b, depth - 1);
}
}

Window::Window()
{
    setWindowTitle(QStringLiteral("Снежинка Коха"));
    setMinimumSize(540, 360);

    auto* panel = new QWidget(this);
    panel->setStyleSheet("background: #eeeeee; color: #202020;");
    auto* layout = new QHBoxLayout(panel);
    auto* iterationLabel = new QLabel(this);
    auto* slider = new QSlider(Qt::Horizontal, panel);
    slider->setRange(0, 7);
    slider->setValue(iterations_);
    slider->setMinimumWidth(120);
    slider->setAccessibleName(QStringLiteral("Число итераций"));
    iterationLabel->setText(QStringLiteral("Итерации: %1").arg(iterations_));
    auto* reset = new QPushButton(QStringLiteral("Сброс позиции"), panel);
    auto* fpsLabel = new QLabel("FPS: 0", panel);
    fpsLabel->setMinimumWidth(70);
    layout->addWidget(iterationLabel);
    layout->addWidget(slider);
    layout->addWidget(reset);
    layout->addWidget(fpsLabel);
    panel->adjustSize();
    panel->move(10, 10);

    auto* hint = new QLabel(QStringLiteral("Колесо - масштаб, лкм — перемещение"), this);
    hint->setStyleSheet("color: white;");
    hint->setAttribute(Qt::WA_TransparentForMouseEvents);
    hint->adjustSize();
    hint->move(10, panel->height() + 16);

    connect(slider, &QSlider::valueChanged, this, [this, iterationLabel](int value) {
        iterations_ = value;
        geometryDirty_ = true;
        iterationLabel->setText(QStringLiteral("Итерации: %1").arg(value));
        update();
    });
    connect(reset, &QPushButton::clicked, this, [this] {
        center_ = {};
        viewHeight_ = 3.2;
        update();
    });
    connect(this, &Window::fpsUpdated, fpsLabel, [fpsLabel](double fps) {
        fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 0));
    });
}

Window::~Window()
{
    makeCurrent();
    vao_.destroy();
    vbo_.destroy();
    program_.reset();
    doneCurrent();
}

void Window::onInit()
{
    program_ = std::make_unique<QOpenGLShaderProgram>();
    if (!program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/template.vs") ||
        !program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/template.fs") ||
        !program_->link()) {
        qFatal("Shader error: %s", qPrintable(program_->log()));
    }

    vao_.create();
    vao_.bind();
    vbo_.create();
    vbo_.bind();
    vbo_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    program_->bind();
    program_->enableAttributeArray(0);
    program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(Vertex));
    vao_.release();
    vbo_.release();
    program_->release();
    glClearColor(0.035f, 0.055f, 0.09f, 1.0f);
}

void Window::rebuildSnowflake()
{
    std::vector<Vertex> vertices;
    const QPointF a(0.0, 1.0);
    const QPointF b(-std::sqrt(3.0) / 2.0, -0.5);
    const QPointF c(std::sqrt(3.0) / 2.0, -0.5);
    addTriangle(vertices, a, b, c);
    addKochEdge(vertices, a, b, iterations_);
    addKochEdge(vertices, b, c, iterations_);
    addKochEdge(vertices, c, a, iterations_);
    vbo_.bind();
    vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(Vertex)));
    vbo_.release();
    vertexCount_ = static_cast<int>(vertices.size());
    geometryDirty_ = false;
}

void Window::onRender()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (geometryDirty_)
        rebuildSnowflake();

    program_->bind();
    program_->setUniformValue("center", QVector2D(center_));
    const float halfHeight = static_cast<float>(viewHeight_ / 2.0);
    program_->setUniformValue("halfExtent", QVector2D(halfHeight * width() / std::max(height(), 1), halfHeight));
    vao_.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    vao_.release();
    program_->release();
    update();
}

void Window::onResize(size_t, size_t)
{
}

QPointF Window::worldPosition(const QPointF& position) const
{
    const double unitsPerPixel = viewHeight_ / std::max(height(), 1);
    return center_ + QPointF(position.x() - width() / 2.0,
                             height() / 2.0 - position.y()) * unitsPerPixel;
}

void Window::wheelEvent(QWheelEvent* event)
{
    const auto anchor = worldPosition(event->position());
    const double steps = event->angleDelta().y() / 120.0;
    viewHeight_ = std::clamp(viewHeight_ * std::pow(0.85, steps), 0.0001, 100.0);
    center_ += anchor - worldPosition(event->position());
    event->accept();
    update();
}

void Window::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePosition_ = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        center_ += worldPosition(lastMousePosition_) - worldPosition(event->position());
        lastMousePosition_ = event->position();
        update();
        event->accept();
    }
}

void Window::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        unsetCursor();
        event->accept();
    }
}
