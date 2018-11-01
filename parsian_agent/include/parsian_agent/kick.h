#ifndef KICK_H
#define KICK_H

#include <parsian_agent/gotopointavoid.h>
#include <QtCore/QQueue>
#include <parsian_util/core/ball.h>
#include <parsian_util/core/movingobject.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/core/knowledge.h>


enum class KMode {
    NOMODE          = 0,
    DIRECT          = 1,
    AVOIDOPPENALTY  = 2,
    DONTKICK        = 3,


};

class CSkillKick : public CSkill, public KickAction {
private:
    KMode decideMode();
    _PID *angPid;
    _PID *speedPid;
    _PID *posPid;


    Circle2D kickerArea;
    double distThr;

    AngleDeg kickFinalDir;
    Vector2D finalDirVec;
    Vector2D finalPos;
    void direct();
    void avoidPenalty();
    void indirect();
    void jTurn();
    void turnForKick();
    void avoidOppPenalty();
    void avoidOurPenalty();
    void doNotKick();
    void findPosToGo();
    void validateKickerState();
    double oneTouchAngle(Vector2D pos, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double landa, double gamma);
    CSkillGotoPointAvoid *gpa;
    bool kickerOn;

public:
    explicit CSkillKick(Agent* _agent);
    ~CSkillKick();
    void execute() override;

    Vector2D findMostPossible();
};


#endif // KICK_H