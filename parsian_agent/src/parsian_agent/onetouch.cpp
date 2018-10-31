//
// Created by parsian-ai on 9/21/17.
//

#include <parsian_agent/onetouch.h>



CSkillKickOneTouch::CSkillKickOneTouch(Agent *_agent) : CSkill(_agent) {
    gotopointavoid = new CSkillGotoPointAvoid(agent);
    kickSkill = new CSkillKick(_agent);
    timeAfterForceKick = new QTime();
    timeAfterForceKick->start();
}

CSkillKickOneTouch::~CSkillKickOneTouch() {
    delete gotopointavoid;
    delete kickSkill;
    delete timeAfterForceKick;
}

Vector2D CSkillKickOneTouch::findMostPossible() {

    QList <Circle2D> obstacles;
    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
        obstacles.append(Circle2D(wm->opp.active(i)->pos, Robot::robot_radius_new + 0.01));
    }
    for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        if (wm->our.active(i)->id != agent->id()) {
            obstacles.append(Circle2D(wm->our.active(i)->pos, Robot::robot_radius_new + 0.01));
        }
    }

    double prob, angle, biggestAngle;
    CKnowledge::getEmptyAngle(*wm->field, wm->ball->pos - (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15, wm->field->oppGoalL(), wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);
    Segment2D goalSeg(wm->field->oppGoalL(), wm->field->oppGoalR());
    return goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI * (angle) / 180), sin(_PI * (angle) / 180)).norm() * 12));
}


void CSkillKickOneTouch::execute() {
    gotopointavoid->setOnetouchmode(false);
    gotopointavoid->setNoavoid(false);

    if (shotToEmptySpot) target = findMostPossible();
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
    gotopointavoid->init(waitPos, oneTouchDir);
    gotopointavoid->execute();
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

    validatePoint(intersectPos);

    Vector2D addVec = (intersectPos - target).norm() * stopParam;
    Vector2D oneTouchDir = Vector2D::unitVector(CKnowledge::oneTouchAngle(agent->pos(), agent->vel(), wm->ball->vel, agent->pos() - wm->ball->pos, target, conf->Landa, conf->Gamma, 6.5));
    gotopointavoid->init(intersectPos + addVec, oneTouchDir);
    gotopointavoid->setNoavoid(true);
    gotopointavoid->setOnetouchmode(true);
    gotopointavoid->setNoavoid(true);
    gotopointavoid->execute();
    drawer->draw(intersectPos);

    if (agent->pos().dist(wm->ball->pos) < 1) {
        if (chip) agent->setChip(kickSpeed);
        else agent->setKick(kickSpeed);
    }
    agent->setRoller(0);
}

void CSkillKickOneTouch::validatePoint(Vector2D &_point) {
    const Rect2D& biggerOppPenalty = wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, false);
    const Rect2D& biggerOurPenalty = wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, false);

    if (biggerOppPenalty.contains(_point)) CSkillReceivePass::validatePointFromPenaltyWithTarget(_point, biggerOppPenalty, waitPos);
    else if (biggerOurPenalty.contains(_point)) CSkillReceivePass::validatePointFromPenaltyWithTarget(_point, biggerOurPenalty, waitPos);
    else if (!wm->field->fieldRect().contains(_point)) CSkillReceivePass::validatePointOutofField(_point);

}
