//
// Created by ali on 3/12/18.
//

#ifndef PARSIAN_UTIL_BLACKBOARD_H
#define PARSIAN_UTIL_BLACKBOARD_H

#include <ros/ros.h>
#include <QVector2D>

#define D_ERROR      0
#define D_GAME       1
#define D_EXPERIMENT 2
#define D_DEBUG      3
#define D_NADIA      4
#define D_MAHI       5
#define D_ALI        6
#define D_MHMMD      7
#define D_FATEME     8
#define D_AHZ        9
#define D_AMIN       10
#define D_PARSA      11
#define D_HAMED      12
#define D_SEPEHR     13
#define D_KK         14
#define D_HOSSEIN    15
#define D_ATOUSA     16


#define PDEBUG(prefix, value, type) MONITOR(prefix,value,type)
#define NDEBUG(value, type) MONITOR(#value,value,type)
#define PDEBUGV2D(prefix, value, type) MONITOR_V2D(prefix,value,type)
#define NDEBUGV2D(value, type) MONITOR_V2D(#value,value,type)
#define DBUG(value, type) MONITOR("",value,type)
#define DEBUG(value, type) MONITOR("",value,type)

#define MONITOR_V2D(prefix,var,type) \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug);\
    ROS_DEBUG_STREAM(std::string(ROSCONSOLE_NAME_PREFIX) + "::" +QString("%1::").arg(type).toStdString()<<\
                            (prefix)<< " = x:"<<var.x<<"  y:"<<var.y);\
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);

#define MONITOR(prefix,var,type) \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug);\
    ROS_DEBUG_STREAM(std::string(ROSCONSOLE_NAME_PREFIX) + "::" + QString("%1::").arg(type).toStdString()<<\
                           (prefix)<<" = "<<QString("%1").arg(var).toStdString()); \
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info);

#endif //PARSIAN_UTIL_BLACKBOARD_H
