#ifndef HANDLESESSIONS_H
#define HANDLESESSIONS_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

class HandleSessions : public QObject
{
    Q_OBJECT

private:
    QTimer *sessionTimer;
    int duration;
    void updateProgress();
    bool isPaused = false;


public:
    HandleSessions(QObject* parent = nullptr);

    void start();
    void pause();
    void resume();
    void stop();
    void finished();


signals:
    void sessionUpdated(int timeT);
    void paused();
    void resumed();
    void done();


public slots:
    // Timeout signal
    void sessionTimerT();
};

#endif
