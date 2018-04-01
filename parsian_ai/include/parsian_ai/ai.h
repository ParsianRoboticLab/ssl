//
// Created by parsian-ai on 9/22/17.
//

#ifndef PARSIAN_AI_AI_H
#define PARSIAN_AI_AI_H

#include <parsian_msgs/parsian_team_config.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_msgs/ssl_refree_wrapper.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_util/core/knowledge.h>
#include <parsian_ai/soccer.h>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_ai/config.h>


class AI {
public:
    AI();
    ~AI();

    void execute();

    void updateWM(const parsian_msgs::parsian_world_modelConstPtr&);
    void updateRobotStatus(const parsian_msgs::parsian_robotConstPtr&);
    void updateReferee(const parsian_msgs::ssl_refree_wrapperConstPtr&);
    CSoccer* getSoccer();

    parsian_msgs::parsian_draw   getDraw() {
        return drawer->draws;
    }

    parsian_msgs::parsian_robot_task getTask(int robotID);

private:
    parsian_msgs::parsian_robot_task robotsTask[_MAX_NUM_PLAYERS];

};

#endif //PARSIAN_AI_AI_H
