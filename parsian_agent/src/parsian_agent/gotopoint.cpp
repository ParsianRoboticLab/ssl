#include <parsian_agent/gotopoint.h>

INIT_SKILL(CSkillGotoPoint, "gotopoint");
CSkillGotoPoint::CSkillGotoPoint(Agent *_agent) : CSkill(_agent) {
    lookAt.invalidate();
    agent = _agent;
    speedPidX = new _PID(1, 0, 0, 0, 0);
    speedPidY = new _PID(1, 0, 0, 0, 0);
    posPid = new _PID(1, 0, 0, 0, 0);
    velPid = new _PID(1, 0, 0, 0, 0);
    angPid = new _PID(2, 0, 0, 0, 0);
    posXpid = new _PID(1, 0, 0, 0, 0);
    posYpid = new _PID(1, 0, 0, 0, 0);
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
    //    delete speedPidX;
    //    delete speedPidY;
    //    delete posPid;
    //    delete angPid;
    //    delete posXpid;
    //    delete posYpid;
    //    delete thPid;
}

double CSkillGotoPoint::timeNeeded() {
    return 0 ;
}
gpMode CSkillGotoPoint::decideMode() {
    //    posPidDist = 1/agentVc;
    //    if(posPidDist > 1)
    //        posPidDist = 1;
    if (agentDist < posPidDist + posThr) {
        decThr = 0;
        return GPPOS;
    } else {
        agentX3 = fabs(((posPidDist * posPid->kp) * (posPidDist * posPid->kp) - (agentVc * agentVc)) / (2 * maxDeceleration)) + 0.05 * agentVc;

        if (agentDist <= agentX3 + decThr) {
            if (agentVc < 0.5) {
                decThr = 0;
            } else {
                decThr = 0.5;
            }
            return GPDEC1;
        } else if (agentVc >= maxVelocity) {
            decThr = 0;
            return GPVCONST;
        } else {
            decThr = 0;
            return GPACC1;
        }

    }

}

void CSkillGotoPoint::trajectoryPlanner() {
    agentMovementTh = (targetPos - agentPos).th();
    //////////////////acc dec
    appliedAcc = 1.5 * maxAcceleration;

    if (smooth) {
        if ((agentMovementTh - lastPath).degree() > 20 && (agentMovementTh - lastPath).degree() < 100 && agentVc > 1) {
            agentMovementTh = lastPath + 60;
            maxVelocity = 1;
            appliedAcc = 0;

        } else if ((agentMovementTh - lastPath).degree() < -20 && (agentMovementTh - lastPath).degree() > -100 && agentVc > 1) {
            agentMovementTh = lastPath - 60;
            maxVelocity = 1;
            appliedAcc = 0;

        } else if ((agentMovementTh - lastPath).degree() >= 100 && agentVc > 1) {
            agentMovementTh = lastPath + 80;
            maxVelocity = 0.5;
            appliedAcc = 0;

        } else if ((agentMovementTh - lastPath).degree() <= -100 && agentVc > 1) {
            agentMovementTh = lastPath - 80;
            maxVelocity = 0.5;
            appliedAcc = 0;

        }
    }
    ///////////////////////////////////////////// th pid
    thPid->kp = 0;
    thPid->error = (agentMovementTh - agent->vel().norm().th()).radian();
    if ((fabs(thPid->error) > 1)
            || agentVc < 0.5
            || agentDist > 3
            || fabs((agentMovementTh - agent->dir().th()).degree()) > 80 && fabs((agentMovementTh - agent->dir().th()).degree()) < 100) {
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

//    targetValidate();

    /////////////////decide and exec

    agentPos = agent->pos();
    agentVel = agent->vel();
    agentDist = agentPos.dist(targetPos);
    currentGPmode = decideMode();
    angPid->error = (targetDir.th() - agent->dir().th()).radian();
    agentVc = agentVel.length();
    //////////////// set params
    posPid->kd = 3;
    if (startingPoint.dist(agentPos) < 0.05) {
        posPid->kp = 4;
        posPid->kd = 0;
    } else if (startingPoint.dist(agentPos) < 0.3) {
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
        startingPoint = agentPos;
    }


    ////////////////////////////
    if (currentGPmode == GPPOS) {
        ////////////////ACC + DEC
        agent->_ACC = 0;// conf->AccMaxNormal; // TODO : is correct? in ai is zero
        agent->_DEC = 0;// conf->DecMax; // TODO : is correct?
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
    } else if (currentGPmode == GPVCONST) {
        /////////////////ACC + DEC
        agent->_ACC = 0;
        agent->_DEC = 0;
        agentVDesire = maxVelocity;
        velPid->_I = 0;
        ////////////////
        _Vx = maxVelocity * cos(appliedTh);
        _Vy = maxVelocity * sin(appliedTh);

    } else if (currentGPmode == GPDEC1) {
        agent->_ACC = 0;
        agent->_DEC = 0;
        agentVDesire = sqrt(fabs(2 * maxDeceleration * agentDist * moreDec) + vp * vp) - decOffset;
        _Vx =  agentVDesire * cos(appliedTh) ;
        _Vy =  agentVDesire * sin(appliedTh) ;

    } else if (currentGPmode == GPACC1) {
        if (agentVc > 0.3) {
            agent->_ACC = appliedAcc;
            agent->_DEC = 0;
            agentVDesire = maxVelocity ;
        } else if (!slowMode && !penaltyKick) {
            agent->_ACC = 0;
            agent->_DEC = 0;
            agentVDesire = 0.7;
        } else {
            agent->_ACC = 0;
            agent->_DEC = 0;
            agentVDesire = 0.5;
        }
        ////////////////
        _Vx = agentVDesire * cos(appliedTh);
        _Vy = agentVDesire * sin(appliedTh);
        ///////////////////////////////////////////////PID Previous error
    } else {
        agent->waitHere();
        velPid->_I = 0;
    }
    ROS_INFO_STREAM("DI : " << agentDist);
    ROS_INFO_STREAM("DIST : " << agentDist);

    agent->setRobotAbsVel(_Vx, _Vy, angPid->PID_OUT());
    angPid->pError = angPid->error;


    lastPath = agentVel.th();

}

double CSkillGotoPoint::progress() {
    if (agent == nullptr) {
        return 0;
    }
    double d = (agentPos - targetPos).length();
    if (d < 0.04) {
        return 1.0;
    }
    if (d < 0.05) {
        return 0.8;
    }
    if (d < 0.10) {
        return 0.7;
    }
    if (d < 0.20) {
        return 0.6;
    }
    return 0;
}
