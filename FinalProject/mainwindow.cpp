#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <QObject>
#include <iostream>
#include <cmath>
#include "handlesessions.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
    // Hiding the date input and list on startup
    ui->dateInput->hide();
    ui->listWidget->hide();

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
            ui->option1->setStyleSheet("background-color: white;");

            // Enabling the buttons since on correct screen
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(true);
            ui->pauseButton->setEnabled(true);

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


void MainWindow::powerButton(){
    qInfo("Power Button Pressed");

    QPushButton *powerButtonPressed = qobject_cast<QPushButton*>(sender());
       if (powerButtonPressed) {

           // Disabling the buttons until on correct screen, and ensuring lights are off
           threeButtonsOff();
           lightsOff();
           ui->dateInput->hide();
           ui->listWidget->hide();

           // Setting up the battery
           batteryTimer = new QTimer(this);
           connect(batteryTimer, &QTimer::timeout, this, &MainWindow::decreaseBatteryLevel);
           batteryTimer->start(30000); // Every 30 seconds


           ui->option1->setText("NEW SESSION");
           ui->option2->setText("SESSION LOG");
           ui->option3->setText("TIME AND DATE");

           ui->option1->setStyleSheet("background-color: yellow;");


       }




    //QPushButton *powerButton = qobject_cast<QPushButton*>(sender());
    //if (powerButton){ powerButton->setStyleSheet("background-color: yellow;");}

}

void MainWindow::menuButton() {
    qInfo("Menu Button Pressed");

    ui->option1->setText("NEW SESSION");
    ui->option2->setText("SESSION LOG");
    ui->option3->setText("TIME AND DATE");
    ui->option1->setStyleSheet("background-color: yellow;");

    lightsOff();
    threeButtonsOff();
    ui->dateInput->hide();
    ui->listWidget->hide();
}

void MainWindow::pauseButton() {
    qInfo("Pause Button Pressed");
}

void MainWindow::startButton() {
    qInfo("Start Button Pressed");

    // Setting Treatment Light on and the rest off
    ui->treatmentLight->setStyleSheet("background-color: green; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: gray; border-radius: 9px;");

    // Disabling menu and power while running
     ui->menuButton->setEnabled(false);
     ui->powerButton->setEnabled(false);
}

void MainWindow::stopButton() {
    qInfo("Stop Button Pressed");
    // Setting contectLost Light on and the rest off
    ui->treatmentLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLight->setStyleSheet("background-color: gray; border-radius: 9px;");
    ui->contactLostLight->setStyleSheet("background-color: red; border-radius: 9px;");

    // Allowing menu and power buttons again
     ui->menuButton->setEnabled(true);
     ui->powerButton->setEnabled(true);
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
    //qDebug() << "shiii";
    // Allowing menu and power buttons again
     ui->menuButton->setEnabled(true);
     ui->powerButton->setEnabled(true);
     ui->option1->setText("Done");


     // Adding sessions
     QString curDate = ui->dateInput->date().toString("yyyy-MM-dd");
     ui->listWidget->addItem(curDate);
}













