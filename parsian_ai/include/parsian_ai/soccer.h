//
// Created by parsian-ai on 9/22/17.
//

#ifndef PARSIAN_AI_SOCCER_H
#define PARSIAN_AI_SOCCER_H

#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_util/base.h>
#include <parsian_ai/util/agent.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/coach.h>
#include <parsian_ai/gamestate.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/onetouchaction.h>
#include <parsian_util/action/autogenerate/receivepassaction.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/exprimental.h>


class CSoccer {

public:

    CSoccer();
    ~CSoccer();
    void execute();

    virtual void customControl(bool& custom);

    CCoach* getCoach();

    Agent **agents;
    CCoach *coach;
    Exprimental* exp;
    clock_t realTimeTime;
    struct timeval t1, t2;
private:


};

extern CSoccer* soccer;


#endif //PARSIAN_AI_SOCCER_H
