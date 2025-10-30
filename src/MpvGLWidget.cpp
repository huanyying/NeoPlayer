#include "MpvGLWidget.h"
#include <QByteArray>
#include <QFile>
#include <QMetaObject>
#include <QOpenGLContext>
#include <stdexcept>

MpvGLWidget::MpvGLWidget(QWidget* parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
    // 设置日志
    m_logger = Log::LogInit(Log::LOG_TYPE::CONSOLE);
    // 创建 mpv 客户端：mpv [options] [url|path/]filename
    mpv = mpv_create();
    if (!mpv)
    {
        // spdlog::error("mpv_create() failed");
        throw std::runtime_error("mpv_create() failed");
        return;
    }
    // 设置启动属性
    mpv_set_option_string(mpv, "terminal", "yes");
    mpv_set_option_string(mpv, "msg-level", "all-v");
    // 基本输入设置（可选，便于验证）
    mpv_set_option_string(mpv, "input-default-bindings", "yes"); // 允许绑定mpv默认按键
    /*
    * 底层机制
        事件处理路径：
            默认模式（input-vo-keyboard=no）：键盘事件 → 系统输入管理器 → MPV 核心输入处理 → 执行快捷键
            VO模式（input-vo-keyboard=yes）：键盘事件 → 视频输出层（VO） → 转发给MPV核心 → 执行快捷键
    * 不影响系统按键
    * 开启vo模式：1、vo层能够将alt、command、多媒体键传给os 2、兼容macos的command键，配合窗口管理器
    */
    mpv_set_option_string(mpv, "input-vo-keyboard", "yes"); // 允许键盘输入控制

    // 事件唤醒回调：把 drainEvents 投递到 Qt 线程
    // mpv_set_wakeup_callback(mpv, &MpvGLWidget::onMpvEvents, this);

    // 初始化 mpv
    if (mpv_initialize(mpv) < 0)
    {
        m_logger->error("mpv_initialize() failed");
        throw std::runtime_error("mpv_initialize() failed");
        return;
    }
    // 测试，设置属性：硬解
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");
    // 监听属性：媒体总时长，当前播放位置
    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    // 请求级别为"info"或更高级别的日志消息。它们以 MPV_EVENT_LOG_MESSAGE 的形式接收
    mpv_request_log_messages(mpv, "info");

    // 设置mpv事件回调函数(通知)
    mpv_set_wakeup_callback(mpv, wakeup, this);

    mpvEventThread = new std::thread([this]() {
        eventLoop();
    });

    // // 也可用定时器轮询（与 wakeup 二选一；此处留作兜底）
    // pollTimer_.setInterval(0);
    // connect(&pollTimer_, &QTimer::timeout, this, &MpvGLWidget::drainEvents);
    // pollTimer_.start();
}

MpvGLWidget::~MpvGLWidget()
{
    makeCurrent(); // 确保释放的是当前线程的opengl渲染
    if (mpvRender) // 释放渲染上下文
    {
        mpv_render_context_free(mpvRender);
        mpvRender = nullptr;
    }
    if (mpv) // 终止 mpv
    {
        mpv_terminate_destroy(mpv);
        mpv = nullptr;
    }
}

/*
 * OpenGL 函数
 */
void MpvGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // mpvRender初始化参数：获取openGL函数（兼容不同平台），上下文
    mpv_opengl_init_params glInit = {&MpvGLWidget::get_proc_address,  this};
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)}, // OpenGL渲染
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInit},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &allowAdvancedCtl},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    if (mpv_render_context_create(&mpvRender, mpv, params) < 0)
    {
        throw std::runtime_error("mpv_render_context_create() failed");
    }
    // 渲染更新回调
    // 参数：接收人，回调函数，回调参数
    mpv_render_context_set_update_callback(mpvRender, &MpvGLWidget::on_render_update, this);
}

void MpvGLWidget::paintGL() // 视频渲染
{
    if (!mpvRender)
    {
        return;
    }

    // 将 mpv 的输出渲染到 QT的 QOpenGLWidget 的默认 FBO（帧缓冲），大小是 MpvGLWidget 的大小
    mpv_opengl_fbo fbo {static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    // QOpenGLWidget 的 FBO Y 轴通常需要翻转
    int flip_y = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    mpv_render_context_render(mpvRender, params); // mpv解码的视频帧 -> qt openGL窗口上
}

void MpvGLWidget::resizeGL(int, int)
{
    // mpv 会根据 FBO 尺寸自动适配，此处通常无需额外处理
}

void MpvGLWidget::wakeup(void *ctx) {
    // 使用invokeMethod直接调用槽函数
    // QMetaObject::invokeMethod(static_cast<MpvGLWidget *>(ctx), "onMpvEvents", Qt::QueuedConnection);
    // m_logger->info("wakeup -------------");
    // ((MpvGLWidget*)ctx)->onMpvEvents();
}

void *MpvGLWidget::get_proc_address(void *ctx, const char *name)
{
    Q_UNUSED(ctx);
    QOpenGLContext *glCtx = QOpenGLContext::currentContext();
    if (!glCtx)
    {
        return nullptr;
    }
    return reinterpret_cast<void *>(glCtx->getProcAddress(QByteArray(name))); // name：openGL函数名
}

void MpvGLWidget::on_render_update(void *cb_ctx)
{
    QMetaObject::invokeMethod((MpvGLWidget*)cb_ctx, "maybeUpdate");
}

/*
 * mpv 控制函数
 */
void MpvGLWidget::command(const QVariant &params) // 发送命令
{
    /*
     * list: 转化为mpv_node数组，命令数组
     * string: 字符串，命令字符串
     * map: 键值对，设置属性
     */
    mpv::qt::command_variant(mpv, params);
}

void MpvGLWidget::setProperty(const QString &name, const QVariant &value) // 设置属性
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvGLWidget::getProperty(const QString &name) const // 获取属性
{
    return mpv::qt::get_property_variant(mpv, name);
}

/*
 *  mpv 事件处理
 */
void MpvGLWidget::onMpvEvents(mpv_event *mpvEvent)
{
    switch (mpvEvent->event_id)
    {
        case MPV_EVENT_PROPERTY_CHANGE: // 监听的属性变化
        {
            auto *mpvProp = reinterpret_cast<mpv_event_property *>(mpvEvent->data);
            if (strcmp(mpvProp->name, "time-pos") == 0) // 当前播放位置
            {
                if (mpvProp->format == MPV_FORMAT_DOUBLE)
                {
                    double curPlayTime = *static_cast<double*>(mpvProp->data);
                    m_logger->info("time-pos {}", curPlayTime);
                    Q_EMIT positionChanged(curPlayTime);
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void MpvGLWidget::maybeUpdate()
{
    if (window()->isMinimized()) // 最小化时，手动渲染视频帧
    {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    }
    else
    {
        update(); // 正常渲染
    }
}

void MpvGLWidget::eventLoop()
{
    while (m_running)
    {
        mpv_event *mpvEvent = mpv_wait_event(mpv, 10000); // 阻塞
        if (mpvEvent != nullptr && mpvEvent->event_id != MPV_EVENT_NONE)
        {
            onMpvEvents(mpvEvent);
        }
        else
        {
            continue;
        }
    }
}
