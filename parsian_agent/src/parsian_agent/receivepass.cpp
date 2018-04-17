#include "parsian_agent/receivepass.h"

INIT_SKILL(CSkillReceivePass, "receivepass");

CSkillReceivePass::CSkillReceivePass(Agent *_agent) {
    agent = _agent;
    gotopointavoid = new CSkillGotoPointAvoid(_agent);
    slow = true;
    receiveRadius = 1.0;
    cirThresh = 0;
    kickCirThresh = 0;
    ignoreAngle = true;

}

double CSkillReceivePass::progress() {
    return 0.0;
}

CSkillReceivePass::~CSkillReceivePass() = default;

kkRPMode CSkillReceivePass::decideMode() {
    Circle2D tempCircle(target, 0.2 + cirThresh);
    Circle2D tempCircle2(target, receiveRadius);

    cirThresh = 1.0;
    Circle2D tempKickCircle(kkAgentPos, 0.3 + kickCirThresh);
    Segment2D tempBallPath(ballPos, ballPos + wm->ball->vel.norm() * 10);
    drawer->draw(tempBallPath, QColor(Qt::yellow));
    Vector2D sol1, sol2;
    if ((tempCircle2.intersection(tempBallPath, &sol1, &sol2) && wm->ball->vel.length() > 0.2)) {
        kickCirThresh = 0;
        return RPINTERSECT;
    }

    return RPWAITPOS;

}

void CSkillReceivePass::execute() {
    ballRealVel = wm->ball->vel.length(); // TODO : Check //knowledge->getRealBallVel();
    gotopointavoid->setSlowmode(slow);
    gotopointavoid->setAgent(agent);
    gotopointavoid->setNoavoid(false);
    gotopointavoid->setBallobstacleradius(0.4);
    ballPos = wm->ball->pos;
    kkAgentPos = agent->pos();
    receiveArea.assign(target, receiveRadius);
    receivePassMode = decideMode();
    Segment2D ballPath;
    Vector2D sol1, sol2;
    ballPath.assign(ballPos, ballPos + wm->ball->vel.norm() * (20));
    drawer->draw(receiveArea, QColor(Qt::cyan));


    Vector2D agentPos = agent->pos();
    Vector2D oneTouchDir;
    oneTouchDir = (ballPos - kkAgentPos).norm();


    //    Vector2D addVec = Vector2D(0.095*cos((target-kkAgentPos).th().radian()), 0.095*sin((target-kkAgentPos).th().radian()));
    Vector2D intersectPos;
    Line2D agentDirLine(kkAgentPos , kkAgentPos + oneTouchDir.norm());
    Line2D agentPerLine(kkAgentPos, kkAgentPos + oneTouchDir.norm());
    agentPerLine = agentDirLine.perpendicular(kkAgentPos);
    Line2D tempBallPath(ballPos, ballPos + wm->ball->vel.norm());
    double tempDampSpeed;

    Vector2D tempVecDamp, tempDampTarget;
    switch (receivePassMode) {
    case RPWAITPOS:
        if (target.valid()) {
            gotopointavoid->init(target, oneTouchDir);
        } else {
            gotopointavoid->init(agent->pos(), oneTouchDir);
        }
        agent->setRoller(0);
        gotopointavoid->setSlowmode(false);
        gotopointavoid->execute();
        DEBUG("RPWAITPOS", D_KK);
        break;
    case  RPDAMP:
    case RPRECEIVE:
        tempVecDamp = (kkAgentPos - ballPos).norm();
        tempDampSpeed = (ballRealVel - agent->vel().length()) * 0.05;
        if (tempDampSpeed > 0.003) {
            tempDampSpeed = 0.003;
        }
        tempDampTarget = ballPos + (kkAgentPos - ballPos).norm() * 0.10 + tempVecDamp * tempDampSpeed;
        gotopointavoid->init(tempDampTarget, oneTouchDir);
        gotopointavoid->execute();
        DEBUG("RPdamp-Back", D_KK);
        break;
    case RPINTERSECT:

        Vector2D kickerPoint = agent->pos();
        double agentTime = 0;
        Vector2D dummy;
            gotopointavoid->setOnetouchmode(false);
        if (Circle2D(agentPos, 0.15).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)), &dummy, &dummy)) {
            intersectPos = ballPath.nearestPoint(kickerPoint);

        } else {
            bool posFound  = false;
            for (double i = 0 ; i < 5 ; i += 0.1) {

                intersectPos = wm->ball->getPosInFuture(i);// - (target-wm->ball->getPosInFuture(i)).norm()*0.15;
                QList <int> dummy;
                agentTime = CSkillGotoPointAvoid::timeNeeded(agent, intersectPos, conf->VelMax, dummy, dummy, false, 0, true);


                if (agentTime < (i - (0.5 /*- min(wm->ball->vel.length(),4)/8 */))) {
                    posFound  = true;
                    break;
                }
            }
            Vector2D s1,s2;
            if (posFound == false || !wm->field->isInField(intersectPos )) {

                if (wm->field->fieldRect().intersection(ballPath,&s1,&s2)) {
                    if(s1.isValid()) {
                        intersectPos = s1;
                    } else {
                        intersectPos = s2;
                    }
                } else {
                    intersectPos = ballPath.nearestPoint(agentPos);
                }
            }
        }

        if (!receiveArea.contains(intersectPos)) {
            receiveArea.intersection(ballPath, &sol1, &sol2);
            if (sol2.dist(intersectPos) < sol1.dist(intersectPos)) {
                sol1 = sol2;
            }
            intersectPos = sol1;
        }
            if(intersectPos.x >wm->field->_FIELD_WIDTH/2 -  wm->field->_PENALTY_DEPTH - 0.1 && fabs(intersectPos.y) < wm->field->_PENALTY_WIDTH/2 +0.1 ) {
                if(wm->field->oppBigPenaltyArea(1,0.1,0).intersection(ballPath,&sol1,&sol2)) {
                    if(sol1.dist(intersectPos) > sol2.dist(intersectPos)) {
                        if(sol2.x != wm->field->oppGoal().x) {
                            sol1 = sol2;
                        }
                    }
                    if(sol1.x == wm->field->oppGoal().x)
                        sol1 = sol2;
                    intersectPos = sol1;
                }
            }
            if(intersectPos.x < -1 * wm->field->_FIELD_WIDTH/2 +  wm->field->_PENALTY_DEPTH + 0.1 && fabs(intersectPos.y) < wm->field->_PENALTY_WIDTH/2 +0.1 ) {
                if(wm->field->ourBigPenaltyArea(1,0.1,0).intersection(ballPath,&sol1,&sol2)) {
                   drawer->draw(wm->field->ourBigPenaltyArea(1,0.1,0),QColor(Qt::red),true);
                    if(sol1.dist(intersectPos) > sol2.dist(intersectPos)) {
                        if(sol2.x >= -1 * wm->field->_FIELD_WIDTH/2 + 0.02) {
                            sol1 = sol2;
                        }
                    }
                    if(sol1.x == wm->field->ourGoal().x)
                        sol1 = sol2;
                    intersectPos = sol1;
                }
            }
            drawer -> draw(intersectPos,QColor(Qt::red));
            if(agentPos.dist(intersectPos) < 0.5) {
                gotopointavoid->setOnetouchmode(true);
            }
            agent->setRoller(1);
            oneTouchDir = ballPos - intersectPos;
            gotopointavoid->init(intersectPos, oneTouchDir);
            gotopointavoid->setSlowmode(false);
            gotopointavoid->execute();
        break;

    }

}
