#include <behavior/mahi/mahi.h>

BehaviorMahi::BehaviorMahi()
    : Behavior("Mahi", "describe me") {

}

BehaviorMahi::~BehaviorMahi() {

}

int BehaviorMahi::execute() {

}

double BehaviorMahi::eval(parsian_msgs::parsian_behaviorPtr _behav) {
    if (wm->ball->pos.x > 0) {
        prob = 1;
    } else {
        prob = 0.5;
    }
//
//    _behav->roles.push_back(_behav->GK);
//    _behav->roles.push_back(_behav->PLAYMAKE);
//    _behav->roles.push_back(_behav->POSITION);
//    _behav->roles.push_back(_behav->SPECIAL);
//    _behav->roles.push_back(_behav->IDLE);
//    _behav->roles.push_back(_behav->IDLE);
//    _behav->roles.push_back(_behav->IDLE);
//    _behav->roles.push_back(_behav->IDLE);

//    _behav->target.x = wm->field->_FIELD_WIDTH/4;
//    if (wm->ball->pos.y > 0.5) {
//        _behav->target.y = -wm->field->_FIELD_WIDTH/4;
//    } else if (wm->ball->pos.y < -0.5) {
//        _behav->target.y = wm->field->_FIELD_WIDTH/4;
//    } else {
//        _behav->target.x = wm->field->oppGoal().x;
//        _behav->target.y = wm->field->oppGoal().y;
//    }
//
//    _behav->second_target = wm->field->oppGoal().toParsianMessage();
    return prob;
}
