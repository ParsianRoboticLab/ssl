#ifndef VISIONCLIENT_H
#define VISIONCLIENT_H

#include <fstream>
#include <ostream>
#include <vector>

#include <QByteArray>
#include <QDebug>
#include <QTime>
#include <QList>

#include <parsian_util/geom/geom.h>
#include <parsian_util/mathtools.h>
#include <parsian_msgs/ssl_vision_detection.h>

#include <parsian_world_model/wm/visionbelief.h>
#include <parsian_world_model/config.h>
#include <parsian_msgs/parsian_team_config.h>


#define CAMERA_NUM 8
#define OUT_OF_SIGHT_THRESHOLD 40

class CVisionClient {
public:
    QTime *vcTimer;
    CVisionBelief v[CAMERA_NUM];
//    CVisionBelief mv[CAMERA_NUM];
    CVisionBelief res;
    double sampleT;
    int lastCamera;
    int activeCameras;
    int frameCnt;

    std::vector<Vector2D> boundaries[CAMERA_NUM];

    CVisionClient();
    ~CVisionClient();

    void parse(const parsian_msgs::ssl_vision_detectionConstPtr& packet);
    void merge(int camera_count = CAMERA_NUM);
};

#define MAX_OBJECT 5
#define __RECEIVE_ROBOTS_DATA(__COLOR__,__TEAM__)\
for (int i = 0; i < packet->##__COLOR__##.size();i++) { \
    int rob_id = packet->##__COLOR__##[i].robot_id; \
    if (v[id].__TEAM__##Team[rob_id].count() >= MAX_OBJECT) continue; \
    CRawObject raw = CRawObject(frameCnt, Vector2D(packet->##__COLOR__##[i].pos.x / 1000.0f,packet->##__COLOR__##[i].pos.y / 1000.0f), \
                                packet->##__COLOR__##[i].orientation*180.0f/M_PI \
                                ,i ,packet->##__COLOR__##[i].confidence, nullptr, id); \
    for (int k=0;k<v[id].__TEAM__##Team[rob_id].count();k++) \
    { \
            if ((v[id].__TEAM__##Team[rob_id][k].pos - raw.pos).length() < 0.5) \
            { \
                    v[id].__TEAM__##Team[rob_id].removeAt(k); \
            } \
    } \
    v[id].__TEAM__##Team[rob_id].append(raw); \
    v[id].outofsight_##__TEAM__##Team[rob_id] = 0; \
    __TEAM__##_insight[rob_id] = true; \
}


#endif // VISIONCLIENT_H
