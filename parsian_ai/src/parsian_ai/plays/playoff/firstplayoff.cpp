//
// Created by parsian-ai on 11/7/18.
//

#include <parsian_ai/plays/playoff/firstplayoff.h>

#include "parsian_ai/plays/playoff/firstplayoff.h"

CFirstPlayOff::CFirstPlayOff() {
    kickOffPos[0] = Vector2D(-0.36, 0);
    kickOffPos[1] = Vector2D(-0.36,  3);
    kickOffPos[2] = Vector2D(-0.36, -3);
    kickOffPos[3] = Vector2D(-2.4,  0);
    kickOffPos[4] = Vector2D(-0.9,  1.2);
    kickOffPos[5] = Vector2D(-0.9, -1.2);
    kickOffPos[6] = Vector2D(-0.75, 1.7);
    kickOffPos[7] = Vector2D(-0.75, -1.7);

    reset();

}

CFirstPlayOff::~CFirstPlayOff() {
}

void CFirstPlayOff::init(const QList<Agent*>& _agent) {
    agents.clear();
    agents.append(_agent);
}

void CFirstPlayOff::reset() {
    firstStepEnums = Stay;
    for (auto &i : roleAgents) {
        i->reset();
    }
    firstFinished = false;

}

void CFirstPlayOff::execute() {

    if (gameState->ourKickoff()) {
        kickOffStopModePlay(agents.size());
        if (gameState->canKickBall()) firstFinished = true;
    } else {
        firstPlayForOppCorner(agents.size());
    }

    matchAgent();


    int finisher = 0;
    for (int i = 0; i < agents.size(); i++) {
        if (roleAgents[i]->getTarget().dist(roleAgents[i]->getAgent()->pos()) < 0.4) {
            finisher++;
        }
    }

    if (finisher == agents.size() - 1) {
        firstStepEnums = Done;
        firstFinished = true;
    }


    for (auto &roleAgent : roleAgents) roleAgent->execute();


    // TODO : a function that calculate opponent mark strategy :D

    int shotBlocked = shotBlockers();
    int passBlocked = passBlockers();

    double averageMarkDist = distAverageOppMark();
    if (averageMarkDist < 2) shotSpot = FarNear | FarFar | FarCenter;
    else if (averageMarkDist > 3) shotSpot = KillSpot;
    else shotSpot = EveryWhere;

    // TODO : fix tagging ;)
    DBUG(QString("PB : %1, SB : %2").arg(passBlocked).arg(shotBlocked), D_MAHI);
    DBUG(QString("ShotSpot : %1 ").arg(shotSpot), D_MAHI);


}

void CFirstPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < tAgentsize; i++) {
        if (!roleAgents[i]->isRoleUpdated()) {
            roleAgents[i]->setUpdated(true);
            roleAgents[i]->setRoleUpdate(true);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setAvoidPenaltyArea(true);
            roleAgents[i]->setSelectedSkill(RoleSkill::GotopointAvoid);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setTarget(kickOffPos[i]);
            roleAgents[i]->setLookAt(wm->field->oppGoal());
        }
    }
    roleAgents[0]->setLookAt(wm->ball->pos);

}

void CFirstPlayOff::firstPlayForOppCorner(int _agentSize) {

    DBUG(QString("mode :%1").arg(firstStepEnums), D_NADIA);
    for (int i = 0; i < _agentSize; i++) {
        if (!roleAgents[i]->isRoleUpdated()) {
            roleAgents[i]->setUpdated(true);
            roleAgents[i]->setRoleUpdate(true);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setAvoidPenaltyArea(true);
            roleAgents[i]->setSelectedSkill(RoleSkill::GotopointAvoid);
        }
    }

    roleAgents[0]->setTarget(wm->ball->pos - Vector2D(0.2, 0));
    roleAgents[0]->setLookAt(wm->ball->pos);

    switch (firstStepEnums) {
        case Stay:
            stayPoisitioning();
            break;
        case Done:
            donePositioning();
            break;
    }

}

bool CFirstPlayOff::isFirstFinished() {
    return firstFinished;
}

void CFirstPlayOff::resetFirstPlayFinishedFlag() {
    firstStepEnums = Stay;
}


