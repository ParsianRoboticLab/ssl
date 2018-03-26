#ifndef BEHAVIORDIRECT_H
#define BEHAVIORDIRECT_H

#include<parsian_ai/util/behavior.h>

class BehaviorDirect: public Behavior {

public:
    BehaviorDirect();
    virtual ~BehaviorDirect();
    virtual int execute();
    virtual double eval(parsian_msgs::parsian_behaviorPtr _behav);
};

#endif // BEHAVIORDIRECT_H
