//
// Created by parsian-ai on 9/28/17.
//

#include <parsian_util/tools/drawer.h>
#include <ros/ros.h>

Drawer* drawer;

Drawer::Drawer() {
    draws.node = ros::this_node::getName();
}

void Drawer::draw(const Rect2D& _rect, const QColor &_color, bool _filled) {
    parsian_msgs::parsian_draw drawRect;
    drawRect.type = drawRect.RECT;
    drawRect.color = toColorRGBA(_color);
    drawRect.filled = static_cast<unsigned char>(_filled);
    drawRect.primary = toParsianVec(_rect.topLeft());
    drawRect.secondary = toParsianVec(_rect.bottomRight());
    drawRect.type = parsian_msgs::parsian_draw::RECT;
    draws.draws.push_back(drawRect);

}

void Drawer::draw(const QString& _text, const Vector2D& _pos, const QColor& _color, int _size) {

    parsian_msgs::parsian_draw drawText;
    drawText.text = _text.toStdString();
    drawText.primary = toParsianVec(_pos);
    drawText.size = _size;
    drawText.color = toColorRGBA(_color);
    drawText.type = parsian_msgs::parsian_draw::TEXT;
    draws.draws.push_back(drawText);


}

void Drawer::draw(const Circle2D& _circle, int _startAng, int _endAng, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw drawCircle;

    drawCircle.color = toColorRGBA(_color);
    drawCircle.primary = toParsianVec(_circle.center());
    drawCircle.size = _circle.radius();
    drawCircle.filled = static_cast<unsigned char>(_filled);
    drawCircle.secondary.x = _startAng;
    drawCircle.secondary.y = _endAng;
    drawCircle.type = parsian_msgs::parsian_draw::CIRCLE;

    draws.draws.push_back(drawCircle);


}

void Drawer::draw(const Circle2D& _circle, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw drawCircle;

    drawCircle.color = toColorRGBA(_color);
    drawCircle.primary = toParsianVec(_circle.center());
    drawCircle.size = _circle.radius();
    drawCircle.filled = static_cast<unsigned char>(_filled);
    drawCircle.secondary.x = 0;
    drawCircle.secondary.y = 2*M_PI;
    drawCircle.type = parsian_msgs::parsian_draw::CIRCLE;

    draws.draws.push_back(drawCircle);

}

void Drawer::draw(const Polygon2D& _polygon, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw drawPolygon;

    for (auto vector : _polygon.vertex()) {
        drawPolygon.polygon.push_back(std::move(toParsianVec(vector)));
    }
    drawPolygon.filled = static_cast<unsigned char>(_filled);
    drawPolygon.color = toColorRGBA(_color);
    drawPolygon.type = parsian_msgs::parsian_draw::POLYGON;

    draws.draws.push_back(drawPolygon);
}

void Drawer::draw(const Segment2D& _seg, const QColor& _color) {
    parsian_msgs::parsian_draw drawSegment;

    drawSegment.primary   = toParsianVec(_seg.a());
    drawSegment.secondary = toParsianVec(_seg.b());
    drawSegment.color = toColorRGBA(_color);
    drawSegment.type = parsian_msgs::parsian_draw::SEGMENT;

    draws.draws.push_back(drawSegment);

}

void Drawer::draw(const Vector2D& _point, const QColor& _color, const double& size) {

    parsian_msgs::parsian_draw drawVector;

    drawVector.primary = toParsianVec(_point);
    drawVector.color   = toColorRGBA(_color);
    drawVector.type = parsian_msgs::parsian_draw::VECTOR;
    drawVector.size = size;
    draws.draws.push_back(drawVector);
}

std_msgs::ColorRGBA Drawer::toColorRGBA(const QColor &_color) {
    std_msgs::ColorRGBA colorRGBA;
    colorRGBA.a = static_cast<float>(_color.alphaF());
    colorRGBA.r = static_cast<float>(_color.redF());
    colorRGBA.g = static_cast<float>(_color.greenF());
    colorRGBA.b = static_cast<float>(_color.blueF());
    return colorRGBA;
}

parsian_msgs::vector2D Drawer::toParsianVec(const Vector2D &_vec) {
    parsian_msgs::vector2D vector2D;
    vector2D.x = _vec.x;
    vector2D.y = _vec.y;
    return vector2D;

}

const parsian_msgs::parsian_draws &Drawer::getDraws() {
    return draws;
}

void Drawer::clear() {
    draws.draws.clear();
}
