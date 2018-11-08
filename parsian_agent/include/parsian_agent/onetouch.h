#ifndef PARSIAN_SKILLS_ONETOUCH_H
#define PARSIAN_SKILLS_ONETOUCH_H

#include <parsian_agent/skill.h>
#include <parsian_agent/kick.h>
#include <parsian_util/action/autogenerate/onetouchaction.h>
#include <parsian_agent/receivepass.h>

enum class OTMode {
    None = 0,
    Wait = 1,
    Kick = 2,
    Intersect = 3
};

class CSkillKickOneTouch : public CSkill, public OnetouchAction {
private:
    OTMode decideMode();
    void wait();
    void kick();
    void intersect();
    CSkillGotoPointAvoid* gotoPointAvoid;
    CSkillKick* kickSkill;
    QTime* timeAfterForceKick;
public:
    explicit CSkillKickOneTouch(Agent* _agent);
    ~CSkillKickOneTouch();
    void execute() override;
};

#endif //PARSIAN_SKILLS_ONETOUCH_H
