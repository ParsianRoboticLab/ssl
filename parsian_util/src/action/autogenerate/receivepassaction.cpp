// auto-generated don't edit !

#include <parsian_util/action/autogenerate/receivepassaction.h>

ReceivepassAction::ReceivepassAction() {
       slow = false;
       receiveRadius = 0.0;
       ignoreAngle = false;
}

void ReceivepassAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_receivePass msg = *((parsian_msgs::parsian_skill_receivePass*)_msg);
        slow = msg.slow;
        receiveRadius = msg.receiveRadius;
        ignoreAngle = msg.ignoreAngle;
        target = msg.target;
        IATargetDir = msg.IATargetDir;

}

void* ReceivepassAction::getMessage() {
    parsian_msgs::parsian_skill_receivePass* _msg = new parsian_msgs::parsian_skill_receivePass;
    _msg->slow = slow;
    _msg->receiveRadius = receiveRadius;
    _msg->ignoreAngle = ignoreAngle;
    _msg->target = target.toParsianMessage();
    _msg->IATargetDir = IATargetDir.toParsianMessage();
    return _msg;

}


QString ReceivepassAction::getActionName(){
    return SActionName();
}

QString ReceivepassAction::SActionName(){
    return QString{"ReceivepassAction"};
}

