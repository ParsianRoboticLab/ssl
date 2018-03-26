#ifndef GotoPoint_H
#define GotoPoint_H

#include <parsian_agent/skill.h>
#include <parsian_util/mathtools.h>
#include <parsian_agent/newbangbang.h>
#include <QTime>
#include <QFile>
#include <QDebug>
#include <QFile>
#include <parsian_util/mathtools.h>
#include <algorithm>
#include <math.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>

enum gpMode {
    GPACC1 = 0,
    GPACC2 = 1,
    GPVCONST = 2,
    GPDEC1 = 3,
    GPDEC2 = 4,
    GPPOS = 5,
    GPDIVE = 6
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillGotoPoint : public CSkill, public GotopointAction {


private:
    _PID *angPid;
    _PID *posPid;
    _PID *posXpid;
    _PID *posYpid;
    _PID *speedPidX;
    _PID *speedPidY;
    _PID *thPid;
    _PID *velPid;
    Vector2D startingPoint;
    ///////////////////////
    void trajectoryPlanner();
    double appliedTh;
    //////////////////////


protected:

    Vector2D agentPos;
    Vector2D agentVel;
    double agentDist;
    double agentVc;
    double _Vx, _Vy;
    double appliedAcc;
    double posPidDist;
    double agentVDesire;
    double decThr;
    double posThr;

    double agentX3;

    gpMode decideMode();
    AngleDeg agentMovementTh;
    AngleDeg lastPath;

public:
    virtual double timeNeeded();
    DEF_SKILL(CSkillGotoPoint);
    gpMode currentGPmode;
};


#endif // GOTOPOINT_H
