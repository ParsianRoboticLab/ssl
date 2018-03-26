//
// Created by parsian-ai on 9/21/17.
//

#include "parsian_agent/onetouch.h"

INIT_SKILL(CSkillKickOneTouch, "kickonetouch");

CSkillKickOneTouch::CSkillKickOneTouch(Agent *_agent) : CSkill(_agent) {
    gotopointavoid = new CSkillGotoPointAvoid(agent);
    gotopointavoid->setAgent(_agent);
    kick = new CSkillKick(_agent);
    timeAfterForceKick = new QTime();
    timeAfterForceKick->start();
    waitPos.invalidate();
    kickSpeed = 800;
    chip = false;
}

CSkillKickOneTouch::~CSkillKickOneTouch() {
    delete gotopointavoid;
    delete kick;
    delete timeAfterForceKick;
}

double CSkillKickOneTouch::progress() {
    return 0.0;
}

double CSkillKickOneTouch::oneTouchAngle(Vector2D pos,
        Vector2D vel,
        Vector2D ballVel,
        Vector2D ballDir,
        Vector2D goal,
        double landa,
        double gamma) {
    float ang1 = (-ballDir).th().degree();
    float ang2 = (goal - pos).th().degree();
    float theta = AngleDeg::normalize_angle(ang2 - ang1);
    float th = fabs(theta) * _DEG2RAD;
    float vkick = 8; // agent->self()->kickValueSpeed(kickSpeed, false);// + Vector2D::unitVector(self().pos.d).innerProduct(self().vel);
    float v = (ballVel - vel).length();
    float th1 = th * 0.5;
    float f, fmin = 1e10;
    float th1best;
    for (int k = 0; k < 6000; k++) {
        th1 = ((float)k / 6000.0) * th;
        f  = gamma * v * (1.0 / tan(th - th1)) * sin(th1) - landa * v * cos(th1) - vkick;
        if (fabs(f) < fmin) {
            fmin = fabs(f);
            th1best = th1;
        }
    }
    th1 = th1best;
    th1 *= _RAD2DEG;
    AngleDeg::normalize_angle(th1);
    th  *= _RAD2DEG;
    float ang = 0;
    if (theta > 0) {
        ang = ang1 + th1;
    } else {
        ang = ang1 - th1;
    }

    return ang;
}


Vector2D CSkillKickOneTouch::findMostPossible() {

    QList<int> tempObstacles;
    QList <Circle2D> obstacles;
    obstacles.clear();
    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
        obstacles.append(Circle2D(wm->opp.active(i)->pos, 0.1));
    }

    for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        if (wm->our.active(i)->id != agent->id()) {
            obstacles.append(Circle2D(wm->our.active(i)->pos, 0.1));
        }
    }
    double prob, angle, biggestAngle;

    CKnowledge::getEmptyAngle(*wm->field, wm->ball->pos - (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15, wm->field->oppGoalL(), wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);
    DEBUG(QString("prob: %1 , angle :%2, biggest:%3").arg(prob).arg(angle).arg(biggestAngle), D_MHMMD);

    Segment2D goalSeg(wm->field->oppGoalL(), wm->field->oppGoalR());
    Vector2D sol1, sol2;
    DEBUG(QString("ang %1").arg(angle), D_MHMMD);
    drawer->draw(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI * (angle) / 180), sin(_PI * (angle) / 180)).norm() * 12));

    return  goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI * (angle) / 180), sin(_PI * (angle) / 180)).norm() * 12));
}


