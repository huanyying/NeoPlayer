#include "MainWindow.h"
#include <QFileDialog>
#include <QLayout>
#include <QPushButton>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    // 初始化日志
    m_logger = Log::LogInit(Log::LOG_TYPE::CONSOLE);
    // 创建 mpv窗口实例,父亲是主窗口
    m_mpv = new MpvGLWidget(this);
    // 创建一个slider
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal); // 设置水平布局
    // 创建两个按钮
    m_openBtn = new QPushButton("Open");
    m_playBtn = new QPushButton("Pause");
    // 创建 QHBoxLayout
    QHBoxLayout *hb = new QHBoxLayout(); // 水平布局管理器
    hb->addWidget(m_openBtn);
    hb->addWidget(m_playBtn);
    QVBoxLayout *vl = new QVBoxLayout(); // 垂直布局管理器
    vl->addWidget(m_mpv); // 视频播放区域
    vl->addWidget(m_slider); // 滑块
    vl->addLayout(hb); // 包含按钮的水平布局
    // 设置主界面的布局
    setLayout(vl); // 设置布局到当前窗口
    resize(800, 600);

    // 连接槽和信号
    connect(m_slider, &QSlider::sliderMoved, this, &MainWindow::seek); // 播放进度条
    connect(m_openBtn, &QPushButton::clicked, this, &MainWindow::openMedia); // 打开视频
    connect(m_playBtn, &QPushButton::clicked, this, &MainWindow::pauseResume); // 播放/暂停
    connect(m_mpv, &MpvGLWidget::positionChanged, this, &MainWindow::setSliderPosition); // 更新进度条
    connect(m_mpv, &MpvGLWidget::durationChanged, this, &MainWindow::setSliderRange); // 设置进度条的范围
}
MainWindow::~MainWindow()
{
    spdlog::drop_all();

}

void MainWindow::openMedia()
{
    QString file = QFileDialog::getOpenFileName(this, "Open a Video");
    if (file.isEmpty())
    {
        spdlog::error("open videoName none");
        return;
    }
    m_mpv->command(QStringList() << "loadfile" << file); // 打开视频文件
    m_logger->info("open video {}", file.toStdString());
}

void MainWindow::seek(int pos)
{
    m_mpv->command(QVariantList() << "seek" << pos << "absolute");
    m_logger->info("video seek {}",pos);
}

void MainWindow::pauseResume()
{
    const bool paused = m_mpv->getProperty("pause").toBool();
    m_mpv->setProperty("pause", !paused);
}

void MainWindow::setSliderRange(int duration)
{
    m_slider->setRange(0, duration);
}


void MainWindow::setSliderPosition(int curPos)
{
    m_slider->setValue(curPos);
}
