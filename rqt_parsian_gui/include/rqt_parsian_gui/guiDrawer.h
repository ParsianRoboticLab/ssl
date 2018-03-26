//
// Created by rebinnaf on 10/19/17.
//

#ifndef RQT_PARSIAN_GUI_DRAWER_H
#define RQT_PARSIAN_GUI_DRAWER_H


#include <QQueue>
#include <QColor>
#include <GL/gl.h>
#include <parsian_util/geom/geom.h>
#include <parsian_msgs/parsian_draw_circle.h>
#include <parsian_msgs/parsian_draw_polygon.h>
#include <parsian_msgs/parsian_draw_rect.h>
#include <parsian_msgs/parsian_draw_segment.h>
#include <parsian_msgs/parsian_draw_vector.h>
#include <parsian_msgs/parsian_draw_text.h>

class guiBall {
public:

};
class CGraphicalRobot {
public:
    CGraphicalRobot(Vector2D _pos = Vector2D(0, 0), Vector2D _dir = Vector2D(1, 0), QColor _color = QColor(255, 255, 0), int _ID = 0, int _comID = 0, QString _str = "", bool _newRobots = false) {
        pos = _pos;
        dir = _dir;
        color = _color;
        ID = _ID;
        comID = _comID;
        str = _str;
        newRobots = _newRobots;
    }
    int ID, comID;
    QColor color;
    Vector2D pos;
    Vector2D dir;
    QString str;
    bool newRobots;
};


class CguiDrawer {
public:
    QQueue<parsian_msgs::parsian_draw_rect> *rectBuffer;
    QQueue<parsian_msgs::parsian_draw_circle> *arcBuffer;
    QQueue<parsian_msgs::parsian_draw_polygon> *polygonBuffer;
    QQueue<parsian_msgs::parsian_draw_segment> *segBuffer;
    QQueue<parsian_msgs::parsian_draw_vector> *pointBuffer;
    QQueue<parsian_msgs::parsian_draw_text> *textBuffer;
    QQueue<CGraphicalRobot> robotBuffer;
    struct GuiBall {
        Vector2D pos;
        double inSight;
        double radius;
    } guiBall;

    CguiDrawer();
    ~CguiDrawer();

    void drawRobot(Vector2D _pos = Vector2D(0, 0), Vector2D _dir = Vector2D(0, 0), QColor _color = QColor(255, 255, 0), int _ID = 0, int _comID = 0, QString _str = "", bool newRobots = false);

    void clear();
};



#endif //RQT_PARSIAN_GUI_DRAWER_H

