//
// Created by rebinnaf on 9/13/18.
//

#ifndef LOGANALYZER_STATISTICALANALYZER_H
#define LOGANALYZER_STATISTICALANALYZER_H


#include <ros/ros.h>
#include <iostream>
#include <fstream>
#include <parsian_msgs/parsian_world_model.h>
#include <parsian_msgs/ssl_refree_wrapper.h>
#include <ros/package.h>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <parsian_util/core/field.h>
#include <jmorecfg.h>


enum class BallPossesion {
    ours = 0,
    theirs = 1,
    draw=2
};


class StatisticalAnalyzer {
public:
    StatisticalAnalyzer();
    ~StatisticalAnalyzer();
    ros::NodeHandle n;
    ros::NodeHandle n_private;

    ros::Subscriber wm_sub;
    ros::Subscriber ref_sub;

    void wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
    void refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref);
    void preprocess();
    bool validShot();
    bool validPass();
    bool validPossession();
    void writeToShot();
    void writeToPass();
    void writeToPossession();
    QFile possessionFile, shotFile, passFile;
    QTextStream possessionDS, shotDS, passDS;
    Vector2D ballvel,ballPos,
            passerRobot,receiverRobot,passDir,
            shotterRobot,shotTarget,shotDir;

    bool shotInGoal=false;
    BallPossesion BP;
    int ourBPID,oppBPID;
    bool shotFlag=false, PassFlag;

    bool isPlayingTime();
    BallPossesion getPossession();
    void updatewm();
    void getNearestRobotToPoint(Vector2D _point);

private:
    parsian_msgs::parsian_world_modelConstPtr wm;
    parsian_msgs::ssl_refree_wrapperConstPtr ref;
    int refcommand;
    int refstage;
    CField field;
};


#endif //LOGANALYZER_STATISTICALANALYZER_H
