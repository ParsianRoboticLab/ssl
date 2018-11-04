
#include <parsian_ai/plays/playoff/playoff.h>

#include "parsian_ai/plays/playoff/playoff.h"


CPlayOff::CPlayOff() : CMasterPlay() {
    blockerState = 0;
    blockerID = -1;
    ROS_INFO("Bring yourself back online playoff");

    for (auto &i : positionAgent) {
        i.stateNumber = 0;
    }
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        roleAgent[i] = new CRolePlayOff();
        newRoleAgent[i] = new CRolePlayOff();
    }
    isBallIn = false;
    tempAgent = new CRolePlayOff();
    doPass = false;
    doAfterlife = false;
    setTimer = true;
    ////////////

    masterPlan = nullptr;
    kickOffPos[0] = Vector2D(wm->ball->pos.x - 0.36, wm->ball->pos.y);
    kickOffPos[1] = Vector2D(-0.36,  3);
    kickOffPos[2] = Vector2D(-0.36, -3);
    kickOffPos[3] = Vector2D(-2.4  ,  0);
    kickOffPos[4] = Vector2D(-0.9,  1.2);
    kickOffPos[5] = Vector2D(-0.9, -1.2);
    // TODO : fill kickoff pos for rest of robots if needed
    kickOffPos[6] = Vector2D(-0.75, 1.7);
    kickOffPos[7] = Vector2D(-0.75, -1.7);


    initial    = true;
    playOnFlag = false;
    firstPass  = true;
    havePassInPlan    = false;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = 0;
    firstStepEnums = Stay;
    blockerStep = S0;

    criticalInit = true;
    criticalKick = new KickAction();

}

CPlayOff::~CPlayOff() {
    ROS_INFO("Playoff is gone");
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        delete roleAgent[i];
        delete newRoleAgent[i];
    }
    delete tempAgent;
}

void CPlayOff::globalExecute() {

    if (!initial && ros::Time::now().sec - lastTime > 10 && lastBallPos.dist(wm->ball->pos) <= 0.06) {
        if (wm->ball->vel.length() > 0.5) {
            criticalPlay();
            playOnFlag = true;
        }
        return;
    }
    ROS_INFO_STREAM("lastTime: " <<ros::Time::now().sec - lastTime<<" masterMode: "<< masterMode <<" balldist: "<<lastBallPos.dist(wm->ball->pos) );
    mainExecute();
}

void CPlayOff::mainExecute() {
    switch (masterMode) {
        case NGameOff::StaticPlay:
            staticExecute();
            break;
        case NGameOff::DynamicPlay:
            dynamicExecute();
            break;
        case NGameOff::FirstPlay:
            firstExecute();
            break;
        case NGameOff::FastPlay:
            fastExecute();
            break;
        default:
            break;
    }
}

void CPlayOff::staticExecute() {

    if (masterPlan != nullptr) {
        if (initial) {
            lastBallPos = wm->ball->pos;
            lastTime = ros::Time::now().sec;
            assignTasks(masterPlan);
            ROS_INFO_STREAM("task assigned: " << agents.size());
        } else {
            if (gameState->canKickBall()) {
                fillRoleProperties();
                posExecute();
                checkEndState();
                ROS_INFO_STREAM("IDD : " << 1 << ", ST : " << positionAgent[1].stateNumber);

                if (masterPlan->common.currentSize > 1 && havePassInPlan) {
                    passManager();
                }

                if (isPlanEnd()) {
                    playOnFlag = true;
                }
            }
        }

    } else {
        ROS_ERROR("master is null");
        initial = true;
    }

}

void CPlayOff::kickoffPositioning(int playersNum) {
    if (gameState->ourKickoff()){
        kickOffStopModePlay(playersNum);
        for (int i = 0; i < playersNum; i++) {
            newRoleAgent[i]->execute();
        }
    }
}

void CPlayOff::dynamicExecute() {

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
        roleAgent[i]->execute();
    }
}


void CPlayOff::dynamicAssignID() {
    lastTime = ros::Time::now().sec;
    lastBallPos = wm->ball->pos;
    initial = false;

    dynamicAgentSize = _NUM_PLAYERS;
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        if (dynamicMatch[i] != -1) {
            roleAgent[i] -> setAgent(agents[dynamicMatch[i]]);
            roleAgent[i] -> setAgentID(dynamicMatch[i]);
        } else {
            dynamicAgentSize = i;
            break;
        }
    }
}

