// auto-generated don't edit !

#include <parsian_util/action/autogenerate/onetouchaction.h>

OnetouchAction::OnetouchAction() {
       chip = false;
       iskickdischargetime = false;
       kickSpeed = 0.0;
       chipdist = 0.0;
       kickdischargetime = 0.0;
       shotToEmptySpot = false;
       fastestPoint = false;
       reachBeforeBallTime = 0.0;
}

void OnetouchAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_oneTouch msg = *((parsian_msgs::parsian_skill_oneTouch*)_msg);
        chip = msg.chip;
        iskickdischargetime = msg.iskickdischargetime;
        kickSpeed = msg.kickSpeed;
        chipdist = msg.chipdist;
        kickdischargetime = msg.kickdischargetime;
        shotToEmptySpot = msg.shotToEmptySpot;
        fastestPoint = msg.fastestPoint;
        reachBeforeBallTime = msg.reachBeforeBallTime;
        waitPos = msg.waitPos;
        target = msg.target;

}

void* OnetouchAction::getMessage() {
    parsian_msgs::parsian_skill_oneTouch* _msg = new parsian_msgs::parsian_skill_oneTouch;
    _msg->chip = chip;
    _msg->iskickdischargetime = iskickdischargetime;
    _msg->kickSpeed = kickSpeed;
    _msg->chipdist = chipdist;
    _msg->kickdischargetime = kickdischargetime;
    _msg->shotToEmptySpot = shotToEmptySpot;
    _msg->fastestPoint = fastestPoint;
    _msg->reachBeforeBallTime = reachBeforeBallTime;
    _msg->waitPos = waitPos.toParsianMessage();
    _msg->target = target.toParsianMessage();
    return _msg;

}


QString OnetouchAction::getActionName(){
    return SActionName();
}

QString OnetouchAction::SActionName(){
    return QString{"OnetouchAction"};
}

