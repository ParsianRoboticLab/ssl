#include <parsian_ai/soccer.h>
#include "parsian_ai/plays/playoff.h"


CPlayOff::CPlayOff() : CMasterPlay() {
    blockerState = 0;
    blockerID = -1;
    blockerStopStates = Diversion;
    ROS_INFO("Bring yourself back online playoff");

    decidePlan = true;
    agentSize = 1;
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

    currentPlan = new SPlayOffPlan();
    masterPlan = nullptr;
    kickOffPos[0] = Vector2D(wm->ball->pos.x - 0.36, wm->ball->pos.y);
    kickOffPos[1] = Vector2D(-0.36,  3);
    kickOffPos[2] = Vector2D(-0.36, -3);
    kickOffPos[3] = Vector2D(-2.4  ,  0);
    kickOffPos[4] = Vector2D(-0.9,  1.2);
    kickOffPos[5] = Vector2D(-0.9, -1.2);
    // TODO : fill kickoffpos for rest of robots if needed
    kickOffPos[6] = Vector2D(-0.75, 1.7);
    kickOffPos[7] = Vector2D(-0.75, -1.7);


    initial    = true;
    playOnFlag = false;
    firstPass  = true;
    isOnetouch = false;
    havePassInPlan    = false;
    playoff_badPasserID = -1;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = -1;
    dynamicSelect = NOSELECT;
    firstStepEnums = Stay;
    blockerStep = S0;

    criticalInit = true;
    criticalKick = new KickAction();

}

CPlayOff::~CPlayOff() {
    qDebug() << "Playoff is gone";
    for (int i = 0; i < _NUM_PLAYERS; i++) {
        delete roleAgent[i];
        delete newRoleAgent[i];
    }
    delete tempAgent;
}

bool CPlayOff:: isBlockDisturbing() {
    if (blockerState == 7) {
        return true;
    }
    return conf.UseForcedBlock;

}

