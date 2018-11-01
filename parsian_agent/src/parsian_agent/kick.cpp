#include <parsian_agent/kick.h>
#include <parsian_agent/config.h>

CSkillKick::CSkillKick(Agent *_agent) : CSkill(_agent) {

    gpa = new CSkillGotoPointAvoid(_agent);
    gpa->setSlowmode(false);
    gpa->setAddvel(Vector2D(0, 0));
    angPid = new _PID(2, 0, 0, 0, 0);
    speedPid = new _PID(1, 0, 0, 0, 0);
    posPid = new _PID(1, 0, 0, 0, 0);

}

CSkillKick::~CSkillKick() {

    delete gpa;
    delete angPid;
    delete speedPid;
    delete posPid;
}


KMode CSkillKick::decideMode() {

    Polygon2D robotKickArea;

    double distCoef = (4.5 / wm->ball->pos.dist(target)) / 100;
    distCoef = std::min(distCoef, 0.02);
    distCoef = std::max(distCoef, 0.01);

    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.01 + agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.35 + agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.35 - agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.01 - agent->dir().rotate(90).norm()*distCoef);

    Circle2D dribblerArea(agent->pos() + agent->dir().norm() * 0.1, 0.25);
    if (passProfiler || kickWithCenterOfDribbler) {
        kickerOn = dribblerArea.contains(wm->ball->pos) && robotKickArea.contains(wm->ball->pos);
    } else {
        kickerOn = dribblerArea.contains(wm->ball->pos);
    }

    if (playMakeMode && wm->field->isInOurPenaltyArea(wm->ball->pos)) {
        return KMode::AVOIDOPPENALTY;
    }


    Circle2D robotArea(agent->pos(), 1);
    if (agent->pos().dist(wm->ball->pos) < 1.5) {
        robotArea.assign(agent->pos(), std::max(agent->pos().dist(wm->ball->pos) - 0.1, 0.01));
    }

    if (goalieMode) {
        robotArea.assign(agent->pos(), 0.5);
    }

    if (wm->ball->pos.dist(agent->pos()) < 1) {
        robotArea.assign(agent->pos(), max(wm->ball->pos.dist(agent->pos()) - 0.01, 0.01));
    }

    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * (12));
    if ((!penaltyKick) && wm->field->isInOppPenaltyArea(wm->ball->pos) && !passProfiler && avoidOppPenaltyArea && !((robotArea.intersection(ballPath) == 2 && wm->ball->vel.length() > 1))) {
        return KMode::AVOIDOPPENALTY;
    }

    if (dontKick) {
        return KMode::DONTKICK;
    }

    return KMode::DIRECT;


}

void CSkillKick::doNotKick() {

    Vector2D finalPos;
    gpa->setSlowmode(false);
    finalPos = wm->ball->pos - (target - wm->ball->pos).norm() * 0.3;
    gpa->setBallobstacleradius(0.4);
    if (fabs((kickFinalDir - agent->dir().th()).degree()) < 10) {
        agent->setRoller(0);
        finalPos = wm->ball->pos - (target - wm->ball->pos).norm() * 0.13;
        gpa->setSlowmode(true);
        gpa->setBallobstacleradius(0);
    }
    gpa->setDivemode(false);
    gpa->setNoavoid(false);
    gpa->init(finalPos, wm->ball->pos - agent->pos());
    gpa->execute();
}

void CSkillKick::direct() {
    findPosToGo();
}


void CSkillKick::avoidOurPenalty() {
    gpa->setSlowmode(slow);
    Vector2D finalPos , dummyPos1, dummyPos;
    Vector2D tempVector;
    tempVector = wm->ball->pos - wm->field->ourGoal();
    Circle2D penaltyCircle;
    Segment2D ballSeg;
    Segment2D ballPosSeg;
    ballPosSeg.assign(wm->field->ourGoal(), wm->field->ourGoal() + tempVector.norm() * 2);
    ballSeg.assign(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 10);
    penaltyCircle.assign(wm->field->ourGoal(), 1.8);


    penaltyCircle.intersection(ballPosSeg, &dummyPos1, &dummyPos);

    if (wm->field->isInField(dummyPos1)) {
        finalPos = dummyPos1;
    }
//    draw(finalPos);
    if (wm->ball->pos.dist(agent->pos()) > 0.2) {
        finalDirVec = target - agent->pos();
    }
    gpa->setDivemode(false);
    gpa->setAvoidpenaltyarea(true);
    gpa->setNoavoid(false);

    gpa->init(finalPos, finalDirVec);
    gpa->execute();
//    agent->setKick(kickSpeed);
}

