#include "parsian_ai/roles/dynamicrole.h"

CRoleDynamic::CRoleDynamic() {
    shotSkill     = new KickAction;
    receiveSkill  = new ReceivepassAction;
    moveSkill     = new GotopointavoidAction;
    oneTouchSkill = new OnetouchAction;
    positionSkill = PositionSkill ::NoSkill;
    playMakeSkill = PlayMakeSkill ::NoSkill;
    isplaymake = false;

    agent = nullptr;
}

CRoleDynamic::CRoleDynamic(const CRoleDynamic &_copy) {
    shotSkill     = new KickAction;
    receiveSkill  = new ReceivepassAction;
    moveSkill     = new GotopointavoidAction;
    oneTouchSkill = new OnetouchAction;
    positionSkill = _copy.positionSkill;
    playMakeSkill = _copy.playMakeSkill;
    agent = _copy.agent;
//    shotSkill->setMessage(_copy.shotSkill->getMessage());
}

CRoleDynamic::CRoleDynamic(CRoleDynamic && _move) noexcept {
    shotSkill     = new KickAction;
    receiveSkill  = new ReceivepassAction;
    moveSkill     = new GotopointavoidAction;
    oneTouchSkill = new OnetouchAction;
    positionSkill = _move.positionSkill;
    playMakeSkill = _move.playMakeSkill;
    agent = _move.agent;
    shotSkill->setMessage(_move.shotSkill->getMessage());
}

CRoleDynamic::~CRoleDynamic() {
    delete shotSkill;
    delete receiveSkill;
    delete moveSkill;
    delete oneTouchSkill;
}

void CRoleDynamic::update() {
    updated = false;

    shotSkill->setPlaymakemode(true);
    shotSkill->setKickwithcenterofdribbler(true);
    if(!isplaymake)
    {
        switch (positionSkill) {
        case PositionSkill ::Ready:
            receiveSkill->setTarget(target);
            receiveSkill->setReceiveradius(receiveRadius);
            break;
        case PositionSkill ::Move:
            moveSkill->setTargetpos(target);
            moveSkill->setTargetdir(targetDir);
            moveSkill->setAvoidpenaltyarea(true);
            moveSkill->setSlowmode(false);
            break;
        case PositionSkill ::OneTouch:
            oneTouchSkill->setWaitpos(waitPos);
            oneTouchSkill->setTarget(target);
            break;
        case PositionSkill ::NoSkill:
        default:
            break;
        }
    }
    if(isplaymake)
    {
        switch (playMakeSkill) {
        case PlayMakeSkill ::Shot:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setAvoidopppenaltyarea(true);
            shotSkill->setPlaymakemode(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryfine(veryFine);
            shotSkill->setDontkick(false);
            if(isdischargetime)
            {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            }
            else
            {
                shotSkill->setIskickchargetime(false);
                if(chip)
                {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                }
                else
                {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
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
            if(isdischargetime)
            {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            }
            else
            {
                shotSkill->setIskickchargetime(false);
                if(chip)
                {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                }
                else
                {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill ::Pass:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setAvoidopppenaltyarea(true);
            shotSkill->setPlaymakemode(true);
            shotSkill->setChip(chip);
            shotSkill->setDontkick(noKick);
            shotSkill->setVeryfine(veryFine);
            if(isdischargetime)
            {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            }
            else
            {
                shotSkill->setIskickchargetime(false);
                if(chip)
                {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                }
                else
                {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill ::CatchBall:
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidpenaltyarea(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryfine(false);
            if(isdischargetime)
            {
                shotSkill->setIskickchargetime(true);
                shotSkill->setKickchargetime(static_cast<double>(dischargetime));
            }
            else
            {
                shotSkill->setIskickchargetime(false);
                if(chip)
                {
                    shotSkill->setChipdist(chipdist);
                    shotSkill->setKickspeed(0);
                }
                else
                {
                    shotSkill->setKickspeed(kickSpeed);
                    shotSkill->setChipdist(0);
                }
            }
            break;
        case PlayMakeSkill ::Keep:
            break;
        case PlayMakeSkill ::NoSkill:
        default:
            break;
        }
    }

}

void CRoleDynamic::execute() {

    /*if (updated) {
        update();
    }*/
    update();
    if(!isplaymake)
    {
        switch (positionSkill) {
        case PositionSkill ::Ready:
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: ready");
            agent->action = receiveSkill;
            break;
            DBUG(QString("[dynamicRole] kickSpeed : %1").arg(kickSpeed), D_MAHI);
            agent->action = shotSkill;
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: shotSkill");
            break;
        case PositionSkill::Move:
            agent->action = moveSkill;
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: move");
            break;
        case PositionSkill::OneTouch:
            agent->action = oneTouchSkill;
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: oneTouchSkill");
            break;
        case PositionSkill::NoSkill:
        default:
            agent->action = nullptr;
            break;
        }
    }
    if(isplaymake)
    {
        switch (playMakeSkill) {
        case PlayMakeSkill ::Shot:
        case PlayMakeSkill ::Chip:
        case PlayMakeSkill ::Pass:
        case PlayMakeSkill ::CatchBall:
            DBUG(QString("[dynamicRole] kickSpeed : %1").arg(kickSpeed), D_MAHI);
            ROS_INFO_STREAM("kian: akharesh: ID:" << agent->id() << ", action: playmake hame");
            agent->action = shotSkill;
            break;
        case PlayMakeSkill::NoSkill:
        default:
            agent->action = nullptr;
            break;
        }
    }
//    if(playMakeSkill == PlayMakeSkill::NoSkill && positionSkill == PositionSkill::NoSkill)
//        agent->action = nullptr;
}

CRoleDynamic& CRoleDynamic::operator=(CRoleDynamic &&_move) noexcept {
    return *this;
}

CRoleDynamic& CRoleDynamic::operator=(const CRoleDynamic& _copy) {
    return *this;
}