void CPlayOff::dynamicPlayChipToGoal(bool isChip) {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(isChip);
        roleAgent[0] -> setKickSpeed(6.5); // TODO: Use Global Constants
        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setLookForward(false);
        roleAgent[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {
        roleAgent[0] -> setDoPass(true);
        shot = false;
    }
}

void CPlayOff::dynamicPlayBlocker() {
    if (initial) {
        dynamicAssignID();
        ready = true;
    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(false);
        roleAgent[0] -> setKickSpeed(1023);//knowledge->getProfile(roleAgent[0]->getAgentID(), 7.8, false, false)); // Vartypes This TODO
        roleAgent[0] -> setTarget(wm->field->oppGoal().rotatedVector((wm->ball->pos.y < 0 ? 90 : -90)));
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setLookForward(false);
        roleAgent[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {

        roleAgent[0] -> setKickSpeed(7);//knowledge->getProfile(roleAgent[0]->getAgentID(), 7.8, false, false)); // Vartypes This TODO
        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(true);
        roleAgent[0] -> setTargetDir(wm->field->oppGoal());
        roleAgent[0] -> setSelectedSkill(RoleSkill::Kick);
        shot = false;
    }

}

void CPlayOff::dynamicPlayKhafan() {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(true);
        roleAgent[0] -> setKickSpeed(6.5); // Vartypes This
        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setTargetDir(wm->field->oppGoal());
        roleAgent[0] -> setSelectedSkill(RoleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(RoleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (pass) {
        roleAgent[0] -> setDoPass(true);
        pass = false;
        DBUG("DYNAMIC :D ", D_MAHI);

    } else if (shot) {
        roleAgent[1] -> setAvoidCenterCircle(false);
        roleAgent[1] -> setAvoidPenaltyArea(true);
        roleAgent[1] -> setChip(false);
        roleAgent[1] -> setKickSpeed(1023); // Vartypes This
        roleAgent[1] -> setTarget(wm->field->oppGoal());
        roleAgent[1] -> setDoPass(true);
        roleAgent[1] -> setIntercept(false);
        roleAgent[1] -> setTargetDir(wm->field->oppGoal());
        roleAgent[1] -> setSelectedSkill(RoleSkill::Kick);
        shot = false;

        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setAvoidBall(true);
        roleAgent[0] -> setTimeBased(false);
        roleAgent[0] -> setTarget(Vector2D(0, -2));
        roleAgent[0] -> setTargetDir(wm->field->oppGoal() - roleAgent[0]->getAgent()->pos());
        roleAgent[0] -> setEventDist(0.3);
        roleAgent[0] -> setSlow(false);
        roleAgent[0] -> setSelectedSkill(RoleSkill::GotopointAvoid);


    }

}


void CPlayOff::checkEndKhafan() {
    ROS_INFO_STREAM("TIMENS: "<< ros::Time::now().sec << " TIMES: "<< ros::Time::now().sec);
    if (ready) {
        dynamicState = 2;
    } else if (pass) {
        dynamicState = 4;
    } else if (shot) {
        dynamicState = 6;
    }

    if (dynamicState == 2) {
        if (roleAgent[1] -> getAgent() -> pos().dist(roleAgent[1] -> getTarget())
            < roleAgent[1] -> getEventDist()) {
            dynamicState = 4;
            pass = true;
        }
    }

    if (dynamicState == 4) {

        DBUG(QString("ENDKHAFAN : %1").arg(ros::Time::now().sec - dynamicStartTime), D_MAHI);
        if (wm->ball->pos.dist(wm->field->oppGoal()) - 0.5 < roleAgent[1]->getAgent()->pos().dist(wm->field->oppGoal())) {
            pass = false;
            shot = true;
            dynamicState = 6;
        }
        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos) && dynamicStartTime == -1) {
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

void CPlayOff::checkEndBlocker() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            if (Circle2D(roleAgent[0] -> getAgent() -> pos() + roleAgent[0]->getAgent()->dir().norm() * 0.6, 0.3).contains(wm->opp.active(i)->pos))
                if (roleAgent[0]->getAgent()->dir().norm().dist(roleAgent[0]->getTarget().norm()) < 0.1) {
                    dynamicState = 6;
                    shot = true;
                }
        }

        dynamicStartTime = ros::Time::now().sec;

    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (ros::Time::now().sec - dynamicStartTime > 3 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

void CPlayOff::checkEndChipToGoal() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        if (Circle2D(wm->ball->pos, 0.5).contains(roleAgent[0]->getAgent()->pos())) {
            shot = true;
            dynamicState = 6;
            dynamicStartTime = ros::Time::now().sec;
        }
    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (ros::Time::now().sec - dynamicStartTime > 2 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

Vector2D CPlayOff::getDynamicTarget(int i) {
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

bool CPlayOff::isFirstFinished() {
    return (firstStepEnums == Done);
}

void CPlayOff::resetFirstPlayFinishedFlag() {
    firstStepEnums = Stay;
    blockerStep = S0;
}

int CPlayOff::getShotSpot() {
    return shotSpot;
}
void CPlayOff::firstDegree() {

    if (conf.UseBlockBlocker) {
        newRoleAgent[0]->setTargetDir(wm->field->oppGoal() - wm->ball->pos);
        newRoleAgent[0]->setTarget(wm->ball->pos - newRoleAgent[0]->getTargetDir().norm() * 0.3);
        blockersPenaltyArea.clear();

        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            if (Triangle2D(newRoleAgent[0]->getAgent()->pos()
                    , wm->field->oppGoal()
                    , wm->field->oppGoal() / 2).contains(wm->opp.active(i)->pos)
                && Line2D(newRoleAgent[0]->getAgent()->pos(), newRoleAgent[0]->getAgent()->pos() + newRoleAgent[0]->getAgent()->dir() * 5).dist(wm->opp.active(i)->pos) < 0.3
                && wm->opp.active(i)->pos.dist(newRoleAgent[0]->getAgent()->pos()) < 2) {
                if (!blockersPenaltyArea.contains(i)) {
                    DBUG(QString("penaltyAreaID:%1").arg(i), D_NADIA);
                    blockersPenaltyArea.append(i);
                }
            }
        }
    } else {
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(-0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->oppGoal());
    }
}

void CPlayOff::secondDegree() {

    if (conf.UseBlockBlocker) {

        newRoleAgent[0]->setTargetDir(wm->field->oppGoal() / 2 - wm->ball->pos);
        newRoleAgent[0]->setTarget(wm->ball->pos - newRoleAgent[0]->getTargetDir().norm() * 0.3);

        blockersCentralRegion.clear();
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            if (Triangle2D(newRoleAgent[0]->getAgent()->pos()
                    , wm->field->oppGoal() / 2
                    , Vector2D(0, 0)).contains(wm->opp.active(i)->pos)
                && Line2D(newRoleAgent[0]->getAgent()->pos()
                    , newRoleAgent[0]->getAgent()->pos() + newRoleAgent[0]->getAgent()->dir() * 5).dist(wm->opp.active(i)->pos) < 0.3) {
                if (!blockersCentralRegion.contains(i)) {
                    DBUG(QString("centralID:%1").arg(i), D_NADIA);
                    blockersCentralRegion.append(i);
                }
            }
        }
    }

    else {
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(-0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->oppGoal());
    }

}

void CPlayOff::thirdDegree() {


    if (conf.UseBlockBlocker) {

        newRoleAgent[0]->setTargetDir(wm->field->ourGoal() - wm->ball->pos);
        newRoleAgent[0]->setTarget(wm->ball->pos - newRoleAgent[0]->getTargetDir().norm() * 0.3);

        blockersRoundRegion.clear();
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
            if (Triangle2D(newRoleAgent[0]->getAgent()->pos()
                    , Vector2D(0, 0)
                    , wm->field->ourGoal()).contains(wm->opp.active(i)->pos)
                && Line2D(newRoleAgent[0]->getAgent()->pos(), newRoleAgent[0]->getAgent()->pos() + newRoleAgent[0]->getAgent()->dir() * 5).dist(wm->opp.active(i)->pos) < 0.3) {
                if (!blockersRoundRegion.contains(i)) {
                    DBUG(QString("RoundId:%1").arg(i), D_NADIA);
                    blockersRoundRegion.append(i);
                }
            }
        }
    } else {
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(-0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->oppGoal());
    }





    blockerState = 0;

    DBUG(QString("blocker state1:%1").arg(blockerState), D_NADIA);
    for (int i : blockersPenaltyArea) {
        DBUG(QString("penaltyArea:%1").arg(blockersPenaltyArea.at(i)), D_NADIA);
        //        if(blockersCentralRegion.contains(blockersPenaltyArea.at(i))){
        blockerState += penaltyAreaBlock;
        //        }
        if (blockersCentralRegion.contains(i)) {
            blockerID = i;
        }
    }

    for (int i : blockersCentralRegion) {
        DBUG(QString("central:%1").arg(blockersCentralRegion.at(i)), D_NADIA);
        //        if(blockersRoundRegion.contains(blockersCentralRegion.at(i)) && i==blockerID)
        if (i == blockerID) {
            blockerState += centralRegionBlock;
        }
    }


    for (int i : blockersRoundRegion) {
        DBUG(QString("Rounds:%1").arg(blockersRoundRegion.at(i)), D_NADIA);
        if (i == blockerID) {
            blockerState += RoundRegionBlock;
        }
    }


    DBUG(QString("blocker state2:%1").arg(blockerState), D_NADIA);

}

void CPlayOff::doneDegree() {

    if (conf.UseBlockBlocker) {
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->ourGoal() - wm->ball->pos);

    } else {
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(-0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->oppGoal());
    }
}

void CPlayOff::stayPoistioning() {

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

void CPlayOff::movePositioning() {
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

void CPlayOff::donePositioning() {


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



void CPlayOff::fastExecute() {
    // TODO : Write fast Execution (playoff)

}

void CPlayOff::firstExecute() {
    // TODO : Write first Execution (playoff)
    if (initial) {
        //        firstStepEnums = Stay;
        //        dynamicAssignIDNEW();
    }

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

void CPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < tAgentsize; i++) {
        if (!newRoleAgent[i]->isRoleUpdated()) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(getAgent(0));
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

void CPlayOff::firstPlayForOppCorner(int _agentSize) {

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
            stayPoistioning();
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

    switch (blockerStep) {
        case S0:
            firstDegree();
            break;
        case S1:
            secondDegree();
            break;
        case S2:
            thirdDegree();
            break;
        case S3:
            doneDegree();
            break;
    }
    int finisher = 0;
    for (int i = 0; i < _agentSize; i++) {
        if (newRoleAgent[i]->getTarget().dist(newRoleAgent[i]->getAgent()->pos()) < 0.4) {
            finisher++;
        }
    }
    if (!conf.UseBlockBlocker) {
        blockerStep = S3;
    }

    if (finisher == _agentSize - 1) {
        firstStepEnums = Done;
    } else if (newRoleAgent[0]->getTarget().dist(newRoleAgent[0]->getAgent()->pos()) < 0.1
               && newRoleAgent[0]->getTargetDir().angleWith(newRoleAgent[0]->getAgent()->dir()).degree() < 5) {
        if (blockerStep == S0) {
            blockerStep = S1;
        } else if (blockerStep == S1) {
            blockerStep = S2;
        } else if (blockerStep == S2) {
            blockerStep = S3;
        }
    }


}

bool CPlayOff::isPlanEnd() {
    return isPlanDone() || isPlanFailed();
}

bool CPlayOff::isPlanDone() {

    return isFinalShotDone();
}


bool CPlayOff::isPlanFailed() {
    return isTimeOver() || firstKickFailed() || isBallDirChanged();
}

bool CPlayOff::isTimeOver() {

    if (setTimer) startTime = ros::Time::now().sec;

    if (!Circle2D(lastBallPos, 0.06).contains(wm->ball->pos)) {
        setTimer = false;
        ROS_INFO_STREAM("MAHI: Time That Left: " << ros::Time::now().sec - startTime);
        if(ros::Time::now().sec - startTime >= 3*masterPlan->execution.passCount) { // 3 Second TODO: ADD TO CFG
            setTimer = true;
            return true;
        }
    }
    return false;
}

bool CPlayOff::isBallDirChanged() {
    // TODO: Need Test And Refine
    if (masterPlan->execution.passCount != 1) {
        return false;
    }

    //// USE PASSER FORM INITIAL LEVEL
    const int& passer = masterPlan->execution.passer.at(0).id;
    const int& receiver = masterPlan->execution.receiver.at(0).id;
    const int receiverID = masterPlan->common.matchedID.value(receiver);
    if (wm->ball->pos.dist(lastBallPos) > 0.5 && !roleAgent[passer]->getChip()) {
        Circle2D  c(roleAgent[receiverID]->getWaitPos(), 1); // TODO : CHECK radius
        drawer->draw(wm->ball->seg(), QColor(Qt::blue));
        drawer->draw(c, QColor(Qt::red));
        return !c.intersection(wm->ball->seg());
    }
    return false;

}

bool CPlayOff::isFinalShotDone() {

    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan hasn't a final shoot
    if (tLastState == -1 || tLastAgent == -1) {
        return false;
    }

    Agent* tAgent = getAgent(tLastAgent); // TODO: FIX IT NOW

    Circle2D cir(tAgent->pos() + tAgent->dir().norm() * 0.08, 0.16);
    Circle2D cir2(tAgent->pos() + tAgent->dir().norm() * 0.20, 0.40);

    drawer->draw(cir , QColor(Qt::blue));
    drawer->draw(cir2, QColor(Qt::blue));

    if (positionAgent[tLastAgent].stateNumber == tLastState) {
        if (cir.contains(wm->ball->pos)) {
            isBallIn = true;

        } else if (isBallIn && !cir2.contains(wm->ball->pos)) {
            isBallIn = false;
            return true;

        }
    }

    return false;
}

Vector2D CPlayOff::getEmptyTarget(const Vector2D& _position, const double& _radius) {
    Vector2D tempTarget, finalTarget;
    bool opp;
    finalTarget = _position;
    for (double dist = 0.2 ; dist <= _radius ; dist += 0.2) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 18.0/dist) {
            opp = false;
            tempTarget = _position + Vector2D::polar2vector(dist, ang);
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.25).contains(tempTarget)
                    || !wm->field->isInField(tempTarget)
                    || wm->field->isInOppPenaltyArea(tempTarget)
                    || wm->field->isInOurPenaltyArea(tempTarget)) {
                    opp = true;
                    break;
                }

            }
            if (!opp) {
                finalTarget = tempTarget;
                dist = _radius*2; // to break upper loop
                break;
            }
        }
    }

    return finalTarget;
}
///////////////PassManager///////////////////
void CPlayOff::passManager() {
    const AgentPoint &r = masterPlan->execution.receiver.at(0);
    const AgentPoint &p = masterPlan->execution.passer.at(0);

    const int &i = masterPlan->common.matchedID.value(r.id);

    Agent *c = getAgent(r.id); //// Receiver
    if (positionAgent[r.id].stateNumber == r.state
        || positionAgent[r.id].stateNumber == r.state + 1) {
        DBUG(QString("RC : %1, %2").arg(r.id).arg(r.state), D_MAHI);
        drawer->draw(Circle2D(positionAgent[r.id].getAbsArgs(r.state).staticPos, masterPlan->common.lastDist),
                     QColor(Qt::darkMagenta));
        doPass = positionAgent[r.id].getAbsArgs(r.state).staticPos.dist(c->pos())
                 <= masterPlan->common.lastDist;
        doAfterlife = !Circle2D(lastBallPos, 0.1).contains(wm->ball->pos);
        roleAgent[p.id]->setDoPass(doPass);
    }

}

/**
 * @brief CPlayOff::isTaskDone
 * @param _roleAgent
 * @return true if the task get done
 */
bool CPlayOff::isTaskDone(CRolePlayOff* _roleAgent) {

    switch (_roleAgent->getSelectedSkill()) {
        case RoleSkill::Gotopoint:
        case RoleSkill::GotopointAvoid:
            return isMoveDone(_roleAgent);
            break;
        case RoleSkill::Kick:
            return isKickDone(_roleAgent);
            break;
        case RoleSkill::OneTouch:
            return isOneTouchDone(_roleAgent);
            break;
        case RoleSkill::ReceivePass:
            return isReceiveDone(_roleAgent);
            break;
            //  Life
        case RoleSkill::Mark:
        case RoleSkill::Support:
        case RoleSkill::Defense:
            qDebug() << "got it";
            _roleAgent->setRoleUpdate(false);
            return false;
            break;
    }
}

void CPlayOff::posExecute() {
    for (int i = 0; i < masterPlan->common.currentSize; i++) {
        if (roleAgent[i]->getAgent() != nullptr) {
            roleAgent[i]->execute();
        }
    }
}

void CPlayOff::checkEndState() {

    for (int i = 0; i < masterPlan->common.currentSize; i++) {
        if (roleAgent[i]->getAgent() == nullptr) {
            continue;
        }

        Agent *firstPasser = getAgent(masterPlan->execution.passer.at(0).id);

        if (isTaskDone(roleAgent[i]) || (doAfterlife && roleAgent[i]->getAgent()->id() != firstPasser->id())) {

            roleAgent[i]->setRoleUpdate(false);
            roleAgent[i]->resetTime();

            POFFSKILL last_skill = positionAgent[i].positionArg.at(
                    positionAgent[i].positionArg.size() - 1).staticSkill;

            if (last_skill == POFFSKILL::Position) {
                if (!doAfterlife && positionAgent[i].stateNumber == positionAgent[i].positionArg.size() - 2) {
                    roleAgent[i]->setRoleUpdate(true);
                } else if (doAfterlife){
                    positionAgent[i].stateNumber = positionAgent[i].positionArg.size() - 1;
                }
            } else if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                if (positionAgent[i].getArgs(1).staticSkill    == POFFSKILL::Defense
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Support
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Position
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Goalie
                    || positionAgent[i].getArgs(1).staticSkill == POFFSKILL::Mark) {

                    if (doAfterlife) {
                        positionAgent[i].stateNumber++;
                    }
                    continue;
                }
                positionAgent[i].stateNumber++;
                DBUG(QString("IDDD : %1, ST : %2").arg(i).arg(positionAgent[i].stateNumber), D_MAHI);
            } else {
                //                positionAgent[i].zombie = true;
                ///Temp
                //                SPositioningArg tempPA;
                //                tempPA = positionAgent[i].getAbsArgs(positionAgent[i].positionArg.size() - 1);
                //                tempPA.staticSkill = MoveSkill;
                //                positionAgent[i].positionArg.append(tempPA);
            }
        }
    }
}

void CPlayOff::fillRoleProperties() {
    for (int i = 0; i < agents.size(); i++) {
        if (masterPlan->common.matchedID.contains(i) && !roleAgent[i]->isRoleUpdated()) {

            roleAgent[i]->setFirstMove(positionAgent[i].stateNumber == 0);
            roleAgent[i]->setAgent(agents[i]);

            //// Handle OneTouch Faster
            if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                if (positionAgent[i].getArgs().staticSkill  == POFFSKILL::Move &&
                    positionAgent[i].getArgs(1).staticSkill == POFFSKILL::OneTouch) {
                    positionAgent[i].stateNumber++;
                }
            }

            //// Change Receive and Pass to OneTouch (2nd Pass)
            for (auto x : masterPlan->execution.passer) {
                if (x.id == i) {
                    if (i > 0 && masterPlan->execution.receiver.at(i - 1).id == x.id) {
                        if (positionAgent[i].getArgs().staticSkill == POFFSKILL::ReceivePass) {
                            int index = -1;
                            for (int j = 1;
                                 j < positionAgent[i].positionArg.size() - positionAgent[i].stateNumber; j++) {
                                if (positionAgent[i].getArgs(j).staticSkill == POFFSKILL::Pass) {
                                    index = j;
                                    break;
                                }
                            }
                            if (index != -1) {
                                Vector2D pass_target =
                                        positionAgent[positionAgent[i].getArgs(index).PassToId].getAbsArgs(
                                                positionAgent[i].getArgs(index).PassToState).staticPos;

                                Vector2D v1 = wm->ball->pos - positionAgent[i].getArgs(index).staticPos;
                                Vector2D v2 = pass_target - positionAgent[i].getArgs(index).staticPos;
                                double onetouchAngle = (v1.th() - v2.th()).degree();
                                ROS_INFO_STREAM("onetouchAngle " << onetouchAngle << " max OnetouchAngle"
                                                                 << conf.MaxOnetouchAngle);
                                if (onetouchAngle < conf.MaxOnetouchAngle) {
                                    positionAgent[i].stateNumber++;
                                    firstPass = false;
                                }
                            }
                        }
                    }
                }
            }

            roleAgent[i]->setRoleUpdate(true);
            roleAgent[i]->resetTime();
            assignTask(roleAgent[i], positionAgent[i]);
            ROS_INFO_STREAM("NEW TASK: " << roleAgent[i]->getTarget().x << roleAgent[i]->getTarget().y);

        } else {
            qWarning() << "[Warning] coach -> Match function doesn't work :( ";
            if (!roleAgent[i]->isRoleUpdated()) {
                roleAgent[i]->setAgent(agents.at(i));
                assignTask(roleAgent[i], positionAgent[i]);
                roleAgent[i]->setRoleUpdate(true);
            }
        }
    }
}

void CPlayOff::assignTask(CRolePlayOff* _roleAgent, const SPositioningAgent& _positionAgent) {
    switch (_positionAgent.getArgs().staticSkill) {
        case POFFSKILL::Pass:
            assignPass(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::ReceivePass:
            assignReceive(_roleAgent, _positionAgent, false);
            break;
        case POFFSKILL::ReceivePassIA:
            assignReceive(_roleAgent, _positionAgent, true);
            break;
        case POFFSKILL::ShotToGoal:
            assignKick(_roleAgent, _positionAgent, false);
            break;
        case POFFSKILL::ChipToGoal:
            assignKick(_roleAgent, _positionAgent, true);
            break;
        case POFFSKILL::OneTouch:
            assignOneTouch(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Move:
            assignMove(_roleAgent, _positionAgent);
            break;
            // After Life
        case POFFSKILL::Defense:
            _roleAgent->setRoleUpdate(false);
            assignDefense(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Support:
            _roleAgent->setRoleUpdate(false);
            assignSupport(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Position:
            _roleAgent->setRoleUpdate(false);
            assignPosition(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Goalie:
            _roleAgent->setRoleUpdate(false);
            assignGoalie(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::Mark:
            _roleAgent->setRoleUpdate(false);
            assignMark(_roleAgent, _positionAgent);
            break;
        case POFFSKILL::None:
            break;
    }
}

void CPlayOff::assignPass(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent->setAvoidCenterCircle(false);
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setChip(chipOrNot(_posAgent.getArgs()));
    if (_roleAgent->getChip()) _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().rightData)/100.0);
    else _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100.0);
    _roleAgent->setTarget(positionAgent[_posAgent.getArgs().PassToId].getAbsArgs(_posAgent.getArgs().PassToState).staticPos);
    _roleAgent->setDoPass(doPass);
    _roleAgent->setIntercept(false);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng);
    _roleAgent->setSelectedSkill((firstPass) ? RoleSkill::Kick : RoleSkill::OneTouch);

}

void CPlayOff::assignReceive(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _ignoreAngle) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setIgnoreAngle(_ignoreAngle);
    _roleAgent->setTarget(_posAgent.getArgs().staticPos);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng); /** Just Matter when we use Ignore mode **/
    _roleAgent->setReceiveRadius(_posAgent.getArgs().leftData / 100);
    _roleAgent->setSelectedSkill(RoleSkill::ReceivePass);
}

void CPlayOff::assignKick(CRolePlayOff* _roleAgent,
                          const SPositioningAgent& _posAgent, bool _chip) {

    _roleAgent->setChip(_chip);
    _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData) / 100.0);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setIntercept(false);
    _roleAgent->setSelectedSkill(RoleSkill::Kick);
}