void CPlayOff::globalExecute() {

    if (!initial && ros::Time::now().sec - lastTime > 10 && lastBallPos.dist(wm->ball->pos) <= 0.06) {
        qDebug()<< "nkmiujgt5fr4";
        if (criticalPlay()) {
            playOnFlag = true;
        }
        return;
    } else
        ROS_INFO_STREAM("llastTime: " <<ros::Time::now().sec - lastTime<<" masterMode: "<< masterMode <<" balldist: "<<lastBallPos.dist(wm->ball->pos) );

    if (masterMode == NGameOff::StaticPlay) {

        if (masterPlan != nullptr) {
            DBUG(QString("Plan Number : %1 ==> ").arg(masterPlan->gui.planFile), D_MAHI);

            if (initial) {
                qDebug() << *masterPlan;
                lastBallPos = wm->ball->pos;
                lastTime = ros::Time::now().sec;

                if (isBlockDisturbing() && conf.UseBlockBlocker) {
                    masterPlan->common.currentSize -= 1;
                    BlockerStopperID = masterPlan->common.matchedID[masterPlan->common.currentSize];


                }
                staticExecute();

            }
            DBUG(QString("policy:%1").arg(conf.UseBlockBlocker), D_NADIA);
            if (isBlockDisturbing() && conf.UseBlockBlocker) {
                agents.removeOne(soccer->agents[BlockerStopperID]);
                if (BlockerExecute(BlockerStopperID)) {
                    staticExecute();
                }
            } else {
                staticExecute();

            }


        } else {
            qDebug() << "master is null";
            initial = true;
            return;
        }

    } else {
        mainExecute();
    }
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

bool CPlayOff::BlockerExecute(int agentID) {

    bool result = false;
    Vector2D pos1, pos2;
    Vector2D virtualPos;
    Segment2D blockerLine;
    Circle2D blockerCircle(wm->ball->pos, wm->opp[blockerID]->pos.dist(wm->ball->pos));
    int ID;

    switch (blockerStopStates) {
    case Diversion:
        if (!newRoleAgent[0]->getRoleUpdate()) {
            newRoleAgent[0]->setUpdated(true);
            newRoleAgent[0]->setAgent(soccer->agents[dynamicMatch[0]]);
            newRoleAgent[0]->setRoleUpdate(true);
            newRoleAgent[0]->setAvoidBall(true);
            newRoleAgent[0]->setAvoidPenaltyArea(true);
            newRoleAgent[0]->setSelectedSkill(RoleSkill::GotopointAvoid);
        }
        newRoleAgent[0]->setTarget(wm->ball->pos + Vector2D(0.3, 0));
        newRoleAgent[0]->setTargetDir(wm->field->ourGoal());
        if (newRoleAgent[0]->getTargetDir().angleWith(newRoleAgent[0]->getAgent()->dir()).degree() < 5) {
            blockerStopStates = BlockStop ;
        } else {
            newRoleAgent[0]->execute();
        }

        break;
    case TurnAndKick:

        ID = masterPlan->common.currentSize;
        newRoleAgent[ID]->setUpdated(true);
        newRoleAgent[ID]->setAgent(soccer->agents[BlockerStopperID]);
        newRoleAgent[ID]->setRoleUpdate(true);
        newRoleAgent[ID]->setAvoidBall(true);
        newRoleAgent[ID]->setAvoidPenaltyArea(true);
        newRoleAgent[ID]->setSelectedSkill(RoleSkill::GotopointAvoid);

        virtualPos = wm->opp[blockerID]->pos + Vector2D(0.15, 0);
        blockerLine = Segment2D(wm->ball->pos, wm->ball->pos + (virtualPos - wm->ball->pos).norm() * 3);
        blockerCircle.intersection(blockerLine, &pos1, &pos2);

        // Nadia
        newRoleAgent[ID]->setTarget(pos1);
        newRoleAgent[ID]->setTargetDir(wm->ball->pos - pos1);


        result = true;
        DBUG("plan ejra sho", D_NADIA);
        break;
    case BlockStop:
        ID = masterPlan->common.currentSize;
        newRoleAgent[ID]->setUpdated(true);
        newRoleAgent[ID]->setAgent(soccer->agents[BlockerStopperID]);
        newRoleAgent[ID]->setRoleUpdate(true);
        newRoleAgent[ID]->setAvoidBall(true);
        newRoleAgent[ID]->setAvoidPenaltyArea(true);
        newRoleAgent[ID]->setSelectedSkill(RoleSkill::GotopointAvoid);

        virtualPos = wm->opp[blockerID]->pos + Vector2D(0.15, 0);
        blockerLine = Segment2D(wm->ball->pos, wm->ball->pos + (virtualPos - wm->ball->pos).norm() * 3);
        blockerCircle.intersection(blockerLine, &pos1, &pos2);

        // Nadia
        newRoleAgent[ID]->setTarget(pos1);
        newRoleAgent[ID]->setTargetDir(wm->ball->pos - pos1);

        if (newRoleAgent[ID]->getTarget().dist(newRoleAgent[ID]->getAgent()->pos()) > 0.15) {
            newRoleAgent[ID]->execute();
        } else {
            blockerStopStates = TurnAndKick;
        }

        //        debug(QString("The ID is:%1").arg(newRoleAgent[4]->getAgent()->id()),D_NADIA);

        break;

    }

    return result;


}


void CPlayOff::staticExecute() {
    ROS_INFO("Static");
    if (initial) {
        ROS_INFO_STREAM("task assigned: " << agents.size());
        assignTasks();

    } else {

        if (gameState->canKickBall()) {

            fillRoleProperties();
            posExecute();
            checkEndState();
            ROS_INFO_STREAM("IDD : " << 1 << ", ST : " << positionAgent[1].stateNumber);

            if (masterPlan->common.currentSize > 1 && havePassInPlan) {
                passManager();
                ROS_INFO("Pass");

            }

            if (isPlanEnd()) {
                playOnFlag = true;
                ROS_INFO("Playoff Ends");
            }

        } else {
//            kickOffStopModePlay(masterPlan->common.currentSize);
//            for (int i = 0; i < masterPlan->common.currentSize; i++) {
//                newRoleAgent[i]->execute();
//            }

        }
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

    ROS_INFO_STREAM("dynamicSelect: "<<dynamicSelect);

    if (dynamicSelect == CHIP && false) {
        dynamicPlayChipToGoal(true);
        checkEndChipToGoal();
    } else if(dynamicSelect == KICK){
        dynamicPlayChipToGoal(false);
        checkEndChipToGoal();
    } else if (dynamicSelect == KHAFAN || 1) {
        dynamicPlayKhafan();
        checkEndKhafan();
    } else if (dynamicSelect == BLOCKER) {
        dynamicPlayBlocker();
        checkEndBlocker();
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
            roleAgent[i] -> setAgent(soccer->agents[dynamicMatch[i]]);
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
        double speed = 6.5;//knowledge->getProfile(roleAgent[0]->getAgentID(), roleAgent[0]->getAgent()->pos().dist(wm->field->oppGoal()), false, false); TODO
        roleAgent[0] -> setKickSpeed(speed); // Vartypes This
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

    int sign;
    sign = wm->ball->pos.y > 0 ? 1 : -1;

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

    int sign;
    sign = wm->ball->pos.y > 0 ? 1 : -1;

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
    for (int i = 0; i < oppMark.size(); i++) {
        sumDist += wm->opp[oppMark.at(i)]->pos.dist(wm->field->oppGoal());
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

void CPlayOff::mahiDebug(int limit) {
    for (size_t i = 0; i < limit; i++) {
        DBUG(QString("No.%1 : %2").arg(i).arg(debugs[i]), D_MAHI);
    }
}

void CPlayOff::mahiCircle(int limit) {
    for (size_t i = 0; i < limit; i++) {
        drawer->draw(circles[i], QColor(Qt::red));
    }
}

void CPlayOff::mahiVector(int limit) {
    for (size_t i = 0; i < limit; i++) {
        drawer->draw(draws[i], QColor(Qt::blue));
    }
}

void CPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < tAgentsize; i++) {
        if (!newRoleAgent[i]->getRoleUpdate()) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(soccer->agents[i]);
            newRoleAgent[i]->setRoleUpdate(true);
            newRoleAgent[i]->setAvoidBall(true);
            newRoleAgent[i]->setAvoidPenaltyArea(true);
            newRoleAgent[i]->setSelectedSkill(RoleSkill::GotopointAvoid);
            newRoleAgent[i]->setAvoidBall(true);

        }
    }

    if (tAgentsize > 0) {
        newRoleAgent[0]->setTarget(kickOffPos[0]);
        newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);

        for (int i = 1; i < tAgentsize; i++) {
            newRoleAgent[i]->setTarget(kickOffPos[i]);
            newRoleAgent[i]->setTargetDir(-newRoleAgent[i]->getAgent()->pos() + wm->field->oppGoal());
        }
    }

}

void CPlayOff::firstPlayForOppCorner(int _agentSize) {

    DBUG(QString("mode :%1").arg(firstStepEnums), D_NADIA);
    for (int i = 0; i < _agentSize; i++) {
        if (!newRoleAgent[i]->getRoleUpdate()) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(soccer->agents[dynamicMatch[i]]);
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
        if (firstStepEnums == Stay && blockerStep == S3) {
            firstStepEnums = Done;
        } else {
            firstStepEnums = Done;
        }
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

void CPlayOff::oneBehindBall() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
}

void CPlayOff::oneLeftOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
}

void CPlayOff::oneRightOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSidesOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-activeAgents.at(2)->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreOneDef() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[3]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[3]->getAgent()->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreTwoDef() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[4]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[4]->setTarget(kickOffPos[5]);
    newRoleAgent[4]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreTwoDefAndGoalie() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[3]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[3]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[4]->setTarget(kickOffPos[4]);
    newRoleAgent[4]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[5]->setTarget(kickOffPos[5]);
    newRoleAgent[5]->setTargetDir(-newRoleAgent[5]->getAgent()->pos() + wm->field->oppGoal());
}

