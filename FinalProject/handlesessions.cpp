#include "handlesessions.h"
#include <QFile>
#include <QTextStream>

HandleSessions::HandleSessions(QObject *parent) : QObject(parent), duration(0) {
    sessionTimer = new QTimer(this);
    connect(sessionTimer, &QTimer::timeout, this, &HandleSessions::sessionTimerT);
}

void HandleSessions::start() {
        duration = 0;
        sessionTimer->start(100); // 1000 = 1 second,      100 = 0.1 second
}

// Called when pause button is clicked
void HandleSessions::pause() {
    if(isPaused && duration != 0) {
        resume();
    } else {
        sessionTimer->stop();
        emit paused();
        isPaused = true;
    }
}

void HandleSessions::resume() {
    isPaused = false;
    sessionTimer->start();
    emit resumed();
}

// Increaseses Timer and calls finished when done
void HandleSessions::sessionTimerT() {
    if(duration == 100) {
        finished();
    } else {
        //qDebug() << "test";
        duration++;
        emit sessionUpdated(duration);
    }

}

void HandleSessions::updateProgress() {
    //
}

// Called when Stop button is clicked
void HandleSessions::stop() {
    if(sessionTimer->isActive()) {
        sessionTimer->stop();
    }
    duration = 0;
}

// Called when session is finished
void HandleSessions::finished() {
    if(sessionTimer->isActive()) {
        sessionTimer->stop();
    }
    duration = 0;
    emit done();
    // Code or a call to a function for saving the session
}























