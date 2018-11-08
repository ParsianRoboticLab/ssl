//
// Created by parsian-ai on 9/21/17.
//
#include "parsian_util/core/worldmodel.h"

//CWorldModel* wm;

CWorldModel::CWorldModel() : our{false, false}, opp{true, true} {
    field = new CField();
    ball  = new CBall();
    ROS_INFO("WM_UTIL");
}

CWorldModel::CWorldModel(const parsian_msgs::parsian_world_model &_pwm) :
    our{_pwm.isYellow, _pwm.isLeft, _pwm.our},
    opp{!_pwm.isYellow, !_pwm.isLeft, _pwm.opp} {
    this->field = new CField();
    this->ball = new CBall(_pwm.ball);
}

void CWorldModel::update(const parsian_msgs::parsian_world_modelConstPtr& _newWM) {

    this->ball->update(_newWM->ball);

    this->our.updateRobot(_newWM->our);
    this->our.setColor(_newWM->isYellow);
    this->our.setSide(_newWM->isLeft);

    this->opp.updateRobot(_newWM->opp);
    this->opp.setColor(!_newWM->isYellow);
    this->opp.setSide(!_newWM->isLeft);
}
