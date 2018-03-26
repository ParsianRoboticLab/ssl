// Created by parsian-ai on 9/28/17.
//

#ifndef PARSIAN_UTIL_DRAWER_H_H
#define PARSIAN_UTIL_DRAWER_H_H

#include "parsian_util/geom/geom.h"
#include "parsian_msgs/parsian_draw.h"

#include <QtCore/QString>
#include <QtGui/QColor>

class Drawer {
public:
    void draw(const Rect2D&    _rect = Rect2D(Vector2D(0, 0), Vector2D(0, 0)), const QColor& _color = QColor(255, 255, 255), bool _filled = false);
    void draw(const QString&   _text = "", const Vector2D& _pos = Vector2D(0, 0), const QColor& _color = QColor("black"), int _size = 12);
    void draw(const Circle2D&  _circle = Circle2D(Vector2D(0, 0), 0), const QColor& _color = QColor(255, 255, 255), bool _filled = false);
    void draw(const Circle2D&  _circle = Circle2D(Vector2D(0, 0), 0), int _startAng = 0, int _endAng = 360, const QColor& _color = QColor(255, 255, 255), bool _filled = false);
    void draw(const Polygon2D& _polygon = Polygon2D(), const QColor& _color = QColor(255, 255, 255), bool _filled = false);
    void draw(const Segment2D& _seg = Segment2D(Vector2D(0, 0), Vector2D(0, 0)), const QColor& _color = QColor(255, 255, 255));
    void draw(const Vector2D&  _point = Vector2D(0, 0), const QColor& _color = QColor(255, 255, 255));

    parsian_msgs::parsian_draw draws;

private:
    std_msgs::ColorRGBA toColorRGBA(const QColor& _color);
    parsian_msgs::vector2D toParsianVec(const Vector2D& _vec);
};
extern Drawer* drawer;
#endif //PARSIAN_UTIL_DRAWER_H_H