void CSkillKick::avoidOppPenalty() {
    gpa->setSlowmode(slow);
    Vector2D finalPos , dummyPos1, dummyPos;
    Circle2D penaltyCircle;
    Segment2D ballSeg;
    Segment2D ballPosSeg;
    Segment2D penaltyStraightLine;

    Rect2D penalty = wm->field->oppPenaltyRect();
    //drawer->draw(penalty , QColor(Qt::red));
    ballSeg.assign(wm->field->oppGoal(),wm->field->oppGoal() + (wm->ball->pos - wm->field->oppGoal()).norm()* 10);

    if(wm->ball->vel.length() > 0.3 && !wm->field->isInOppPenaltyArea(wm->ball->getPosInFuture(1000)) && wm->field->isInField(wm->ball->getPosInFuture(1000))) {
        ballSeg.assign(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*10);
    }
    penalty.intersection(ballSeg,&dummyPos,&dummyPos1);
    if(dummyPos == wm->field->oppGoal()) {
        finalPos = dummyPos1 + (wm->ball->pos - wm->field->oppGoal()).norm()* 0.1;
    } else {
        finalPos = dummyPos + (wm->ball->pos - wm->field->oppGoal()).norm()* 0.1;
    }

    drawer->draw(finalPos , QColor(Qt::red));

    gpa->setAvoidpenaltyarea(true);
    gpa->init(finalPos, finalDirVec);
    gpa->execute();
//    agent->setKick(kickSpeed); // TODO : Robot Command
}

void CSkillKick::indirect() {

    if (shotEmptySpot) {
        target = findMostPossible();
    }
    Vector2D sol1, sol2;

    Circle2D ballArea(wm->ball->pos, 0.15);
    ballArea.tangent(agent->pos(), &sol1, &sol2);
    Vector2D finalPos;
    finalPos = wm->ball->pos - (target - wm->ball->pos).norm() * 0.15;

    if (sol1.dist(finalPos) >= sol2.dist(finalPos)) {
        sol1 = sol2;
    }

    if (wm->ball->vel > 0.3) {
        for (double i = 0 ; i < 5 ; i += 0.1) {
            if (wm->ball->getPosInFuture(i).dist(agent->pos()) / 1 < i) {
                sol1 = wm->ball->getPosInFuture(i) - (target - wm->ball->getPosInFuture(i)).norm() * 0.15;
                break;
            }
        }
    }

    gpa->setSlowmode(slow);
    gpa->setDivemode(false);
    gpa->init(sol1  , target - sol1);
    gpa->execute();
}

void CSkillKick::jTurn() {

    if (shotEmptySpot) {
        target = findMostPossible();
    }
    //    //// vars
    bool isFinalController = false;
    double posPidKp = 1;
    double speedPidKp = 1;
    Vector2D targetForJturnSpeed, targetForJturnPos;
    Vector2D idealPass;
    Vector2D movementThSpeed, movementThPos;
    double movementDir = ((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree();
    double shift = 0;
    double distCoef = 0.15;

    idealPass = (wm->ball->pos - agent->pos()).norm() * distCoef;

    posPid->error = 0;
    posPid->kd = 0;
    if (movementDir < 17 && movementDir > -17) {
        shift = 0;

    } else if (movementDir > 50) {
        shift = 25 + (1 - agent->pos().dist(wm->ball->pos)) * 61;
    } else if (movementDir < -50) {
        shift = -25 - (1 - agent->pos().dist(wm->ball->pos)) * 61;
    } else if (movementDir > 30) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = 30 + (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = 20 + (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = 20 + (1 - agent->pos().dist(wm->ball->pos)) * 30;
        }
        distCoef = 0.17;
    } else if (movementDir < -30) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = -30 - (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = -20 - (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = -20 - (1 - agent->pos().dist(wm->ball->pos)) * 30;
        }

        distCoef = 0.17;
    } else if (movementDir > 0) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = 25 + (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 12;
        }
        distCoef = 0.17;
    } else if (movementDir < 0) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = -25 - (1 - agent->pos().dist(wm->ball->pos)) * 20;
        }
        else if (wm->ball->vel.length() < 0.1) {
            shift = -15 - (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = -15 - (1 - agent->pos().dist(wm->ball->pos)) * 12;
        }

        distCoef = 0.17;
    }

    idealPass.rotate(shift);
    targetForJturnSpeed = agent->pos() + idealPass;

    movementThSpeed = (targetForJturnSpeed - agent->pos()).norm();
    double dirReduce;
    speedPid->error = targetForJturnSpeed.dist(agent->pos());

    ////////////set Active adaptive PIDs

    dirReduce = (fabs(movementDir)/50) *(fabs(movementDir)/50);
    if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15) &&
        agent->pos().dist(wm->ball->pos) < 0.35) {
        dirReduce -= 2;
    }
    if(isPlayoff) {
        dirReduce -= 1;
    }
    drawer->draw(QString("error: %1").arg(posPid->error),Vector2D(2,2));
    //posPid->kp = 0.001;
    speedPid->kp = 6 + 4 * agent->pos().dist(wm->ball->pos) + dirReduce*2 ;

    if (penaltyKick) {
        angPid->kp = 7;
        speedPid->kp = 4;
    }

    angPid->kp = 4.5;

    double vx = movementThSpeed.x * speedPid->PID_OUT();// + posPid->PID_OUT() * cos (agent->dir().th().radian() + _PI /2);
    double vy = movementThSpeed.y * speedPid->PID_OUT();// + posPid->PID_OUT() * sin (agent->dir().th().radian() + _PI /2);
    angPid->error = (kickFinalDir - agent->dir().th()).radian();
    agent->setRobotAbsVel(wm->ball->vel.x*1 + vx, wm->ball->vel.y*1 + vy, angPid->PID_OUT());
    speedPid->pError = speedPid->error;

    posPid->pError = posPid->error;

    //TODO: test this

    agent->accelerationLimiter(0, false);

}

