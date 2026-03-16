//
// Created by parsian-ai on 9/29/17.
//

#include <parsian_agent/gotopointavoid.h>
#include <parsian_agent/config.h>


CSkillGotoPointAvoid::CSkillGotoPointAvoid(Agent *_agent) : CSkill(_agent) {
    counter = 0;
    avoidPenaltyArea = static_cast<unsigned char>(true);
    gotopoint = new CSkillGotoPoint(_agent);
    bangBang = new CNewBangBang();
    dynamicStart = static_cast<unsigned char>(true);
    noAvoid = false;
    avoidCenterCircle = false;
    averageDir.assign(0, 0);
    addVel.assign(0, 0);
    diveMode = false;
    oneTouchMode = false;
    drawPath = false;
}

CSkillGotoPointAvoid::~CSkillGotoPointAvoid() {
    delete gotopoint;
    delete bangBang;
}

void CSkillGotoPointAvoid::execute() {

    double dVx, dVy, dW;
    bangBang->setDecMax(conf->DecMax);
    bangBang->setOneTouch(oneTouchMode);
    bangBang->setDiveMode(diveMode);
    double effectiveVelMax = conf->VelMax;
    if (maxVelocity > 0.0f) {
        effectiveVelMax = min(effectiveVelMax, static_cast<double>(maxVelocity));
    }
    if (slowMode) {
        bangBang->setVelMax(1.4);
        bangBang->setSlow(true);
    } else {
        bangBang->setSlow(false);
        bangBang->setVelMax(effectiveVelMax);
    }

    if (!Vector2D(targetPos).valid()) {
        agent->waitHere();
        return;
    }

    if (!targetVel.valid()) {
        targetVel.assign(0, 0);
    }

    if (drawPath) {
        if (agent->vel().length() < 0.1) {
            pathPoints.clear();
        } else {
            pathPoints.append(agent->pos());
            for (auto pathPoint : pathPoints) {
            }
        }
    } else {
        pathPoints.clear();
    }


    /////////////////
    if (targetPos.x < wm->field->ourCornerL().x - 0.2) {
        targetPos.x = wm->field->ourCornerL().x;
    }
    if (targetPos.x > wm->field->oppCornerL().x + 0.2) {
        targetPos.x = wm->field->oppCornerL().x;
    }
    if (targetPos.y < wm->field->ourCornerR().y - 0.2) {
        targetPos.y = wm->field->ourCornerR().y;
    }
    if (targetPos.y > wm->field->ourCornerL().y + 0.2) {
        targetPos.y = wm->field->ourCornerL().y;
    }


    if (lookAt.valid()) {
        targetDir = (lookAt - agent->pos()).norm();
    }

    if (lookAt.valid()) {
        targetDir = (lookAt - agent->pos()).norm();
    }

    QList<Vector2D> result;
    if (!noAvoid) {

        /*********** PLANNER ***************/
        ourRelaxList.clear();
        for (auto id : ourrelax) {
            ourRelaxList.append(static_cast<int>(id));
        }
        oppRelaxList.clear();
        for (auto id : theirrelax) {
            oppRelaxList.append(static_cast<int>(id));
        }
        agent->initPlanner(targetPos , ourRelaxList , oppRelaxList , avoidPenaltyArea , avoidCenterCircle , ballObstacleRadius);
        auto plannerSize = agent->pathPlannerResult.size();
        if (plannerSize > 0) {
            for (long i = static_cast<long>(plannerSize) - 1; i >= 0; --i) {
                result.append(agent->pathPlannerResult[static_cast<std::size_t>(i)]);
            }
        }
    }


    Vector2D dir(0, 0);

    double alpha = 0, vf = 0;
    Vector2D tempTarget;
    if (result.size() >= 3) {
        alpha = fabs(Vector2D::angleBetween(result[1] - result[0] , result[2] - result[1]).degree());
        tempTarget = result[1];

        vf = -1.8 * log(alpha) + 11.5 - (agent->vel().length()) * 1;
        vf = max(vf , 0.5);
        vf = min(vf, 4);
    } else {
        vf = 0;
        tempTarget = targetPos;
    }
    ////////////////////// avoid goal posts
    Segment2D goalPostL, goalPostR;
    goalPostL.assign(wm->field->ourGoalL() - Vector2D(0.2, 0), wm->field->ourGoalL() + Vector2D(0.1, 0));
    goalPostR.assign(wm->field->ourGoalR() - Vector2D(0.2, 0), wm->field->ourGoalR() + Vector2D(0.1, 0));
    Segment2D agentPath(agent->pos(), tempTarget);
    if (agentPath.intersection(goalPostL).isValid()) {
        tempTarget = wm->field->ourGoalL() + Vector2D(0.12, 0);
    } else if (agentPath.intersection(goalPostR).isValid()) {
        tempTarget = wm->field->ourGoalR() + Vector2D(0.12, 0);
    }
    /////////////////////
    if (noAvoid || result.size() < 3) {
        tempTarget = targetPos;
        vf = 0;
    }

    bangBang->setSmooth(true);// = false;

    bangBang->bangBangSpeed(agent->pos(), agent->vel(), agent->dir(), tempTarget, targetDir, vf, 0.016, dVx, dVy, dW);

    if (!addVel.isValid()) {
        addVel = Vector2D(0, 0);
    }
    agent->setRobotAbsVel(dVx + addVel.x, dVy + addVel.y, dW);
    agent->accelerationLimiter(vf, oneTouchMode);
    QList <int> dumm;

    drawer -> draw(QString("time : %1").arg(timeNeeded(agent, targetPos, conf->VelMax)), Vector2D(1, 1));

    counter ++;
    if(oneTouchFlag){
        if (agent->pos().dist(wm->ball->pos) < 1) {
            if (chip) {
                agent->setChip(chipDist);
            } else {
                agent->setKick(kickSpeed);
            }
        }
    }
}

