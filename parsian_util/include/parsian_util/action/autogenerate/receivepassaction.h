// auto-generated don't edit !

#ifndef ReceivepassAction_HEADER_
#define ReceivepassAction_HEADER_



#include <parsian_util/action/action.h>
#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_skill_receivePass.h>
#include <list>

class ReceivepassAction : public Action {

public:
    ReceivepassAction();
    void setMessage(const void* _msg);

    void* getMessage();

    QString getActionName() override;
    static QString SActionName();


    SkillProperty(ReceivepassAction, bool, Slow, slow);
    SkillProperty(ReceivepassAction, double, Receiveradius, receiveRadius);
    SkillProperty(ReceivepassAction, bool, Ignoreangle, ignoreAngle);
    SkillProperty(ReceivepassAction, Vector2D, Target, target);
    SkillProperty(ReceivepassAction, Vector2D, Iatargetdir, IATargetDir);


};

#endif // ReceivepassAction_HEADER_
