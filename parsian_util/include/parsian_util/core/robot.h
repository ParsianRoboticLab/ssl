//
// Created by parsian-ai on 10/13/17.
//

#ifndef PARSIAN_UTIL_ROBOT_H
#define PARSIAN_UTIL_ROBOT_H

#include "parsian_util/geom/geom.h"
#include "parsian_msgs/parsian_robot.h"
#include "parsian_util/core/movingobject.h"
#include "parsian_util/base.h"

namespace Robot {
constexpr double robot_radius_old = 0.090;
constexpr double robot_radius_new = 0.0890;
constexpr double center_from_kicker_old = 0.075;
constexpr double center_from_kicker_new = 0.06;
constexpr double kicker_width_old = 0.074;
constexpr double kicker_width_new = 0.082;
constexpr double wheel_rad_old = 0.035;
constexpr double wheel_rad_new = 0.027;
};

using namespace Robot;

class CRobot : public CMovingObject {
public:

    CRobot(int id);
    CRobot(const parsian_msgs::parsian_robot& _robot);

//    bool isActive();
//    void setActive(bool _active);
    int id;


    void setNewRobot(bool _new) {
        newRobot = _new;
    }

    bool isNewRobot() {
        return newRobot;
    }

    double robotRadius() {
        return (newRobot) ? robot_radius_new : robot_radius_old;
    }

    double kickerWidth() {
        return (newRobot) ? kicker_width_new : kicker_width_old;
    }

    double centerFromKicker() {
        return (newRobot) ? center_from_kicker_new : center_from_kicker_old;
    }

    double wheelRadius() {
        return (newRobot) ? wheel_rad_new : wheel_rad_old;
    }

    Vector2D getKickerPos(double margin = 0) {
        return pos + dir * (centerFromKicker() + margin);
    }




private:

    Property(bool, Active, active);
    bool newRobot;
//    bool active;

};



#endif //PARSIAN_UTIL_ROBOT_H
