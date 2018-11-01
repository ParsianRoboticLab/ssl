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
        return KMode::AvoidOurPenalty;
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
        return KMode::AvoidOppPenalty;
    }

    if (dontKick) {
        return KMode::DONTKICK;
    }
////////////
    Segment2D targetNormalSeg(target + wm->ball->vel.norm().rotate(90) * 10, target - wm->ball->vel.norm().rotate(90) * 10);
    if (wm->ball->vel.length() > 0.5 - distThr) {
        distThr = 0.45;
        //TODO : penalty area
        if(((agent->pos().dist(wm->ball->pos + wm->ball->vel*0.5) < 0.6) && isKhafan)) {
            return KMode::JTurn;
        }
        if ((ballPath.intersection(targetNormalSeg).isValid()) && ((agent->pos().dist(wm->ball->pos) < 1) || isKhafan) && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
            return KMode::JTurn;
        }
    } else {
        distThr = 0;
        if (wm->ball->pos.dist(agent->pos()) < 0.6) {
            if (fabs((kickFinalDir - agent->dir().th()).degree()) > 30 && kickerArea.contains(wm->ball->pos)) {
                return KMode::TurnForKick;
            } else {
                return KMode::JTurn;
            }
        }
    }

    return KMode::DIRECT;


}

void CSkillKick::doNotKick() {

    Vector2D finalPos;
    if (std::fabs((kickFinalDir - agent->dir().th()).degree()) < 10) {
        agent->setRoller(0);
        gpa->setSlowmode(true);
        gpa->setBallobstacleradius(0);
        finalPos = wm->ball->pos - (target - wm->ball->pos).norm() * 0.13;
    } else {
        agent->setRoller(spin);
        gpa->setSlowmode(false);
        gpa->setBallobstacleradius(0.4);
        finalPos = wm->ball->pos - (target - wm->ball->pos).norm() * 0.3;
    }
    gpa->setDivemode(false);
    gpa->setNoavoid(false);
    gpa->init(finalPos, wm->ball->pos - agent->pos());
    gpa->execute();
}

void CSkillKick::avoidOurPenalty() {
    gpa->setSlowmode(slow);
    Vector2D finalPos , dummyPos1, dummyPos;
    Vector2D tempVector = wm->ball->pos - wm->field->ourGoal();
    Circle2D penaltyCircle(wm->field->ourGoal(), 1.8);
    Segment2D ballSeg(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 10);
    Segment2D ballPosSeg(wm->field->ourGoal(), wm->field->ourGoal() + tempVector.norm() * 2);

    penaltyCircle.intersection(ballPosSeg, &dummyPos1, &dummyPos);

    if (wm->field->isInField(dummyPos1)) {
        finalPos = dummyPos1;
    }
    if (wm->ball->pos.dist(agent->pos()) > 0.2) {
        finalDirVec = target - agent->pos();
    }
    gpa->setDivemode(false);
    gpa->setAvoidpenaltyarea(true);
    gpa->setNoavoid(false);

    gpa->init(finalPos, finalDirVec);
    gpa->execute();
}

void CSkillKick::avoidOppPenalty() {
    gpa->setSlowmode(slow);
    Vector2D finalPos , dummyPos1, dummyPos;
    Segment2D ballSeg(wm->field->oppGoal(),wm->field->oppGoal() + (wm->ball->pos - wm->field->oppGoal()).norm()* 10);
    Rect2D penalty = wm->field->oppPenaltyRect();

    if(wm->ball->vel.length() > 0.3 && !wm->field->isInOppPenaltyArea(wm->ball->getPosInFuture(1000)) && wm->field->isInField(wm->ball->getPosInFuture(1000))) {
        ballSeg.assign(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*10);
    }
    penalty.intersection(ballSeg,&dummyPos,&dummyPos1);
    if(dummyPos.x == wm->field->oppGoal().x) {
        finalPos = dummyPos1 + (wm->ball->pos - wm->field->oppGoal()).norm()* 0.1;
    } else {
        finalPos = dummyPos + (wm->ball->pos - wm->field->oppGoal()).norm()* 0.1;
    }

    drawer->draw(finalPos , QColor(Qt::red));

    gpa->setAvoidpenaltyarea(true);
    gpa->init(finalPos, finalDirVec);
    gpa->execute();
    agent->setKick(kickSpeed);
}

