#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTimer>
#include <QObject>
#include <iostream>
#include <cmath>


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
            //add a timer to the screen
        } else if (ui->option2->palette().color(QPalette::Window) == Qt::yellow) {
            //show session log

        } else if (ui->option3->palette().color(QPalette::Window) == Qt::yellow) {
            //add input for user to set date and time


        }



    }



}


void MainWindow::powerButton(){
    qInfo("Power Button Pressed");

    QPushButton *powerButtonPressed = qobject_cast<QPushButton*>(sender());
       if (powerButtonPressed) {


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
}

void MainWindow::pauseButton() {
    qInfo("Pause Button Pressed");
}

void MainWindow::startButton() {
    qInfo("Start Button Pressed");
}

void MainWindow::stopButton() {
    qInfo("Stop Button Pressed");
}

