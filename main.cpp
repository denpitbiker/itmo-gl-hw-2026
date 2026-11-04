#include "window.h"

#include <QApplication>
#include <QSurfaceFormat>

constexpr auto g_samples = 4;
constexpr auto g_gl_major_version = 3;
constexpr auto g_gl_minor_version = 3;

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QSurfaceFormat format;
    format.setSamples(g_samples);
    format.setVersion(g_gl_major_version, g_gl_minor_version);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    Window window;
    window.resize(900, 700);
    window.show();

    return a.exec();
}