void CPlayOff::assignOneTouch(CRolePlayOff* _roleAgent,
                              const SPositioningAgent& _posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setWaitPos(_posAgent.getArgs().staticPos);
    _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100.0);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setSelectedSkill(RoleSkill::OneTouch);
}

void CPlayOff::assignMove(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent -> setAvoidPenaltyArea(true);
    _roleAgent -> setTime(_posAgent.getArgs().rightData); //// ignore duration -> time is wait
    _roleAgent -> setAvoidBall(true);
    _roleAgent -> setTimeBased(_roleAgent->getTime() != 0);
    _roleAgent -> setEventDist(_posAgent.getArgs().staticEscapeRadius);
    _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
    _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
    _roleAgent -> setSlow(false);
    _roleAgent -> setMaxVelocity(getMaxVel(_roleAgent, _posAgent.getArgs()));
    _roleAgent -> setSelectedSkill(RoleSkill::GotopointAvoid);

    if (_posAgent.getArgs().staticPos == BEHIND_BALL_POS) { //// First Passer
        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(wm->ball->pos - Vector2D(0.20, 0));
        _roleAgent -> setTargetDir(wm->ball->pos - _roleAgent->getAgent()->pos());
        _roleAgent -> setSlow(true);
        _roleAgent -> setMaxVelocity(1);
    }
}

