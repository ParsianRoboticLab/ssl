#ifndef BEHAVIORDIRECT_H
#define BEHAVIORDIRECT_H

#include<parsian_ai/util/behavior.h>

class BehaviorDirect: public Behavior {

public:
    BehaviorDirect();
    virtual ~BehaviorDirect();
    virtual int execute();
    virtual double eval(parsian_msgs::parsian_behaviorPtr _behav);

protected:
    void findKicker();
    void assignKick(Agent* const _kicker);
    void generateFormation();
    void getFormation();
    void match(QList<Agent*>& _agents, GotopointavoidAction* _gpa[]);
protected:
    Agent* kicker;
    KickAction* kick;
    GotopointavoidAction* gpa[12];
};

#endif // BEHAVIORDIRECT_H
