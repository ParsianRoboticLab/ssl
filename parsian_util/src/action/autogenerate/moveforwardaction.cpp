// auto-generated don't edit !

#include <parsian_util/action/autogenerate/moveforwardaction.h>

MoveforwardAction::MoveforwardAction() {
       kickSpeed = 0.0;
       spin = 0;
       iskickchargetime = false;
       kickchargetime = 0.0;
}

void MoveforwardAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_moveForward msg = *((parsian_msgs::parsian_skill_moveForward*)_msg);
        kickSpeed = msg.kickSpeed;
        spin = msg.spin;
        iskickchargetime = msg.iskickchargetime;
        kickchargetime = msg.kickchargetime;
        targetPos = msg.targetPos;

}

void* MoveforwardAction::getMessage() {
    parsian_msgs::parsian_skill_moveForward* _msg = new parsian_msgs::parsian_skill_moveForward;
    _msg->kickSpeed = kickSpeed;
    _msg->spin = spin;
    _msg->iskickchargetime = iskickchargetime;
    _msg->kickchargetime = kickchargetime;
    _msg->targetPos = targetPos.toParsianMessage();
    return _msg;

}


QString MoveforwardAction::getActionName(){
    return SActionName();
}

QString MoveforwardAction::SActionName(){
    return QString{"MoveforwardAction"};
}

