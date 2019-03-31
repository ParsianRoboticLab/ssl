
#include <parsian_ai/roles/playmake.h>

#include "parsian_ai/roles/playmake.h"

using namespace std;


CRolePlayMake::CRolePlayMake(Agent *_agent) : CRole(_agent) {
    justTurn = false;
    kickToTheirDefense = false;
    chip = slow = false;
    shadowPass = false;
    indirectKhafan = false;
    chipIndirect = false;
    through = false;
    noKick = false;
    penaltyTarget.invalidate();
    manualPassReceive = false;
    allowonetouch = false;
    allowonepass = false;
    indirectTiny = false;
    passReceiver = -1;
    timerStartFlag = true;

    //    spinPass = new CBehaviourSpinPass;
    initialPoint.invalidate();

    lastBounceDataFile.setFileName("lastBounce");
    out.setDevice(&lastBounceDataFile);
}

CRolePlayMake::~CRolePlayMake() {
}

void CRolePlayMake::stopBehindBall(bool penalty) {
    if (penalty) {
        if (gameState->isStop()) {
            DBUG("stop, reset changeDirPenaltyStriker flag", D_FATEME);

        }

        if (gameState->penaltyShootout()) {
//            gotopoint->setTargetpos(wm->ball->pos + (wm->ball->pos - wm->field->oppGoal()) * 0.03);
//            gotopoint->setTargetdir(wm->ball->pos - agent->pos());
        } else {
            Vector2D direction, position;

            direction = wm->ball->pos - agent->pos();
            direction.y *= 1.2;
            position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + Vector2D(0, 0.2)).norm() * (0.13);
//            gotopoint->setTargetpos(position);
//            gotopoint->setTargetdir(direction);
        }

//        gotopoint->setSlowmode(true);
//        gotopoint->setNoavoid(false);
//        gotopoint->setPenaltykick(true);
//        gotopoint->setAvoidpenaltyarea(false);
//        gotopoint->setAvoidcentercircle(false);

//        gotopoint->setBallobstacleradius(0.2);
//        agent->action = gotopoint;
//        gotopoint->setNoavoid(false);
//        gotopoint->setSlowmode(false);

    } else {
        Vector2D shadowPoint = wm->ball->pos + Vector2D(wm->ball->pos - wm->field->oppGoal()).norm() * 0.3;
        if (kickoffmode || kickoffWing) {
            shadowPoint = wm->ball->pos + Vector2D(wm->field->oppGoal() - wm->ball->pos).norm() * 0.3;
        }
//        gotopoint->setSlowmode(true);
//        gotopoint->setNoavoid(false);
//        gotopoint->setAvoidpenaltyarea(true);
//        gotopoint->setAvoidcentercircle(false);
//        gotopoint->setBallobstacleradius(static_cast<float>(4 * CBall::radius));
//        gotopoint->setTargetpos(shadowPoint);
//        gotopoint->setTargetdir(Vector2D(1.0, 0.0));
//        gotopoint->setLookat(wm->ball->pos);
//        agent->action = gotopoint;
    }
}

void CRolePlayMake::kickPass(double kickSpeed) {
    Vector2D behindTheBall = wm->ball->pos + Vector2D(wm->ball->pos - pointToPass).norm() * 0.2;
    drawer->draw(behindTheBall, QColor(100, 0, 0), 1);
    DEBUG("ooooooooooomadddd", D_ALI);
    if (kickPassMode == KickPassFirst && agent->pos().dist(behindTheBall) > 0.01) {
        finalTarget = wm->ball->pos;
//        gotopoint->setTargetpos(behindTheBall);
//        gotopoint->setTargetdir(Vector2D(1.0, 0.0));
//        gotopoint->setLookat(pointToPass);
//        gotopoint->setSlowmode(true);
//        gotopoint->setNoavoid(false);
//        gotopoint->setAvoidpenaltyarea(true);
//        gotopoint->setAvoidcentercircle(false);
//        gotopoint->setBallobstacleradius(0.2);
//        agent->action = gotopoint;
    } else {
        kickPassMode = KickPassSecond;
        if (kickPassCyclesWait > 4 && agent->pos().dist(finalTarget) > 0.01) {
//            gotopoint->setTargetpos(finalTarget);
//            gotopoint->setTargetdir(Vector2D(1.0, 0.0));
//            gotopoint->setLookat(pointToPass);
//            gotopoint->setSlowmode(true);
//            gotopoint->setNoavoid(true);
//            gotopoint->setAvoidpenaltyarea(true);
//            gotopoint->setAvoidcentercircle(false);
//            gotopoint->setBallobstacleradius(0);
//            agent->action = gotopoint;
        } else {
            kickPassCyclesWait++;
        }
    }
}

