//
// Created by parsian-ai on 9/19/17.
//

#ifndef PARSIAN_WORLD_MODEL_WORLDMODEL_H
#define PARSIAN_WORLD_MODEL_WORLDMODEL_H

#include "QObject"
#include "QDebug"
#include <nodelet/nodelet.h>


#include "parsian_msgs/parsian_world_model.h"
#include "parsian_msgs/ssl_vision_detection.h"
#include "parsian_msgs/ssl_vision_geometry.h"
#include <parsian_world_model/wm/visionclient.h>
#include <parsian_world_model/wm/halfworld.h>
#include <parsian_world_model/wm/ball.h>
#include <parsian_world_model/wm/robot.h>

//#include "parsian_world_model/world_modelConfig.h"

class WorldModel {
public:
    WorldModel();
    ~WorldModel();

    void updateDetection(const parsian_msgs::ssl_vision_detectionConstPtr&);
    void updateGeom(const parsian_msgs::ssl_vision_geometryConstPtr&);
    void execute();
    void merge(int frame);
    void init();
    void setMode(bool isSimulation);

    double vForwardCmd[12], vNormalCmd[12], vAngCmd[12];

    parsian_msgs::parsian_world_modelPtr getParsianWorldModel();
    Robot* them[_MAX_NUM_PLAYERS];
    Robot* us[_MAX_NUM_PLAYERS];

private:
    parsian_msgs::parsian_robot rosRobots[_MAX_NUM_PLAYERS * 2];
    parsian_msgs::parsian_robot rosBall;
    CVisionClient *vc;

    CHalfWorld w;
    CHalfWorld mergedHalfWorld;

    CBall* ball;
    bool simulationMode;
    void run();
    void update(CHalfWorld*);
    void testFunc(const parsian_msgs::ssl_vision_detectionConstPtr & packet);
    void printRobotInfo(const parsian_msgs::ssl_vision_detection_robot &robot);

    void toParsianMessage(const Robot* _robot, int id);
    void toParsianMessage(const CBall* _ball);


    parsian_msgs::ssl_vision_detectionConstPtr detection;

    double visionFPS;
    double visionLatency;
    double visionTimestep;
    double visionProcessTime;

    int packs;


};


#endif //PARSIAN_WORLD_MODEL_WORLDMODEL_H
