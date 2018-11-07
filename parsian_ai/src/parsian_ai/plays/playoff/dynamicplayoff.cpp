//
// Created by parsian-ai on 11/7/18.
//

#include <parsian_ai/plays/playoff/dynamicplayoff.h>

#include "parsian_ai/plays/playoff/dynamicplayoff.h"

CDynamicPlayoff::CDynamicPlayoff() : CMasterPlay() {
    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = 0;
    for (auto &roleAgent : roleAgents) roleAgent = new CRolePlayOff();
    initial = true;
    playOnFlag = false;

}

CDynamicPlayoff::~CDynamicPlayoff(){
    for (auto &roleAgent : roleAgents) delete roleAgent;

}

void CDynamicPlayoff::execute_x() {
    dynamicExecute();
}

void CDynamicPlayoff::reset() {

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = 0;
    initial = false;
}




void CDynamicPlayoff::dynamicExecute() {

    switch (dynamicSelect) {
        case DynamicSelect::NoSelect:
            break;
        case DynamicSelect::Chip:
            dynamicPlayChipToGoal(true);
            checkEndChipToGoal();
            break;
        case DynamicSelect::Kick:
            dynamicPlayChipToGoal(false);
            checkEndChipToGoal();
            break;
        case DynamicSelect::Blocker:
            dynamicPlayBlocker();
            checkEndBlocker();
            break;
        case DynamicSelect::Khafan:
            dynamicPlayKhafan();
            checkEndKhafan();
            break;
    }

    for (int i = 0; i < dynamicAgentSize; i++) {
        roleAgents[i]->execute();
    }
}


void CDynamicPlayoff::dynamicAssignID() {
    lastTime = ros::Time::now().sec;
    lastBallPos = wm->ball->pos;
    initial = false;

    dynamicAgentSize = _NUM_PLAYERS;
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        if (dynamicMatch[i] != -1) {
            roleAgents[i] -> setAgent(agents[dynamicMatch[i]]);
            roleAgents[i] -> setAgentID(dynamicMatch[i]);
        } else {
            dynamicAgentSize = i;
            break;
        }
    }
}

