#ifndef PARSIAN_SKILLS_ONETOUCH_H
#define PARSIAN_SKILLS_ONETOUCH_H

#include <parsian_agent/skill.h>
#include <parsian_agent/kick.h>
#include <parsian_util/action/autogenerate/onetouchaction.h>

class CSkillKickOneTouch : public CSkill, public OnetouchAction {
protected:
    CSkillGotoPointAvoid* gotopointavoid;
    CSkillKick* kick;
    QTime* timeAfterForceKick;
private:
    Vector2D findMostPossible();
    Vector2D ballLastVel;
    Vector2D lastInterceptPos;
public:
    static double oneTouchAngle(Vector2D pos, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double landa, double gamma);
    DEF_SKILL(CSkillKickOneTouch);
};

#endif //PARSIAN_SKILLS_ONETOUCH_H