void CSkillKick::jTurn() {

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
    } else if (movementDir < -30) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = -30 - (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = -20 - (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = -20 - (1 - agent->pos().dist(wm->ball->pos)) * 30;
        }

    } else if (movementDir > 0) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = 25 + (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 12;
        }
    } else if (movementDir < 0) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = -25 - (1 - agent->pos().dist(wm->ball->pos)) * 20;
        }
        else if (wm->ball->vel.length() < 0.1) {
            shift = -15 - (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = -15 - (1 - agent->pos().dist(wm->ball->pos)) * 12;
        }
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

void CSkillKick::direct() {
    Circle2D agentNearArea(agent->pos(), 0.15);
    Vector2D finalDir;
    Segment2D ballPath(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm() * 100);
    kickerArea.assign(agent->pos() + agent->dir().norm() * 0.09 , 0.15);
    Segment2D kickerSeg(agent->pos() + agent->dir().norm() * 0.08 + agent->dir().rotate(90).norm() * 0.02 , agent->pos() + agent->dir().norm() * 0.08 - agent->dir().rotate(90).norm() * 0.02);
    Segment2D targetNormalSeg(target + wm->ball->vel.norm().rotate(90) * 10, target - wm->ball->vel.norm().rotate(90) * 10);
    Vector2D kickerPoint = agent->pos() + agent->dir().norm() * 0.08;
    Vector2D addVec = agent->dir().norm() * 0.08;
    gpa->setOnetouchmode(false);

    if (wm->ball->vel.length() > 0.5 - distThr) {
        distThr = 0.45;
        Vector2D bestPoint = CSkillReceivePass::bestPointToIntersect(agent);
        if (!bestPoint.valid() || Circle2D(agent->pos(), 0.15).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)))) {
            bestPoint = ballPath.nearestPoint(agent->pos());
        }
        bestPoint -= addVec;
        CSkillReceivePass::validatePoint(bestPoint, wm->ball->pos);
        finalPos = bestPoint;

        if (Circle2D(agent->pos(), 0.1).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)))) {
            if (fabs(((target - agent->pos()).th().degree() - (wm->ball->pos - agent->pos()).th().degree())) < 60 || isKhafan) {
                finalDir = Vector2D::unitVector(
                        CKnowledge::oneTouchAngle(agent->pos(), agent->vel(), wm->ball->vel, agent->pos() - wm->ball->pos, target,
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


        if ((ballPath.intersection(targetNormalSeg).isValid())  && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
            finalDir = target - agent->pos();

        }

    } else {
        distThr = 0;
        finalPos = wm->ball->pos - (target - finalPos).norm() * 0.15;
        finalDir = Vector2D(cos(kickFinalDir.radian()), sin(kickFinalDir.radian()));
    }

    Vector2D temp = finalPos;
    CSkillReceivePass::validatePoint(finalPos, agent->pos());
    if (temp != finalPos) finalDir = wm->ball->pos - finalPos;

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

    validateKickerState();

    KMode kickMode = decideMode();
    switch (kickMode) {
        case KMode::DIRECT:
            direct();
            break;
        case KMode::AvoidOurPenalty:
            avoidOurPenalty();
            break;
        case KMode::AvoidOppPenalty:
            avoidOppPenalty();
            break;
        case KMode::DONTKICK:
            doNotKick();
            break;
        case KMode::JTurn:
            jTurn();
            break;
        case KMode::TurnForKick:
            turnForKick();
            break;
        case KMode::NOMODE:break;
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
