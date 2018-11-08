#ifndef NEWBANGBANG_H
#define NEWBANGBANG_H

#include <parsian_util/base.h>
#include <parsian_util/geom/geom.h>
#include <parsian_util/tools/blackboard.h>
#include <parsian_util/tools/drawer.h>

enum bangBangMode {
    _bangBangAcc = 0,
    _bangBangConst = 1,
    _bangBangDec = 2,
    _bangBangPosPID
};


class _PID {
public:
    double kp, kd, ki, _I, _P, _D, I_saturate, I_clear_error, error, pError;
    _PID() : kp(0), kd(0), ki(0), _I(0), _P(0), _D(0), I_saturate(0), I_clear_error(0), pError(0) {}
    _PID(double _kp, double _kd, double _ki, double _I_sat, double _I_clear_error) : kp(_kp), kd(_kd), ki(_ki), _I(0), _P(0), _D(0), I_saturate(_I_sat), I_clear_error(_I_clear_error), pError(0) {}

    double PID_OUT() {
        _P = kp * error;

        if (fabs(error) < I_clear_error) {
            _I = 0;
        } else if (_I <= I_saturate || I_saturate == 0) {
            _I += error;
        } else {
            _I = I_saturate;
        }

        _D = error - pError;
//        debug(QString("sag :%1").arg(pError),D_MHMMD);
        return _P + ki * _I + kd * _D;
    }
};

class CNewBangBang {
public:
    CNewBangBang();

    double plan(Vector2D _dir , double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double &a, double &t, int id, double _tDesired = -1, bool fastEnd = false, bool end = true, bool newGenerationRobots = true, bool _oneTouch = false, bool _slow = false, bool _NoPID = false);

    void bangBangSpeed(Vector2D _agentPos, Vector2D _agentVel, Vector2D _agentDir, Vector2D _pos2, Vector2D _dir2, double _V2, double dt, double & _Vx, double & _Vy, double & _W);
    bangBangMode decidePlan();
    QList<double> lastV;
    QList<double> lastV1;

    double diff;




    Property(double, Distance, x);
    Property(double, AngKp, angKp);
    Property(double, LastDistance, lx);
    Property(double, DistanceError, sumErr);
    Property(double, CurrentVelocity, v1);
    Property(double, FinalVelocity, v2);
    Property(double, AccMax, amax);
    Property(double, DecMax, dmax);
    Property(double, VelMax, vmax);
    Property(double, DesiredTime, tDesired);
    Property(double, TimeStep, dt);
    Property(bool, FastEnd, fastend);
    Property(bool, EndPoint, endPoint);
    Property(bool, OneTouch, oneTouch);
    Property(bool, DiveMode, diveMode);
    Property(bool, Slow, slow);
    Property(bool, NoPid, noPid);
    Property(bool, NewRobots, newRobots);
    PropertyGet(double, Velocity, v);
    PropertyGet(double, Acceleration, a);
    PropertyGet(double, TimeNeeded, t);
    Property(bool, Smooth, smooth);
    Property(bool, AngInPath, angPath);

private:
    //void optimalAccOrDec(double agentDir, bool dec);
    void trajectoryPlanner();
    double posPidDist, posPidThr;
    double decThr;
    double currentVel;
    double vDes;
    double constThr;
    Vector2D pos2;
    Vector2D dir2;
    double Vel2;
    Vector2D agentPos;
    Vector2D agentVel;
    Vector2D agentDir;
    Vector2D movementTh;
    AngleDeg lastPath;
    double desiredVx, desiredVy;
    double lastVx, lastVy;
    double velMax;
    _PID *posPid;
    _PID *angPid;
    _PID *thPid;
    AngleDeg agentMovementTh;
    double appliedTh;
};

#endif // NEWBANGBANG_H