void CPlayOff::assignGoalie(CRolePlayOff * _roleAgent, const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setTarget(wm->field->ourGoal() + Vector2D(0, 1));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CPlayOff::assignMark(CRolePlayOff * _roleAgent, const SPositioningAgent &_posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    _roleAgent->setTarget(CDefPos::getStaticDefPositions(wm->ball->pos, 2, 2, 3).pos[1]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CPlayOff::assignPosition(CRolePlayOff * _roleAgent,
                              const SPositioningAgent &_posAgent) {
    assignMove(_roleAgent, _posAgent); // TODO : check this
}

void CPlayOff::assignSupport(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    Vector2D supportPosition = (lastBallPos.dist(wm->ball->pos) < 0.1)
                          ? _roleAgent->getAgent()->pos()
                          : (wm->ball->pos + wm->ball->vel * 0.5) - Vector2D(1, 0);

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(_roleAgent->getAgent()->pos() - wm->ball->pos);
    _roleAgent->setTarget(getEmptyTarget(supportPosition, .5));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}

void CPlayOff::assignDefense(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    _roleAgent->setTarget(CDefPos::getStaticDefPositions(wm->ball->pos, 2, 2, 3).pos[0]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}

Vector2D CPlayOff::getMoveTarget(const SPositioningArg& _posArg) {
    return getEmptyTarget(_posArg.staticPos, _posArg.staticEscapeRadius);
}

double CPlayOff::getMaxVel(const CRolePlayOff* _roleAgent,
                           const SPositioningArg& _posArg) {
    Vector2D tAgentPos =  _roleAgent->getAgent()->pos();
    double dist = tAgentPos.dist(_posArg.staticPos);
    double vel = std::min(std::max(dist / _posArg.leftData, 1.5), 4.0);
    return vel;
}

Vector2D CPlayOff::getGoalTarget(long _y) {
    _y = std::min(std::max(_y, 0L), 1000L);
    double tempYPos = (double)(_y) / 1000.0 + wm->field->oppGoalR().y;
    return Vector2D{wm->field->oppGoal().x, tempYPos};
}

bool CPlayOff::chipOrNot(const SPositioningArg& _posArg) {
    if (_posArg.leftData < 0) return true;
    else if (_posArg.rightData < 0) return false;
    else {
        const int& id = _posArg.PassToId;
        const int& ps = _posArg.PassToState;
        return !isPathClear(wm->ball->pos,
                            positionAgent[id].getAbsArgs(ps).staticPos,
                            0.5,   // Radius
                            0.1);  // Threshold
    }
}

bool CPlayOff::isPathClear(Vector2D _pos1,
                           Vector2D _pos2,
                           double _radius,
                           double threshold) {

    Polygon2D poly = getPathPolygon(_pos1, _pos2, _radius, threshold);
    drawer->draw(poly, "red");

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (poly.contains(wm->opp.active(i)->pos)) return false;
    }

    return true;
}

void CPlayOff::assignTasks(const SPlan* _plan) {
    const int &sym = _plan->execution.symmetry;
    for (size_t i = 0; i < _plan->common.currentSize; i++) {

        positionAgent[i].positionArg.clear();

        Q_FOREACH (playOffRobot agentPlan, _plan->execution.AgentPlan[i]) {
            SPositioningArg tempPosArg;
            tempPosArg.staticPos = agentPlan.pos;
            tempPosArg.staticAng = Vector2D::polar2vector(1, agentPlan.angle);
            tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -1 * sym * tempPosArg.staticAng.y);
            tempPosArg.staticPos.assign(tempPosArg.staticPos.x, sym * tempPosArg.staticPos.y);
            tempPosArg.staticEscapeRadius = agentPlan.tolerance;

            Q_FOREACH (playOffSkill skill, agentPlan.skill) {
                tempPosArg.leftData    = skill.data[0];
                tempPosArg.rightData   = skill.data[1];
                tempPosArg.staticSkill = skill.name;
                tempPosArg.PassToId    = skill.targetAgent;
                tempPosArg.PassToState = skill.targetIndex;

                if (skill.name == POFFSKILL::Pass && positionAgent[i].positionArg.back().staticSkill == POFFSKILL::Move) {
                    positionAgent[i].positionArg.back().staticPos = BEHIND_BALL_POS;
                } else if (      skill.name == POFFSKILL::ShotToGoal
                                 || skill.name == POFFSKILL::ChipToGoal
                                 || skill.name == POFFSKILL::OneTouch) {

                    if (sym < 0) tempPosArg.rightData = 1000 - tempPosArg.rightData;

                }
                positionAgent[i].positionArg.append(tempPosArg);
            }
        }
    }
}

int CPlayOff::findReceiver(int _passer, int _state) {
    if (_state == 0) {
        return 0;
    }
    for (int i = _state; i < positionAgent[_passer].positionArg.size(); i++) {
        if (positionAgent[_passer].getArgs(i).staticSkill == POFFSKILL::Pass) {
            SBallOwner temp{};
            temp.id = _passer;
            temp.state = i;
            ownerList.append(temp);
            findReceiver(positionAgent[_passer].getArgs(i).PassToId,
                         positionAgent[_passer].getArgs(i).PassToState);
        }
    }
}

void CPlayOff::reset() {


    qDebug() << "Bring yourself back online playoff";

    blockerState = 0;
    blockerID = -1;

    for (int i = 0; i < _NUM_PLAYERS; i++) {
        positionAgent[i].stateNumber = 0;
        roleAgent[i]->reset();
        newRoleAgent[i]->reset();
        positionAgent[i].zombie = false;
    }
    isBallIn = false;
    tempAgent = new CRolePlayOff();
    doAfterlife = false;
    doPass = false;
    criticalInit = true;

    setTimer = true;
    ////////////

    masterPlan = nullptr;

    initial    = true;
    playOnFlag = false;
    havePassInPlan = false;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = 0;

    executedCycles = 0;
    activeAgents.clear();
    markAgents.clear();

    firstStepEnums = Stay;
    blockerStep = S0;

    firstPass = true;

    DBUG(QString("reset Plan"), D_MAHI);
    ROS_INFO("reset Plan");
}

void CPlayOff::init(const QList<Agent*>& _agents) {
    setAgentsID(_agents);
    initMaster();

}

void CPlayOff::execute_x() {
    globalExecute();
}

////////////////////////////////

void CPlayOff::setMasterPlan(SPlan *_thePlan) {
    masterPlan = _thePlan;
}

void CPlayOff::setMasterMode(EMode _mode) {
    masterMode = _mode;
}

EMode CPlayOff::getMasterMode() {
    return masterMode;
}
///////////////////////////////////////
/////////////Check Execution///////////

bool CPlayOff::firstKickFailed() {
    return (lastBallPos.dist(wm->ball->pos) > 0.25 && wm->ball->vel.length() < 0.1);
}

/*!
*   \brief check if ball get distance from robot,
*
*          in case that it's direct shoot
*          also check that ball is in right direction or not.
*
*
*/
bool CPlayOff::isKickDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.2).contains(wm->ball->pos)) {
        _roleAgent->setBallIsNear(true);
        return false;
    } else if (!Circle2D(lastBallPos, 0.6).contains(wm->ball->pos) && _roleAgent->getBallIsNear()) {
        _roleAgent->setBallIsNear(false);
        return true;
    }

}


