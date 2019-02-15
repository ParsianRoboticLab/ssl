
#include <parsian_agent/receivepass.h>

#include "parsian_agent/receivepass.h"


CSkillReceivePass::CSkillReceivePass(Agent *_agent) : CSkill(_agent) {
    gotopointavoid = new CSkillGotoPointAvoid(_agent);

}

CSkillReceivePass::~CSkillReceivePass() {
    delete gotopointavoid;
}

RPMode CSkillReceivePass::decideMode() {
    Circle2D receiveCircle(target, receiveRadius);
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 10);
    drawer->draw(ballPath, QColor(Qt::yellow));
    if ((receiveCircle.intersection(ballPath) && wm->ball->vel.length() > 0.2)) { // TODO : Add Threshold
        return RPMode::RPINTERSECT;
    }

    return RPMode::RPWAITPOS;

}

void CSkillReceivePass::execute() {

    gotopointavoid->setSlowmode(slow);
    gotopointavoid->setNoavoid(false);
    gotopointavoid->setBallobstacleradius(0.4);

    RPMode receivePassMode = decideMode();
    switch (receivePassMode) {
        case RPMode::RPNONE:
            break;
        case RPMode::RPWAITPOS:
            waitPos();
            break;
        case RPMode::RPRECEIVE:
            receive();
            break;
        case RPMode::RPINTERSECT:
            intersect();
            break;

    }

    gotopointavoid->execute();

}

void CSkillReceivePass::waitPos() {

    gotopointavoid->setTargetpos((target.valid()) ? target : agent->pos());
    gotopointavoid->setTargetdir(wm->ball->pos - agent->pos());
    gotopointavoid->setTargetvel(Vector2D(0, 0));
    gotopointavoid->setSlowmode(false);
    agent->setRoller(0);

}

void CSkillReceivePass::intersect() {

    Vector2D bestPoint = bestPointToIntersect();
    drawer -> draw(bestPoint,QColor(Qt::black), 0.07);
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 20);
    if (!bestPoint.valid() || Circle2D(agent->pos(), 0.15).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)))) {
        bestPoint = ballPath.nearestPoint(agent->pos());
        drawer -> draw(bestPoint,QColor(Qt::blue), 0.07);

    }
    //validatePoint(bestPoint);
    agent->setRoller(1);

    gotopointavoid->setOnetouchmode(false);
    if(agent->pos().dist(bestPoint) < 0.5) gotopointavoid->setOnetouchmode(true);
    gotopointavoid->init(bestPoint, wm->ball->pos - bestPoint);
    gotopointavoid->setSlowmode(false);

    drawer -> draw(bestPoint,QColor(Qt::red), 0.07);
}

void CSkillReceivePass::receive() {
    Vector2D oneTouchDir = (wm->ball->pos - agent->pos()).norm();
    double tempDampSpeed = std::min((wm->ball->vel.length() - agent->vel().length()) * 0.05, 0.003);
    Vector2D tempVecDamp = (agent->pos() - wm->ball->pos).norm();
    Vector2D tempDampTarget = wm->ball->pos + (agent->pos() - wm->ball->pos).norm() * 0.10 + tempVecDamp * tempDampSpeed;
    gotopointavoid->init(tempDampTarget, oneTouchDir);
}

void CSkillReceivePass::validatePoint(Vector2D& _point) {
    validatePoint(_point, target);
}

Vector2D CSkillReceivePass::bestPointToIntersect() {
    return bestPointToIntersect(agent);
}

void CSkillReceivePass::validatePointFromPenalty(Vector2D &_point, const Rect2D& _penalty) {
    validatePointFromPenalty(_point, _penalty, target);
}

void CSkillReceivePass::validatePointFromPenalty(Vector2D &_point, const Rect2D &_penalty, const Vector2D &_target) {
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 20);
    Vector2D sol1, sol2;
    sol1.invalidate(); sol2.invalidate();

    if(_penalty.intersection(ballPath, &sol1, &sol2)) {

        if (sol1.x == wm->field->oppGoal().x || sol1.x == wm->field->ourGoal().x) sol1.invalidate();
        if (sol2.x == wm->field->oppGoal().x || sol2.x == wm->field->ourGoal().x) sol2.invalidate();

        if      (!sol1.isValid() && !sol2.isValid())    _point = _target;
        else if ( sol1.isValid() && !sol2.isValid())    _point = sol1;
        else if (!sol1.isValid() &&  sol2.isValid())    _point = sol2;
        else if (sol1.dist(_point) < sol2.dist(_point)) _point = sol1;
        else                                            _point = sol2;

    } else {

        ROS_WARN("Receive Point is in Penalty Area.");
    }
}

void CSkillReceivePass::validatePointOutofField(Vector2D &_point) {
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 20);
    Vector2D sol1, sol2;
    sol1.invalidate(); sol2.invalidate();

    if(wm->field->fieldRect().intersection(ballPath, &sol1, &sol2)) {
        _point = (sol1.valid()) ? sol1 : sol2;

    } else {
        ROS_WARN("Receive Point is in OUT OF FIELD.");
    }
}

Vector2D CSkillReceivePass::bestPointToIntersect(const Agent *_agent, const double& reachBeforeBall) {
    Vector2D best; best.invalidate();
    for (double i = 0 ; i < 5 ; i += 0.1) {
        const Vector2D& futureBall = wm->ball->getPosInFuture(i);
        double agentTime = CSkillGotoPointAvoid::timeNeeded(_agent, futureBall, conf->VelMax);
        if (agentTime < (i - reachBeforeBall)) {
            best = futureBall;
            break;
        }
    }
    return best;
}

void CSkillReceivePass::validatePoint(Vector2D &_point, const Vector2D &_default) {
    const Rect2D& biggerOppPenalty = wm->field->oppBigPenaltyArea(1, Robot::robot_radius_new, false);
    const Rect2D& biggerOurPenalty = wm->field->ourBigPenaltyArea(1, Robot::robot_radius_new, false);

    if (biggerOppPenalty.contains(_point)) validatePointFromPenalty(_point, biggerOppPenalty, _default);
    else if (biggerOurPenalty.contains(_point)) validatePointFromPenalty(_point, biggerOurPenalty, _default);
    else if (!wm->field->fieldRect().contains(_point)) validatePointOutofField(_point);


// TODO: Another Area Should be defined for this option [MAHI/2018/10/30]
//    Circle2D receiveArea{target, receiveRadius};
//    Vector2D sol1, sol2;
//    drawer->draw(receiveArea, QColor(Qt::cyan));
//    if (!receiveArea.contains(intersectPos)) {
//        receiveArea.intersection(ballPath, &sol1, &sol2);
//        if (sol2.dist(intersectPos) < sol1.dist(intersectPos)) {
//            sol1 = sol2;
//        }
//        intersectPos = sol1;
//    }
}
