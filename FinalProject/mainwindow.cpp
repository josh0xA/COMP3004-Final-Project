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
    connect(sessionPTR, &HandleSessions::sessionUpdated, this, [this](int timeT) {
        int secondsLeft = 100 - timeT;
        int progress = timeT;

        QString timeString = QString::asprintf("Time Remaining -> %2d Seconds", secondsLeft);
        QString progressString = QString::asprintf("Progress = %3d%", progress);
        ui->option1->setText(timeString);
        ui->option2->setText(progressString);
    });


    connect(ui->startButton, &QPushButton::clicked, sessionPTR, &HandleSessions::start);
    connect(ui->pauseButton, &QPushButton::clicked, sessionPTR, &HandleSessions::pause);
    connect(ui->stopButton, &QPushButton::clicked, sessionPTR, &HandleSessions::stop);

    connect(sessionPTR, &HandleSessions::done, this, &MainWindow::onDone);

    baselineCalculator = new BaselineCalculator(this);
    connect(baselineCalculator, &BaselineCalculator::baselineCalculated, this, &MainWindow::onBaselineCalculated);
    connect(baselineCalculator, &BaselineCalculator::allBaselinesCalculated, this, &MainWindow::onAllBaselinesCalculated);
    connect(baselineCalculator, &BaselineCalculator::treatmentApplied, this, &MainWindow::onTreatmentApplied);
    connect(baselineCalculator, &BaselineCalculator::treatmentComplete, this, &MainWindow::onTreatmentComplete);

    connect(baselineCalculator, &BaselineCalculator::treatmentProgress, this, &MainWindow::updateProgressBar);
    connect(baselineCalculator, &BaselineCalculator::currentElectrode, this, &MainWindow::plotWaveform);


    contactEstablished = true;
    contactTimer = new QTimer(this);
    connect(contactTimer, &QTimer::timeout, this, &MainWindow::checkContactStatus);
    contactTimer->start(5000);

    contactLossTimer = new QTimer(this);
    connect(contactLossTimer, &QTimer::timeout, this, &MainWindow::handleContactLost);
    contactLossTimer->setSingleShot(true);

    connect(sessionPTR, &HandleSessions::paused, baselineCalculator, &BaselineCalculator::handlePause);
    connect(sessionPTR, &HandleSessions::quit, baselineCalculator, &BaselineCalculator::handleQuit);
    connect(ui->batteryCharge, &QPushButton::clicked, this, &MainWindow::chargeBattery);
    rechargeTimer = new QTimer(this);
    connect(rechargeTimer, &QTimer::timeout, this, &MainWindow::incrementBatteryLevel);
    connect(ui->contactButton, &QPushButton::clicked, this, &MainWindow::contactButton);

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
        if (ui->option1->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option1->setStyleSheet("");
            ui->option3->setStyleSheet("background-color: yellow;");
        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option2->setStyleSheet("");
            ui->option1->setStyleSheet("background-color: yellow;");
        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option3->setStyleSheet("");
            ui->option2->setStyleSheet("background-color: yellow;");
        }
    }

}

