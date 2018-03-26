#ifndef BEHAVIORMAHI_H
#define BEHAVIORMAHI_H

#include<parsian_ai/util/behavior.h>

class BehaviorMahi: public Behavior {

public:
    BehaviorMahi();
    virtual ~BehaviorMahi();
    virtual int execute();
    virtual double eval(parsian_msgs::parsian_behaviorPtr _behav);
};

#endif // BEHAVIORMAHI_H
