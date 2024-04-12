# COMP3004-Final-Project
Final Project for Team 78. Authored by: Josh Schiavone, Connor Lindblad and Maansinh Jadeja
## Initial Draft Components (For Amirali - April 12th)
This is to show our progress for the draft, programmatically - everything is going smoothly and all of the other deliverables will be present for the final submission. <br/>
![image](https://github.com/josh0xA/COMP3004-Final-Project/assets/29878743/de4b7944-757e-4b87-a6f2-b1d2c5368f39)
![image](https://github.com/josh0xA/COMP3004-Final-Project/assets/29878743/e416750f-ac7e-4ce5-b104-e8b5871b3ea5)
![image](https://github.com/josh0xA/COMP3004-Final-Project/assets/29878743/202f6d68-9d61-425b-8dba-758394acb0cd)
![image](https://github.com/josh0xA/COMP3004-Final-Project/assets/29878743/73ed18e9-ece7-41a3-bc3b-afa151619fdd) <br/>
### Signal Processing Logic, Waveform Generation, and Baseline Calculation
```cpp
double BaselineCalculator::processSignal(const QVector<double>& waveData) {
    double sum = std::accumulate(waveData.begin(), waveData.end(), 0.0);
    return sum / waveData.size();
}

QVector<double> BaselineCalculator::generateWaveform(int electrodeIndex, int sampleRate, int duration) {
    QVector<double> waveform;

    // the range for the gamma band frequencies
    double minFrequency = 25.0; 
    double maxFrequency = 140.0; 

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
        int count = 0; // Initialize the count for each site

        connect(treatmentTimer, &QTimer::timeout, this, [this, siteIndex, baselineFrequency, treatmentTimer, treatmentDuration, &count]() mutable {
            if(!isTimerPaused && !hasQuit) {
                if (count < treatmentDuration) {
                    int treatmentFrequency = baselineFrequency + 5; // Apply a 5Hz offset
                    emit treatmentApplied(siteIndex, treatmentFrequency);
                    count++;
                    int progress = (count * 100) / treatmentDuration; 
                    emit treatmentProgress(progress); 
                } else {
                    treatmentTimer->stop(); 
                    treatmentTimer->deleteLater(); 
                    count = 0;
                    if (siteIndex >= baselineFrequencies.size() - 1) {
                        // All treatments have been applied
                        emit treatmentComplete(); // Signal that treatment is complete
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
```
