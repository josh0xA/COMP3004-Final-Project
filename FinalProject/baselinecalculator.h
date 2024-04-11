#ifndef BASELINECALCULATOR_H
#define BASELINECALCULATOR_H

#include <QObject>
#include <QVector>

class BaselineCalculator : public QObject
{
    Q_OBJECT

public:
    static const int totalNumberOfSites = 7;
    explicit BaselineCalculator(QObject *parent = nullptr);
    void startBaselineCalculation();
    void applyTreatment();
    void startSession();

    QVector<double> generateWaveform(int electrodeIndex, int sampleRate, int duration);
    double processSignal(const QVector<double>& waveData);
    double generateDominantFrequencyForSite(int siteIndex);
    void handlePause();
    void handleQuit();



signals:
    void baselineCalculated(int siteIndex, int frequency);
    void treatmentApplied(int siteIndex, int treatmentFrequency);
    void allBaselinesCalculated();
    void treatmentComplete();

    void treatmentProgress(int progress);
    void sessionCompleted();
    void currentElectrode(int siteIndex);


private:
    int currentEEGSite;
    int baselineCount;
    void reset();

    const int baselineDuration = 60; // Duration in seconds for baseline calculation
    QVector<int> baselineFrequencies;

    bool isTimerPaused = false;
    bool hasQuit = false;

private slots:
    void calculateBaseline();
\
};

#endif // BASELINECALCULATOR_H