bool CPlayOff::isReceiveDone(const CRolePlayOff * _roleAgent) {
    return Circle2D(_roleAgent->getAgent()->pos(), 0.3).contains(wm->ball->pos) && wm->ball->vel.length() < 0.5;
}

bool CPlayOff::isOneTouchDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.4).contains(wm->ball->pos)) {
        ROS_INFO("playoff one-touch BallIsNear");
        _roleAgent->setBallIsNear(true);
    } else if (!Circle2D(_roleAgent->getAgent()->pos(), 0.6).contains(wm->ball->pos)
               && _roleAgent->getBallIsNear()) {
        _roleAgent->setBallIsNear(false);

        if (_roleAgent->getChip()) {
            DBUG("[playoff] chip Done", D_MAHI);
            ROS_INFO("playofff chip one-touch is done");
            return true;
        } else {
            // check ball direction
            Vector2D sol1, sol2;
            if (wm->ball->vel.length() > 0.6 && Circle2D(_roleAgent->getTarget(), 0.75).intersection(
                    Ray2D(wm->ball->pos, wm->ball->pos + wm->ball->vel), &sol1, &sol2)) {
                DBUG("[playoff] direction is correct", D_MAHI);
                ROS_INFO("playofff one-touch is now done");
                return true;
            }

        }
    }

    return false;
}


