// auto-generated don't edit !

#include <parsian_util/action/autogenerate/gotopointaction.h>

GotopointAction::GotopointAction() {
    dynamicStart = false;
    maxAcceleration = 0.0;
    maxDeceleration = 0.0;
    maxVelocity = 0.0;
    oneTouchMode = false;
    slowMode = false;
    penaltyKick = false;
    diveMode = false;
    smooth = false;
    roller = 0;
}

void GotopointAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_gotoPoint msg = *((parsian_msgs::parsian_skill_gotoPoint*)_msg);
    dynamicStart = msg.dynamicStart;
    maxAcceleration = msg.maxAcceleration;
    maxDeceleration = msg.maxDeceleration;
    maxVelocity = msg.maxVelocity;
    oneTouchMode = msg.oneTouchMode;
    slowMode = msg.slowMode;
    penaltyKick = msg.penaltyKick;
    diveMode = msg.diveMode;
    smooth = msg.smooth;
    roller = msg.roller;
    targetPos = msg.targetPos;
    targetDir = msg.targetDir;
    targetVel = msg.targetVel;
    lookAt = msg.lookAt;

}

void* GotopointAction::getMessage() {
    parsian_msgs::parsian_skill_gotoPoint* _msg = new parsian_msgs::parsian_skill_gotoPoint;
    _msg->dynamicStart = dynamicStart;
    _msg->maxAcceleration = maxAcceleration;
    _msg->maxDeceleration = maxDeceleration;
    _msg->maxVelocity = maxVelocity;
    _msg->oneTouchMode = oneTouchMode;
    _msg->slowMode = slowMode;
    _msg->penaltyKick = penaltyKick;
    _msg->diveMode = diveMode;
    _msg->smooth = smooth;
    _msg->roller = roller;
    _msg->targetPos = targetPos.toParsianMessage();
    _msg->targetDir = targetDir.toParsianMessage();
    _msg->targetVel = targetVel.toParsianMessage();
    _msg->lookAt = lookAt.toParsianMessage();
    return _msg;

}


QString GotopointAction::getActionName() {
    return SActionName();
}

QString GotopointAction::SActionName() {
    return QString{"GotopointAction"};
}

