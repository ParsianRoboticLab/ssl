//
// Created by parsian-ai on 9/28/17.
//

#include "parsian_util/tools/debuger.h"
Debugger* debugger;
void Debugger::debug(const QString& _text , unsigned long _type , const QColor& _color) {

    parsian_msgs::parsian_debug parsianDebug;
    parsianDebug.author = _type;
    parsianDebug.value  = _text.toStdString();
    parsianDebug.color  = toColorRGBA(_color);

    debugs.debugs.push_back(parsianDebug);
}

std_msgs::ColorRGBA Debugger::toColorRGBA(const QColor &_color) {
    std_msgs::ColorRGBA colorRGBA;
    colorRGBA.a = _color.alpha();
    colorRGBA.r = _color.red();
    colorRGBA.g = _color.green();
    colorRGBA.b = _color.blue();
    return colorRGBA;

}