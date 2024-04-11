#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QMessageBox>
#include <iostream>
#include <cmath>
#include "handlesessions.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , treatmentPaused(false)
{
    ui->setupUi(this);
    connect(ui->upButton, SIGNAL (released()), this, SLOT (upButton()));
    connect(ui->downButton, SIGNAL (released()), this, SLOT (downButton()));
    connect(ui->powerButton, SIGNAL (released()), this, SLOT (powerButton()));
    connect(ui->selectButton, SIGNAL (released()), this, SLOT (selectButton()));
    connect(ui->menuButton, SIGNAL (released()), this, SLOT (menuButton()));
    connect(ui->pauseButton, SIGNAL (released()), this, SLOT (pauseButton()));
    connect(ui->startButton, SIGNAL (released()), this, SLOT (startButton()));
    connect(ui->stopButton, SIGNAL (released()), this, SLOT (stopButton()));

    HandleSessions *sessionPTR = new HandleSessions(this);
    // Updates the remaining time and progress bar when session is on
    connect(sessionPTR, &HandleSessions::sessionUpdated, this, [this](int timeT) {
        int secondsLeft = 100 - timeT;
        int progress = timeT;
        //qDebug() << secondsLeft;

        QString timeString = QString::asprintf("Time Remaining -> %2d Seconds", secondsLeft);
        QString progressString = QString::asprintf("Progress = %3d%", progress);
        ui->option1->setText(timeString);
        ui->option2->setText(progressString);
    });

    // Connecting buttons to functions in handlesessions
    connect(ui->startButton, &QPushButton::clicked, sessionPTR, &HandleSessions::start);
    connect(ui->pauseButton, &QPushButton::clicked, sessionPTR, &HandleSessions::pause);
    connect(ui->stopButton, &QPushButton::clicked, sessionPTR, &HandleSessions::stop);

    connect(sessionPTR, &HandleSessions::done, this, &MainWindow::onDone);

    baselineCalculator = new BaselineCalculator(this); // Ensure it's properly instantiated
    connect(baselineCalculator, &BaselineCalculator::baselineCalculated, this, &MainWindow::onBaselineCalculated);
    connect(baselineCalculator, &BaselineCalculator::allBaselinesCalculated, this, &MainWindow::onAllBaselinesCalculated);
    connect(baselineCalculator, &BaselineCalculator::treatmentApplied, this, &MainWindow::onTreatmentApplied);
    connect(baselineCalculator, &BaselineCalculator::treatmentComplete, this, &MainWindow::onTreatmentComplete);

    connect(baselineCalculator, &BaselineCalculator::treatmentProgress, this, &MainWindow::updateProgressBar);
    connect(baselineCalculator, &BaselineCalculator::currentElectrode, this, &MainWindow::plotWaveform);


    contactEstablished = true; // Assume initial contact is established
    contactTimer = new QTimer(this);
    connect(contactTimer, &QTimer::timeout, this, &MainWindow::checkContactStatus);
    contactTimer->start(1000); // Check contact status every second

    contactLossTimer = new QTimer(this);
    connect(contactLossTimer, &QTimer::timeout, this, &MainWindow::handleContactLost);
    contactLossTimer->setSingleShot(true); // Only trigger once

    connect(sessionPTR, &HandleSessions::paused, baselineCalculator, &BaselineCalculator::handlePause);
    connect(sessionPTR, &HandleSessions::quit, baselineCalculator, &BaselineCalculator::handleQuit);


    // Hiding the date input and list on startup
    ui->dateInput->hide();
    ui->listWidget->hide();
    ui->treatmentLabel->hide();
    ui->progressBar->hide();
    ui->plotWidget->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::upButton(){
    qInfo("Up Button Pressed");

    QPushButton *upButtonPressed = qobject_cast<QPushButton*>(sender());
    if (upButtonPressed) {
        // Check the background color of each QLabel and update accordingly
        if (ui->option1->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option1->setStyleSheet(""); // Reset background color of option1
            ui->option3->setStyleSheet("background-color: yellow;"); // Set background color of option2 to yellow
        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option2->setStyleSheet(""); // Reset background color of option2
            ui->option1->setStyleSheet("background-color: yellow;"); // Set background color of option3 to yellow
        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option3->setStyleSheet(""); // Reset background color of option3
            ui->option2->setStyleSheet("background-color: yellow;"); // Set background color of option1 to yellow
        }
    }


}

void MainWindow::downButton() {
    qInfo("Down Button Pressed");

    QPushButton *downButtonPressed = qobject_cast<QPushButton*>(sender());
    if (downButtonPressed) {
        // Check the background color of each QLabel and update accordingly
        if (ui->option1->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option1->setStyleSheet(""); // Reset background color of option1
            ui->option2->setStyleSheet("background-color: yellow;"); // Set background color of option2 to yellow
        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option2->setStyleSheet(""); // Reset background color of option2
            ui->option3->setStyleSheet("background-color: yellow;"); // Set background color of option3 to yellow
        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option3->setStyleSheet(""); // Reset background color of option3
            ui->option1->setStyleSheet("background-color: yellow;"); // Set background color of option1 to yellow
        }
    }
}

void MainWindow::selectButton(){

    QPushButton *selectButtonPressed = qobject_cast<QPushButton*>(sender());
    if (selectButtonPressed) {

        if (ui->option1->palette().color(QPalette::Window) == Qt::yellow) {
            // Adding a timer to the screen
            ui->option1->setText("Ready"); // Needs
            ui->option2->setText("Progress = 0%");
            ui->option3->setText("");
            ui->progressBar->show();
            ui->plotWidget->show();

            ui->option1->setStyleSheet("background-color: white;");

            // Enabling the buttons since on correct screen
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
            ui->pauseButton->setEnabled(false);

        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            //show session log
            ui->option1->setText("");
            ui->option2->setText("");
            ui->option3->setText("");
            ui->option2->setStyleSheet("background-color: white;");
            ui->listWidget->show();


        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            //add input for user to set date and time
            ui->option1->setText("Input Date"); // Needs
            ui->option2->setText("");
            ui->option3->setText("");
            ui->option3->setStyleSheet("background-color: white;");
            ui->dateInput->show();

        }

    }

}

void MainWindow::plotWaveform(int electrodeIndex) {
    QVector<double> waveform = baselineCalculator->generateWaveform(electrodeIndex, 100, 1); // Sample rate 100 Hz, duration 1 second
    QVector<double> x(waveform.size());
    std::iota(x.begin(), x.end(), 0); // Generate x-axis values

    ui->plotWidget->clearGraphs(); // Clear any existing graphs
    ui->plotWidget->addGraph(); // Add a new graph
    ui->plotWidget->graph(0)->setData(x, waveform); // Set the data for the graph
    ui->plotWidget->graph(0)->setPen(QPen(Qt::blue)); // Set the line color to blue
    ui->plotWidget->xAxis->setRange(0, waveform.size()); // Set the x-axis range
    ui->plotWidget->yAxis->setRange(-2, 2); // Set the y-axis range to fit the sine wave
    ui->plotWidget->replot(); // Replot the graph
}

void MainWindow::updateElectrode() {
    currentElectrodeIndex = (currentElectrodeIndex + 1) % BaselineCalculator::totalNumberOfSites;
    plotWaveform(currentElectrodeIndex);
}

void MainWindow::powerButton(){
    qInfo("Power Button Pressed");
    systemOn = !systemOn;
    if (systemOn) {
       ui->powerButton->setStyleSheet("background-color: red;");

       systemOn = true;
       ui->menuButton->setEnabled(true);
       ui->selectButton->setEnabled(true);
       ui->downButton->setEnabled(true);
       ui->upButton->setEnabled(true);


       // Setting up the battery
       batteryTimer = new QTimer(this);
       connect(batteryTimer, &QTimer::timeout, this, &MainWindow::decreaseBatteryLevel);
       batteryTimer->start(30000); // Every 30 seconds

     } else {
       ui->powerButton->setStyleSheet("background-color: green;");
       threeButtonsOff();
       lightsOff(); ui->option1->setText("");
       ui->dateInput->hide(); ui->option2->setText("");
       ui->listWidget->hide(); ui->option3->setText("");
       ui->treatmentLabel->setText("");
       ui->progressBar->hide();
       ui->plotWidget->clearGraphs();


       if (batteryTimer && batteryTimer->isActive()) {
           batteryTimer->stop();
       }


  }
}

void MainWindow::menuButton() {
    qInfo("Menu Button Pressed");
    ui->option1->setStyleSheet("background-color: yellow;");

    ui->option1->setText("NEW SESSION");
    ui->option2->setText("SESSION LOG");
    ui->option3->setText("TIME AND DATE");
    ui->option1->setStyleSheet("background-color: yellow;");

    lightsOff();
    threeButtonsOff();
    ui->dateInput->hide();
    ui->listWidget->hide();
    ui->treatmentLabel->hide();
    ui->progressBar->hide();
    ui->plotWidget->clearGraphs();
    ui->plotWidget->hide();

}


void MainWindow::pauseButton() {
    qInfo("Pause Button Pressed");

//    if (systemOn && contactEstablished) {
//        if (!sessionPTR->isPaused) {
//            // Pause the session
//            ui->pauseButton->setText("Resume"); // Change the button text to indicate resuming is possible
//        } else {
//            // Resume the session
//            ui->pauseButton->setText("Pause"); // Change the button text back to "Pause"
//        }
//    } else {
//        QMessageBox::warning(this, "Cannot Pause", "The system is not currently in an active session or contact is lost.");
//    }
}

void MainWindow::startButton() {
    qInfo("Start Button Pressed");

    // Setting Treatment Light on and the rest off
    ui->treatmentLight->setStyleSheet("background-color: green; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: blue; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    // Check if the system is on and contact is established
   if (systemOn && contactEstablished) {
          // Start a new session
           ui->startButton->setEnabled(false); // Disable the start button
           ui->stopButton->setEnabled(true); // Enable the stop button
           ui->pauseButton->setEnabled(true);
           baselineCalculator->startSession();
           sessionActive = true;

          // Update the UI

     // ui->treatmentLabel->show(); // Show the treatment label
     // ui->progressBar->show(); // Show the progress bar
    } else {
      QMessageBox::warning(this, "Cannot Start", "The system is not on or contact is not established.");
      }
}

void MainWindow::stopButton() {
    qInfo("Stop Button Pressed");
    sessionActive = false;

    ui->treatmentLabel->clear(); // Clear the treatment label text

    // Setting contectLost Light on and the rest off
    ui->treatmentLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: red; border-radius: 9px;");

    // Reset the UI
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
//    ui->treatmentLabel->hide();
//    ui->progressBar->hide();
}


// Decreaseing battery every 30 seconds device is on, can add more features for certain activies
void MainWindow::decreaseBatteryLevel() {
    int val = ui->battery->value();
    if(val != 0) {
        int newValue = val - 1;
        ui->battery->setValue(newValue);
    } else {
        batteryTimer->stop();
    }
}

void MainWindow::checkContactStatus() {
    // For simulation, randomly change contact status
    if (qrand() % 40 == 0 && contactEstablished) { // Simulate contact loss once in every 80 checks
        contactEstablished = false;
        ui->contactLostLight->setStyleSheet("background-color: red;"); // Flash red light
        contactLossTimer->start(5 * 60 * 1000); // Start 5-minute timer for contact reestablishment

        // Show a message box indicating that contact is lost
        QMessageBox::warning(this, "Contact Lost", "Contact with the EEG device has been lost. Please reestablish contact.");
    } else {
        handleContactReestablished();
    }
}


void MainWindow::handleContactLost() {
    // Automatically turn off the device and erase the session
    systemOn = false;
    ui->powerButton->setStyleSheet("background-color: green;");
    lightsOff();
    threeButtonsOff();
    ui->dateInput->hide();
    ui->listWidget->hide();
    ui->treatmentLabel->hide();
}

void MainWindow::handleContactReestablished() {
    if (!contactEstablished) {
        contactEstablished = true;
        ui->contactLight->setStyleSheet("background-color: blue;"); // Indicate contact initiation with blue light
        contactLossTimer->stop(); // Stop the contact loss timer
    }
}

void MainWindow::lightsOff() {
    // Setting all Lights off
    ui->treatmentLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: gray; border-radius: 9px;");
}

void MainWindow::threeButtonsOff() {
    // Turning off the buttons for the session when not in that section
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
}


void MainWindow::onDone() {
    // Allowing menu and power buttons again
     ui->menuButton->setEnabled(true);
     ui->powerButton->setEnabled(true);
     ui->option1->setText("Done");

     ui->treatmentLabel->setText("Treatment completed");

     // Adding sessions
     QString curDate = ui->dateInput->date().toString("yyyy-MM-dd");
     ui->listWidget->addItem(curDate);
}

void MainWindow::onBaselineCalculated(int siteIndex) {
    plotWaveform(siteIndex);
}

void MainWindow::onAllBaselinesCalculated() {
    ui->treatmentLabel->show();
    baselineCalculator->applyTreatment();

}

void MainWindow::onTreatmentApplied(int siteIndex, int treatmentFrequency) {
    // Update the UI to indicate treatment is being applied
    ui->treatmentLabel->setText(QString("Treating site %1 with frequency %2 Hz").arg(siteIndex + 1).arg(treatmentFrequency));
    ui->treatmentLight->setStyleSheet("background-color: green");
}

void MainWindow::onTreatmentComplete() {
    // Update the UI to indicate treatment is complete
    ui->treatmentLabel->setText("Treatment completed for all sites");
}

void MainWindow::updateProgressBar(int progress) {
    // Update the progress bar with the current progress
    ui->progressBar->setValue(progress);
}












