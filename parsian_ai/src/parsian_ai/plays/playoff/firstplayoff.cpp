//
// Created by parsian-ai on 11/7/18.
//

#include <parsian_ai/plays/playoff/firstplayoff.h>

#include "parsian_ai/plays/playoff/firstplayoff.h"

CFirstPlayOff::CFirstPlayOff() {
    kickOffPos[0] = Vector2D(wm->ball->pos.x - 0.36, wm->ball->pos.y);
    kickOffPos[1] = Vector2D(-0.36,  3);
    kickOffPos[2] = Vector2D(-0.36, -3);
    kickOffPos[3] = Vector2D(-2.4  ,  0);
    kickOffPos[4] = Vector2D(-0.9,  1.2);
    kickOffPos[5] = Vector2D(-0.9, -1.2);
    // TODO : fill kickoff pos for rest of robots if needed
    kickOffPos[6] = Vector2D(-0.75, 1.7);
    kickOffPos[7] = Vector2D(-0.75, -1.7);

    firstStepEnums = Stay;
    for (auto &roleAgent : roleAgents) roleAgent = new CRolePlayOff();

}

CFirstPlayOff::~CFirstPlayOff() {
    for (auto &roleAgent : roleAgents) delete roleAgent;
}

void CFirstPlayOff::init(const QList<Agent*>& _agent) {
    agents = _agent;
}

void CFirstPlayOff::reset() {
    firstStepEnums = Stay;
    for (auto &i : roleAgents) {
        i->reset();
    }
}

void CFirstPlayOff::execute() {

    if (gameState->ourKickoff()) {
        kickOffStopModePlay(agents.size());
    } else {
        firstPlayForOppCorner(agents.size());
    }

    // TODO : a function that calculate opponent mark streategy :D

    int shotBlocked = shotBlockers();
    int passBlocked = passBlockers();

    double averageMarkDist = distAverageOppMark();
    if (averageMarkDist < 2) shotSpot = FarNear | FarFar | FarCenter;
    else if (averageMarkDist > 3) shotSpot = KillSpot;
    else shotSpot = EveryWhere;

    // TODO : fix tagging ;)
    DBUG(QString("PB : %1, SB : %2").arg(passBlocked).arg(shotBlocked), D_MAHI);
    DBUG(QString("ShotSpot : %1 ").arg(shotSpot), D_MAHI);


    for (auto &roleAgent : roleAgents) roleAgent->execute();

}

void CFirstPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < tAgentsize; i++) {
        if (!roleAgents[i]->isRoleUpdated()) {
            roleAgents[i]->setUpdated(true);
            roleAgents[i]->setAgent(agents[0]);
            roleAgents[i]->setRoleUpdate(true);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setAvoidPenaltyArea(true);
            roleAgents[i]->setSelectedSkill(RoleSkill::GotopointAvoid);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setTarget(kickOffPos[i]);
            roleAgents[i]->setTargetDir(-roleAgents[i]->getAgent()->pos() + wm->field->oppGoal());
        }
    }
    roleAgents[0]->setTargetDir(-roleAgents[0]->getAgent()->pos() + wm->ball->pos);

}

void CFirstPlayOff::firstPlayForOppCorner(int _agentSize) {

    DBUG(QString("mode :%1").arg(firstStepEnums), D_NADIA);
    for (int i = 0; i < _agentSize; i++) {
        if (!roleAgents[i]->isRoleUpdated()) {
            roleAgents[i]->setUpdated(true);
            roleAgents[i]->setAgent(agents[dynamicMatch[i]]);
            roleAgents[i]->setRoleUpdate(true);
            roleAgents[i]->setAvoidBall(true);
            roleAgents[i]->setAvoidPenaltyArea(true);
            roleAgents[i]->setSelectedSkill(RoleSkill::GotopointAvoid);

        }
    }

    switch (firstStepEnums) {
        case Stay:
            stayPoisitioning();
            break;
        case Move:
            movePositioning();
            break;
        case Done:
            donePositioning();
            break;
        default:
            break;
    }


    int finisher = 0;
    for (int i = 0; i < _agentSize; i++) {
        if (roleAgents[i]->getTarget().dist(roleAgents[i]->getAgent()->pos()) < 0.4) {
            finisher++;
        }
    }

    if (finisher == _agentSize - 1) {
        firstStepEnums = Done;
    }


}

void CFirstPlayOff::kickoffPositioning(int playersNum) {
    if (gameState->ourKickoff()){
        kickOffStopModePlay(playersNum);
        for (int i = 0; i < playersNum; i++) {
            roleAgents[i]->execute();
        }
    }
}

bool CFirstPlayOff::isFirstFinished() {
    return (firstStepEnums == Done);
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
    roleAgents[1]->setTargetDir(wm->field->oppGoal());
    roleAgents[2]->setTarget(Vector2D(1 - m, -1.5));
    roleAgents[2]->setTargetDir(wm->field->oppGoal());
    roleAgents[3]->setTarget(Vector2D(1 + m, 1.5));
    roleAgents[3]->setTargetDir(wm->field->oppGoal());
    roleAgents[4]->setTarget(Vector2D(1 - 2 * m, -2.5));
    roleAgents[4]->setTargetDir(wm->field->oppGoal());
    roleAgents[5]->setTarget(Vector2D(1 + 2 * m, 2.5));
    roleAgents[5]->setTargetDir(wm->field->oppGoal());
    // TODO : check roleAgents for new robots
    roleAgents[6]->setTarget(Vector2D(1 - 3 * m, 3.5));
    roleAgents[6]->setTargetDir(wm->field->oppGoal());
    roleAgents[7]->setTarget(Vector2D(1 + 3 * m, 3.5));
    roleAgents[7]->setTargetDir(wm->field->oppGoal());

}

void CFirstPlayOff::movePositioning() {
    int sign;
    sign = wm->ball->pos.y > 0 ? 1 : -1;

    roleAgents[1]->setTarget(Vector2D(1, -1.5));
    roleAgents[1]->setTargetDir(wm->field->oppGoal());
    roleAgents[2]->setTarget(Vector2D(1, .5));
    roleAgents[2]->setTargetDir(wm->field->oppGoal());
    roleAgents[3]->setTarget(Vector2D(-.5, 1.5));
    roleAgents[3]->setTargetDir(wm->field->oppGoal());
    roleAgents[4]->setTarget(Vector2D(2.5, -2.5));
    roleAgents[4]->setTargetDir(wm->field->oppGoal());
    roleAgents[5]->setTarget(Vector2D(2, 2.5));
    roleAgents[5]->setTargetDir(wm->field->oppGoal());
    // TODO : check roleAgents for new robots
    roleAgents[6]->setTarget(Vector2D(3, 3.5));
    roleAgents[6]->setTargetDir(wm->field->oppGoal());
    roleAgents[7]->setTarget(Vector2D(2.5, -3.5));
    roleAgents[7]->setTargetDir(wm->field->oppGoal());

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
    // TODO : check roleAgents for new robots
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

