//
// Created by parsian-ai on 10/5/17.
//

#ifndef PARSIAN_WORLD_MODEL_VISIONBELIEF_H
#define PARSIAN_WORLD_MODEL_VISIONBELIEF_H

#include <QList>
#include <parsian_world_model/wm/movingobject.h>

#define _MAX_NUM_PLAYERS 12

class CVisionBelief {
public:
    QList<CRawObject> ourTeam[_MAX_NUM_PLAYERS];
    QList<CRawObject> oppTeam[_MAX_NUM_PLAYERS];
    QList<CRawObject> ball;
    double visionLatency;
    double timeStep, ltcapture;
    double time;
    int cam_id;

    //count of frames that each object was out of sight
    int outofsight_ourTeam[_MAX_NUM_PLAYERS];
    int outofsight_oppTeam[_MAX_NUM_PLAYERS];
    int outofsight_ball;

    bool updated; //indicates that camera is ever updated or not
    int lastUpdateTime;
    CVisionBelief();
    void reset();
};

#endif //PARSIAN_WORLD_MODEL_VISIONBELIEF_H
