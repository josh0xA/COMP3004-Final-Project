#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "handlesessions.h"
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void lightsOff();

private:
    Ui::MainWindow *ui;
    QTimer *batteryTimer;
    HandleSessions *sessionPTR;

private slots:
    void upButton();
    void downButton();
    void powerButton();
    void menuButton();
    void pauseButton();
    void startButton();
    void stopButton();
    void selectButton();
    void decreaseBatteryLevel();
    void onDone();
    void threeButtonsOff();

signals:
    void selectValueSignal(QString s); ///

};
#endif // MAINWINDOW_H
