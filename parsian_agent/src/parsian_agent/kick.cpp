#include <parsian_agent/kick.h>
#include <parsian_agent/config.h>

CSkillKick::CSkillKick(Agent *_agent) : CSkill(_agent) {

    gpa = new CSkillGotoPointAvoid(_agent);
    gpa->setSlowmode(false);
    gpa->setAddvel(Vector2D(0, 0));
    angPid = new _PID(2, 0, 0, 0, 0);
    speedPid = new _PID(1, 0, 0, 0, 0);
    posPid = new _PID(1, 0, 0, 0, 0);
    distThr = 0;
}

CSkillKick::~CSkillKick() {

    delete gpa;
    delete angPid;
    delete speedPid;
    delete posPid;
}

KMode CSkillKick::decideMode() {

    KMode mode = KMode::NOMODE;
    Circle2D kickerArea(agent->pos() + agent->dir().norm() * 0.09 , 0.15);
    AngleDeg kickFinalDir = (target - wm->ball->pos).th();
    if (dontKick) mode = KMode::DONTKICK;
    else if ((playMakeMode && wm->field->isInOurPenaltyArea(wm->ball->pos)) || avoidPenaltyArea) mode = KMode::AvoidOurPenalty;
    else if (isOppPenaltyMode()) mode = KMode::AvoidOppPenalty;
    else if (wm->ball->vel.length() < 0.5 && kickerArea.contains(wm->ball->pos) && std::fabs((kickFinalDir - agent->dir().th()).degree()) > 30) mode = KMode::TurnForKick;
    else {
        Segment2D targetNormalSeg(target + wm->ball->vel.norm().rotate(90) * 10, target - wm->ball->vel.norm().rotate(90) * 10);
        if (wm->ball->vel.length() > 0.5 - distThr) {
            distThr = 0.45;
            if(((agent->pos().dist(wm->ball->pos + wm->ball->vel*0.5) < 0.6) && isKhafan)) {
                return KMode::JTurn;
            }
            if ((wm->ball->seg().intersection(targetNormalSeg).isValid()) && ((agent->pos().dist(wm->ball->pos) < 1) || isKhafan) && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
                return KMode::JTurn;
            }
        } else {
            distThr = 0;
            if (wm->ball->pos.dist(agent->pos()) < 0.6) {
                return KMode::JTurn;
            }
        }

        mode = KMode::DIRECT;
    }

    return mode;


}

void CSkillKick::doNotKick() {

    Vector2D finalPos;
    AngleDeg kickFinalDir = (target - wm->ball->pos).th();
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
    Vector2D finalDirVec = (target - wm->ball->pos);

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

    Vector2D finalDirVec = (target - wm->ball->pos);
    gpa->setAvoidpenaltyarea(true);
    gpa->init(finalPos, finalDirVec);
    gpa->execute();
    agent->setKick(kickSpeed);
}

void CSkillKick::jTurn() {
    AngleDeg kickFinalDir = (target - wm->ball->pos).th();
    double movementDir = ((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree();
    double shift = 0;
    double distCoef = 0.15;

    Vector2D idealPass = (wm->ball->pos - agent->pos()).norm() * distCoef;

    posPid->error = 0;
    posPid->kd = 0;
    if (movementDir < 17 && movementDir > -17) {
        shift = 0;

    } else if (std::fabs(movementDir) > 50) {
        shift = 25 + (1 - agent->pos().dist(wm->ball->pos)) * 61;
    } else if (std::fabs(movementDir) > 30) {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = 30 + (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = 20 + (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = 20 + (1 - agent->pos().dist(wm->ball->pos)) * 30;
        }
    } else {
        if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15)) {
            shift = 25 + (1 - agent->pos().dist(wm->ball->pos)) * 20;
        } else if (wm->ball->vel.length() < 0.1) {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 10;
        } else {
            shift = 15 + (1 - agent->pos().dist(wm->ball->pos)) * 12;
        }
    }

    shift = (movementDir < 0) ? -shift : shift;

    idealPass.rotate(shift);

    Vector2D targetForJturnSpeed = agent->pos() + idealPass;
    Vector2D movementThSpeed = (targetForJturnSpeed - agent->pos()).norm();
    speedPid->error = targetForJturnSpeed.dist(agent->pos());

    ////////////set Active adaptive PIDs

    double dirReduce = (std::fabs(movementDir)/50) * (std::fabs(movementDir)/50);
    if (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15) &&
        agent->pos().dist(wm->ball->pos) < 0.35) {
        dirReduce -= 2;
    }

    if(isPlayoff) {
        dirReduce -= 1;
    }

    speedPid->kp = 6 + 4 * agent->pos().dist(wm->ball->pos) + dirReduce*2 ;

    if (penaltyKick) {
        speedPid->kp = 4;
    }

    angPid->kp = 4.5;

    double vx = movementThSpeed.x * speedPid->PID_OUT();
    double vy = movementThSpeed.y * speedPid->PID_OUT();
    angPid->error = (kickFinalDir - agent->dir().th()).radian();
    agent->setRobotAbsVel(wm->ball->vel.x*1 + vx, wm->ball->vel.y*1 + vy, angPid->PID_OUT());
    speedPid->pError = speedPid->error;

    posPid->pError = posPid->error;

    //TODO: test this
    agent->accelerationLimiter(0, false);

}

