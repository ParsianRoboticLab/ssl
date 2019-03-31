#ifndef ROLE_H
#define ROLE_H

#include <parsian_util/base.h>
#include <parsian_ai/util/worldmodel.h>
#include <QString>
#include <QList>
#include <parsian_ai/util/agent.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/gamestate.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/action/autogenerate/onetouchaction.h>
#include <parsian_util/action/autogenerate/receivepassaction.h>
#include <parsian_util/action/autogenerate/noaction.h>

class CRoleInfo;

enum class Roles {
    None = 0b000000,
    PlayMake = 0b100001,
    Positioning = 0b100010,
    Supporter = 0b100100,

};

enum class PlayMakeSkill {
    NoSkill = 0b000000,
    Pass = 0b100001,
    CatchBall = 0b100010,
    Shot = 0b100100,
    Keep = 0b101000,
    Chip = 0b110000,
    Dribble = 0b001000

};

enum class PositionSkill {
    NoSkill = 0b000000,
    Ready = 0b000001,
    OneTouch = 0b000010,
    Move = 0b000100
};
enum class SupporterSkill {
    NoSkill = 0b000000,
    Ready = 0b000001,
    OneTouch = 0b000010,
    Move = 0b000100
};


enum class DynamicRegion {
    NoMatter = 0b0000000,
    Near = 0b0000001,
    Forward = 0b0000010,
    Far = 0b0000100,
    Goal = 0b0001000,
    Reflect = 0b0010000,
    Best = 0b0100000,
    Supporter = 0b1000000
};

class CRole {
protected:
    KickAction *shotSkill;
    ReceivepassAction *receiveSkill;
    GotopointavoidAction *moveSkill;
    OnetouchAction *oneTouchSkill;
public:
    bool isdischargetime = false;
    int dischargetime = 0;

    CRole();

    ~CRole();

    virtual void execute() = 0;
    virtual void update() = 0;
    explicit CRole(Agent *_agent);

    SkillProperty(CRole, Agent*, Agent, agent);
    SkillProperty(CRole, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CRole, Vector2D, Target, target);
    SkillProperty(CRole, Vector2D, TargetDir, targetDir);
    SkillProperty(CRole, Roles, Role, role);
    SkillProperty(CRole, double, Tolerance, tolerance);
    SkillProperty(CRole, bool, Chip, chip);
    SkillProperty(CRole, double, KickSpeed, kickSpeed);
    SkillProperty(CRole, double, ChipDist, chipdist);
    SkillProperty(CRole, double, ReceiveRadius, receiveRadius);
    SkillProperty(CRole, Vector2D, WaitPos, waitPos);
    SkillProperty(CRole, bool, VeryFine, veryFine);
    SkillProperty(CRole, bool, EmptySpot, emptySpot);
    SkillProperty(CRole, bool, NoKick, noKick);
};

class CRoleInfo {
protected:
    QString roleName;
public:
    QString getRoleName();

    explicit CRoleInfo(QString _roleName);

    QList<Agent *> agents;

    virtual Agent *robot(int i);

    virtual int count();


    virtual void addAgent(Agent *agent);

    virtual void reset();
};

#endif // ROLE_H
