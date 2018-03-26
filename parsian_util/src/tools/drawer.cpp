//
// Created by parsian-ai on 9/28/17.
//

#include <parsian_util/tools/drawer.h>

Drawer* drawer;

void Drawer::draw(const Rect2D& _rect, const QColor &_color, bool _filled) {
    parsian_msgs::parsian_draw_rect drawRect;

    parsian_msgs::rect2D rect2D;
    rect2D.left_x = (float) _rect.left();
    rect2D.top_y  = (float) _rect.top();
    rect2D.length = (float) _rect.size().length();
    rect2D.width  = (float) _rect.size().width();
    drawRect.rect = rect2D;

    drawRect.color = toColorRGBA(_color);

    drawRect.filled = _filled;

    draws.rects.push_back(drawRect);

}

void Drawer::draw(const QString& _text, const Vector2D& _pos, const QColor& _color, int _size) {

    parsian_msgs::parsian_draw_text drawText;
    drawText.value = _text.toStdString();
    drawText.position = toParsianVec(_pos);
    drawText.size = _size;
    drawText.color = toColorRGBA(_color);

    draws.texts.push_back(drawText);


}

void Drawer::draw(const Circle2D& _circle, int _startAng, int _endAng, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw_circle drawCircle;

    drawCircle.color = toColorRGBA(_color);

    parsian_msgs::circle2D circle2D;
    circle2D.center = toParsianVec(_circle.center());
    circle2D.radius = _circle.radius();
    drawCircle.circle = circle2D;

    drawCircle.filled = _filled;
    drawCircle.startAng = _startAng;
    drawCircle.endAng   = _endAng;

    draws.circles.push_back(drawCircle);


}

void Drawer::draw(const Circle2D& _circle, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw_circle drawCircle;


    drawCircle.color = toColorRGBA(_color);

    parsian_msgs::circle2D circle2D;
    circle2D.center = toParsianVec(_circle.center());
    circle2D.radius = _circle.radius();
    drawCircle.circle = circle2D;

    drawCircle.filled = _filled;
    drawCircle.startAng = 0;
    drawCircle.endAng   = 360;

    draws.circles.push_back(drawCircle);

}

void Drawer::draw(const Polygon2D& _polygon, const QColor& _color, bool _filled) {

    parsian_msgs::parsian_draw_polygon drawPolygon;

    for (auto vector : _polygon.vertex()) {
        drawPolygon.points.push_back(std::move(toParsianVec(vector)));
    }

    drawPolygon.filled = _filled;
    drawPolygon.color = toColorRGBA(_color);

}

void Drawer::draw(const Segment2D& _seg, const QColor& _color) {
    parsian_msgs::parsian_draw_segment drawSegment;

    drawSegment.start = toParsianVec(_seg.a());
    drawSegment.end   = toParsianVec(_seg.b());
    drawSegment.color = toColorRGBA(_color);
    drawSegment.line  = false;
    drawSegment.ray   = false;

    draws.segments.push_back(drawSegment);

}

void Drawer::draw(const Vector2D& _point, const QColor& _color) {

    parsian_msgs::parsian_draw_vector drawVector;

    drawVector.vector = toParsianVec(_point);
    drawVector.color  = toColorRGBA(_color);

    draws.vectors.push_back(drawVector);
}

std_msgs::ColorRGBA Drawer::toColorRGBA(const QColor &_color) {
    std_msgs::ColorRGBA colorRGBA;
    colorRGBA.a = _color.alpha();
    colorRGBA.r = _color.red();
    colorRGBA.g = _color.green();
    colorRGBA.b = _color.blue();
    return colorRGBA;
}

parsian_msgs::vector2D Drawer::toParsianVec(const Vector2D &_vec) {
    parsian_msgs::vector2D vector2D;
    vector2D.x = _vec.x;
    vector2D.y = _vec.y;
    return vector2D;

}
