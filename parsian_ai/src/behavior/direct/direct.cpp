#include <behavior/direct/direct.h>

BehaviorDirect::BehaviorDirect()
: Behavior("Direct", "describe me") {

}

BehaviorDirect::~BehaviorDirect() {

}

int BehaviorDirect::execute() {
    return 0;
}

double BehaviorDirect::eval(parsian_msgs::parsian_behaviorPtr _behav) {
    _behav = nullptr;
    return 0.0;
}

