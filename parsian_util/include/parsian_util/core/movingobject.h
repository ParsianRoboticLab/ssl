//
// Created by parsian-ai on 9/22/17.
//

#ifndef PARSIAN_UTIL_MOVINGOBJECT_H
#define PARSIAN_UTIL_MOVINGOBJECT_H

#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_robot.h>
#include <parsian_util/tools/blackboard.h>
#include <parsian_util/tools/drawer.h>

class CMovingObject {
public:
    CMovingObject();
    CMovingObject(const parsian_msgs::parsian_robot& _pr);
    //Final Specifications
    Vector2D pos;
    Vector2D dir;
    Vector2D vel;
    Vector2D acc;

    double angularVel;

    double inSight;
    double obstacleRadius;
    int cam_id;

    void update(const parsian_msgs::parsian_robot& _newMO);

};

#endif //PARSIAN_UTIL_MOVINGOBJECT_H
