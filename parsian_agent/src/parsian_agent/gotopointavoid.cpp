//
// Created by parsian-ai on 9/29/17.
//

#include <parsian_agent/gotopointavoid.h>
#include <parsian_agent/config.h>


INIT_SKILL(CSkillGotoPointAvoid, "gotopointavoid");


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
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::noRelax() {
    ourRelaxList.clear();
    oppRelaxList.clear();
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::ourRelax(int element) {
    if (!ourRelaxList.contains(element)) {
        ourRelaxList.append(element);
    }
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::oppRelax(int element) {
    if (!oppRelaxList.contains(element)) {
        oppRelaxList.append(element);
    }
    return this;
}

void CSkillGotoPointAvoid::execute()

{
    //drawer->draw(Circle2D(Vector2D(1,0),0.1),QColor(Qt::red),true);
    if (agent == nullptr) {
        return;
    }
    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    agentVel = agent->vel();
    double dVx, dVy, dW;
    bangBang->setDecMax(conf->DecMax);
    bangBang->setOneTouch(oneTouchMode);
    bangBang->setDiveMode(diveMode);
    if (slowMode) {
        bangBang->setVelMax(1.4);
        bangBang->setSlow(true);
    } else {
        bangBang->setSlow(false);
        bangBang->setVelMax(conf->VelMax);
    }

    if (!Vector2D(targetPos).valid()) {
        agent->waitHere();
        return;
    }

    if (!targetVel.valid()) {
        targetVel.assign(0, 0);
    }

    if (drawPath) {
        if (agentVel.length() < 0.1) {
            pathPoints.clear();
        } else {
            pathPoints.append(agentPos);
            for (auto pathPoint : pathPoints) {
                //    drawer->draw(Circle2D(pathPoint ,0.02),QColor(Qt::blue),true); // TODO : should uncommented
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
        targetDir = (lookAt - agentPos).norm();
    }

    //    knowledge->plotWidgetCustom[1] = agentVel.length();
    //    debug(QString("speed: %1").arg(agentVel.length()),D_MHMMD);
    ///////////

    //    if (targetPos.x < wm->field->ourCornerL().x - 0.2) targetPos.x = wm->field->ourCornerL().x;
    //    if (targetPos.x > wm->field->oppCornerL().x + 0.2) targetPos.x = wm->field->oppCornerL().x;
    //    if (targetPos.y < wm->field->ourCornerR().y - 0.2) targetPos.y = wm->field->ourCornerR().y;
    //    if (targetPos.y > wm->field->ourCornerL().y + 0.2) targetPos.y = wm->field->ourCornerL().y;

    //    if (false) { //conf()->LocalSettings_ParsianWorkShop()) {
    //        if(conf()->LocalSettings_OurTeamSide() == "Right")
    //        {
    //            if(targetPos.x < 0.2)
    //            {
    //                targetPos.x = 0.2;
    //            }
    //        }
    //        else
    //        {
    //            if(targetPos.x > 4.3)
    //            {
    //                targetPos.x = 4.3;
    //            }
    //        }
    //    }

    if (lookAt.valid()) {
        targetDir = (lookAt - agentPos).norm();
    }

    if (noAvoid) {
        result.clear();
    } else {
        /*********** PLANNER ***************/

        agent->initPlanner(targetPos , ourRelaxList , oppRelaxList , avoidPenaltyArea , avoidCenterCircle , ballObstacleRadius);
        result.clear();
        for (long i = agent->pathPlannerResult.size() - 1 ; i >= 0 ; i--) {
            // ROS_INFO_STREAM("POS : " << agent->pathPlannerResult[i].x << " , " << agent->pathPlannerResult[i].y);
            result.append(agent->pathPlannerResult[i]);
            //            drawer->draw(Circle2D(agent->pathPlannerResult[i],0.01),QColor(Qt::red));
        }
    }


    double dist = 0.0;
    bool flag = false;
    Vector2D dir(0, 0);

    if (result.size() > 1) {
        dir = (result[1] - result[0]).norm();
    }

    double D = 0 , alpha = 0 , d = 0 , vf = 0;
    Vector2D lllll ;
    if (!result.empty()) {
        lllll = result.last();
    }



    //        for( int i=1 ; i<result.size() ; i++ ){
    //            alpha = 0;
    //            if(fabs(Vector2D::angleBetween(result[i] - result[0] , result[i+1] - result[i]).degree()) > 3 ){
    //                if( i+1 == result.size() ){
    //                    vf = 0;
    //                    alpha = 0;
    //                }
    //                else{
    //                    alpha = fabs(Vector2D::angleBetween(result[i] - result[0] , result[i+1] - result[i]).degree());
    //                    vf = -1.0259280143 * log(alpha) + 4.570475303;
    //                    vf = max(vf , 0.5);
    //                }
    //                D = result[i].dist(result[0]);
    //                lllll = result[i];
    //                d = dist - D;
    //
    //
    //                flag = false;
    //                break;
    //            }
    //        }

    if (result.size() >= 3) {
        alpha = fabs(Vector2D::angleBetween(result[1] - result[0] , result[2] - result[1]).degree());
        DEBUG(QString("alpha : %1").arg(alpha), D_MHMMD);
        lllll = result[1];

        vf = -1.8 * log(alpha) + 11.5 - (agentVel.length()) * 1;
        vf = max(vf , 0.5);
        vf = min(vf, 4);
    } else {
        vf = 0;
        lllll = targetPos;
    }
    //    drawer->draw(QString("vf : %1").arg(vf),Vector2D(1,0));
    ////////////////////// avoid goal posts
    Segment2D goalPostL, goalPostR;
    goalPostL.assign(wm->field->ourGoalL() - Vector2D(0.2, 0), wm->field->ourGoalL() + Vector2D(0.1, 0));
    goalPostR.assign(wm->field->ourGoalR() - Vector2D(0.2, 0), wm->field->ourGoalR() + Vector2D(0.1, 0));
    Segment2D agenPath(agent->pos(), lllll);
    if (agenPath.intersection(goalPostL).isValid()) {
        lllll = wm->field->ourGoalL() + Vector2D(0.12, 0);
    } else if (agenPath.intersection(goalPostR).isValid()) {
        lllll = wm->field->ourGoalR() + Vector2D(0.12, 0);
    }
    /////////////////////


    if (noAvoid || result.size() < 3) {
        lllll = targetPos;
        vf = 0;
    }

    bangBang->setSmooth(true);// = false;

    bangBang->bangBangSpeed(agentPos, agentVel, agent->dir(), lllll, targetDir, vf, 0.016, dVx, dVy, dW);
    //    bangBang->bangBangSpeed(agentPos, agentVel, agent->dir(), lllll, lllll - agentPos, vf, 0.016, dVx, dVy, dW);

    if (!addVel.isValid()) {
        addVel = Vector2D(0, 0);
    }
    agent->setRobotAbsVel(dVx + addVel.x, dVy + addVel.y, dW);
    agent->accelerationLimiter(vf, oneTouchMode);
    // ROS_INFO_STREAM("vx: "<<dVx<<"vy: "<<dVy<<"w: "<< dW);
    // ROS_INFO_STREAM("x: "<<agentPos.x<<"y: "<<agentPos.y<<"w: "<< dW);
    QList <int> dumm;

    drawer -> draw(QString("time : %1").arg(timeNeeded(agent, targetPos, conf->VelMax, dumm, dumm, 0, 0, 0)), Vector2D(1, 1));

    counter ++;
    if(oneTouchFlag){
        if (agentPos.dist(ballPos) < 1) {
            if (chip) {
                agent->setChip(chipDist);
            } else {
                agent->setKick(kickSpeed);
            }
        }
    }
}

double CSkillGotoPointAvoid::progress() {
    if (agentPos.dist(targetPos) < 0.05) {
        return 1;
    }
    return 0;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::setTargetLook(Vector2D finalPos, Vector2D lookAtPoint) {
    setTargetpos(finalPos);
    setTargetdir(Vector2D{1.0, 0.0});
    setTargetvel(Vector2D(0.0, 0.0));
    setLookat(lookAtPoint);
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::setTarget(Vector2D finalPos, Vector2D finalDir) {
    setTargetpos(finalPos);
    setTargetdir(finalDir);
    setTargetvel(Vector2D(0.0, 0.0));
    setLookat(Vector2D::INVALIDATED);
    return this;
}

void CSkillGotoPointAvoid::init(Vector2D target, Vector2D _targetDir, Vector2D _targetVel) {
    targetPos = target;
    targetDir = _targetDir;
    targetVel = _targetVel;
}

double CSkillGotoPointAvoid::timeNeeded(Agent *_agentT, Vector2D posT, double vMax, QList <int> _ourRelax, QList <int> _oppRelax , bool avoidPenalty, double ballObstacleReduce, bool _noAvoid) {

    double _x3;
    double acc = conf->AccMaxForward;
    double dec = conf->DecMax;
    double xSat;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double veltan = (tAgentVel.x) * cos(tAgentDir.th().radian()) + (tAgentVel.y) * sin(tAgentDir.th().radian());
    double offset = 0;
    double velnorm = -1 * (tAgentVel.x) * sin(tAgentDir.th().radian()) + (tAgentVel.y) * cos(tAgentDir.th().radian());
    double distCoef = 1, distEffect = 1, angCoef = 0.003;
    double dist = 0;
    double rrtAngSum = 0;
    QList <Vector2D> _result;
    Vector2D _target;

    double tAgentVelTanjent =  tAgentVel.length() * cos(Vector2D::angleBetween(posT - _agentT->pos() , _agentT->vel().norm()).radian());
    /*if(_noAvoid)
    {
        _result.clear();
    }
    else
    {

        _agentT->initPlanner(posT,_ourRelax,_oppRelax,avoidPenalty,false,ballObstacleReduce);
        _result.clear();
        for(unsigned long i = _agentT->pathPlannerResult.size()-1 ; i>=0 ; i-- )
        {
            _result.append(_agentT->pathPlannerResult[i]);
        }
    }

    if( _result.size() >= 3) {
        for(int i = 0 ; i < _result.size() - 1; i++)
        {
            dist += _result[i].dist(_result[i+1]);
        }
        for(int i = 1 ; i < _result.size() - 1; i++)
        {
            rrtAngSum += fabs(Vector2D::angleBetween(_result[i] - _result[i-1] , _result[i+1] - _result[i]).degree());
        }
        distEffect = dist / _agentT->pos().dist(posT);
        distEffect += rrtAngSum*angCoef;
        distEffect = std::max(1.0, distEffect);
    }
    */
    double vXvirtual = (posT - _agentT->pos()).x;
    double vYvirtual = (posT - _agentT->pos()).y;
    double veltanV = (vXvirtual) * cos(tAgentDir.th().radian()) + (vYvirtual) * sin(tAgentDir.th().radian());
    double velnormV = -1 * (vXvirtual) * sin(tAgentDir.th().radian()) + (vYvirtual) * cos(tAgentDir.th().radian());
    double accCoef = 1, realAcc = 4;

    accCoef = atan(fabs(veltanV) / fabs(velnormV)) / _PI * 2;
    acc = accCoef * conf->AccMaxForward + (1 - accCoef) * conf->AccMaxNormal;

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
