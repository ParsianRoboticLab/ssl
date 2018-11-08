#ifndef DYNAMICROLE_H
#define DYNAMICROLE_H

#include <parsian_util/base.h>
#include <parsian_util/core/agent.h>
#include <parsian_ai/roles/role.h>

enum class DynamicMode {
    NoMode          = 0b0000000000,
    CounterAttack   = 0b0000000001,
    DefenseClear    = 0b0000000010,
    DirectKick      = 0b0000000100,
    Fast            = 0b0000001000,
    Critical        = 0b0000010000,
    NotWeHaveBall   = 0b0000100000,
    Plan            = 0b0001000000,
    Forward         = 0b0010000000,
    NoPositionAgent = 0b0100000000,
    BallInOppJaw    = 0b1000000000
};

enum class PlayMakeSkill {
    NoSkill     = 0b000000,
    Pass        = 0b100001,
    CatchBall   = 0b100010,
    Shot        = 0b100100,
    Keep        = 0b101000,
    Chip        = 0b110000,
    Dribble     = 0b001000

};

enum class PositionSkill {
    NoSkill     = 0b000000,
    Ready       = 0b000001,
    OneTouch    = 0b000010,
    Move        = 0b000100
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

    int dischargetime = 0;
    bool isdischargetime = false;

    void update();
    bool updated;

    SkillProperty(CRoleDynamic, Agent*, Agent, agent);
    SkillProperty(CRoleDynamic, PositionSkill, SelectedPositionSkill, positionSkill);
    SkillProperty(CRoleDynamic, PlayMakeSkill, SelectedPlayMakeSkill, playMakeSkill);
    SkillProperty(CRoleDynamic, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CRoleDynamic, Vector2D, Target, target);
    SkillProperty(CRoleDynamic, Vector2D, TargetDir, targetDir);
    SkillProperty(CRoleDynamic, bool, IsPlayMake, isplaymake);
    SkillProperty(CRoleDynamic, double, Tolerance, tolerance);
    SkillProperty(CRoleDynamic, bool, Chip, chip);
    SkillProperty(CRoleDynamic, double, KickSpeed, kickSpeed);
    SkillProperty(CRoleDynamic, double, ChipDist, chipdist);
    SkillProperty(CRoleDynamic, double, ReceiveRadius, receiveRadius);
    SkillProperty(CRoleDynamic, Vector2D, WaitPos, waitPos);
    SkillProperty(CRoleDynamic, bool, VeryFine, veryFine);
    SkillProperty(CRoleDynamic, bool, EmptySpot, emptySpot);
    SkillProperty(CRoleDynamic, bool, NoKick, noKick);

public:
    CRoleDynamic* setKickdischargetime(double _dischargetime) {
        dischargetime = _dischargetime;
        isdischargetime = true;
        return this;
    }

    CRoleDynamic* setAddKickRealSpeed(double val) {
//        kickSpeed = knowledge -> getProfile(agent->id(), agent->pos().dist(target) + val, !chip, false);
        updated = true;
        return this;
    }

};

#endif // DYNAMICROLE_H