void CDynamicPlayoff::dynamicPlayChipToGoal(bool isChip) {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgents[0] -> setAvoidCenterCircle(false);
        roleAgents[0] -> setAvoidPenaltyArea(true);
        roleAgents[0] -> setChip(isChip);
        roleAgents[0] -> setKickSpeed(6.5); // TODO: Use Global Constants
        roleAgents[0] -> setTarget(wm->field->oppGoal());
        roleAgents[0] -> setDoPass(false);
        roleAgents[0] -> setIntercept(false);
        roleAgents[0] -> setLookForward(false);
        roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgents[i] -> setAvoidPenaltyArea(true);
                roleAgents[i] -> setAvoidBall(true);
                roleAgents[i] -> setTimeBased(false);
                roleAgents[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgents[i] -> setTargetDir(wm->field->oppGoal() - roleAgents[i]->getAgent()->pos());
                roleAgents[i] -> setEventDist(0.3);
                roleAgents[i] -> setSlow(false);
                roleAgents[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {
        roleAgents[0] -> setDoPass(true);
        shot = false;
    }
}

void CDynamicPlayoff::dynamicPlayBlocker() {
    if (initial) {
        dynamicAssignID();
        ready = true;
    } else if (ready) {
        roleAgents[0] -> setAvoidCenterCircle(false);
        roleAgents[0] -> setAvoidPenaltyArea(true);
        roleAgents[0] -> setChip(false);
        roleAgents[0] -> setKickSpeed(1023);//knowledge->getProfile(roleAgents[0]->getAgentID(), 7.8, false, false)); // Vartypes This TODO
        roleAgents[0] -> setTarget(wm->field->oppGoal().rotatedVector((wm->ball->pos.y < 0 ? 90 : -90)));
        roleAgents[0] -> setDoPass(false);
        roleAgents[0] -> setIntercept(false);
        roleAgents[0] -> setLookForward(false);
        roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgents[i] -> setAvoidPenaltyArea(true);
                roleAgents[i] -> setAvoidBall(true);
                roleAgents[i] -> setTimeBased(false);
                roleAgents[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgents[i] -> setTargetDir(wm->field->oppGoal() - roleAgents[i]->getAgent()->pos());
                roleAgents[i] -> setEventDist(0.3);
                roleAgents[i] -> setSlow(false);
                roleAgents[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {

        roleAgents[0] -> setKickSpeed(7);//knowledge->getProfile(roleAgents[0]->getAgentID(), 7.8, false, false)); // Vartypes This TODO
        roleAgents[0] -> setTarget(wm->field->oppGoal());
        roleAgents[0] -> setDoPass(true);
        roleAgents[0] -> setTargetDir(wm->field->oppGoal());
        roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);
        shot = false;
    }

}

void CDynamicPlayoff::dynamicPlayKhafan() {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgents[0] -> setAvoidCenterCircle(false);
        roleAgents[0] -> setAvoidPenaltyArea(true);
        roleAgents[0] -> setChip(true);
        roleAgents[0] -> setKickSpeed(6.5); // Vartypes This
        roleAgents[0] -> setTarget(wm->field->oppGoal());
        roleAgents[0] -> setDoPass(false);
        roleAgents[0] -> setIntercept(false);
        roleAgents[0] -> setTargetDir(wm->field->oppGoal());
        roleAgents[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgents[i] -> setAvoidPenaltyArea(true);
                roleAgents[i] -> setAvoidBall(true);
                roleAgents[i] -> setTimeBased(false);
                roleAgents[i] -> setTarget(getDynamicTarget(i));
                roleAgents[i] -> setTargetDir(wm->field->oppGoal() - roleAgents[i]->getAgent()->pos());
                roleAgents[i] -> setEventDist(0.3);
                roleAgents[i] -> setSlow(false);
                roleAgents[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (pass) {
        roleAgents[0] -> setDoPass(true);
        pass = false;
        DBUG("DYNAMIC :D ", D_MAHI);

    } else if (shot) {
        roleAgents[1] -> setAvoidCenterCircle(false);
        roleAgents[1] -> setAvoidPenaltyArea(true);
        roleAgents[1] -> setChip(false);
        roleAgents[1] -> setKickSpeed(1023); // Vartypes This
        roleAgents[1] -> setTarget(wm->field->oppGoal());
        roleAgents[1] -> setDoPass(true);
        roleAgents[1] -> setIntercept(false);
        roleAgents[1] -> setTargetDir(wm->field->oppGoal());
        roleAgents[1] -> setSelectedSkill(RoleSkill::Kick);
        shot = false;

        roleAgents[0] -> setAvoidPenaltyArea(true);
        roleAgents[0] -> setAvoidBall(true);
        roleAgents[0] -> setTimeBased(false);
        roleAgents[0] -> setTarget(Vector2D(0, -2));
        roleAgents[0] -> setTargetDir(wm->field->oppGoal() - roleAgents[0]->getAgent()->pos());
        roleAgents[0] -> setEventDist(0.3);
        roleAgents[0] -> setSlow(false);
        roleAgents[0] -> setSelectedSkill(RoleSkill::GotopointAvoid);


    }

}


void CDynamicPlayoff::checkEndKhafan() {
    ROS_INFO_STREAM("TIMENS: "<< ros::Time::now().sec << " TIMES: "<< ros::Time::now().sec);
    if (ready) {
        dynamicState = 2;
    } else if (pass) {
        dynamicState = 4;
    } else if (shot) {
        dynamicState = 6;
    }

    if (dynamicState == 2) {
        if (roleAgents[1] -> getAgent() -> pos().dist(roleAgents[1] -> getTarget())
            < roleAgents[1] -> getEventDist()) {
            dynamicState = 4;
            pass = true;
        }
    }

    if (dynamicState == 4) {

        DBUG(QString("ENDKHAFAN : %1").arg(ros::Time::now().sec - dynamicStartTime), D_MAHI);
        if (wm->ball->pos.dist(wm->field->oppGoal()) - 0.5 < roleAgents[1]->getAgent()->pos().dist(wm->field->oppGoal())) {
            pass = false;
            shot = true;
            dynamicState = 6;
        }
        if (!Circle2D(roleAgents[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos) && dynamicStartTime == -1) {
            dynamicStartTime = ros::Time::now().sec;
        }

        if (wm->ball->vel.length() < 0.2 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if ((ros::Time::now().sec - dynamicStartTime) > 3 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;

        }
    }

    if (dynamicState == 6) {
        // TODO : check this
        playOnFlag = true;
        shot = false;
        if (wm->ball->vel.length() < 0.2) {
            playOnFlag = true;
            dynamicState = 0;
        }
        DBUG(QString("[dastan] : %1").arg(ros::Time::now().sec - dynamicStartTime), D_MAHI);

        if (ros::Time::now().sec - dynamicStartTime > 2 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;

        }

    }

}

void CDynamicPlayoff::checkEndBlocker() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            if (Circle2D(roleAgents[0] -> getAgent() -> pos() + roleAgents[0]->getAgent()->dir().norm() * 0.6, 0.3).contains(wm->opp.active(i)->pos))
                if (roleAgents[0]->getAgent()->dir().norm().dist(roleAgents[0]->getTarget().norm()) < 0.1) {
                    dynamicState = 6;
                    shot = true;
                }
        }

        dynamicStartTime = ros::Time::now().sec;

    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgents[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (ros::Time::now().sec - dynamicStartTime > 3 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

void CDynamicPlayoff::checkEndChipToGoal() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        if (Circle2D(wm->ball->pos, 0.5).contains(roleAgents[0]->getAgent()->pos())) {
            shot = true;
            dynamicState = 6;
            dynamicStartTime = ros::Time::now().sec;
        }
    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgents[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (ros::Time::now().sec - dynamicStartTime > 2 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

Vector2D CDynamicPlayoff::getDynamicTarget(int i) {
    Vector2D first = wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 3;
    first.y += 0.3;

    switch (i) {
        case 1:
            return first;
        case 2:
            return Vector2D{3.2, 0.7};
        case 3:
            return Vector2D{3.2, -0.7};
        case 4:
            return Vector2D{3, 1.5};
        case 5:
            return Vector2D{3, -1.5};
        default:
            return Vector2D::INVALIDATED;
    }
}

void CDynamicPlayoff::execute() {

}

void CDynamicPlayoff::initDynamicPlay(const QList<Agent*> &_ourplayers) {

    for (int i = 0; i < _NUM_PLAYERS; i++) {
        if (i >= _ourplayers.size()) {
            dynamicMatch[i] = -1;
        } else {
            dynamicMatch[i] = i;
        }
    }
    if (_ourplayers.size() < 2) {
        dynamicSelect = DynamicSelect::Chip;
    } else {
        dynamicSelect = DynamicSelect::Khafan;
    }


    double dis = 1000000;
    int index = 0;
    int swapID = 0;
    for (int i = 0; i < _ourplayers.size(); i++) {
        double tempDis = _ourplayers.at(i)->pos().dist(wm->ball->pos) ;
        if (tempDis < dis) {
            dis = tempDis;
            index = i;
            swapID = i;
        }
    }

    int tempID = dynamicMatch[0];
    dynamicMatch[0] = index;
    dynamicMatch[swapID] = tempID;
    initial = true;

}