// auto-generated don't edit !

#ifndef GotopointAction_HEADER_
#define GotopointAction_HEADER_



#include <parsian_util/action/action.h>
#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_skill_gotoPoint.h>
#include <list>

class GotopointAction : public Action {

public:
    GotopointAction();
    void setMessage(const void* _msg);

    void* getMessage();

    QString getActionName() override;
    static QString SActionName();


    SkillProperty(GotopointAction, bool, Dynamicstart, dynamicStart);
    SkillProperty(GotopointAction, float, Maxacceleration, maxAcceleration);
    SkillProperty(GotopointAction, float, Maxdeceleration, maxDeceleration);
    SkillProperty(GotopointAction, float, Maxvelocity, maxVelocity);
    SkillProperty(GotopointAction, bool, Onetouchmode, oneTouchMode);
    SkillProperty(GotopointAction, bool, Slowmode, slowMode);
    SkillProperty(GotopointAction, bool, Penaltykick, penaltyKick);
    SkillProperty(GotopointAction, bool, Divemode, diveMode);
    SkillProperty(GotopointAction, bool, Smooth, smooth);
    SkillProperty(GotopointAction, unsigned char, Roller, roller);
    SkillProperty(GotopointAction, Vector2D, Targetpos, targetPos);
    SkillProperty(GotopointAction, Vector2D, Targetdir, targetDir);
    SkillProperty(GotopointAction, Vector2D, Targetvel, targetVel);
    SkillProperty(GotopointAction, Vector2D, Lookat, lookAt);


};

#endif // GotopointAction_HEADER_
