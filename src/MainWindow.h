#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MpvGLWidget.h"
#include <QtWidgets/QWidget>
#include "import/log.h"

class MpvGLWidget;
class QSlider;
class QPushButton;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public Q_SLOTS:
    void openMedia();
    void seek(int pos);
    void pauseResume();
public:
    MpvGLWidget *m_mpv;
private Q_SLOTS:
    void setSliderRange(int duration);
    void setSliderPosition(int curPos);
private:
    std::shared_ptr<spdlog::logger> m_logger;
    QSlider *m_slider;
    QPushButton *m_openBtn;
    QPushButton *m_playBtn;
};

#endif // MAINWINDOW_H
