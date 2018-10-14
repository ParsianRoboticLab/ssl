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

enum class BallPossession {
    ours = 0,
    theirs = 1,
    draw=2,
    noOne=3
};



class DefensePreprocess {

public:
    DefensePreprocess();
    ~DefensePreprocess();
    ros::NodeHandle n;
    ros::NodeHandle n_private;

    ros::Subscriber wm_sub;
    ros::Subscriber ref_sub;

    ros::Publisher monitor_pub;

    void wmCb(const parsian_msgs::parsian_world_modelConstPtr& _wm);
    void refCb(const parsian_msgs::ssl_refree_wrapperConstPtr & _ref);
    void preprocess();
    void coachProcess();
    void writeData();
    void clearLists();
    QFile myfile;
    QTextStream AnalyzeDS;
    QList<double> ourImeasure,oppImeasure;
    QList<int> ourSIndex,oppSIndex;
    QList<double> ourdistances,oppdistances;
    QList<Vector2D> ourVelNorm,oppVelNorm,ourVels,oppVels;
    QList<double> ourangles,oppangles,ourvellength,oppVellength;

    QMap<int, int> markMap;

    double anglemeasure,distmeasure;
    double balldistance;
    double ballangle;



    int oppCoachDef, oppCoachMark;




    bool outflag=false;
    Vector2D ballvel,ballPos;

    BallPossession BP, BPsaved=BallPossession ::ours, BPLast=BallPossession ::ours;
    int ourBPID,oppBPID;


    bool isPlayingTime();
    BallPossession getPossession();
    void updatewm();
    void getNearestRobotToPoint(Vector2D _point);
    bool validPossession();


private:
    parsian_msgs::parsian_world_modelConstPtr wm;
    parsian_msgs::ssl_refree_wrapperConstPtr ref;
    int refcommand;
    int refstage;
    CField field;




};


#endif //LOGANALYZER_DEFENSEPREPROCESS_H