bool CPlayOff::isPlanEnd() {

    if (isPlanDone()) {
        DBUG("Plan Succeded", D_MAHI);
        return true;

    } else if (isPlanFailed()) {
        if (lastBallPos.dist(wm->ball->pos) > 0.06) {
            DBUG("Plan Fully Failed", D_MAHI);
            return true;
        } else {
            DBUG("rePlaning", D_MAHI);
            return false;
        }
    }
    return false;
}

bool CPlayOff::isPlanDone() {
    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan doesn't include a final shoot
     if (tLastState == -1 || tLastAgent == -1) {
        if (isAllTasksDone()) {
            ROS_INFO("MAHIS: all task");

            DBUG("Done By Fully Tasks Done", D_MAHI);
            masterPlan->common.addHistory(10); //FULL
            return true;
        }
    } else if (isFinalShotDone()) {
        ROS_INFO("MAHIS: Final Shot");

        DBUG("Done By Final Shot !", D_MAHI);
        // TODO : IF GOAL THEN 10 ELSE 9
        masterPlan->common.addHistory(10); //FULL
        return true;
    }
    return false;
}


bool CPlayOff::isPlanFailed() {
    SFail fail = isAnyTaskFaild();
    if (isTimeOver()) {
        DBUG("Faild By Time Over", D_MAHI);
        ROS_INFO("MAHIS: Faild By Time Over");
        //        masterPlan->common.addHistory(); // Not Changeing History
        return true;

    } else if(firstKickFailed()){
            DBUG("playofff : passer didn't kick the ball properly", D_FATEME);
            return true;

    } else if (isBallDirChanged()) {
        DBUG("Faild By Ball Dir Changed", D_MAHI);
        ROS_INFO("MAHIS: Faild By Ball Dir Changed");
        masterPlan->common.addHistory(0); // MIN
        return true;
    } else if (fail = isAnyTaskFaild(), fail.fail) {
        ROS_INFO("MAHIS: Faild By task");

        DBUG(QString("Task Fail : TaskID %1").arg(fail.taskID), D_MAHI);
        DBUG(QString("Task Fail : AgentID %1").arg(fail.agentID), D_MAHI);
        DBUG(QString("Task Fail : PlanID %1").arg(fail.planID), D_MAHI);
        DBUG(QString("Task Fail : RoleID %1").arg(fail.roleID), D_MAHI);
        DBUG(QString("Task Fail : Mode %1").arg(fail.mode), D_MAHI);
        DBUG(QString("Task Fail : Skill %1").arg(static_cast<int>(fail.skill)), D_MAHI);
        masterPlan->common.addHistory(fail.succesRate);
        return true;
    }
    return false;
}

SFail CPlayOff::isAnyTaskFaild() {
    SFail faliure{};
    faliure.fail = false;
    // TODO : complete function
    // TODO : calculate succes rate
    return faliure;
}

bool CPlayOff::isAllTasksDone() {

    // if there's a agent that is not zombie
    for (size_t i = 0; i < masterPlan->common.currentSize; i++) {
        if (!positionAgent[i].zombie) {
            return false;
        }
    }

    return true;
}

bool CPlayOff::isBallPushed() {
    CRolePlayOff *currPasser;
    bool passerFound = false;
    int index = 0;
    for (auto ag: roleAgent) {
        if (ag->getSelectedSkill() == RoleSkill::Kick && ag->getAgent()->pos().dist(wm->ball->pos) < 0.17) {
            currPasser = ag;
            passerFound = true;
            break;
        }
    }

    if (passerFound) {
        if (lastBallPos.dist(wm->ball->pos) > 0.05
            && wm->ball->pos.dist(currPasser->getAgent()->pos()) < 0.13) {
            ROS_ERROR_STREAM("agent " << currPasser->getAgent()->id() << " PUSHED  THE BALL");

            // switch passer
            double second_min_dist = 100;
            CRolePlayOff *secondary_passer;

            for (auto ag: roleAgent) {
                if (ag->getAgent() != NULL) {
                    if (ag->getAgent()->id() != currPasser->getAgent()->id()) {
                        double temp = ag->getAgent()->pos().dist(wm->ball->pos);
                        if (temp < second_min_dist) {
                            second_min_dist = temp;
                            secondary_passer = ag;
                        }
                    }
                }
            }

            if (second_min_dist < 100) {
                ROS_ERROR_STREAM("agent " << secondary_passer->getAgent()->id() << " will be replaced i guess :/");
                Agent *curr = currPasser->getAgent();
                currPasser->setAgent(secondary_passer->getAgent());
                secondary_passer->setAgent(curr);
                return false;
            } else {
                return true;
            }
        }

        return false;
    }
}

