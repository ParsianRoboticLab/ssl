//
// Created by atiyeh on 3/27/19.
//

#include "parsian_agent/moveforward.h"

CSkillMoveForward::CSkillMoveForward(Agent *_agent) : CSkill(_agent) {
    recPass = new CSkillReceivePass(_agent);
    Kick = new CSkillKick(_agent);}

CSkillMoveForward::~CSkillMoveForward(){
    delete recPass;
    delete Kick; }

void CSkillMoveForward::kickForward() {
    setKickchargetime(100);
    setSpin(1);
    Kick->execute();
    };

MFMode CSkillMoveForward::decideMode() {
    Vector2D destPos = getTarget();
    Vector2D originPos = getWaitreceivepos();
    Circle2D receiveAreaCircle(originPos, 0.5); // 0.5 is the radius within which RP is active
    Circle2D startMoveCircle(originPos, 0.05 /*receive radius*/);
    Circle2D destCircle(destPos, 0.03);
    if ((receiveAreaCircle.contains(wm->ball->pos) && wm->ball->vel.length() > 0.2)) {return MFMode::RECEIVE;};
    if(wm->ball->vel.length() < 0.2 && startMoveCircle.contains(wm->ball->pos)) {return MFMode::KICKFORWARD;};
    if(destCircle.contains(agent->pos()) && destCircle.contains(wm->ball->pos)) {return MFMode::WAITHERE;};
}


void CSkillMoveForward::execute() {
    MFMode moveForwardMode = decideMode();
    switch (moveForwardMode){
        case MFMode::RECEIVE:
            recPass->execute();
            break;
        case MFMode::KICKFORWARD:
            kickForward();
            break;
        case MFMode::WAITHERE:
            agent->waitHere();
            break;
    };}