void CRolePlayMake::execute() {
    update();

    switch (playMakeSkill) {
        case PlayMakeSkill::Shot:
        case PlayMakeSkill::Chip:
        case PlayMakeSkill::Pass:
        case PlayMakeSkill::CatchBall:
        DBUG(QString("[dynamicRole] kickSpeed : %1").arg(kickSpeed), D_MAHI);
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: playmake hame");
            agent->action = shotSkill;
            break;
        case PlayMakeSkill::NoSkill:
        default:
            agent->action = nullptr;
            break;
    }

    return;
    ROS_INFO_STREAM("shootout: gameState->ourPenaltyShootout(): " << gameState->ourPenaltyShootout());
    cyclesExecuted++;
    if (wm->ball->inSight <= 0
        || !wm->ball->pos.

            valid()

        || !wm->field->

                    marginedField()

            .
                    contains(wm
                                     ->ball->pos)) {
//        wait->setWaithere(true);
//        agent->action = wait;
        ROS_INFO_STREAM("shootout: in first if: ");
        return;
    }

    double region;
    QList<int> ourRelax;
    ourRelax.

            clear();

    ourRelax.
            append(wm
                           ->our.data->activeAgents);
    QList<int> oppRelax;
    Vector2D target = know->getEmptyPosOnGoal(wm->ball->pos, region, true, ourRelax, oppRelax);
    double kickSpeed = 5;

//    if (!noKick) {
//        ROS_INFO_STREAM("shootout: in second if: ");
//        return;
//    }


    if (cyclesExecuted < cyclesToWait) {
        stopBehindBall(false);
        return;
    }

    if (noKick) {
        return;
    }

    if (kickMode == FixedPass) {
        DBUG("HERE", D_KK);
        setThrough(false);
        target = pointToPass;
        kickSpeed = agent->pos().dist(target);
//        kick->setSlow(false);
//        kick->setChip(chip);
//        kick->setTarget(target);
        kickPass(kickSpeed);
        return;
    } else if (kickMode == FixedShoot) {
        target = pointToShoot;
        setChip(false);
        kickSpeed = agent->pos().dist(target);
//        kick->setSlow(false);
//        kick->setTarget(target);
//        kick->setKickspeed(kickSpeed);
//        kick->setTolerance(0.06);
//        kick->setAvoidpenaltyarea(true);
//        agent->action = kick;
        DBUG("HERE2", D_KK);
    }

    DBUG("HERE3", D_KK);
//    kick->setSlow(false);
//    kick->setTarget(target);
//    kick->setKickspeed(kickSpeed);
//    kick->setTolerance(0.06);
//    kick->setAvoidpenaltyarea(true);
//    agent->action = kick;
}