void CSkillKick::turnForKick() {
    AngleDeg kickFinalDir = (target - wm->ball->pos).th();
    agent->setRoller(0);
    double angReduce = 1;

    if (isPlayoff) {
        if (std::fabs((agent->dir().th() - kickFinalDir).degree()) < 80) {
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
    return findMostPossible(agent);
}

Vector2D CSkillKick::findMostPossible(const Agent* _agent) {

    QList<int> tempObstacles;
    QList <Circle2D> obstacles;
    obstacles.clear();
    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
        obstacles.append(Circle2D(wm->opp.active(i)->pos, 0.1));
    }
    for (int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        if (wm->our.active(i)->id != _agent->id()) {
            obstacles.append(Circle2D(wm->our.active(i)->pos, 0.1));
        }
    }
    double prob, angle, biggestAngle;
    CKnowledge::getEmptyAngle(*wm->field, wm->ball->pos - (wm->field->oppGoal() - wm->ball->pos).norm() * 0.15, wm->field->oppGoalL(), wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);
    Segment2D goalSeg(wm->field->oppGoalL(), wm->field->oppGoalR());

    return  goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI * (angle) / 180), sin(_PI * (angle) / 180)).norm() * 12));
}

void CSkillKick::direct() {
    Circle2D agentNearArea(agent->pos(), 0.15);
    Vector2D finalDir, finalPos;
    Circle2D kickerArea(agent->pos() + agent->dir().norm() * 0.09 , 0.15);
    Segment2D kickerSeg(agent->pos() + agent->dir().norm() * 0.08 + agent->dir().rotate(90).norm() * 0.02 , agent->pos() + agent->dir().norm() * 0.08 - agent->dir().rotate(90).norm() * 0.02);
    Segment2D targetNormalSeg(target + wm->ball->vel.norm().rotate(90) * 10, target - wm->ball->vel.norm().rotate(90) * 10);
    Vector2D kickerPoint = agent->pos() + agent->dir().norm() * 0.08;
    Vector2D addVec = agent->dir().norm() * 0.08;
    gpa->setOnetouchmode(false);
    AngleDeg kickFinalDir = (target - wm->ball->pos).th();

    if (wm->ball->vel.length() > 0.5 - distThr) {
        distThr = 0.45;
        Vector2D bestPoint = CSkillReceivePass::bestPointToIntersect(agent);
        if (!bestPoint.valid() || Circle2D(agent->pos(), 0.15).intersection(Segment2D(wm->ball->pos, wm->ball->getPosInFuture(0.5)))) {
            bestPoint = wm->ball->seg().nearestPoint(agent->pos());
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


        if ((wm->ball->seg().intersection(targetNormalSeg).isValid())  && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60)) {
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

    if (!((wm->ball->seg().intersection(targetNormalSeg).isValid())  && (fabs(((wm->ball->pos - agent->pos()).th() - kickFinalDir).degree()) < 60) )&&(finalPosArea.intersection(directPath, &s1, &s2))) {
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
    Polygon2D robotKickArea;

    double distCoef = (4.5 / wm->ball->pos.dist(target)) / 100;
    distCoef = std::min(distCoef, 0.02);
    distCoef = std::max(distCoef, 0.01);

    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.01 + agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.35 + agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.35 - agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agent->pos() + agent->dir().norm() * 0.01 - agent->dir().rotate(90).norm()*distCoef);

    Circle2D dribblerArea(agent->pos() + agent->dir().norm() * 0.1, 0.25);
    bool kickerOn;
    if (passProfiler || kickWithCenterOfDribbler) {
        kickerOn = dribblerArea.contains(wm->ball->pos) && robotKickArea.contains(wm->ball->pos);
    } else {
        kickerOn = dribblerArea.contains(wm->ball->pos);
    }

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

bool CSkillKick::isOppPenaltyMode() {
    if (penaltyKick || passProfiler || !avoidOppPenaltyArea || !wm->field->isInOppPenaltyArea(wm->ball->pos)) return false;

    double robotAreaRaduis = (goalieMode) ? 0.5 : 1;
    double robotAreaOffset = 0.1;
    Circle2D robotArea(agent->pos(), robotAreaRaduis - robotAreaOffset);
    if (agent->pos().dist(wm->ball->pos) < robotAreaRaduis) {
        robotArea.assign(agent->pos(), std::max(agent->pos().dist(wm->ball->pos) - robotAreaOffset, 0.01));
    }
    return robotArea.intersection(wm->ball->seg()) != 2 || wm->ball->vel.length() < 1;
}
