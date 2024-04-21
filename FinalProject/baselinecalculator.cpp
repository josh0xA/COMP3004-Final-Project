#include "baselinecalculator.h"
#include <QtGlobal>
#include <QTimer>
#include <QEventLoop>
#include <QCoreApplication>
#include <math.h>
#include <iostream>
#include <QDebug>

BaselineCalculator::BaselineCalculator(QObject *parent)
    : QObject(parent),
      currentEEGSite(0),
      baselineCount(0) {}

double BaselineCalculator::processSignal(const QVector<double>& waveData) {
    double sum = std::accumulate(waveData.begin(), waveData.end(), 0.0);
    return sum / waveData.size();
}

QVector<double> BaselineCalculator::generateWaveform(int electrodeIndex, int sampleRate, int duration) {
    QVector<double> waveform;

    double minFrequency = 25.0;
    double maxFrequency = 140.0;

    double frequencyStep = (maxFrequency - minFrequency) / BaselineCalculator::totalNumberOfSites;
    double f1 = minFrequency + frequencyStep * (electrodeIndex % BaselineCalculator::totalNumberOfSites);
    double f2 = minFrequency + frequencyStep * ((electrodeIndex + 2) % BaselineCalculator::totalNumberOfSites); // Offset by 2 to ensure it's different from f1
    double f3 = minFrequency + frequencyStep * ((electrodeIndex + 4) % BaselineCalculator::totalNumberOfSites); // Offset by 4 to ensure it's different from f1 and f2

    double A1 = 1.0;
    double A2 = 0.8;
    double A3 = 0.6;

    for (int i = 0; i < sampleRate * duration; ++i) {
        double t = i / static_cast<double>(sampleRate);
        double value = A1 * std::sin(2.0 * M_PI * f1 * t) +
                       A2 * std::sin(2.0 * M_PI * f2 * t) +
                       A3 * std::sin(2.0 * M_PI * f3 * t);
        waveform.append(value);
    }

    return waveform;
}

double BaselineCalculator::generateDominantFrequencyForSite(int siteIndex) {
    double minFrequency = 25.0;
    double maxFrequency = 140.0;


    double frequencyVariance = (maxFrequency - minFrequency) / BaselineCalculator::totalNumberOfSites;
    double dominantFrequency = minFrequency + (frequencyVariance * (siteIndex % BaselineCalculator::totalNumberOfSites)) +
            static_cast<double>(qrand() % static_cast<int>(frequencyVariance));

    return dominantFrequency;
}


void BaselineCalculator::startBaselineCalculation() {
    reset();
    for (int siteIndex = 0; siteIndex < BaselineCalculator::totalNumberOfSites; ++siteIndex) {
        double frequency = generateDominantFrequencyForSite(siteIndex);

        baselineFrequencies.append(frequency);
        emit baselineCalculated(siteIndex, frequency);
    }

    emit allBaselinesCalculated();
}

void BaselineCalculator::applyTreatment() {
    const int treatmentDuration = 16; // 1/16th of a second for one second
    isTimerPaused = false;
    for (int siteIndex = 0; siteIndex < baselineFrequencies.size(); ++siteIndex) {
        int baselineFrequency = baselineFrequencies[siteIndex];
        emit currentElectrode(siteIndex);
        QTimer* treatmentTimer = new QTimer(this);
        treatmentTimer->setInterval(1000 / 16); // Set the timer for 1/16th of a second
        int count = 0;
        connect(treatmentTimer, &QTimer::timeout, this, [this, siteIndex, baselineFrequency, treatmentTimer, treatmentDuration, &count]() mutable {
            if(!isTimerPaused && !hasQuit) {
                if (count < treatmentDuration) {
                    int treatmentFrequency = baselineFrequency + 5;

                    emit treatmentApplied(siteIndex, treatmentFrequency);
                    count++;
                    int progress = (count * 100) / treatmentDuration;
                    emit treatmentProgress(progress);
                } else {
                    treatmentTimer->stop();
                    treatmentTimer->deleteLater();
                    count = 0;
                    if (siteIndex >= baselineFrequencies.size() - 1) {
                        emit treatmentComplete();
                    }
                }
            }
        });
            treatmentTimer->start();
            while (treatmentTimer->isActive()) {
                QCoreApplication::processEvents(); // Process events to avoid blocking the UI
            }

    }
}



void BaselineCalculator::startSession() {
    startBaselineCalculation();
    applyTreatment();
}

void BaselineCalculator::reset()
{
    currentEEGSite = 0;
    baselineCount = 0;
    baselineFrequencies.clear();
    hasQuit = false;

}

void BaselineCalculator::calculateBaseline() {}


void BaselineCalculator::handlePause() {
    isTimerPaused = !isTimerPaused;
}

void BaselineCalculator::handleQuit() {
    hasQuit = true;
}




