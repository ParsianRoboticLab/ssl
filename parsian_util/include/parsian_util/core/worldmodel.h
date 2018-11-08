#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include "parsian_util/geom/geom.h"
#include "parsian_msgs/parsian_world_model.h"
#include "parsian_util/core/field.h"
#include "parsian_util/core/ball.h"
#include "parsian_util/core/agent.h"
#include "parsian_util/core/team.h"
#include "parsian_msgs/ssl_refree_wrapper.h"

class CWorldModel {
public:
    CWorldModel();
    CWorldModel(const parsian_msgs::parsian_world_model& _pwm);
    CField* field;
    CBall* ball;
    CTeam our, opp;

    void update(const parsian_msgs::parsian_world_modelConstPtr& _newWM);

private:

};

//extern CWorldModel* wm;

#endif // WORLD_MODEL_H
