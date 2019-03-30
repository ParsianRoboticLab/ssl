// auto-generated don't edit !

#ifndef MoveforwardAction_HEADER_
#define MoveforwardAction_HEADER_



#include <parsian_util/action/action.h>
#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_skill_moveForward.h>
#include <list>

class MoveforwardAction : public Action {

public:
    MoveforwardAction();
    void setMessage(const void* _msg);

    void* getMessage();

    QString getActionName() override;
    static QString SActionName();


    SkillProperty(MoveforwardAction, double, Kickspeed, kickSpeed);
    SkillProperty(MoveforwardAction, int, Spin, spin);
    SkillProperty(MoveforwardAction, bool, Iskickchargetime, iskickchargetime);
    SkillProperty(MoveforwardAction, double, Kickchargetime, kickchargetime);
    SkillProperty(MoveforwardAction, Vector2D, Target, target);
    SkillProperty(MoveforwardAction, Vector2D, Waitreceivepos, waitReceivePos);


};

#endif // MoveforwardAction_HEADER_
