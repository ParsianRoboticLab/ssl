#ifndef PLAYOFFROLE_H
#define PLAYOFFROLE_H

#include "parsian_ai/roles/role.h"

#include <QTime>

enum class RoleSkill {
    Gotopoint      = 0,
    GotopointAvoid = 1,
    Kick           = 2,
    ReceivePass    = 3,
    OneTouch       = 4,
    Mark           = 5,  // After Life (Steady)
    Support        = 6,  // After Life (Move for ball)
    Defense        = 7   // After Life (Move back to our field)
};

class CRolePlayOff {
private:
    void update();
    bool updated;
    bool roleUpdate;
    bool dont;
    QTime timer;
public:

    CRolePlayOff();
    ~CRolePlayOff();
    void reset();
    void execute();
    int resetTime();
    int getElapsed() const;
    bool deleted;
    KickAction *kickSkill;
    ReceivepassAction *receivePassSkill;
    OnetouchAction *oneTouchSkill;
    GotopointavoidAction *gotoPointAvoidSkill;

    ClassProperty(CRolePlayOff, int, AgentID, agentID, updated);
    ClassProperty(CRolePlayOff, Agent*, Agent, agent, updated);
    ClassProperty(CRolePlayOff, RoleSkill, SelectedSkill, selectedSkill, updated);
    ClassProperty(CRolePlayOff, Vector2D, Target, target, updated);
    ClassProperty(CRolePlayOff, Vector2D, TargetDir, targetDir, updated);
    ClassProperty(CRolePlayOff, Vector2D, TargetVel, targetVel, updated);
    ClassProperty(CRolePlayOff, bool, AvoidPenaltyArea, avoidPenaltyArea, updated);
    ClassProperty(CRolePlayOff, bool, AvoidCenterCircle, avoidCenterCircle, updated);
    ClassProperty(CRolePlayOff, double, Tolerance, tolerance, updated);
    ClassProperty(CRolePlayOff, bool, Chip, chip, updated);
    ClassProperty(CRolePlayOff, int, KickSpeed, kickSpeed, updated);
    ClassProperty(CRolePlayOff, bool, Slow, slow, updated);
    ClassProperty(CRolePlayOff, float, ReceiveRadius, receiveRadius, updated);
    ClassProperty(CRolePlayOff, Vector2D, WaitPos, waitPos, updated);
    ClassProperty(CRolePlayOff, float, MaxVelocity, maxVelocity, updated);
    ClassProperty(CRolePlayOff, bool, Intercept, intercept, updated);
    ClassProperty(CRolePlayOff, bool, AvoidBall, avoidBall, updated);
    ClassProperty(CRolePlayOff, bool, IgnoreAngle, ignoreAngle, updated);
    ClassProperty(CRolePlayOff, double, EventDist, eventDist, updated);
    ClassProperty(CRolePlayOff, bool, DoPass, doPass, updated);
    ClassProperty(CRolePlayOff, bool, LookForward, lookForward, updated);
    ClassProperty(CRolePlayOff, bool, NoAvoid, noAvoid, updated);

    ClassProperty(CRolePlayOff, bool, Spin, spin, updated);


    ////////Not Executive Property
    ClassProperty(CRolePlayOff, long long, Time, time, dont);
    ClassProperty(CRolePlayOff, bool, BallIsNear, ballIsNear, dont);
    ClassProperty(CRolePlayOff, bool, TimeBased, timeBased, dont);
    ClassProperty(CRolePlayOff, bool, FirstMove, firstMove, dont);


public:
    void setUpdated(bool _updated);
    bool getUpdated();
    inline void setRoleUpdate(bool _updated) {
        roleUpdate = _updated;
    }
    inline bool getRoleUpdate() {
        return roleUpdate;
    }

    CRolePlayOff* setKickRealSpeed(double val) {
        kickSpeed = val;
        DBUG(QString("[playoffrole] setkickrealspeed : %1 %2").arg(val).arg(kickSpeed), D_MAHI);
        updated = true;
        return this;
    }

protected:
};

#endif // PLAYOFFROLE_H