bool CPlayOff::isMoveDone(const CRolePlayOff * _roleAgent) {

    if (_roleAgent->getFirstMove() && _roleAgent->getTarget() != BEHIND_BALL_POS) {
        return true ;
    }

    if (_roleAgent->getTimeBased()) {
        DBUG(QString("EL : %1").arg(_roleAgent->getElapsed()), D_HOSSEIN);
        DBUG(QString("GT : %1").arg(_roleAgent->getTime()), D_HOSSEIN);
        if (_roleAgent->getElapsed() > _roleAgent->getTime()) {
            DBUG("D------------------", D_HOSSEIN);
            return true;
        }
    } else {
        // TODO : vartypes this
        if (_roleAgent->getAgent()->pos().dist(_roleAgent->getTarget()) < max(0.3, _roleAgent->getEventDist() / 100)) {
            return true;
        }
    }
    return false;
}

void CPlayOff::setInitial(bool _init) {
    initial = _init;
}

QPair<int, int> CPlayOff::findTheLastShoot(const SExecution &_plan) {
    QPair<int, int> last;
    last.first = last.second = -1;

    QList<POFFSKILL> finalSkills;
    finalSkills.append(POFFSKILL::ShotToGoal);
    finalSkills.append(POFFSKILL::ChipToGoal);
    finalSkills.append(POFFSKILL::OneTouch);

    int counter = 0;
    Q_FOREACH (QList<playOffRobot> agent, _plan.AgentPlan) {
        int counter2 = 0;
        Q_FOREACH (playOffRobot node, agent) {
            Q_FOREACH (playOffSkill skill, node.skill) {
                if (finalSkills.contains(skill.name)) {
                    last.first  = counter;
                    last.second = counter2;
                }
                counter2++;
            }
        }
        counter++;
    }
    return last;
}