bool CPlayOff::isTimeOver() {

    if (setTimer) {
        tempStart = ros::Time::now().sec;
    }

    if (!Circle2D(lastBallPos, 0.06).contains(wm->ball->pos)) {
        setTimer = false;
        ROS_INFO_STREAM("MAHIS: Time That Left: " << ros::Time::now().sec - tempStart);
        if(ros::Time::now().sec - tempStart >= 3*masterPlan->execution.passCount) { // 2 Second
            setTimer = true;
            return true;
        }
    }

    return false;
}

bool CPlayOff::isBallDirChanged() {

    if (masterPlan->execution.passCount != 1) {
        return false;
    }

    // USE PASSER FORM INITIAL LEVEL
    const int& passer = masterPlan->execution.passer.at(0).id;
    const int& recieve = masterPlan->execution.reciver.at(0).id;
    const int recive = masterPlan->common.matchedID.value(recieve);
    Vector2D& b  = wm->ball->pos;
    if (b.dist(lastBallPos) > 0.5 && !roleAgent[passer]->getChip()) {
        Vector2D  bv = b + wm->ball->vel.norm() * wm->field->_MAX_DIST;
        Circle2D  c(roleAgent[recive]->getWaitPos(), 1); // TODO : CHECK radius
        Segment2D s(b, bv);
        drawer->draw(s, QColor(Qt::blue));
        drawer->draw(c, QColor(Qt::red));
        Vector2D v1, v2;
        if (!c.intersection(s, &v1, &v2)) {
            return true;
        }
    } else {
        return false;
    }

}

bool CPlayOff::isFinalShotDone() {

    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan hasn't a final shoot
    if (tLastState == -1 || tLastAgent == -1) {
        return false;
    }

    Agent* tAgent = soccer->agents[masterPlan -> common.matchedID[tLastAgent]];

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
    const AgentPoint &r = masterPlan->execution.reciver.at(0);
    const AgentPoint &p = masterPlan->execution.passer.at(0);

    const int &i = masterPlan->common.matchedID.value(r.id);

    Agent *c = soccer->agents[i];
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

        Agent *firstPasser = soccer->agents[masterPlan->execution.passer.at(0).id];

        if (isTaskDone(roleAgent[i]) || (doAfterlife && roleAgent[i]->getAgent()->id() != firstPasser->id())) {

            roleAgent[i]->setRoleUpdate(false);
            roleAgent[i]->resetTime();

            POffSkills last_skill = positionAgent[i].positionArg.at(
                    positionAgent[i].positionArg.size() - 1).staticSkill;

            if (last_skill == Position) {
                if (!doAfterlife && positionAgent[i].stateNumber == positionAgent[i].positionArg.size() - 2) {
                    roleAgent[i]->setRoleUpdate(true);
                } else if (doAfterlife){
                    positionAgent[i].stateNumber = positionAgent[i].positionArg.size() - 1;
                }
            } else if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                if (positionAgent[i].getArgs(1).staticSkill == Defense
                    || positionAgent[i].getArgs(1).staticSkill == Support
                    || positionAgent[i].getArgs(1).staticSkill == Position
                    || positionAgent[i].getArgs(1).staticSkill == Goalie
                    || positionAgent[i].getArgs(1).staticSkill == Mark) {

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
        if (masterPlan->common.matchedID.contains(i)) {
            if (!roleAgent[i]->getRoleUpdate()) {

                roleAgent[i]->setFirstMove((positionAgent[i].stateNumber == 0));
                roleAgent[i]->setAgent(soccer->agents[masterPlan->common.matchedID.value(i)]);

                // handle onetouch faster
                if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                    if (positionAgent[i].getArgs().staticSkill == MoveSkill &&
                        positionAgent[i].getArgs(1).staticSkill == OneTouchSkill) {
                        positionAgent[i].stateNumber++;
                    }
                }

                for (auto x : masterPlan->execution.passer) {
                    if (x.id == i) {
                        if (i > 0 && masterPlan->execution.reciver.at(i - 1).id == x.id) {
                            if (positionAgent[i].getArgs().staticSkill == ReceivePassSkill) {
                                int index = -1;
                                for (int j = 1;
                                     j < positionAgent[i].positionArg.size() - positionAgent[i].stateNumber; j++) {
                                    if (positionAgent[i].getArgs(j).staticSkill == PassSkill) {
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
                                    float onetouchAngle = (float) (v1.th() - v2.th()).degree();
                                    ROS_INFO_STREAM("onetouchAngle " << onetouchAngle << " max OnetouchAngle"
                                                                     << conf.MaxOnetouchAngle);
                                    if (onetouchAngle < conf.MaxOnetouchAngle) {
                                        positionAgent[i].stateNumber++;
                                        firstPass = false;
                                        isOnetouch = true;
                                    }
                                }
                            }
                        }
                    }
                }

                roleAgent[i]->setRoleUpdate(true);
                roleAgent[i]->resetTime();
                assignTask(roleAgent[i], positionAgent[i]);
                ROS_INFO_STREAM("NEWTASK: " << roleAgent[i]->getTarget().x << roleAgent[i]->getTarget().y);
            }

        } else {
            qWarning() << "[Warning] coach -> Match function doesn't work :( ";
            if (!roleAgent[i]->getRoleUpdate()) {
                roleAgent[i]->setAgent(agents.at(i));
                assignTask(roleAgent[i], positionAgent[i]);
                roleAgent[i]->setRoleUpdate(true);
            }
        }
    }
}

void CPlayOff::assignTask(CRolePlayOff* _roleAgent, const SPositioningAgent& _positionAgent) {
    switch (_positionAgent.getArgs().staticSkill) {
    case PassSkill:
        assignPass(_roleAgent, _positionAgent);
        break;
    case ReceivePassSkill:
        assignReceive(_roleAgent, _positionAgent, false);
        break;
    case ReceivePassIASkill:
        assignReceive(_roleAgent, _positionAgent, true);
        break;
    case ShotToGoalSkill:
        assignKick(_roleAgent, _positionAgent, false);
        break;
    case ChipToGoalSkill:
        assignKick(_roleAgent, _positionAgent, true);
        break;
    case OneTouchSkill:
        assignOneTouch(_roleAgent, _positionAgent);
        break;
    case MoveSkill:
        assignMove(_roleAgent, _positionAgent);
        break;
    // After Life
    case Defense:
        _roleAgent->setRoleUpdate(false);
        assignDefense(_roleAgent, _positionAgent);
        break;
    case Support:
        _roleAgent->setRoleUpdate(false);
        assignSupport(_roleAgent, _positionAgent);
        break;
    case Position:
        _roleAgent->setRoleUpdate(false);
        assignPosition(_roleAgent, _positionAgent);
        break;
    case Goalie:
        _roleAgent->setRoleUpdate(false);
        assignGoalie(_roleAgent, _positionAgent);
        break;
    case Mark:
        _roleAgent->setRoleUpdate(false);
        assignMark(_roleAgent, _positionAgent);
        break;
    case NoSkill:
        //        assignAfterLife(_roleAgent, _positionAgent);
        break;
    }
}

void CPlayOff::assignPass(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent->setAvoidCenterCircle(false);
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setChip(chipOrNot(_posAgent.getArgs()));
    if (_roleAgent->getChip()) {
        _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().rightData)/100.0);
        //_roleAgent->setKickRealSpeed(static_cast <double> (_posAgent.getArgs().rightData)/200);
    } else {
        _roleAgent->setKickSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100.0);
        //        _roleAgent->setKickRealSpeed(static_cast <double> (_posAgent.getArgs().leftData)/100);

    }

    _roleAgent->setTarget(positionAgent[_posAgent.getArgs().PassToId].getAbsArgs(_posAgent.getArgs().PassToState).staticPos);
    _roleAgent->setDoPass(doPass);
    _roleAgent->setIntercept(false);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng);
    if(firstPass || !isOnetouch) {
        _roleAgent->setSelectedSkill(RoleSkill::Kick);
    } else {
        _roleAgent->setSelectedSkill(RoleSkill::OneTouch);

    }
}