void CSkillKick::turnForKick() {
    if (shotEmptySpot) {
        target = findMostPossible();
    }
    agent->setRoller(0);
    double angReduce = 1;

    if (isPlayoff) {
        if (fabs((agent->dir().th() - kickFinalDir).degree()) < 80) {
            angReduce = 0.7;
        }
        if ((agent->dir().th() - kickFinalDir).degree()  < - 10) {
            angPid->kp = 4 * angReduce;

            angPid->error = ((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian();
            agent->setRobotVel((-0.12 + agent->pos().dist(wm->ball->pos)) * 4 , -0.7 * angReduce, angPid->PID_OUT() + 3.5 * angReduce);


        } else if ((agent->dir().th() - kickFinalDir).degree()  > 10) {
            angPid->kp = 4 * angReduce;

            angPid->error = ((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian();
            agent->setRobotVel((-0.12 + agent->pos().dist(wm->ball->pos)) * 4, 0.7 * angReduce, angPid->PID_OUT() - 3.5 * angReduce) ;
        }


    } else {
        angReduce = 1;
        if (fabs((agent->dir().th() - kickFinalDir).degree()) < 80) {
            angReduce = 0.5;
        }

        agent->setRoller(1);
        if ((agent->dir().th() - kickFinalDir).degree()  < - 10) {
            angPid->kp = 4 * angReduce;

            angPid->error = ((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian();
            agent->setRobotVel(1, -0.7 * angReduce, angPid->PID_OUT() + 3 * angReduce);
            agent->accelerationLimiter(0,false);

        } else if ((agent->dir().th() - kickFinalDir).degree()  > 10) {
            angPid->kp = 4 * angReduce;

            angPid->error = ((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian();
            agent->setRobotVel(1, 0.7 * angReduce, angPid->PID_OUT() - 3* angReduce) ;
            agent->accelerationLimiter(0,false);
        }
    }
}

Vector2D CSkillKick::findMostPossible() {

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
    //debug(QString("prob: %1 , angle :%2, biggest:%3").arg(prob).arg(angle).arg(biggestAngle),D_MHMMD);

    Segment2D goalSeg(wm->field->oppGoalL(), wm->field->oppGoalR());
    Vector2D sol1, sol2;
    //    debug(QString("ang %1").arg(angle),D_MHMMD);
//    draw(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI*(angle)/180),sin(_PI*(angle)/180)).norm()*12));

    return  goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI * (angle) / 180), sin(_PI * (angle) / 180)).norm() * 12));
}


double CSkillKick::oneTouchAngle(Vector2D pos,
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

void CSkillKick::findPosToGo() {
    Circle2D  agentNearArea(agent->pos(), 0.15);
    Vector2D sol1, sol2;
    QList<int> ourRelax, oppRelax;
    double agentTime = 0;
    Vector2D finalDir;
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 100);
    Circle2D dribblerArea(agent->pos() + agent->dir().norm() * 0.1, 0.25);
    Circle2D robotArea(agent->pos(), 1);
    gpa->setAddvel(Vector2D(0, 0));
    kickerArea.assign(agent->pos() + agent->dir().norm() * 0.09 , 0.15);
    Segment2D kickerSeg(agent->pos() + agent->dir().norm() * 0.08 + agent->dir().rotate(90).norm() * 0.02 , agent->pos() + agent->dir().norm() * 0.08 - agent->dir().rotate(90).norm() * 0.02);
    Vector2D dummy;
    Segment2D targetNormalSeg(target + wm->ball->vel.norm().rotate(90) * 10, target - wm->ball->vel.norm().rotate(90) * 10);
    Vector2D kickerPoint = agent->pos() + agent->dir().norm() * 0.08;
    Vector2D addVec = agent->dir().norm() * 0.08;
    gpa->setOnetouchmode(false);

    if (wm->ball->vel.length() > 0.5 - distThr  ) {
        distThr = 0.45;
        if(isKhafan)
            distThr = 0.45;
        if (Circle2D(agent->pos(), 0.1).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)), &dummy, &dummy)) {
            gpa->setOnetouchmode(false);
            finalPos = ballPath.nearestPoint(kickerPoint);

        } else {
            bool posFound  = false;
            for (double i = 0.5 ; i < 5 ; i += 0.1) {
                finalPos = wm->ball->getPosInFuture(i);// - (target-wm->ball->getPosInFuture(i)).norm()*0.15;
                QList <int> dummy;
                agentTime = CSkillGotoPointAvoid::timeNeeded(agent, finalPos - addVec, conf->VelMax);


                if (agentTime < (i - (0.5))) {
                    posFound  = true;
                    break;
                }
            }

            if (posFound == false  /*intersectPos.dist(wm->ball->pos) > ballPath.nearestPoint(kickerPoint).dist(wm->ball->pos) ||*/ /*!wm->field->isInField(finalPos + addVec)*/) {
                finalPos = ballPath.nearestPoint(kickerPoint);
            }

        }
        finalPos = finalPos - addVec;
        Vector2D s1,s2;
        if(!wm->field->isInField(finalPos)) {
            wm->field->fieldRect().intersection(ballPath,&s1,&s2);
            if(!s1.isValid())
                finalPos = s2 - addVec;
            else
                finalPos = s1 - addVec;
        }
        if (Circle2D(agent->pos(), 0.1).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)), &dummy, &dummy)) {
            if (fabs(((target - agent->pos()).th().degree() - (wm->ball->pos - agent->pos()).th().degree())) < 60 || isKhafan) {
                finalDir = Vector2D::unitVector(
                        oneTouchAngle(agent->pos(), agent->vel(), wm->ball->vel, agent->pos() - wm->ball->pos, target,
                                      conf->Landa,
                                      conf->Gamma));
            } else {
                finalDir = wm->ball->pos - finalPos;
            }

        } else {
            if (fabs(((target - finalPos).th().degree() - (wm->ball->pos - finalPos).th().degree())) < 60 || isKhafan) {
                finalDir = target - finalPos;
            } else {
                finalDir = wm->ball->pos - finalPos;
            }

        }


        //   drawer->draw(QString("agentT : %1").arg(agentTime) , Vector2D(1,-1));
        if ((ballPath.intersection(targetNormalSeg).isValid())  && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
            finalDir = target - agent->pos();
            // finalPos = wm->ball->pos - (target - finalPos).norm() * 0.15;

        }

        //TODO : penalty area
        if(((agent->pos().dist(wm->ball->pos + wm->ball->vel*0.5) < 0.6) && isKhafan)) {
            jTurn();
            return;
        }
        if ((ballPath.intersection(targetNormalSeg).isValid()) && ((agent->pos().dist(wm->ball->pos) < 1) || isKhafan) && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
            jTurn();
            return;
        }

    } else {
        distThr = 0;
        if (wm->ball->pos.dist(agent->pos()) < 0.6) {
            if (fabs((kickFinalDir - agent->dir().th()).degree()) > 30 && kickerArea.contains(wm->ball->pos)) {
                turnForKick();
            } else {
                jTurn();
            }
            return;
        }
        finalPos = wm->ball->pos - (target - finalPos).norm() * 0.15;
        finalDir = Vector2D(cos(kickFinalDir.radian()), sin(kickFinalDir.radian()));
    }

    if(finalPos.x >wm->field->_FIELD_WIDTH/2 -  wm->field->_PENALTY_DEPTH - 0.1 && fabs(finalPos.y) < wm->field->_PENALTY_WIDTH/2 +0.1 ) {
        if(wm->field->oppBigPenaltyArea(1,0.1,0).intersection(ballPath,&sol1,&sol2)) {
            if(sol1.dist(finalPos) > sol2.dist(finalPos)) {
                if(sol2.x != wm->field->oppGoal().x) {
                    sol1 = sol2;
                }
            }
            if(sol1.x == wm->field->oppGoal().x)
                sol1 = sol2;
            finalPos = sol1;
        }
        finalDir = wm->ball->pos - finalPos;
    }
    if(finalPos.x < -1 * wm->field->_FIELD_WIDTH/2 +  wm->field->_PENALTY_DEPTH + 0.1 && fabs(finalPos.y) < wm->field->_PENALTY_WIDTH/2 +0.1 ) {
        if(wm->field->ourBigPenaltyArea(1,0.1,0).intersection(ballPath,&sol1,&sol2)) {
            // drawer->draw(wm->field->ourBigPenaltyArea(1,0.1,0),QColor(Qt::red),true);
            if(sol1.dist(finalPos) > sol2.dist(finalPos)) {
                if(sol2.x >= -1 * wm->field->_FIELD_WIDTH/2 + 0.02) {
                    sol1 = sol2;
                }
            }
            if(sol1.x == wm->field->ourGoal().x)
                sol1 = sol2;
            finalPos = sol1;
        }
        finalDir = wm->ball->pos - finalPos;
    }

    Vector2D s1, s2;
    Circle2D finalPosArea;
    Segment2D directPath(agent->pos(), finalPos);
    drawer->draw(directPath);
    finalPosArea.assign(wm->ball->pos , 0.145);

    if (!((ballPath.intersection(targetNormalSeg).isValid())  && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60) )&&(finalPosArea.intersection(directPath, &s1, &s2))) {
        finalPosArea.assign(wm->ball->pos , 0.245);
        finalPosArea.tangent(agent->pos(), &s1, &s2);
        if (s2.dist(target) >= s1.dist(target)) {
            s1 = s2;
        }
        s1 = s1 + (s1 - agent->pos()).norm() * (finalPos.dist(wm->ball->pos)) * 1.5;
        finalPos = s1;
    }
    drawer->draw(Segment2D(agent->pos(), finalPos), QColor(Qt::red));

    drawer->draw(finalPos);

    gpa->init(finalPos, finalDir);
    gpa->setNoavoid(false);
    gpa->setAvoidpenaltyarea(true);
    gpa->setBallobstacleradius(0);
    gpa->setSlowmode(slow);
    gpa->setDivemode(false);
    gpa->setAvoidpenaltyarea(true);
    gpa->execute();

}

