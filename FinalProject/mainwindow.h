#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void upButton();
    void downButton();
    void powerButton();
    void menuButton();
    void pauseButton();
    void startButton();
    void stopButton();
    void selectButton();


};
#endif // MAINWINDOW_H
