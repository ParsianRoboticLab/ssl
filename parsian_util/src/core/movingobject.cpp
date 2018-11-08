//
// Created by parsian-ai on 9/22/17.
//

#include "parsian_util/core/movingobject.h"


CMovingObject::CMovingObject() = default;

CMovingObject::CMovingObject(const parsian_msgs::parsian_robot &_pr) :
    pos(_pr.pos),
    vel(_pr.vel),
    dir(_pr.dir),
    acc(_pr.acc),
    angularVel(_pr.angularVel),
    cam_id(_pr.camera_id),
    inSight(_pr.inSight),
    obstacleRadius(_pr.obstacleRadius) {
}

void CMovingObject::update(const parsian_msgs::parsian_robot &_newMO) {
    pos.assign(_newMO.pos.x, _newMO.pos.y);
    vel.assign(_newMO.vel.x, _newMO.vel.y);
    dir.assign(_newMO.dir.x, _newMO.dir.y);
    acc.assign(_newMO.acc.x, _newMO.acc.y);
    angularVel = _newMO.angularVel;
    cam_id = _newMO.camera_id;
    inSight = _newMO.inSight;
    obstacleRadius = _newMO.obstacleRadius;

}