void CPlayOff::analyseShoot() {
    if (masterPlan != nullptr) {
        QPair<int, int> last;
        last = findTheLastShoot(masterPlan->execution);
        masterPlan->execution.theLastAgent = last.first;
        masterPlan->execution.theLastState = last.second;
        havePassInPlan = (last.first != -1  && last.second != -1);
    }
}

void CPlayOff::analysePass() {
    // TODO : need edit for mulitiple pass
    if (masterPlan != nullptr) {
        // first : passer second : receiver
        QList<AgentPair> tPass = findThePasserandReciver(masterPlan->execution);
        havePassInPlan = !tPass.empty();
        masterPlan->execution.passCount = tPass.size();
        if (havePassInPlan) {
            for (int i = 0; i < tPass.size(); i++) {
                if(i == 0) {
                    AgentPoint p;
                    p.id = masterPlan->common.matchedID.value(tPass.at(i).first.id);
                    p.state = tPass.at(i).first.state;
                    masterPlan->execution.passer.append(p);
                } else{
                    masterPlan->execution.passer.append(tPass.at(i).first);
                }

                masterPlan->execution.receiver.append(tPass.at(i).second);
            }
        }
    }

}

void CPlayOff::criticalPlay() {

    ROS_INFO("critical");
    if (criticalInit) {
        criticalInit = false;
        criticalKick->setTarget(wm->field->oppGoal());
        criticalKick->setChip(wm->ball->pos.x < 1);
        criticalKick->setDontkick(false);
        criticalKick->setPassprofiler(false);
        criticalKick->setKickspeed(6.5);
        criticalKick->setChipdist(4);
        criticalKick->setTolerance(0.5);
    }

    double minDist = 100;
    Agent* nearest = nullptr;
    Q_FOREACH(auto& agent, agents) {
        if(agent->pos().dist(wm->ball->pos) < minDist) {
            nearest = agent;
            minDist = agent->pos().dist(wm->ball->pos);
        }
    }

    if(nearest != nullptr){
        nearest->action = criticalKick;
    } else {
        ROS_ERROR("THERE'S NO AGENT FOR CRITICAL PLAY");
    }
}