void CSkillGotoPointAvoid::init(Vector2D target, Vector2D _targetDir, Vector2D _targetVel) {
    targetPos = target;
    targetDir = _targetDir;
    targetVel = _targetVel;
}

double CSkillGotoPointAvoid::timeNeeded(const Agent *_agentT, const Vector2D& posT,const double& vMax) {

    double dec = conf->DecMax;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double dist = 0;
    double tAgentVelTanjent =  tAgentVel.length() * cos(Vector2D::angleBetween(posT - _agentT->pos() , _agentT->vel().norm()).radian());
    double vXvirtual = (posT - _agentT->pos()).x;
    double vYvirtual = (posT - _agentT->pos()).y;
    double veltanV = (vXvirtual) * cos(tAgentDir.th().radian()) + (vYvirtual) * sin(tAgentDir.th().radian());
    double velnormV = -1 * (vXvirtual) * sin(tAgentDir.th().radian()) + (vYvirtual) * cos(tAgentDir.th().radian());
    double accCoef = atan(fabs(veltanV) / fabs(velnormV)) / _PI * 2;
    double acc = accCoef * conf->AccMaxForward + (1 - accCoef) * conf->AccMaxNormal;

    double tDec = vMax / dec;
    double tAcc = (vMax - tAgentVelTanjent) / acc;
    dist = posT.dist(_agentT->pos());
    double dB = tDec * vMax / 2 + tAcc * (vMax + tAgentVelTanjent) / 2;

    if (dist > dB) {
        return tAcc + tDec + (dist - dB) / vMax;
    } else {
        return ((1 / dec) + (1 / acc)) * sqrt(dist * (2 * dec * acc / (acc + dec)) + (tAgentVelTanjent * tAgentVelTanjent / (2 * acc))) - (tAgentVelTanjent) / acc;
    }

}
