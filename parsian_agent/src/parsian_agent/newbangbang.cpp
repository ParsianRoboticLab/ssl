#include <QDebug>
#include <parsian_agent/newbangbang.h>
#include <parsian_agent/config.h>

CNewBangBang::CNewBangBang() {
    lastV.clear();
    lastV1.clear();
    posPidDist = 0.15;
    posPidThr = 0;
    decThr = 0;
    posPid = new _PID(3.5, 2, 0, 0, 0);
    angPid = new _PID(3, 0, 0, 0, 0);
    thPid = new _PID(1.5, 0, 0, 0, 0);
    smooth = false;
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        lastV.append(0.f);
        lastV1.append(0.f);
    }
    vmax = 4;
    angPath = false;

}

bangBangMode CNewBangBang::decidePlan() {
    double _x3;
    _x3 = (Vel2 * Vel2 - vmax * vmax) / (-2 * fabs(dmax)) ;
    if (((agentPos.dist(pos2) < posPidDist + posPidThr) && (Vel2 == 0))) {
        decThr = 0;
        constThr = 0;
        return _bangBangPosPID;
    } else {
        posPidThr = 0;
        if ((_x3 > 0) && (agentPos.dist(pos2) < _x3)) {
            return _bangBangDec;
        } else {
            decThr = 0;
            constThr = 0;
            return _bangBangAcc;
        }
    }
}

void CNewBangBang::trajectoryPlanner() {
    velMax = vmax;
    //    }
    ///////////////////////////////////////////// th pid
    thPid->error = (agentMovementTh - agentVel.norm().th()).radian();
    if (agentVel.length() < 0.3) {
        thPid->error = 0;
    }

    double veltan = (cos(agentMovementTh.radian())) * cos(agentDir.th().radian()) + (sin(agentMovementTh.radian())) * sin(agentDir.th().radian());
    double velnorm = -1 * (cos(agentMovementTh.radian())) * sin(agentDir.th().radian()) + (sin(agentMovementTh.radian())) * cos(agentDir.th().radian());

    double thPIDKCoef =  atan(fabs(veltan) / fabs(velnorm)) / _PI * 2;
    thPid->kp = conf->thKP * thPIDKCoef;
    thPid->ki = conf->thKI;
    thPid->kd = conf->thKD;
//    if(fabs(thPid->error > 1) || currentVel < 0.5 || agentPos.dist(pos2) >3 ||( fabs((agentMovementTh - agentDir.th()).degree()) > 80 && fabs((agentMovementTh - agentDir.th()).degree()) < 100 )   )
//        thPid->error =0;

    appliedTh = agentMovementTh.radian() + thPid->PID_OUT();
    desiredVx = (vDes) * cos(appliedTh);
    desiredVy = (vDes) * sin(appliedTh);

    thPid->pError = thPid->error;
}
void CNewBangBang::bangBangSpeed(Vector2D _agentPos, Vector2D _agentVel, Vector2D _agentDir, Vector2D _pos2, Vector2D _dir2, double _V2, double dt, double & _Vx, double & _Vy, double & _W) {
    pos2 = _pos2;
    dir2 = _dir2;
    Vel2 = _V2;
    agentPos = _agentPos;
    agentVel = _agentVel;
    currentVel = agentVel.length();
    agentDir = _agentDir;
    movementTh = pos2 - agentPos;
    if (angPath) {
        if (angKp) {
            angPid->kp = angKp;
        } else {
            angPid->kp = 1;
        }
    } else {
        angPid->kp = 3;
    }
    angPid->error = (dir2.th() -  agentDir.th()).radian();
    agentMovementTh = movementTh.th();

    if (oneTouch || diveMode) {
        posPidDist = 0.5;
    } else {
        posPidDist = 0.15;
    }
    if (slow) {
        posPid->kp = (conf->posKP) * (0.001 / (agentPos.dist(pos2) * agentPos.dist(pos2)));
        posPid->kp = min(posPid->kp, 2.5);
        posPid->kp = max(posPid->kp, 1.5);

        posPid->kd = conf->posKD;
        posPid->ki = conf->posKI;
    } else if (diveMode) {
        posPid->kp = 7;
        posPid->kd = 20;
        posPid->ki = conf->posKI;
    } else if (oneTouch) {
        posPid->kp = (conf->posKP) * (0.04 / (agentPos.dist(pos2) * agentPos.dist(pos2)));
        DEBUG(QString("kp: %1").arg(posPid->kp), D_MHMMD);
        posPid->kp = min(posPid->kp, conf->posKP * 3);
        posPid->kp = max(posPid->kp, conf->posKP);

        posPid->kd = 15;
        posPid->ki = 0;
    } else {
        posPid->kp = (conf->posKP) * (0.015 / (agentPos.dist(pos2) * agentPos.dist(pos2)));
        DEBUG(QString("kp: %1").arg(posPid->kp), D_MHMMD);
        posPid->kp = min(posPid->kp, conf->posKP * 2);
        posPid->kp = max(posPid->kp, conf->posKP);
        posPid->kd = conf->posKD;
        posPid->ki = conf->posKI;
    }


    //////////////////////// dec calculations
    double vp = (posPidDist * posPid->kp);
    double moreDec = 0.7;
    double decOffset = 0.4;

    switch (decidePlan()) {
    case _bangBangPosPID:
        posPid->error = agentPos.dist(pos2);
        vDes = min(posPid->PID_OUT(), velMax);
        break;
    case _bangBangConst:
        vDes = velMax;
        break;
    case _bangBangDec:
        if (Vel2 == 0) {
            vDes = sqrt(vp * vp + 2 * dmax * agentPos.dist(pos2) * moreDec) - decOffset;
        } else {
            vDes = sqrt(Vel2 * Vel2 + 2 * dmax * agentPos.dist(pos2) * moreDec) - decOffset ;
        }
        break;
    case _bangBangAcc:

        vDes = velMax;
        break;
    }
    trajectoryPlanner();
    lastPath = agentVel.th();

    /////////////////////th pid
    DEBUG(QString("vdes : %1").arg(vDes), D_MHMMD);
    _Vx =  desiredVx;//(vDes)*cos(appliedTh);
    _Vy =  desiredVy;//(vDes)*sin(appliedTh);
    _W = angPid->PID_OUT();
    DEBUG(QString("v1: %1 ").arg(_W), D_MHMMD);

    lastVx = _Vx;
    lastVy = _Vy;
    angPid->pError = angPid->error;
    posPid->pError = posPid->error;
}
