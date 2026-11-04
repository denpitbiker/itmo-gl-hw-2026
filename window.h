#pragma once

#include "openglwidget.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QPointF>
#include <memory>

class Window : public OpenGLWidget
{
    Q_OBJECT

public:
    Window();
    ~Window() override;

    void onInit() override;
    void onRender() override;
    void onResize(size_t width, size_t height) override;

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void rebuildSnowflake();
    QPointF worldPosition(const QPointF& position) const;

    QOpenGLBuffer vbo_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;
    std::unique_ptr<QOpenGLShaderProgram> program_;
    int vertexCount_ = 0;
    int iterations_ = 4;
    bool geometryDirty_ = true;
    double viewHeight_ = 3.2;
    QPointF center_;
    QPointF lastMousePosition_;
};
