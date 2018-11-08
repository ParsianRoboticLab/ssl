// auto-generated don't edit !

#include <parsian_util/action/autogenerate/gotopointavoidaction.h>

GotopointavoidAction::GotopointavoidAction() {
       chip = false;
       kickSpeed = 0.0;
       chipDist = 0.0;
       oneTouchFlag = false;
       avoidPenaltyArea = false;
       noAvoid = false;
       avoidCenterCircle = false;
       ballObstacleRadius = 0.0;
       drawPath = false;
       diveMode = false;
}

void GotopointavoidAction::setMessage(const void* _msg) {
    parsian_msgs::parsian_skill_gotoPointAvoid msg = *((parsian_msgs::parsian_skill_gotoPointAvoid*)_msg);
        chip = msg.chip;
        kickSpeed = msg.kickSpeed;
        chipDist = msg.chipDist;
        oneTouchFlag = msg.oneTouchFlag;
        avoidPenaltyArea = msg.avoidPenaltyArea;
        noAvoid = msg.noAvoid;
        avoidCenterCircle = msg.avoidCenterCircle;
        ballObstacleRadius = msg.ballObstacleRadius;
        drawPath = msg.drawPath;
        diveMode = msg.diveMode;
        addVel = msg.addVel;

        GotopointAction::setMessage(&msg.base);
}

void* GotopointavoidAction::getMessage() {
    parsian_msgs::parsian_skill_gotoPointAvoid* _msg = new parsian_msgs::parsian_skill_gotoPointAvoid;
    _msg->base = *((parsian_msgs::parsian_skill_gotoPoint*) GotopointAction::getMessage());
    _msg->chip = chip;
    _msg->kickSpeed = kickSpeed;
    _msg->chipDist = chipDist;
    _msg->oneTouchFlag = oneTouchFlag;
    _msg->avoidPenaltyArea = avoidPenaltyArea;
    _msg->noAvoid = noAvoid;
    _msg->avoidCenterCircle = avoidCenterCircle;
    _msg->ballObstacleRadius = ballObstacleRadius;
    _msg->drawPath = drawPath;
    _msg->diveMode = diveMode;
    _msg->addVel = addVel.toParsianMessage();
    return _msg;

}


QString GotopointavoidAction::getActionName(){
    return SActionName();
}

QString GotopointavoidAction::SActionName(){
    return QString{"GotopointavoidAction"};
}

void GotopointavoidAction::clearOurrelax() {
    ourrelax.clear();
}

void GotopointavoidAction::addOurrelax(unsigned char _t) {
    ourrelax.push_back(_t);
}
void GotopointavoidAction::removeOurrelax(unsigned char _t) {
    ourrelax.remove(_t);
}
std::list<unsigned char> GotopointavoidAction::getOurrelax() {
    return ourrelax;
}
void GotopointavoidAction::clearTheirrelax() {
    theirrelax.clear();
}

void GotopointavoidAction::addTheirrelax(unsigned char _t) {
    theirrelax.push_back(_t);
}
void GotopointavoidAction::removeTheirrelax(unsigned char _t) {
    theirrelax.remove(_t);
}
std::list<unsigned char> GotopointavoidAction::getTheirrelax() {
    return theirrelax;
}
