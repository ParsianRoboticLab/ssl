#include <parsian_agent/gotopoint.h>

CSkillGotoPoint::CSkillGotoPoint(Agent *_agent) : CSkill(_agent) {
    lookAt.invalidate();
    posPid = new _PID(1, 0, 0, 0, 0);
    velPid = new _PID(1, 0, 0, 0, 0);
    angPid = new _PID(2, 0, 0, 0, 0);
    thPid = new _PID(1, 0, 0, 0, 0);

    maxAcceleration = 4;
    maxDeceleration = 4;


    lookAt.invalidate();

    posPidDist = 0.5;

    decThr = 0.2;
    posThr = 0;

    maxVelocity = 5;

    agentVDesire = 0;
    ////modes
    slowMode = false;
    diveMode = false;
    penaltyKick = false;
    smooth = false;
}

CSkillGotoPoint::~CSkillGotoPoint() {
        delete posPid;
        delete angPid;
        delete velPid;
        delete thPid;
}


GPMode CSkillGotoPoint::decideMode() {
    double agentDist = agent->pos().dist(targetPos);

    if (agentDist < posPidDist + posThr) {
        decThr = 0;
        return GPMode::POS;
    } else {
        agentX3 = fabs(((posPidDist * posPid->kp) * (posPidDist * posPid->kp) - (agentVc * agentVc)) / (2 * maxDeceleration)) + 0.05 * agentVc;

        if (agentDist <= agentX3 + decThr) {
            if (agentVc < 0.5) {
                decThr = 0;
            } else {
                decThr = 0.5;
            }
            return GPMode::DEC1;
        } else if (agentVc >= maxVelocity) {
            decThr = 0;
            return GPMode::VCONST;
        } else {
            decThr = 0;
            return GPMode::ACC1;
        }

    }

}

void CSkillGotoPoint::trajectoryPlanner() {
    double agentDist = agent->pos().dist(targetPos);

    agentMovementTh = (targetPos - agent->pos()).th();
    //////////////////acc dec

    if (smooth) {
        if ((agentMovementTh - lastPath).degree() > 20 && (agentMovementTh - lastPath).degree() < 100 && agentVc > 1) {
            agentMovementTh = lastPath + 60;
            maxVelocity = 1;

        } else if ((agentMovementTh - lastPath).degree() < -20 && (agentMovementTh - lastPath).degree() > -100 && agentVc > 1) {
            agentMovementTh = lastPath - 60;
            maxVelocity = 1;

        } else if ((agentMovementTh - lastPath).degree() >= 100 && agentVc > 1) {
            agentMovementTh = lastPath + 80;
            maxVelocity = 0.5;

        } else if ((agentMovementTh - lastPath).degree() <= -100 && agentVc > 1) {
            agentMovementTh = lastPath - 80;
            maxVelocity = 0.5;

        }
    }
    ///////////////////////////////////////////// th pid
    thPid->kp = 0;
    thPid->error = (agentMovementTh - agent->vel().norm().th()).radian();
    if ((fabs(thPid->error) > 1)
            || agentVc < 0.5
            || agentDist > 3
            || (fabs((agentMovementTh - agent->dir().th()).degree()) > 80 && fabs((agentMovementTh - agent->dir().th()).degree()) < 100)) {
        thPid->error = 0;
    }

    appliedTh = agentMovementTh.radian() + thPid->PID_OUT();


}

void CSkillGotoPoint::execute() {

    maxVelocity = 1;
    if (slowMode || penaltyKick) {
        maxVelocity = 1.5;
    }
    if (diveMode) {
        maxVelocity = 4;
    }

    /////////////////decide and exec

    double agentDist = agent->pos().dist(targetPos);
    angPid->error = (targetDir.th() - agent->dir().th()).radian();
    agentVc = agent->vel().length();
    //////////////// set params
    posPid->kd = 3;
    if (startingPoint.dist(agent->pos()) < 0.05) {
        posPid->kp = 4;
        posPid->kd = 0;
    } else if (startingPoint.dist(agent->pos()) < 0.3) {
        posPid->kp = 0.37 / agentDist;
        if (posPid->kp > 3) {
            posPid->kp = 3;
        }

        posPid->kd = 10;
    } else {
        posPid->kp = 1.9;
    }


    if (slowMode || penaltyKick) {
        posPid->kp = 1.6;
    }
    if (diveMode) {
        posPid->kp = 1.8 / agentDist;
        posPid->kd = 10;
        if (posPid->kp > 4) {
            posPid->kp = 4;
        }
        posPidDist = 1;
    } else {
        posPidDist = 0.5;
    }
    posPid->kd = 1;

    diveMode = false;

    angPid->kp = 3;
    angPid->kd = 1;


    trajectoryPlanner();

    //////////////////////// dec calculations
    double vp = (posPidDist * posPid->kp);
    double moreDec = 0.65;
    double decOffset = 0.8;
    if (agentVc < 0.2) {
        startingPoint = agent->pos();
    }
    double _Vx{0.0}, _Vy{0.0};
    GPMode currentGPmode = decideMode();
    switch (currentGPmode) {

        case GPMode::NoMode:
            _Vx = 0;
            _Vy = 0;
            agent->waitHere();
            velPid->_I = 0;
            break;
        case GPMode::ACC1:
            if (agentVc > 0.3) {
                agentVDesire = maxVelocity ;
            } else if (!slowMode && !penaltyKick) {
                agentVDesire = 0.7;
            } else {

                agentVDesire = 0.5;
            }
            ////////////////
            _Vx = agentVDesire * cos(appliedTh);
            _Vy = agentVDesire * sin(appliedTh);
            ///////////////////////////////////////////////PID Previous error
            break;
        case GPMode::VCONST:
            /////////////////ACC + DEC
            agentVDesire = maxVelocity;
            velPid->_I = 0;
            ////////////////
            _Vx = maxVelocity * cos(appliedTh);
            _Vy = maxVelocity * sin(appliedTh);
            break;
        case GPMode::DEC1:
            agentVDesire = sqrt(fabs(2 * maxDeceleration * agentDist * moreDec) + vp * vp) - decOffset;
            _Vx =  agentVDesire * cos(appliedTh) ;
            _Vy =  agentVDesire * sin(appliedTh) ;

            break;
        case GPMode::POS:
            ////////////////ACC + DEC
            ////////////////
            posPid->error = agentDist;
            _Vx = posPid->PID_OUT() * cos(agentMovementTh.radian());
            _Vy = posPid->PID_OUT() * sin(agentMovementTh.radian());
            if (agentDist  < 0.015) {
                _Vx = 0;
                _Vy = 0;
            }
            ///////////////////////////////////////////////PID Previous error
            posPid->pError = agentDist;
            velPid->_I = 0;
            break;
    }

    ROS_INFO_STREAM("DI : " << agentDist);
    ROS_INFO_STREAM("DIST : " << agentDist);

    agent->setRobotAbsVel(_Vx, _Vy, angPid->PID_OUT());
    angPid->pError = angPid->error;
    lastPath = agent->vel().th();

}
