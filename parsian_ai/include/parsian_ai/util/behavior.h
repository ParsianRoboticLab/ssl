#ifndef BEHAVIOR_H
#define BEHAVIOR_H


#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/ssl_refree_wrapper.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_msgs/parsian_behavior.h>
#include <parsian_ai/util/agent.h>
#include <parsian_msgs/parsian_ai_status.h>

class Behavior
{
public:
    Behavior(std::string _name, std::string _description) : name(_name), description(_description) {}
    virtual int execute() = 0;
    virtual double eval(parsian_msgs::parsian_behaviorPtr _behav) = 0;
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    double probability() const { return prob; }
    virtual void init(QList<Agent*>& _agents, const parsian_msgs::parsian_behaviorConstPtr& _msg) { agents = _agents; msg = _msg;}
    void updateReferee(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref) { /*TODO FIX*/ }
    void updateWM(const parsian_msgs::parsian_world_modelConstPtr& _wm) { wm->update( _wm); }
    void updateAIStatus(const parsian_msgs::parsian_ai_statusConstPtr& _ais) { status = _ais; }
    virtual double process() { return 0.0; } //// From 0 to 1 ~> 1 is Finished
protected:
    QList<Agent*> agents;
    parsian_msgs::parsian_behaviorConstPtr msg;
    parsian_msgs::parsian_ai_statusConstPtr status;
    double prob;
    const std::string name;
    const std::string description;
};

#endif // BEHAVIOR_H
