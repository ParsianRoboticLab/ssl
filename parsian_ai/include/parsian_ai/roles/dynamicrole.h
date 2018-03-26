#ifndef DYNAMICROLE_H
#define DYNAMICROLE_H

#include <parsian_util/base.h>
#include <parsian_util/core/agent.h>
#include <parsian_ai/roles/role.h>

enum class DynamicMode {
    NoMode          = 0b0000000000,
    CounterAttack   = 0b0000000001,
    DefenseClear    = 0b0000000010,
    HighProb        = 0b0000000100,
    Fast            = 0b0000001000,
    Critical        = 0b0000010000,
    NotWeHaveBall   = 0b0000100000,
    Plan            = 0b0001000000,
    BallInOurField  = 0b0010000000,
    NoPositionAgent = 0b0100000000,
    BallInOppJaw    = 0b1000000000
};

enum class DynamicSkill {
    NoSkill     = 0b0000000000,
    ////PlayMake Skills
    Pass        = 0b100001,
    CatchBall   = 0b100010,
    Shot        = 0b100100,
    Keep        = 0b101000,
    Chip        = 0b110000,
    ////Positioning
    Ready       = 0b000001,
    OneTouch    = 0b000010,
    Move        = 0b000100,
    Dribble     = 0b001000
};

enum class DynamicRegion {
    NoMatter    = 0b0000000,
    Near        = 0b0000001,
    Forward     = 0b0000010,
    Far         = 0b0000100,
    Goal        = 0b0001000,
    Reflect     = 0b0010000,
    Best        = 0b0100000,
    Supporter   = 0b1000000
};


class CRoleDynamic {
public:
    CRoleDynamic();
    CRoleDynamic(const CRoleDynamic& _copy);
    CRoleDynamic(CRoleDynamic && _move) noexcept ;
    CRoleDynamic& operator=(const CRoleDynamic& _copy);
    CRoleDynamic& operator=(CRoleDynamic&& _move) noexcept ;
    ~CRoleDynamic();
    void execute();

private:
    KickAction           *shotSkill;
    ReceivepassAction    *receiveSkill;
    GotopointavoidAction *moveSkill;
    OnetouchAction       *oneTouchSkill;
    // CSKill "KEEP"

    void update();
    bool updated;

    SkillProperty(CRoleDynamic, Agent*, Agent, agent);
    SkillProperty(CRoleDynamic, DynamicSkill, SelectedSkill, selectedSkill);
    SkillProperty(CRoleDynamic, Vector2D, Target, target);
    SkillProperty(CRoleDynamic, Vector2D, TargetDir, targetDir);
    SkillProperty(CRoleDynamic, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CRoleDynamic, double, Tolerance, tolerance);
    SkillProperty(CRoleDynamic, bool, Chip, chip);
    SkillProperty(CRoleDynamic, int, KickSpeed, kickSpeed);
    SkillProperty(CRoleDynamic, double, ReceiveRadius, receiveRadius);
    SkillProperty(CRoleDynamic, Vector2D, WaitPos, waitPos);
    SkillProperty(CRoleDynamic, bool, VeryFine, veryFine);
    SkillProperty(CRoleDynamic, bool, EmptySpot, emptySpot);
    SkillProperty(CRoleDynamic, bool, NoKick, noKick);
    SkillProperty(CRoleDynamic, int, AgentID, agentID);

public:
    CRoleDynamic* setKickRealSpeed(double val) {
//        kickSpeed = knowledge -> getProfile(agent->id(), val, !chip, false);
        kickSpeed = static_cast<int>(val * 100);
        DBUG(QString("set kick real speed : %1 %2").arg(val).arg(kickSpeed), D_MAHI);
        updated = true;
        return this;
    }

    CRoleDynamic* setAddKickRealSpeed(double val) {
//        kickSpeed = knowledge -> getProfile(agent->id(), agent->pos().dist(target) + val, !chip, false);
        updated = true;
        return this;
    }

};

#endif // DYNAMICROLE_H
