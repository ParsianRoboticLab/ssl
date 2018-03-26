//
// Created by ali on 3/12/18.
//

#ifndef PARSIAN_UTIL_BLACKBOARD_H
#define PARSIAN_UTIL_BLACKBOARD_H

#include <ros/ros.h>
#include <QVector2D>

#define MONITOR_V2D(var) \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug);\
    ROS_DEBUG_STREAM(/*getName() << */" :: " <<#var <<" = "<< "x:"<<var.x()<<" y:"<< var.y() );\
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);

#define MONITOR(var) \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug);\
    ROS_DEBUG_STREAM(/*getName() <<*/ " :: " <<#var <<" = "<<var ); \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);


#endif //PARSIAN_UTIL_BLACKBOARD_H
