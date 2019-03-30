#ifndef KICK_H
#define KICK_H

#include <parsian_agent/gotopointavoid.h>
#include <QtCore/QQueue>
#include <parsian_util/core/ball.h>
#include <parsian_util/core/movingobject.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/core/knowledge.h>
#include <parsian_agent/receivepass.h>


enum class KMode {
    NOMODE          = 0,
    DIRECT          = 1,
    AvoidOurPenalty = 2,
    AvoidOppPenalty = 3,
    DONTKICK        = 4,
    JTurn           = 5,
    TurnForKick     = 6
};

class CSkillKick : public CSkill, public KickAction {
private:
    KMode decideMode();
    void direct();
    void jTurn();
    void turnForKick();
    void avoidOppPenalty();
    void avoidOurPenalty();
    void doNotKick();
    void validateKickerState();
    bool isOppPenaltyMode();


    CSkillGotoPointAvoid *gpa;
    _PID *angPid;
    _PID *speedPid;
    _PID *posPid;
    double distThr;

public:
    explicit CSkillKick(Agent* _agent);
    ~CSkillKick();
    void execute() override;

    Vector2D findMostPossible();
    static Vector2D findMostPossible(const Agent* _agent);
};


#endif // KICK_H
