//
// Created by parsian-ai on 9/21/17.
//

#include <parsian_agent/onetouch.h>



CSkillKickOneTouch::CSkillKickOneTouch(Agent *_agent) : CSkill(_agent) {
    gotoPointAvoid = new CSkillGotoPointAvoid(agent);
    kickSkill = new CSkillKick(_agent);
    timeAfterForceKick = new QTime();
    timeAfterForceKick->start();
}

CSkillKickOneTouch::~CSkillKickOneTouch() {
    delete gotoPointAvoid;
    delete kickSkill;
    delete timeAfterForceKick;
}

void CSkillKickOneTouch::execute() {
    gotoPointAvoid->setOnetouchmode(false);
    gotoPointAvoid->setNoavoid(false);

    if (shotToEmptySpot) target = CSkillKick::findMostPossible(agent);
    if (!target.valid()) target = wm->field->oppGoal();
    if (!waitPos.isValid()) waitPos = agent->pos();

    OTMode mode = decideMode();

    switch (mode) {
        case OTMode::None:
        case OTMode::Wait:
            wait();
            break;
        case OTMode::Kick:
            kick();
            break;
        case OTMode::Intersect:
            intersect();
            break;
    }

}

OTMode CSkillKickOneTouch::decideMode() {

    double onetouchKickRad = 0.5;
    double stopParam = 0.085;
    double onetouchRad = std::min(2.0, wm->ball->pos.dist(agent->pos()) - stopParam);

    Circle2D oneTouchArea(agent->pos(), onetouchRad);
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 15);

    if (oneTouchArea.intersection(ballPath) && wm->ball->vel.length() > 0.25) {
        return OTMode::Intersect;
    } else if (wm->ball->pos.dist(agent->pos()) < onetouchKickRad) {
        return OTMode::Kick;
    } else {
        return OTMode::Wait;
    }
}

void CSkillKickOneTouch::wait() {
    Vector2D oneTouchDir = Vector2D::unitVector(CKnowledge::oneTouchAngle(agent->pos(), agent->vel(), wm->ball->vel, agent->pos() - wm->ball->pos, target, conf->Landa, conf->Gamma, 6.5));
    gotoPointAvoid->init(waitPos, oneTouchDir);
    gotoPointAvoid->execute();
    agent->setRoller(0);
}

void CSkillKickOneTouch::kick() {
    kickSkill->setTarget(target);
    kickSkill->setKickspeed(kickSpeed);
    kickSkill->setChip(chip);
    kickSkill->execute();
}

void CSkillKickOneTouch::intersect() {
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 15);
    Vector2D intersectPos = CSkillReceivePass::bestPointToIntersect(agent, reachBeforeBallTime);
    double stopParam = 0.085;
    Vector2D kickerPoint = agent->pos() + agent->dir().norm() * stopParam;
    if (!intersectPos.valid() || !fastestPoint || Circle2D(agent->pos(), 0.15).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(reachBeforeBallTime)))) {
        intersectPos = ballPath.nearestPoint(kickerPoint);
    }

    CSkillReceivePass::validatePoint(intersectPos, waitPos);

    Vector2D addVec = (intersectPos - target).norm() * stopParam;
    Vector2D oneTouchDir = Vector2D::unitVector(CKnowledge::oneTouchAngle(agent->pos(), agent->vel(), wm->ball->vel, agent->pos() - wm->ball->pos, target, conf->Landa, conf->Gamma, 6.5));
    gotoPointAvoid->init(intersectPos + addVec, oneTouchDir);
    gotoPointAvoid->setNoavoid(true);
    gotoPointAvoid->setOnetouchmode(true);
    gotoPointAvoid->setNoavoid(true);
    gotoPointAvoid->execute();
    drawer->draw(intersectPos);

    if (agent->pos().dist(wm->ball->pos) < 1) {
        if (chip) agent->setChip(kickSpeed);
        else agent->setKick(kickSpeed);
    }
    agent->setRoller(0);
}