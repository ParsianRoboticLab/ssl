#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QList>
#include <vector>
#include <stdio.h>

#include "parsian_util/tools/drawer.h"                      //must be newd in node
#include "parsian_util/tools/debuger.h"
#include "parsian_util/geom/geom.h"
#include "parsian_util/core/agent.h"
#include "planner/state.h"

#define OBS_RECTANGLE  0
#define OBS_CIRCLE     1

//global function
double boundTo(double x, double low, double high);


class obstacle {
public:
    Vector2D pos;
    Vector2D rad;
    Vector2D vel;  // object velocity
    int type;
public:
    double margin(state s , double obsMargin);           //faseleye marze obstacle ta state(- shoae robot haobsmargin())
    bool check(state s , double obsMargin);              //aya state s ba margine obsmargin dar obstacle hast ya na
    bool check(state s0, state s1 , double obsMargin);   //check kardane 2 noghteE ?????
};


class CObstacles {
public:
    QList <obstacle> obs;
public:
    CObstacles();
    Vector2D targetPosition, agentPos;
    void clear() {
        obs.clear();
    }
    void add_rectangle(double cx, double cy, double w, double h);     //add rect with given properties to obs
    void add_circle(double x, double y, double radius,                //add circle with given properties to obs
                    double vx, double vy);

    //check wether or not the state(s)/vector2D(s) are in obstacles or not
    //the id declare which obstacles are collisened
    bool check(Vector2D p);
    bool check(Vector2D p, QList<int> &id);
    bool check(state s);
    bool check(state s, QList<int> &id);
    bool check(state s0, state s1);
    bool check(state s0, state s1, QList<int> &id);
    bool check(Vector2D p1, Vector2D p2);
    void draw();      //deaw the obstacles

    double obsMargin;
};




#endif // OBSTACLE_H
