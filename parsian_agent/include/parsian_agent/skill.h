#ifndef BASICSKILL_H
#define BASICSKILL_H

#include <parsian_agent/newbangbang.h>
#include <parsian_util/base.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_msgs/parsian_agent.h>
#include <parsian_util/core/worldmodel.h>
#include <parsian_util/geom/geom.h>
#include <QtCore/QStringList>
#include <parsian_util/action/action.h>
#include <parsian_agent/agent.h>
#include <parsian_util/tools/blackboard.h>
#include <parsian_util/tools/drawer.h>

using namespace rcsc;

class CSkill {
public:
    explicit CSkill(Agent* _agent);
    ~CSkill();
    //these functions should be defined in child classes
    virtual void execute() = 0;

protected:
    Agent * const agent;
};

#endif // BASICSKILL_H
