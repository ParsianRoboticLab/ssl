#ifndef PLAYOFF_H
#define PLAYOFF_H

#include "parsian_ai/plays/masterplay.h"
#include <parsian_msgs/parsian_ai_plan_request.h>
#include <parsian_msgs/parsian_plan.h>
#include <parsian_ai/plays/playoff/staticplayoff.h>
#include <parsian_ai/plays/playoff/dynamicplayoff.h>
#include <parsian_ai/plays/playoff/firstplayoff.h>
#include <parsian_ai/plays/playoff/abstractplayoff.h>

enum class POMode {
    None    = 0,
    First   = 1,
    Static  = 2,
    Dynamic = 3
};

class CPlayOff : public CMasterPlay {

public:
    CPlayOff();
    ~CPlayOff() override;
    void execute_x() override;
    void init(QList<Agent*>& _agents) override;

    QString whoami() override { return "PlayOff"; }
    bool deleted;
    void reset() override;

    parsian_ai_plan_request getRequest(const int& _agentSize, const unsigned char& mode);
    void setResponse(const parsian_plan& _plan);
    void decideMode(const int& _agentSize);
    void setPlanClient(ros::ServiceClientPtr _client);
private:

    void reInitPlay();

    ros::ServiceClientPtr client;
    CFirstPlayOff *firstPlayoff;
    CStaticPlayOff *staticPlayOff;
    CDynamicPlayOff *dynamicPlayoff;
    CAbstractPlayOff *selectedPlayoff;
    bool gotPlan;
    POMode mode;


};

#endif // CPLAYOFF_H
