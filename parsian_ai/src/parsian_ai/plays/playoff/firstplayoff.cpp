//
// Created by parsian-ai on 11/7/18.
//

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
    for (auto &i : newRoleAgent) {
        i = new CRolePlayOff();
    }

}

CFirstPlayOff::~CFirstPlayOff() {
    for (auto &i : newRoleAgent) {
        delete i;
    }
}

void CFirstPlayOff::execute() {

}

void CFirstPlayOff::init(const QList<Agent*>& _agent) {
    agents = _agent;
}

void CFirstPlayOff::reset() {
    firstStepEnums = Stay;
    for (auto &i : newRoleAgent) {
        i->reset();
    }
}

void CFirstPlayOff::firstExecute() {
    // TODO : Write first Execution (playoff)


    if (gameState->ourKickoff()) {
//                kickOffStopModePlay(masterPlan->common.currentSize);
    } else {
        firstPlayForOppCorner(agents.size());

    }

    // TODO : a function that calculate opponent mark streategy :D
    int shotBlocked = 0;
    int passBlocked = 0;
    Vector2D sol1, sol2;
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->id == wm->opp.data->goalieID) {
            continue;
        }
        for (auto &j : newRoleAgent) {
            if (j->getAgent() == nullptr) {
                continue;
            }
            if (Circle2D(wm->opp.active(i)->pos, 0.25).intersection(Segment2D(j->getAgent()->pos(), wm->field->oppGoal()), &sol1, &sol2)) {
                shotBlocked++;
            }
        }
    }

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (wm->opp.active(i)->id == wm->opp.data->goalieID) {
            continue;
        }
        for (auto &j : newRoleAgent) {
            if (j->getAgent() == nullptr) {
                continue;
            }
            if (Circle2D(wm->opp.active(i)->pos, 0.25).intersection(Segment2D(j->getAgent()->pos(), wm->ball->pos), &sol1, &sol2)) {
                passBlocked++;
            }
        }
    }


    QList<int> oppMark = wm->opp.data->activeAgents;
    oppMark.removeOne(wm->opp.data->goalieID);
//    oppMark.removeOne(knowledge->nearestOppToBall); // TODO : add to know
    double sumDist = 0.0;
    for (int i : oppMark) {
        sumDist += wm->opp[i]->pos.dist(wm->field->oppGoal());
    }
    sumDist /= oppMark.size();
    if (sumDist < 2) {
        shotSpot = FarNear | FarFar | FarCenter;
    } else if (sumDist > 3) {
        shotSpot = KillSpot;
    } else {
        shotSpot = EveryWhere;
    }



    // TODO : fix tagging ;)
    DBUG(QString("PB : %1, SB : %2").arg(passBlocked).arg(shotBlocked), D_MAHI);
    DBUG(QString("ShotSpot : %1 ").arg(shotSpot), D_MAHI);

//    analyze("OPP MARK SPOT", shotSpot, true);
//    analyze("OPP MARK PASS", passBlocked, true);
//    analyze("OPP MARK SHOT", shotBlocked, true);


    for (auto &i : newRoleAgent) {
        i->execute();
    }

}

void CFirstPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < tAgentsize; i++) {
        if (!newRoleAgent[i]->isRoleUpdated()) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(agents[0]);
            newRoleAgent[i]->setRoleUpdate(true);
            newRoleAgent[i]->setAvoidBall(true);
            newRoleAgent[i]->setAvoidPenaltyArea(true);
            newRoleAgent[i]->setSelectedSkill(RoleSkill::GotopointAvoid);
            newRoleAgent[i]->setAvoidBall(true);
            newRoleAgent[i]->setTarget(kickOffPos[i]);
            newRoleAgent[i]->setTargetDir(-newRoleAgent[i]->getAgent()->pos() + wm->field->oppGoal());
        }
    }
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);

}

void CFirstPlayOff::firstPlayForOppCorner(int _agentSize) {

    DBUG(QString("mode :%1").arg(firstStepEnums), D_NADIA);
    for (int i = 0; i < _agentSize; i++) {
        if (!newRoleAgent[i]->isRoleUpdated()) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(agents[dynamicMatch[i]]);
            newRoleAgent[i]->setRoleUpdate(true);
            newRoleAgent[i]->setAvoidBall(true);
            newRoleAgent[i]->setAvoidPenaltyArea(true);
            newRoleAgent[i]->setSelectedSkill(RoleSkill::GotopointAvoid);

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
        if (newRoleAgent[i]->getTarget().dist(newRoleAgent[i]->getAgent()->pos()) < 0.4) {
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
            newRoleAgent[i]->execute();
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

    newRoleAgent[1]->setTarget(Vector2D(1, .5));
    newRoleAgent[1]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[2]->setTarget(Vector2D(1 - m, -1.5));
    newRoleAgent[2]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[3]->setTarget(Vector2D(1 + m, 1.5));
    newRoleAgent[3]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[4]->setTarget(Vector2D(1 - 2 * m, -2.5));
    newRoleAgent[4]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[5]->setTarget(Vector2D(1 + 2 * m, 2.5));
    newRoleAgent[5]->setTargetDir(wm->field->oppGoal());
    // TODO : check newRoleAgent for new robots
    newRoleAgent[6]->setTarget(Vector2D(1 - 3 * m, 3.5));
    newRoleAgent[6]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[7]->setTarget(Vector2D(1 + 3 * m, 3.5));
    newRoleAgent[7]->setTargetDir(wm->field->oppGoal());

}

void CFirstPlayOff::movePositioning() {
    int sign;
    sign = wm->ball->pos.y > 0 ? 1 : -1;

    newRoleAgent[1]->setTarget(Vector2D(1, -1.5));
    newRoleAgent[1]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[2]->setTarget(Vector2D(1, .5));
    newRoleAgent[2]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[3]->setTarget(Vector2D(-.5, 1.5));
    newRoleAgent[3]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[4]->setTarget(Vector2D(2.5, -2.5));
    newRoleAgent[4]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[5]->setTarget(Vector2D(2, 2.5));
    newRoleAgent[5]->setTargetDir(wm->field->oppGoal());
    // TODO : check newRoleAgent for new robots
    newRoleAgent[6]->setTarget(Vector2D(3, 3.5));
    newRoleAgent[6]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[7]->setTarget(Vector2D(2.5, -3.5));
    newRoleAgent[7]->setTargetDir(wm->field->oppGoal());

}

void CFirstPlayOff::donePositioning() {


    newRoleAgent[1]->setTarget(Vector2D(2, -.5));
    newRoleAgent[1]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[2]->setTarget(Vector2D(3.5, -1.5));
    newRoleAgent[2]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[3]->setTarget(Vector2D(0, 1.5));
    newRoleAgent[3]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[4]->setTarget(Vector2D(2, -2.5));
    newRoleAgent[4]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[5]->setTarget(Vector2D(2, 2.5));
    newRoleAgent[5]->setTargetDir(wm->field->oppGoal());
    // TODO : check newRoleAgent for new robots
    newRoleAgent[6]->setTarget(Vector2D(3, 3.5));
    newRoleAgent[6]->setTargetDir(wm->field->oppGoal());
    newRoleAgent[7]->setTarget(Vector2D(2.5, -3.5));
    newRoleAgent[7]->setTargetDir(wm->field->oppGoal());
}