void CSkillKick::execute() {


    if (shotEmptySpot) target = findMostPossible();
    drawer->draw(target,QColor(Qt::cyan));
    finalDirVec = target - wm->ball->pos;
    kickFinalDir = finalDirVec.th();

    KMode kickMode = decideMode();
    switch (kickMode) {
        case KMode::DIRECT:
            direct();
            break;
        case KMode::AVOIDOPPENALTY:
            avoidPenalty();
            break;
        case KMode::DONTKICK:
            doNotKick();
            break;
        case KMode::NOMODE:break;
    }

    validateKickerState();

}

void CSkillKick::avoidPenalty() {
    if (wm->ball->pos.x  > 0) {
        avoidOppPenalty();
    } else {
        avoidOurPenalty();
    }
}

void CSkillKick::validateKickerState() {
    agent->setRoller((kickerOn) ? spin : 0);
    if (chip) agent->setChip(kickSpeed);
    else      agent->setKick(kickSpeed);

    AngleDeg kickDir = (target - wm->ball->pos).th();
    if (!kickerOn ||
        dontKick  ||
        (veryFine && std::fabs((agent->dir().th() - kickDir).degree()) > 2) ||
        (std::fabs((agent->dir().th() - kickDir).degree()) > tolerance && std::fabs((agent->dir().th() - kickDir).degree()) > 3)) {

        agent->setKick(0);
        agent->setChip(0);
    }
}