void CSkillKickOneTouch::execute() {
    gotopointavoid->setAgent(agent);
    gotopointavoid->setOnetouchmode(false);
    gotopointavoid->setNoavoid(false);

    if (shotToEmptySpot) {
        target = findMostPossible();
    }
    if (!target.valid()) {
        target = wm->field->oppGoal();
    }



    Vector2D ballPos = wm->ball->pos;
    Vector2D agentPos = agent->pos();

    Segment2D ballPath;
    double stopParam = 0.08;
    ballPath.assign(ballPos, ballPos + wm->ball->vel.norm() * 15);
    Segment2D ballLine;
    ballLine.assign(ballPos, ballPos + wm->ball->vel.norm() * (15));
    drawer->draw(ballPath, "red");
    Vector2D kickerPoint = agentPos + agent->dir().norm() * stopParam;



    Vector2D oneTouchDir = Vector2D::unitVector(oneTouchAngle(agentPos, agent->vel(), wm->ball->vel, agentPos - ballPos, target, conf->Landa, conf->Gamma));
    drawer->draw(QString("vball :%1").arg(conf->Landa), Vector2D(0, 0));
    //drawer->draw(Segment2D(Vector2D(0,0), Vector2D(0,0)+oneTouchDir.norm()), QColor(Qt::red));

    Vector2D addVec = (agentPos - target).norm() * stopParam;
    Vector2D intersectPos;
    Vector2D sol1, sol2;
    double onetouchRad = 1.5;
    double onetouchKickRad = 0.5;
    Circle2D oneTouchArea;
    Circle2D oppPenaltyArea(wm->field->oppGoal() + Vector2D(0.15, 0), 1.45);
    Circle2D oppPenaltyAreaWP(wm->field->oppGoal() + Vector2D(0.15, 0), 1.55);
    drawer->draw(oppPenaltyAreaWP, QColor(Qt::red));
    drawer->draw(oppPenaltyArea, QColor(Qt::red));

    if (ballPos.dist(agentPos) <= onetouchRad) {
        onetouchRad = ballPos.dist(agentPos) - stopParam;
    }
    oneTouchArea.assign(agentPos, onetouchRad);

    if ((wm->ball->vel.length() < 0.4 && agentPos.dist(ballPos) > onetouchKickRad) || ((oneTouchArea.intersection(ballPath, &sol1, &sol2) ==
            0) && wm->ball->vel.length() >= 0.4 && agentPos.dist(ballPos) > onetouchKickRad)) {
        if (!waitPos.isValid()) {
            waitPos = agentPos;
        }
        gotopointavoid->init(waitPos, oneTouchDir);
        gotopointavoid->execute();
        agent->setRoller(0);
    } else if ((oneTouchArea.intersection(ballPath, &sol1, &sol2) != 0) && wm->ball->vel.length() > 0.1) {
        gotopointavoid->setNoavoid(false);
        double agentTime = 0 ;
        drawer->draw(QString("agentT dif : %1").arg(kickerPoint.dist(wm->ball->getPosInFuture(0.5))) , Vector2D(2.5, -1));
        Vector2D dummy;
        if (!fastestPoint || Circle2D(agentPos, 0.1).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(reachBeforeBallTime)), &dummy, &dummy)) {
            intersectPos = ballPath.nearestPoint(kickerPoint);
        } else {
            bool posFound  = false;
            for (double i = 0 ; i < 5 ; i += 0.1) {

                intersectPos = wm->ball->getPosInFuture(i);// - (target-wm->ball->getPosInFuture(i)).norm()*0.15;
                QList <int> dummy;
                agentTime = CSkillGotoPointAvoid::timeNeeded(agent, intersectPos + addVec, conf->VelMax, dummy, dummy, false, 0, true);


                if (agentTime < (i - (reachBeforeBallTime))) {
                    posFound  = true;
                    break;
                }
            }

            if (posFound == false || /*intersectPos.dist(ballPos) > ballPath.nearestPoint(kickerPoint).dist(ballPos) ||*/ !wm->field->isInField(intersectPos + addVec)) {
                intersectPos = ballPath.nearestPoint(kickerPoint);
            }
        }
        //        intersectPos = ballPath.nearestPoint(kickerPoint);
        drawer->draw(QString("agentT : %1").arg(agentTime) , Vector2D(1, -1));


        if (wm->field->isInOppPenaltyArea(intersectPos) || oppPenaltyAreaWP.contains(waitPos)) {
            if (oppPenaltyArea.intersection(ballLine, &sol1, &sol2) != 0) {
                if (sol1.dist(waitPos) > sol2.dist(waitPos)) {
                    sol1 = sol2;
                }
                intersectPos = sol1;
            }
        }


        gotopointavoid->init(intersectPos + addVec, oneTouchDir);
        gotopointavoid->setNoavoid(true);
        gotopointavoid->setOnetouchmode(false);
        gotopointavoid->execute();
        drawer->draw(intersectPos);
        if (agentPos.dist(ballPos) < 1) {
            if (chip) {
                agent->setChip(kickSpeed);
            } else {
                agent->setKick(kickSpeed);
            }
        }
        agent->setRoller(3);
    } else if (ballPos.dist(agentPos) < onetouchKickRad) {
        kick->setAgent(agent);
        kick->setTarget(target);
        kick->setKickspeed(kickSpeed);
        kick->setChip(chip);
        kick->execute();
    } else {
        if (!waitPos.isValid()) {
            waitPos = agentPos;
        }
        gotopointavoid->init(waitPos, oneTouchDir);
        gotopointavoid->execute();
    }

    ballLastVel = wm->ball->vel;
    lastInterceptPos = intersectPos;
}
