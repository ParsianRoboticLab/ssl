#include <parsian_util/core/worldmodel.h>
#include "parsian_ai/roles/playoffrole.h"


CRolePlayOff::CRolePlayOff() {
    deleted = false;
    agent = nullptr;
    gotoPointAvoidSkill = new GotopointavoidAction;
    kickSkill = new KickAction;
    oneTouchSkill = new OnetouchAction;
    receivePassSkill = new ReceivepassAction;
    updated = true;
    roleUpdate = false;
    noAvoid = false;
    timer.start();
    agentID = -1;
    lookForward = true;
    spin = 0;
}

CRolePlayOff::~CRolePlayOff() {
    delete gotoPointAvoidSkill;
    delete kickSkill;
    delete oneTouchSkill;
    delete receivePassSkill;
}

void CRolePlayOff::reset() {

    updated = true;
    deleted = false;
    roleUpdate = false;
    timer.start();
    agentID = -1;
    lookForward = true;
    ballIsNear = false;
    noAvoid = false;
    spin = 0;
}

void CRolePlayOff::update() {
    switch (selectedSkill) {
    case RoleSkill::Gotopoint:
        break;
    case RoleSkill::GotopointAvoid:
        gotoPointAvoidSkill->setTargetpos(target);
        gotoPointAvoidSkill->setTargetdir(targetDir);
        gotoPointAvoidSkill->setAvoidpenaltyarea(avoidPenaltyArea);
        gotoPointAvoidSkill->setMaxvelocity(maxVelocity);
        gotoPointAvoidSkill->setNoavoid(noAvoid);
        gotoPointAvoidSkill->setBallobstacleradius(1);

        updated = false;
        break;
    case RoleSkill::Kick:
        kickSkill->setIsplayoff(true);
        kickSkill->setTarget(target);
        kickSkill->setAvoidpenaltyarea(avoidPenaltyArea);
        kickSkill->setInterceptmode(intercept);
        kickSkill->setKickspeed(kickSpeed);
        kickSkill->setChip(chip);
        kickSkill->setDontkick(!doPass);
        kickSkill->setTolerance(0.5);
        kickSkill->setPassprofiler(false);
        kickSkill->setKickwithcenterofdribbler(false);
        kickSkill->setSpin(spin);

        if (!doPass && !chip && lookForward) {
            kickSkill->setTarget(Vector2D(1000, 0));
        }
        updated = false;
        break;
    case RoleSkill::Mark:
        break;
    case RoleSkill::OneTouch:
        oneTouchSkill->setTarget(target);
        oneTouchSkill->setWaitpos(waitPos);
        oneTouchSkill->setChip(false);
        oneTouchSkill->setShottoemptyspot(false);
//            if (wm->getIsSimulMode())
        oneTouchSkill->setKickspeed(8);
//            else
        oneTouchSkill->setKickspeed(1023);
        updated = false;
        break;
    case RoleSkill::ReceivePass:
//        oneTouchSkill->setTarget(targetDir);
//        oneTouchSkill->setWaitPos(target);
//        oneTouchSkill->setAgent(agent);
//        oneTouchSkill->setChip(false);
//        oneTouchSkill->setShotToEmptySpot(false);
//        if (wm->getIsSimulMode())
//            oneTouchSkill->setKickSpeed(8);
//        else
//            oneTouchSkill->setKickSpeed(1023);
//        updated = false;
        receivePassSkill->setTarget(target);
        receivePassSkill->setReceiveradius(receiveRadius);
        if (ignoreAngle) {
            receivePassSkill->setIatargetdir(targetDir);
            receivePassSkill->setIgnoreangle(false);
        }
        updated = false;
        break;
    default:
        break;
    }
}

void CRolePlayOff::execute() {

    if (updated) {
        update();
    }

    switch (selectedSkill) {
    case RoleSkill::Gotopoint:
        break;
    case RoleSkill::GotopointAvoid:
        agent->action = gotoPointAvoidSkill;
        break;
    case RoleSkill::Kick:
        agent->action = kickSkill;
        DBUG(QString("[playoffrole] kickEXE : %2").arg(kickSpeed), D_MAHI);
        break;
    case RoleSkill::Mark:
        break;
    case RoleSkill::OneTouch:
        agent->action = oneTouchSkill;
        break;
    case RoleSkill::ReceivePass:
        agent->action = receivePassSkill;
        break;
    default:
        break;
    }

}

void CRolePlayOff::setUpdated(bool _updated) {
    updated = _updated;
}
bool CRolePlayOff::getUpdated() {
    return updated;
}

int CRolePlayOff::resetTime() {
    return timer.restart();
}

int CRolePlayOff::getElapsed() const {
    return timer.elapsed();
}
