#ifndef MPVGLWIDGET_H
#define MPVGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <qthelper.hpp>
#include "import/log.h"

class LogInit;
class MpvGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit MpvGLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowType::Widget);
    ~MpvGLWidget() override;
    void command(const QVariant& params); // 接受命令
    void setProperty(const QString& name, const QVariant& value); // 设置属性
    QVariant getProperty(const QString& name) const; // 常函数,获取属性值
Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
public:
    std::thread* mpvEventThread   = nullptr;
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void onMpvEvents(mpv_event *mpvEvent);
    void maybeUpdate();
private:
    void eventLoop();
    // 作用域
    static void wakeup(void *ctx);
    static void *get_proc_address(void * ctx, const char * name);
    static void on_render_update(void * cb_ctx);
private:
    mpv_handle* mpv               = nullptr;
    std::shared_ptr<spdlog::logger> m_logger;
    bool m_running                = true;
    mpv_render_context *mpvRender = nullptr;
    // QTimer pollTimer;
    bool allowAdvancedCtl         = true;
};

#endif // MPVGLWIDGET_H