void MainWindow::downButton() {
    qInfo("Down Button Pressed");

    QPushButton *downButtonPressed = qobject_cast<QPushButton*>(sender());
    if (downButtonPressed) {
        if (ui->option1->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option1->setStyleSheet("");
            ui->option2->setStyleSheet("background-color: yellow;");
        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option2->setStyleSheet("");
            ui->option3->setStyleSheet("background-color: yellow;");
        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            ui->option3->setStyleSheet("");
            ui->option1->setStyleSheet("background-color: yellow;");
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
    std::iota(x.begin(), x.end(), 0);

    ui->plotWidget->clearGraphs();
    ui->plotWidget->addGraph();
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

void MainWindow::forcePower() {
    systemOn = false;  // Set the system status to off
    ui->powerButton->setStyleSheet("background-color: green;");
    threeButtonsOff();

    functionalButtonsOff();

    lightsOff();
    ui->option1->setText("");
    ui->dateInput->hide();
    ui->option2->setText("");
    ui->listWidget->hide();
    ui->option3->setText("");
    ui->treatmentLabel->setText("");
    ui->progressBar->hide();
    ui->plotWidget->clearGraphs();
    ui->plotWidget->hide();

    if (batteryTimer->isActive()) {
        batteryTimer->stop();  // Stop the battery timer as well
    }
    ui->batteryCharge->setEnabled(true);

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
       ui->batteryCharge->setEnabled(false);
       ui->contactButton->setEnabled(false);

       // Setting up the battery
       batteryTimer = new QTimer(this);
       connect(batteryTimer, &QTimer::timeout, this, &MainWindow::decreaseBatteryLevel);
       batteryTimer->start(1000); // Every 1 second - perfect for testing and then showing battery recharge

     } else {
        ui->powerButton->setStyleSheet("background-color: green;");
        threeButtonsOff();
        functionalButtonsOff();
        lightsOff(); ui->option1->setText("");
        ui->dateInput->hide(); ui->option2->setText("");
        ui->listWidget->hide(); ui->option3->setText("");
        ui->treatmentLabel->setText("");
        ui->progressBar->hide();
        ui->plotWidget->clearGraphs(); ui->plotWidget->hide();


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

    // Implement further
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

}


void MainWindow::decreaseBatteryLevel() {
    int val = ui->battery->value();
    if (val > 0) {
        ui->battery->setValue(val - 1);  // Decrease battery by 1%
        if (val - 1 <= 20) {
            ui->battery->setStyleSheet("QProgressBar::chunk { background-color: red; }");
            if (val == 1) {
                if (!rechargeTimer->isActive()) {
                    ui->batteryCharge->setEnabled(true);  // Enable the charge button only if not currently charging
                }
            }
        } else {
            ui->battery->setStyleSheet("");  // Reset to default
        }
    } else {
        batteryTimer->stop();
        ui->battery->setStyleSheet("QProgressBar::chunk { background-color: red; }");
        QMessageBox::warning(this, "Battery Depleted", "The device has been powered off due to battery depletion.");
        forcePower();
    }
}

void MainWindow::incrementBatteryLevel() {
    int val = ui->battery->value();
    if (val < 100) {
        ui->battery->setValue(val + 1);
        ui->battery->setStyleSheet("QProgressBar::chunk { background-color: green; }");
        if (val + 1 == 100) {
            rechargeTimer->stop();
            ui->batteryCharge->setEnabled(false);
        }
    } else {
        rechargeTimer->stop();
    }
}


void MainWindow::chargeBattery() {
    if (ui->battery->value() <= 20) {
        rechargeTimer->start(100);
    }
}

void MainWindow::checkContactStatus() {
    if (qrand() % 40 == 0) {  // Simulate contact loss (change the modulo operand for testing purposes, ideal range 5 - 10)
        contactEstablished = false;
        if (!contactEstablished) {
            ui->contactLostLight->setStyleSheet("background-color: red;");
            contactLossTimer->start(5 * 60 * 1000); // change to 10 000 for testing purposes
            ui->contactButton->setEnabled(true);
            QMessageBox::warning(this, "Contact Lost", "Contact with the EEG device has been lost. Please reestablish contact.");

        } else {
            handleContactReestablished();
        }
    }
}


void MainWindow::handleContactLost() {
    QMessageBox::critical(this, "Contact Not Reestablished", "Failed to reestablish contact. The session will be terminated and the device will now turn off.");
    forcePower();  // Force the power off
}

void MainWindow::handleContactReestablished() {
    contactEstablished = true;
    ui->contactLight->setStyleSheet("background-color: blue;");
    ui->contactLostLight->setStyleSheet("background-color: grey;");
    contactLossTimer->stop();
    QMessageBox::warning(this, "Contact Reestablished", "Contact with the EEG device has been reestablished.");

}

void MainWindow::contactButton() {
    handleContactReestablished();
    ui->contactButton->setEnabled(false);
}


void MainWindow::lightsOff() {
    // Setting all Lights off
    ui->treatmentLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: gray; border-radius: 9px;");
}

void MainWindow::threeButtonsOff() {

    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);

}


void MainWindow::functionalButtonsOff() {
    ui->menuButton->setEnabled(false);
    ui->selectButton->setEnabled(false);
    ui->upButton->setEnabled(false);
    ui->downButton->setEnabled(false);
}


void MainWindow::onDone() {
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
    ui->treatmentLabel->setText(QString("Treating site %1 with frequency %2 Hz").arg(siteIndex + 1).arg(treatmentFrequency));
    ui->treatmentLight->setStyleSheet("background-color: green");
}

void MainWindow::onTreatmentComplete() {
    ui->treatmentLabel->setText("Treatment completed for all sites");
}

void MainWindow::updateProgressBar(int progress) {
    ui->progressBar->setValue(progress);
}