void CRolePlayMake::parse(QStringList params) {

    setKickToTheirDefense(false);
    setJustTurn(false);
    setSlow(false);
    setChip(false);
    setIndirectKhafan(false);
    setNoStop(false);
    setKickoffWing(false);
    setIndirectGoogooli(false);
    setChipIndirect(false);
    setThrough(false);
    setManualPassReceive(false);
    setAllowOneTouch(false);
    setAllowOnePass(false);
    setFollowSequence(false);
    setKickoffmode(false);
    setIndirectTiny(false);
    setChipInPenaltyArea(false);
    setShadowPass(false);
    setKhers(false);
    setShadowyPoint(false);
    setLongChip(false);
    setPassReceiver(-1);
    setChipToOppGoal(false);
    setSafeIndirect(false);
    setLocalAgentPassTarget("");
    setPassReceiver(-1);
//    kick->setInterceptmode(false);
    for (int i = 0; i < params.length(); i++) {
        bool ok = false;
        int p = params[i].toInt(&ok);
        if (params[i].toLower() == "slow") {
            setSlow(true);
        } else if (params[i].toLower() == "chip") {
            setChip(true);
        } else if (params[i].toLower() == "indirectkhafan") {
            setIndirectKhafan(true);
        } else if (params[i].toLower() == "nostop") {
            setNoStop(true);
        } else if (params[i].toLower() == "wing") {
            setKickoffWing(true);
        } else if (params[i].toLower() == "indirectgoogooli") {
            setIndirectGoogooli(true);
        } else if (params[i].toLower() == "chipindirect") {
            setChipIndirect(true);
        } else if (params[i].toLower() == "through") {
            setThrough(true);
        } else if (params[i].toLower() == "nokick") {
            setNoKick(true);
        } else if (params[i].toLower() == "allowonetouch") {
            setAllowOneTouch(true);
        } else if (params[i].toLower() == "allowonepass") {
            setAllowOnePass(true);
        } else if (params[i].toLower() == "followsequence") {
            setFollowSequence(true);
        } else if (params[i].toLower() == "kickoff") {
            setKickoffmode(true);
        } else if (params[i].toLower() == "indirecttiny") {
            setIndirectTiny(true);
        } else if (params[i].toLower() == "chipinpenalty") {
            setChipInPenaltyArea(true);
        } else if (params[i].toLower() == "shadowpass") {
            setShadowPass(true);
        } else if (params[i].toLower() == "khers") {
            setKhers(true);
        } else if (params[i].toLower() == "kicktotheirdefense") {
            setKickToTheirDefense(true);
        } else if (params[i].toLower() == "justturn") {
            setJustTurn(true);
        } else if (params[i].toLower() == "longchip") {
            setLongChip(true);
        } else if (params[i].toLower() == "chiptogoal") {
            setChipToOppGoal(true);
        } else if (params[i].toLower() == "shadowypoint") {
            setShadowyPoint(true);
        } else if (params[i].toLower() == "safe") {
            setSafeIndirect(true);
        } else if (params[i].startsWith("@")) {
            localAgentPassTarget = params[i].right(params[i].length() - 1);
        } else if (params[i].toLower() == "%") {
            setManualPassReceive(true);
            setPassReceiver(-5);
        } else if (ok) {
            setManualPassReceive(true);
            setPassReceiver(p);
        }
    }
}

void CRolePlayMake::update() {
    shotSkill->setPlaymakemode(true);
    shotSkill->setKickwithcenterofdribbler(true);
    shotSkill->setIskhafan(true);
    switch (playMakeSkill) {
        case PlayMakeSkill::Shot:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setAvoidopppenaltyarea(true);
            shotSkill->setPlaymakemode(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryfine(veryFine);
            shotSkill->setDontkick(false);
            if (isdischargetime) {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            } else {
                shotSkill->setIskickchargetime(false);
                if (chip) {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                } else {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            shotSkill->setIskickchargetime(true);
            shotSkill->setKickchargetime(1023);

            break;
        case PlayMakeSkill::Chip:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setAvoidopppenaltyarea(true);
            shotSkill->setPlaymakemode(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryfine(veryFine);
            shotSkill->setDontkick(false);
            if (isdischargetime) {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            } else {
                shotSkill->setIskickchargetime(false);
                if (chip) {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                } else {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill::Pass:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setAvoidopppenaltyarea(true);
            shotSkill->setPlaymakemode(true);
            shotSkill->setChip(chip);
            shotSkill->setDontkick(noKick);
            shotSkill->setVeryfine(veryFine);
            if (isdischargetime) {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            } else {
                shotSkill->setIskickchargetime(false);
                if (chip) {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                } else {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill::CatchBall:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryfine(false);
            if (isdischargetime) {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            } else {
                shotSkill->setIskickchargetime(false);
                if (chip) {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                } else {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill::Keep:
            break;
        case PlayMakeSkill::NoSkill:
        default:
            break;
    }
}


CRolePlayMakeInfo::CRolePlayMakeInfo(QString _roleName) : CRoleInfo(_roleName) {

}
