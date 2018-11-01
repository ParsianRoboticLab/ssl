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

enum class GPMode {
    NoMode      = 0,
    ACC1        = 1,
    VCONST      = 2,
    DEC1        = 3,
    POS         = 4
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillGotoPoint : public CSkill, public GotopointAction {


private:
    _PID *angPid;
    _PID *posPid;
    _PID *thPid;
    _PID *velPid;
    Vector2D startingPoint;
    ///////////////////////
    void trajectoryPlanner();
    double appliedTh;
    //////////////////////


protected:

    double agentVc;
    double posPidDist;
    double agentVDesire;
    double decThr;
    double posThr;

    double agentX3;

    GPMode decideMode();
    AngleDeg agentMovementTh;
    AngleDeg lastPath;

public:
    explicit CSkillGotoPoint(Agent* _agent);
    ~CSkillGotoPoint();
    void execute() override;
};


#endif // GOTOPOINT_H
