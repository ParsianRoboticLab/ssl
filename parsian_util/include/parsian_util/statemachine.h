#ifndef STATEMACHINE_H
#define STATEMACHINE_H
#include <QObject>

class CState : public QObject {
    Q_OBJECT
public:
    QString name;
    void exec();
signals:
    void run();
};

class CStateMachine {
    QList<int> curState;
    QList<CState*> states;
    int currently_running_state;
public:
    CStateMachine();
    int currentState();
    QString currentStateName();
    void addCurrentState(int _curState);
    void setCurrentState(int tostate);
    void addState(CState* state);
    QString getStateName(int st);
    void run();
    void reset();
    bool isStateRunning(int st);
};
#endif // STATEMACHINE_H