void CPlayOff::assignReceive(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _ignoreAngle) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setIgnoreAngle(_ignoreAngle);
    _roleAgent->setTarget(_posAgent.getArgs().staticPos);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng); /** Just Matter when we use Ignore mode **/
    _roleAgent->setReceiveRadius((float)_posAgent.getArgs().leftData / 100);
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

void CPlayOff::assignMove(CRolePlayOff* _roleAgent,
                          const SPositioningAgent& _posAgent) {
    _roleAgent -> setAvoidPenaltyArea(true);
    _roleAgent -> setTime(_posAgent.getArgs().rightData); // ignore duration -> time is wait
    _roleAgent -> setAvoidBall(true);


    if (_posAgent.getArgs().staticPos == POBALLPOS) { // First Passer

        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(wm->ball->pos - Vector2D(0.20, 0));
        _roleAgent -> setTargetDir(wm->ball->pos - _roleAgent->getAgent()->pos());
        _roleAgent -> setSlow(true);
        _roleAgent -> setMaxVelocity(1);

    } else if (_roleAgent->getTime() > 10) {

        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
        _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
        _roleAgent -> setSlow(false);
        _roleAgent -> setMaxVelocity(static_cast<float>(getMaxVel(_roleAgent, _posAgent.getArgs())));

    } else {

        _roleAgent -> setTimeBased(false);
        _roleAgent -> setEventDist(_posAgent.getArgs().staticEscapeRadius);
        _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
        _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
        _roleAgent -> setSlow(false);
        _roleAgent -> setMaxVelocity(static_cast<float>(getMaxVel(_roleAgent, _posAgent.getArgs())));

    }
    _roleAgent -> setAvoidBall(true);
    _roleAgent -> setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CPlayOff::assignGoalie(CRolePlayOff * _roleAgent,
                            const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setTarget(wm->field->ourGoal() + Vector2D(0, 1));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid);
}

