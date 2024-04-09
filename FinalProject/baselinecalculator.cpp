#include "baselinecalculator.h"
#include <QtGlobal> // For qrand()
#include <QTimer>
#include <QEventLoop>
#include <QCoreApplication>
#include <math.h>
#include <iostream>

BaselineCalculator::BaselineCalculator(QObject *parent)
    : QObject(parent),
      currentEEGSite(0),
      baselineCount(0) {}

double BaselineCalculator::processSignal(const QVector<double>& waveData) {
    // Placeholder for actual signal processing
    double sum = std::accumulate(waveData.begin(), waveData.end(), 0.0);
    return sum / waveData.size();
}

QVector<double> BaselineCalculator::generateWaveform(int electrodeIndex, int sampleRate, int duration) {
    QVector<double> waveform;

    // Define the range for the gamma band frequencies.
    double minFrequency = 25.0; // Minimum gamma band frequency
    double maxFrequency = 140.0; // Maximum gamma band frequency

    // Create slightly different frequencies for each electrode
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
    // For the gamma band, the frequency range is 25-140 Hz
    double minFrequency = 25.0;
    double maxFrequency = 140.0;

    // Generate a dominant frequency within the gamma range that varies per electrode site
    // Using siteIndex to create a controlled variance for each site
    double frequencyVariance = (maxFrequency - minFrequency) / BaselineCalculator::totalNumberOfSites;
    double dominantFrequency = minFrequency + (frequencyVariance * (siteIndex % BaselineCalculator::totalNumberOfSites)) + static_cast<double>(qrand() % static_cast<int>(frequencyVariance));

    return dominantFrequency;
}


void BaselineCalculator::startBaselineCalculation() {
    reset();
    // Simulate baseline frequency calculation for each EEG site
    for (int siteIndex = 0; siteIndex < BaselineCalculator::totalNumberOfSites; ++siteIndex) {
        // Placeholder for actual frequency calculation
        double frequency = generateDominantFrequencyForSite(siteIndex);

        baselineFrequencies.append(frequency);
        emit baselineCalculated(siteIndex, frequency);
    }

    // Indicate that all baseline calculations are complete
    emit allBaselinesCalculated();
}

void BaselineCalculator::applyTreatment() {
    const int treatmentDuration = 16; // 1/16th of a second for one second
    for (int siteIndex = 0; siteIndex < baselineFrequencies.size(); ++siteIndex) {
        int baselineFrequency = baselineFrequencies[siteIndex];
        emit currentElectrode(siteIndex);
        QTimer* treatmentTimer = new QTimer(this);
        treatmentTimer->setInterval(1000 / 16); // Set the timer for 1/16th of a second
        int count = 0; // Initialize the count for each site

        connect(treatmentTimer, &QTimer::timeout, this, [this, siteIndex, baselineFrequency, treatmentTimer, treatmentDuration, &count]() mutable {
            if (count < treatmentDuration) {
                int treatmentFrequency = baselineFrequency + 5; // Apply a 5Hz offset
                // Here, you would apply the treatmentFrequency to the EEG device or simulation
                emit treatmentApplied(siteIndex, treatmentFrequency);
                count++;
                int progress = (count * 100) / treatmentDuration; // Calculate progress percentage
                emit treatmentProgress(progress); // Emit the progress signal
            } else {
                treatmentTimer->stop(); // Stop the timer after 1 second
                treatmentTimer->deleteLater(); // Ensure the timer is cleaned up
                count = 0;
                if (siteIndex >= baselineFrequencies.size() - 1) {
                    // All treatments have been applied
                    emit treatmentComplete(); // Signal that treatment is complete
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
}

void BaselineCalculator::calculateBaseline()
{
// implement
}