QList<AgentPair> CPlayOff::findThePasserandReciver(const NGameOff::SExecution & _plan) {

    QList<AgentPoint> passer;
    for (int i = 0; i < _plan.AgentPlan.size(); i++) {
        const QList<playOffRobot> & agent = _plan.AgentPlan.at(i);
        for (int j = 0; j < agent.size(); j++) {
            const playOffRobot& node = agent.at(j);
            for (const auto &k : node.skill) {
                const POFFSKILL& skill = k.name;
                if (skill == POFFSKILL::Pass) {
                    passer.append(AgentPoint(i, j));
                }
            }

        }
    }

    ROS_INFO_STREAM("Pass count: " << passer.size());
    QList<AgentPair> pairList;
    for (auto i : passer) {
        const int &id = i.id;
        const int &st = i.state;

        int si = (_plan.AgentPlan[id][st].skill[1].name == POFFSKILL::Pass) ? 1 : 2;
        int rid = _plan.AgentPlan[id][st].skill[si].targetAgent;
        int rs  = _plan.AgentPlan[id][st].skill[si].targetIndex;
        DBUG(QString("PASS : %1, %2, %3, %4").arg(id).arg(st).arg(rid).arg(rs), D_MAHI);
        AgentPoint tempReciver;
        tempReciver.id    = rid;
        tempReciver.state = rs;

        AgentPair ap;
        ap.first  = i;
        ap.second = tempReciver;
        pairList.append(ap);
    }
    return pairList;

}

Polygon2D CPlayOff::getPathPolygon(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold) {
    Vector2D sol1, sol2, sol3, sol4;
    Line2D _path{_pos1, _pos2};
    Circle2D(_pos2, _radius + treshold).intersection(_path.perpendicular(_pos2), &sol1, &sol2);
    Circle2D(_pos1, Robot::robot_radius_new + treshold).intersection(_path.perpendicular(_pos1), &sol3, &sol4);
    Polygon2D _poly;
    _poly.addVertex(sol1);
    _poly.addVertex(sol2);
    _poly.addVertex(sol4);
    _poly.addVertex(sol3);
    _poly.addVertex(sol1);
    return _poly;
}

int CPlayOff::getIndex(int _planID) {
    return masterPlan->matching.common->matchedID.value(_planID);
}

Agent *CPlayOff::getAgent(int _planID) {
    return agents[getIndex(_planID)];
}