void CPlayOff::assignMark(CRolePlayOff * _roleAgent,
                          const SPositioningAgent &_posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    CDefPos tempDefPos;
    kkDefPos tempPos = tempDefPos.getDefPositions(wm->ball->pos, 2, 2, 3);
    _roleAgent->setTarget(tempPos.pos[1]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid

}

void CPlayOff::assignPosition(CRolePlayOff * _roleAgent,
                              const SPositioningAgent &_posAgent) {
    assignMove(_roleAgent, _posAgent); // TODO : check this
}

void CPlayOff::assignSupport(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    Vector2D tempTarget = (lastBallPos.dist(wm->ball->pos) < 0.1)
                          ? _roleAgent->getAgent()->pos()
                          : (wm->ball->pos + wm->ball->vel * 0.5) - Vector2D(1, 0);


    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setSlow(false);
    _roleAgent->setTargetDir(_roleAgent->getAgent()->pos() - wm->ball->pos);
    _roleAgent->setTarget(getEmptyTarget(tempTarget, .5));
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}

void CPlayOff::assignDefense(CRolePlayOff * _roleAgent,
                             const SPositioningAgent &_posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setAvoidBall(false);
    _roleAgent->setNoAvoid(true);
    _roleAgent->setTargetDir(Vector2D(0, 1));
    _roleAgent->setSlow(false);
    CDefPos tempDefPos;
    kkDefPos tempPos = tempDefPos.getDefPositions(wm->ball->pos, 2, 2, 3);
    _roleAgent->setTarget(tempPos.pos[0]);
    _roleAgent->setSelectedSkill(RoleSkill::GotopointAvoid); //GotoPointAvoid
}


Vector2D CPlayOff::getMoveTarget(int agentID,
                                 int agentState) {
    Vector2D tempTarget, finalTarget, position;
    double escapeRad;
    int oppCnt = 0;
    bool posFound = false;
    escapeRad = positionAgent[agentID].positionArg.at(agentState).staticEscapeRadius;
    position = positionAgent[agentID].positionArg.at(agentState).staticPos;
    finalTarget = position;
    for (double dist = 0.0 ; dist <= escapeRad ; dist += 0.2) {
        for (double ang = -180.0 ; ang <= 180.0 ; ang += 60.0) {
            tempTarget = position + Vector2D::polar2vector(dist, ang);
            for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
                if (Circle2D(wm->opp.active(i)->pos, 0.02).contains(tempTarget)) {
                    oppCnt++;
                }
            }
            if (!oppCnt) {
                finalTarget = tempTarget;
                posFound = true;
                break;
            }
        }
        if (posFound) {
            break;
        }
    }

    return finalTarget;
}

Vector2D CPlayOff::getMoveTarget(const SPositioningArg& _posArg) {
    return getEmptyTarget(_posArg.staticPos, _posArg.staticEscapeRadius);
}

double CPlayOff::getMaxVel(int agentID,
                           int agentState) {
    double tempVel, tDist;
    Vector2D tAgentPos;
    tAgentPos = soccer->agents[kkAgentsID[agentID]]->pos();
    tDist = tAgentPos.dist(positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).staticPos);
    tempVel = tDist / positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).leftData;
    if (tempVel >= 4) {
        tempVel = 4;
    } else if (tempVel <= 1.5) {
        tempVel = 1.5;
    }
    return tempVel;
}


double CPlayOff::getMaxVel(const CRolePlayOff* _roleAgent,
                           const SPositioningArg& _posArg) {
    double tempVel, tDist;
    Vector2D tAgentPos;
    tAgentPos = _roleAgent->getAgent()->pos();
    tDist = tAgentPos.dist(_posArg.staticPos);
    tempVel = tDist / _posArg.leftData;

    if (tempVel >= 4) {
        tempVel = 4;
    } else if (tempVel <= 1.5) {
        tempVel = 1.5;
    }

    return tempVel;
}


Vector2D CPlayOff::getGoalTarget(int shoterID, int shoterState) {
    Vector2D tempPos;
    double tempYPos = (double)(positionAgent[shoterID].positionArg.at(shoterState).rightData) / 1000 + wm->field->oppGoalR().y;
    drawer->draw(QString("%1").arg(tempYPos), Vector2D(-2, -2));
    if (tempYPos < wm->field->oppGoalR().y) {
        tempYPos = wm->field->oppGoalR().y;
    }
    if (tempYPos > wm->field->oppGoalL().y) {
        tempYPos = wm->field->oppGoalL().y;
    }
    tempPos.assign(wm->field->oppGoal().x, tempYPos);
    return tempPos;
}

Vector2D CPlayOff::getGoalTarget(const long& _y) {
    Vector2D tempPos;
    double tempYPos = (double)(_y) / 1000 + wm->field->oppGoalR().y;
    drawer->draw(QString("%1").arg(tempYPos), Vector2D(-2, -2));
    if (tempYPos < wm->field->oppGoalR().y) {
        tempYPos = wm->field->oppGoalR().y;
    }
    if (tempYPos > wm->field->oppGoalL().y) {
        tempYPos = wm->field->oppGoalL().y;
    }
    tempPos.assign(wm->field->oppGoal().x, tempYPos);
    return tempPos;
}

bool CPlayOff::chipOrNot(int passerID, int ReceiverID, int ReceiverState) {

    if (positionAgent[passerID].positionArg.at(positionAgent[passerID].stateNumber).leftData < 0) {
        return true;
    } else if (positionAgent[passerID].positionArg.at(positionAgent[passerID].stateNumber).rightData < 0) {
        return false;
    } else return !isPathClear(wm->ball->pos,
                                   positionAgent[ReceiverID].positionArg.at(ReceiverState).staticPos,
                                   0.5,
                                   0.1);
}

bool CPlayOff::chipOrNot(const SPositioningArg& _posArg) {
    if (_posArg.leftData < 0) {
        return true;
    } else if (_posArg.rightData < 0) {
        return false;
    } else {
        const int& id = _posArg.PassToId;
        const int& ps = _posArg.PassToState;
        return !isPathClear(wm->ball->pos,
                            positionAgent[id].getAbsArgs(ps).staticPos,
                            0.5,   // Radius
                            0.1);  // Tereshold
    }
}

bool CPlayOff::isPathClear(Vector2D _pos1,
                           Vector2D _pos2,
                           double _radius,
                           double treshold) {

    Vector2D sol1, sol2, sol3;
    Line2D _path(_pos1, _pos2);
    Polygon2D _poly;
    Circle2D(_pos2, _radius + treshold).intersection(_path.perpendicular(_pos2),
            &sol1,
            &sol2);

    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1, Robot::robot_radius_new + treshold).intersection(_path.perpendicular(_pos1),
            &sol1,
            &sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    drawer->draw(_poly, "red");

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++) {
        if (_poly.contains(wm->opp.active(i)->pos)) {
            return false;
        }
    }
    return true;
}

