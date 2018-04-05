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

    double normalSpeed = normalaizeKickSpeed();
    ROS_INFO_STREAM("playofffff: "<< normalSpeed);
    switch (selectedSkill) {
    case RoleSkill::Gotopoint:
        break;
    case RoleSkill::GotopointAvoid:
        gotoPointAvoidSkill->setTargetpos(target);
        gotoPointAvoidSkill->setTargetdir(targetDir);
        gotoPointAvoidSkill->setAvoidpenaltyarea(avoidPenaltyArea);
        gotoPointAvoidSkill->setMaxvelocity(maxVelocity);
        gotoPointAvoidSkill->setNoavoid(noAvoid);
        gotoPointAvoidSkill->setBallobstacleradius(0.18);

        updated = false;
        break;
    case RoleSkill::Kick:
        kickSkill->setIsplayoff(true);
        kickSkill->setTarget(target);
        kickSkill->setAvoidpenaltyarea(avoidPenaltyArea);
        kickSkill->setInterceptmode(intercept);
        kickSkill->setChip(chip);
            if(chip){
            kickSkill->setChipdist(normalSpeed);
            } else {
                kickSkill->setKickspeed(normalSpeed);
            }
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
        oneTouchSkill->setKickspeed(normalSpeed);
//        oneTouchSkill->setKickdischargetime(kickSpeed);
        oneTouchSkill->setIskickdischargetime(false); // true bud ba khat bala comment shod
        updated = false;
        break;
    case RoleSkill::ReceivePass:
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

double CRolePlayOff::normalaizeKickSpeed() {
    double normalSpeed = 0;

    if (kickSpeed < 0)          normalSpeed = 0;
    else if (kickSpeed <= 6.5)  normalSpeed = kickSpeed;
    else if (kickSpeed <= 12)   normalSpeed = 6.5;
    else if (kickSpeed <= 650)  normalSpeed = kickSpeed / 100.0;
    else if (kickSpeed > 650)   normalSpeed = 6.5;

    return normalSpeed;
}
