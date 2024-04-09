#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "handlesessions.h"
#include "baselinecalculator.h"
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

    BaselineCalculator* baselineCalculator;
    bool systemOn;

    bool contactEstablished;
    bool sessionActive;
    QTimer *contactTimer;
    QTimer *contactLossTimer;

    QTimer *electrodeUpdateTimer;

    int currentElectrodeIndex;
    bool treatmentPaused;

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
    void onBaselineCalculated(int siteIndex);

    void onAllBaselinesCalculated();
    void onTreatmentApplied(int siteIndex, int treatmentFrequency);
    void onTreatmentComplete();

    void checkContactStatus();
    void handleContactLost();
    void handleContactReestablished();

    void updateProgressBar(int progress);

    void plotWaveform(int electrodeIndex);
    void updateElectrode();


signals:
    void selectValueSignal(QString s); ///

};
#endif // MAINWINDOW_H