void CPlayOff::assignTasks() {
    int &sym = masterPlan->execution.symmetry;
    for (size_t i = 0; i < masterPlan->common.currentSize; i++) {

        positionAgent[i].positionArg.clear();

        Q_FOREACH (playOffRobot agentPlan, masterPlan->execution.AgentPlan[i]) {
            SPositioningArg tempPosArg;
            tempPosArg.staticPos = agentPlan.pos;
            tempPosArg.staticAng = Vector2D::polar2vector(1, agentPlan.angle);
            tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -1 * sym * tempPosArg.staticAng.y);
            tempPosArg.staticPos.assign(tempPosArg.staticPos.x, sym * tempPosArg.staticPos.y);
            tempPosArg.staticEscapeRadius = agentPlan.tolerance;

            Q_FOREACH (playOffSkill skill, agentPlan.skill) {
                tempPosArg.leftData = skill.data[0];
                tempPosArg.rightData = skill.data[1];
                tempPosArg.staticSkill = skill.name;
                tempPosArg.PassToId = skill.targetAgent;
                tempPosArg.PassToState = skill.targetIndex;

                if (skill.name == PassSkill && positionAgent[i].positionArg.back().staticSkill == MoveSkill) {
                    positionAgent[i].positionArg.back().staticPos = POBALLPOS;
                } else if (skill.name == ShotToGoalSkill
                           || skill.name == ChipToGoalSkill
                           || skill.name == OneTouchSkill) {

                    if (sym < 0) {
                        tempPosArg.rightData = 1000 - tempPosArg.rightData;
                    }
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
        if (positionAgent[_passer].getArgs(i).staticSkill == PassSkill) {
            SBallOwner temp{};
            temp.id = _passer;
            temp.state = i;
            ownerList.append(temp);
            findReceiver(positionAgent[_passer].getArgs(i).PassToId,
                         positionAgent[_passer].getArgs(i).PassToState);
        }
    }
}

void CPlayOff::getCostRec(double costArr[][_NUM_PLAYERS], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId) {
    if (size <= 1) {
        int tIndex = kkGetIndex(value, 0);
        value.IDs[tIndex] = aId;
        value.value += costArr[tIndex][aId];
        valueList.append(value);
        return;
    }
    for (int i = 0; i < size; i++) {
        kkValue tValue = value;
        int tIndex = kkGetIndex(tValue, i);
        tValue.IDs[tIndex] = aId;
        tValue.value += costArr[tIndex][aId];
        getCostRec(costArr, arrSize, valueList, tValue, size - 1, aId + 1);
    }
}

int CPlayOff::kkGetIndex(kkValue &value, int cIndex) {
    int cnt = 0;
    for (int i = 0; i < value.agentSize; i++) {
        if (value.IDs[i] == -1) {
            if (cnt == cIndex) {
                return i;
            }
            cnt++;
        }
    }
}

POMODE CPlayOff::getPlayOffMode() {
    if (gameState->ourKickoff()) {
        return KICKOFF;
    } else if (gameState->ourIndirectKick()) {
        return INDIRECT;
    } else if (gameState->ourDirectKick()) {
        return DIRECT;
    }
}

void CPlayOff::reset() {


    qDebug() << "Bring yourself back online playoff";

    blockerState = 0;
    blockerID = -1;
    blockerStopStates = Diversion;

    decidePlan = true;
    agentSize = 1;
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

    currentPlan = new SPlayOffPlan();
    masterPlan = nullptr;

    initial    = true;
    playOnFlag = false;
    havePassInPlan = false;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = -1;

    executedCycles = 0;
    activeAgents.clear();
    markAgents.clear();

    firstStepEnums = Stay;
    blockerStep = S0;

    firstPass = true;
    isOnetouch = false;
    playoff_badPasserID = -1;

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
    return false;
    if (lastBallPos.dist(wm->ball->pos) > 0.15 && Circle2D(lastBallPos, 0.5).contains(wm->ball->pos)
        && wm->ball->vel.length() < 0.1) {
        const int &i = masterPlan->execution.passer.at(0).id;

        playoff_badPasserID = soccer->agents[i]->id();
        ROS_INFO("playofff bad kick");
        return true;
    }
    return false;
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
    } else if (!Circle2D(lastBallPos, 0.6).contains(wm->ball->pos)
               && _roleAgent->getBallIsNear()) {
        _roleAgent->setBallIsNear(false);
        if (_roleAgent->getChip()) {
            DBUG("[playoff] chip Done", D_MAHI);
            return true;
        } else {
            /** Ball gonna touch the target point **/

            // check ball speed
//            if (wm->ball->vel.length() / (_roleAgent->getAgent()->pos().dist(_roleAgent->getTarget())) > 1 || true) {
//            DBUG("[playoff] speed is enough", D_MAHI);

            // check ball direction
//            Vector2D sol1, sol2;

//                ((wm->ball->pos - _roleAgent->getAgent()->pos()) - (lastBallPos - _roleAgent->getAgent()->pos())).length() > 0.15
//            if (true || wm->ball->vel.angleWith(_roleAgent->getAgent()->dir()).RAD2DEG < 40 /*&& wm->ball->vel.length() > 0.2*/
//                Circle2D(_roleAgent->getTarget(), 0.5).intersection(
//                        Ray2D(wm->ball->pos, wm->ball->pos + wm->ball->vel), &sol1, &sol2)
//                    ) {
//                DBUG("[playoff] direction is correct", D_MAHI);
            DBUG("[playoff] kick is Done", D_MAHI);
            return true;

//            }
//            }
        }
    }

    return false;
}


bool CPlayOff::isReceiveDone(const CRolePlayOff * _roleAgent) {
    if (Circle2D(_roleAgent->getAgent()->pos(), 0.3).contains(wm->ball->pos)) {
        if (wm->ball->vel.length() < 0.5) {
            return true;
        }
    }
    return false;
}

bool CPlayOff::isOneTouchDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.4).contains(wm->ball->pos)) {
        ROS_INFO("playofff one-touch BallIsNear");
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

    if (_roleAgent->getFirstMove() && _roleAgent->getTarget() != POBALLPOS) {
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


///////////OverLoading Operators
QDebug operator<< (QDebug d, NGameOff::SPlan _plan) {

    QString mode;
    if (_plan.common.planMode == KICKOFF) {
        mode = "KickOff";
    } else if (_plan.common.planMode == DIRECT) {
        mode = "Direct";
    } else if (_plan.common.planMode == INDIRECT) {
        mode = "InDirect";
    }


    d << "------------------->>>";
    d << "<Common>" ;
    d << "Agent Size" << _plan.common.agentSize;
    d << "Chance" << _plan.common.chance;
    d << "Last Dist" << _plan.common.lastDist;
    d << "Plan Mode" << mode;
    d << "Succes Rate" << _plan.common.succesRate;
    d << "Tags" << _plan.common.tags;
    d << "</Common>";
    d << "<<<-------------------";
    return d;
}

void CPlayOff::setInitial(bool _init) {
    initial = _init;
}

QPair<int, int> CPlayOff::findTheLastShoot(const SExecution &_plan) {
    QPair<int, int> last;
    last.first = last.second = -1;

    QList<POffSkills> finalSkills;
    finalSkills.append(ShotToGoalSkill);
    finalSkills.append(ChipToGoalSkill);
    finalSkills.append(OneTouchSkill);

    int counter = 0;
    qDebug() << "SSS" << _plan.AgentPlan.size();
    Q_FOREACH (QList<playOffRobot> agent, _plan.AgentPlan) {
        int counter2 = 0;
        Q_FOREACH (playOffRobot node, agent) {
            Q_FOREACH (playOffSkill skill, node.skill) {
                if (finalSkills.contains(skill.name)) {
                    last.first  = counter;
                    last.second = counter2;
                    qDebug() << "MAHI " << skill.name;
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
        qDebug() << "Last" << last.first << last.second;
        havePassInPlan = (last.first != -1  && last.second != -1);
    }
}

void CPlayOff::analysePass() {
    // TODO : need edit for mulitiple pass
    if (masterPlan != nullptr) {
        // first : passer second : reciver
        QList<AgentPair> tPass;
        findThePasserandReciver(masterPlan->execution, tPass);
        qDebug() << "PASS COUNT" << tPass.size();
//        ROS_ERROR_STREAM("PASS COUNT" << tPass.size());
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

                masterPlan->execution.reciver.append(tPass.at(i).second);
            }
//            qDebug() << "First Pass:";
//            qDebug() << "PI : " << masterPlan->execution.passer.at(0).id;
//            qDebug() << "PS : " << masterPlan->execution.passer.at(0).state;
//            qDebug() << "RI : " << masterPlan->execution.reciver.at(0).id;
//            qDebug() << "RS : " << masterPlan->execution.reciver.at(0).state;
        }
    }

}

bool CPlayOff::criticalPlay() {

    ROS_INFO("criticalll");
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
//    soccer->agents[masterPlan->common.matchedID.value(masterPlan->execution.passer.at(0).id)]->action = criticalKick;
    double minDist = 100;
    int nearestID = -1;
    for(int i=0; i< 8; i++) {
        if(soccer->agents[i] != nullptr){
            if(soccer->agents[i]->pos().dist(wm->ball->pos) < minDist){
                nearestID = i;
                minDist = soccer->agents[i]->pos().dist(wm->ball->pos);
            }
        }
    }
    if(nearestID != -1){
        soccer->agents[nearestID]->action = criticalKick;
    }
    return wm->ball->vel.length() > 0.5;

}

void CPlayOff::findThePasserandReciver(const NGameOff::SExecution & _plan,
                                       QList<AgentPair>& _pairList) {

    QList<AgentPoint> passer;

    for (int i = 0; i < _plan.AgentPlan.size(); i++) {
        const QList<playOffRobot> & agent = _plan.AgentPlan.at(i);
        for (int j = 0; j < agent.size(); j++) {
            const playOffRobot& node = agent.at(j);
            for (const auto &k : node.skill) {
                const POffSkills& skill = k.name;
                if (skill == PassSkill) {
                    passer.append(AgentPoint(i, j));
                }
            }

        }
    }

    ROS_INFO_STREAM("Pass count: " << passer.size());
    for (auto i : passer) {
        const int &id = i.id;
        const int &st = i.state;

        int si = (_plan.AgentPlan[id][st].skill[1].name == PassSkill) ? 1 : 2;
        int rid = _plan.AgentPlan[id][st].skill[si].targetAgent;
        int rs  = _plan.AgentPlan[id][st].skill[si].targetIndex;
        DBUG(QString("PASS : %1, %2, %3, %4").arg(id).arg(st).arg(rid).arg(rs), D_MAHI);
        AgentPoint tempReciver;
        tempReciver.id    = rid;
        tempReciver.state = rs;

        AgentPair ap;
        ap.first  = i;
        ap.second = tempReciver;
        _pairList.append(ap);

    }

}

Vector2D CPlayOff::getMarkTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);
}

Vector2D CPlayOff::getDefenseTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);

}

Vector2D CPlayOff::getSupportTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);

}
