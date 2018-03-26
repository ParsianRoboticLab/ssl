//
// Created by ali on 1/11/18.
//

#ifndef PARSIAN_TOOLS_LATENCY_INFO_H
#define PARSIAN_TOOLS_LATENCY_INFO_H

#include <ros/ros.h>
#include <QString>
#include <QQueue>
#include <parsian_msgs/parsian_robot_task.h>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/parsian_robot_command.h>
#include <parsian_msgs/parsian_path.h>
#include <parsian_msgs/parsian_packets.h>
#include <parsian_msgs/ssl_vision_detection.h>
#include <parsian_msgs/parsian_get_plan.h>

#define buffSize 600
namespace parsian_tools {


template<class T>
class LatencyInfo {
public:
    LatencyInfo(ros::NodeHandle &nh, const char *name, const char *topic);

    void call_back(const T &msg);

    std::string info();

    void updateetMinMax(int value);

private:
    ros::Subscriber subscriber;
    QQueue<int> latency_queue;
    long int min, max, sum;
    std::string name;
};



template<class T>
LatencyInfo<T>::LatencyInfo(ros::NodeHandle& nh, const char* name , const char* topic) {

    sum = 0;
    max = 0;
    min = 0x0fffffff;
    this->name = name;
    subscriber = nh.subscribe(topic, 20, &LatencyInfo::call_back, this);
}

template<class T>
void LatencyInfo<T>::call_back(const T &msg) {
    auto latencyT = ros::Time::now() - msg->header.stamp;
    auto latency = latencyT.sec * 1000000000 + latencyT.nsec;
    sum += latency;
    latency_queue.push_back(latency);

    updateetMinMax(latency);

    if (this->latency_queue.length() > buffSize) {
        sum -= latency_queue.dequeue();
        ROS_INFO_STREAM(this->info());
    }

}

template<class T>
std::string LatencyInfo<T>::info() {
    return name + QString("'s delay: min: %1  , mid: %2   ,  max: %3")
           .arg(min)
           .arg(sum / buffSize)
           .arg(max).toStdString();
}

template<class T>
void LatencyInfo<T>::updateetMinMax(int value) {
    if (value < min) {
        min = value;
    }
    if (value > max) {
        max = value;
    }
}


}


#endif //PARSIAN_TOOLS_LATENCY_INFO_H
