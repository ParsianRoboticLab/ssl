//
// Created by parsian-ai on 9/21/17.
//

#ifndef PARSIAN_UTIL_BALL_H
#define PARSIAN_UTIL_BALL_H

#include <parsian_msgs/parsian_robot.h>
#include "parsian_util/core/movingobject.h"
#include "parsian_msgs/parsian_robot.h"


class CBall : public CMovingObject {
public:
    explicit CBall(const parsian_msgs::parsian_robot& _robot);
    CBall();
    ~CBall();
    double whenBallReachToPoint(double dist) const;
    Vector2D getPosInFuture(double _t) const;
    Segment2D seg(const double& _size = 20) const;
    Ray2D path() const;
//    double distToBallReachToPoint(double time,Vector2D point);

    double getBallAcc() const;

    static const double radius;
private:


};


#endif //PARSIAN_UTIL_BALL_H
