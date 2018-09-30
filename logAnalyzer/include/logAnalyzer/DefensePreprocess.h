//
// Created by rebinnaf on 7/17/18.
//

#ifndef LOGANALYZER_DEFENSEPREPROCESS_H
#define LOGANALYZER_DEFENSEPREPROCESS_H


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

enum class BallPossesion {
    ours = 0,
    theirs = 1,
};


class DefensePreprocess {

public:
    DefensePreprocess();
    ~DefensePreprocess();
    ros::NodeHandle n;
    ros::NodeHandle n_private;

    ros::Subscriber wm_sub;
    ros::Subscriber ref_sub;

    void wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
    void refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref);
    void preprocess();
    void writeData();
    void clearLists();
    int isoppNearest();
    bool isPlayingTime();
    void updateBP();
    void getNearestRobotToPoint(Vector2D _point);
    QFile myfile;
    QTextStream AnalyzeDS;
    QList<double> ourImeasure;
    QList<int> ourSIndex;
    QList<double> ourdistances;
    QList<double> ourangles;
    QList<double> oppImeasure;
    QList<int> oppSIndex;
    QList<double> oppdistances;
    QList<double> oppangles;

    double anglemeasure,distmeasure;
    double balldistance;
    double ballangle;

    BallPossesion BP;
    int ourBPID,oppBPID;


private:
    parsian_msgs::parsian_world_modelConstPtr wm;
    parsian_msgs::ssl_refree_wrapperConstPtr ref;
    int refcommand;
    int refstage;
    CField field;




};


#endif //LOGANALYZER_DEFENSEPREPROCESS_H


