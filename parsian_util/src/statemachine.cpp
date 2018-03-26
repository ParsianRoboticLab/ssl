#include "parsian_util/statemachine.h"

void CState::exec() {
    emit run();
}

CStateMachine::CStateMachine() {
    currently_running_state = -1;
}

int CStateMachine::currentState() {
    if (currently_running_state == -1) {
        return -1;
    }
    return curState[currently_running_state];
}

QString CStateMachine::currentStateName() {
    if (currently_running_state == -1) {
        return "NO-STATE";
    }
    return states[curState[currently_running_state]]->name;
}

QString CStateMachine::getStateName(int st) {
    if (st < 0) {
        return "";
    }
    if (st >= curState.count()) {
        return "";
    }
    if (curState[st] < 0) {
        return "";
    }
    return states[curState[st]]->name;
}

void CStateMachine::addState(CState* state) {
    states.append(state);
}

void CStateMachine::addCurrentState(int _curState) {
    curState.append(_curState);
}

void CStateMachine::setCurrentState(int tostate) {
    if (tostate == -1) {
        curState.removeAt(currently_running_state);
    } else {
        if (currently_running_state == -1) {
            addCurrentState(tostate);
        } else {
            curState[currently_running_state] = tostate;
        }
    }
}

void CStateMachine::run() {
    currently_running_state = -1;
    for (int i = 0; i < curState.count(); i++)
        if (curState[i] >= 0 && curState[i] < states.count()) {
            currently_running_state = i;
            states[curState[i]]->exec();
        }
    for (int i = 0; i < curState.count(); i++) {
        if (curState[i] == -1) {
            curState.removeAt(i);
            i--;
        }
    }
    currently_running_state = -1;
}

bool CStateMachine::isStateRunning(int st) {
    return curState.contains(st);
}

void CStateMachine::reset() {
    currently_running_state = -1;
    curState.clear();
}