int CFirstPlayOff::getShotSpot() {
    return shotSpot;
}

void CFirstPlayOff::stayPoisitioning() {

    double x = wm->ball->pos.x;
    int m;
    if (x > wm->field->_FIELD_WIDTH / 3) {
        m = 0;
    } else if (x > wm->field->_FIELD_WIDTH / 6) {
        m = 1;
    } else {
        m = -1;
    }

    roleAgents[1]->setTarget(Vector2D(1, .5));
    roleAgents[1]->setLookAt(wm->field->oppGoal());
    roleAgents[2]->setTarget(Vector2D(1 - m, -1.5));
    roleAgents[2]->setLookAt(wm->field->oppGoal());
    roleAgents[3]->setTarget(Vector2D(1 + m, 1.5));
    roleAgents[3]->setLookAt(wm->field->oppGoal());
    roleAgents[4]->setTarget(Vector2D(1 - 2 * m, -2.5));
    roleAgents[4]->setLookAt(wm->field->oppGoal());
    roleAgents[5]->setTarget(Vector2D(1 + 2 * m, 2.5));
    roleAgents[5]->setLookAt(wm->field->oppGoal());
    roleAgents[6]->setTarget(Vector2D(1 - 3 * m, 3.5));
    roleAgents[6]->setLookAt(wm->field->oppGoal());
    roleAgents[7]->setTarget(Vector2D(1 + 3 * m, 3.5));
    roleAgents[7]->setLookAt(wm->field->oppGoal());

}

void CFirstPlayOff::donePositioning() {


    roleAgents[1]->setTarget(Vector2D(2, -.5));
    roleAgents[1]->setTargetDir(wm->field->oppGoal());
    roleAgents[2]->setTarget(Vector2D(3.5, -1.5));
    roleAgents[2]->setTargetDir(wm->field->oppGoal());
    roleAgents[3]->setTarget(Vector2D(0, 1.5));
    roleAgents[3]->setTargetDir(wm->field->oppGoal());
    roleAgents[4]->setTarget(Vector2D(2, -2.5));
    roleAgents[4]->setTargetDir(wm->field->oppGoal());
    roleAgents[5]->setTarget(Vector2D(2, 2.5));
    roleAgents[5]->setTargetDir(wm->field->oppGoal());
    roleAgents[6]->setTarget(Vector2D(3, 3.5));
    roleAgents[6]->setTargetDir(wm->field->oppGoal());
    roleAgents[7]->setTarget(Vector2D(2.5, -3.5));
    roleAgents[7]->setTargetDir(wm->field->oppGoal());
}

int CFirstPlayOff::shotBlockers() {
    int shotBlocked = 0;
    Vector2D sol1, sol2;
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->id == wm->opp.data->goalieID) {
            continue;
        }
        for (auto &j : roleAgents) {
            if (j->getAgent() == nullptr) {
                continue;
            }
            if (Circle2D(wm->opp.active(i)->pos, 0.25).intersection(Segment2D(j->getAgent()->pos(), wm->field->oppGoal()), &sol1, &sol2)) {
                shotBlocked++;
            }
        }
    }
    return shotBlocked;
}

int CFirstPlayOff::passBlockers() {
    int passBlocked = 0;
    Vector2D sol1, sol2;

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->id == wm->opp.data->goalieID) {
            continue;
        }
        for (auto &j : roleAgents) {
            if (j->getAgent() == nullptr) {
                continue;
            }
            if (Circle2D(wm->opp.active(i)->pos, 0.25).intersection(Segment2D(j->getAgent()->pos(), wm->ball->pos), &sol1, &sol2)) {
                passBlocked++;
            }
        }
    }
    return passBlocked;

}

double CFirstPlayOff::distAverageOppMark() {
    QList<int> oppMark = wm->opp.data->activeAgents;
    oppMark.removeOne(wm->opp.data->goalieID);
//    oppMark.removeOne(knowledge->nearestOppToBall); // TODO : add to know
    double sumDist = 0.0;
    for (int i : oppMark) {
        sumDist += wm->opp[i]->pos.dist(wm->field->oppGoal());
    }
    sumDist /= oppMark.size();
    return  sumDist;
}

