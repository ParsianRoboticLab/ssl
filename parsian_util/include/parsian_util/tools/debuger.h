//
// Created by parsian-ai on 9/28/17.
//

#ifndef PARSIAN_UTIL_DEBUGER_H
#define PARSIAN_UTIL_DEBUGER_H

#include "parsian_msgs/parsian_debugs.h"
#include <QtGui/QColor>


#define D_ERROR      0
#define D_GAME       1
#define D_EXPERIMENT 2
#define D_DEBUG      4
#define D_NADIA      8
#define D_MANI       16
#define D_ARASH      32
#define D_ALI        64
#define D_SEPEHR     128
#define D_MASOOD     256
#define D_MOHAMMED   512
#define D_HOSSEIN    1024
#define D_KK         2048
#define D_MHMMD      4096
#define D_ERF        8192
#define D_MAHI       16384
#define D_MAHMOOD    32768
#define D_FATEME     32769
#define D_ATOUSA     32770
#define D_AHZ        32772
#define D_AMIN       32776
#define D_PARSA      32784
#define D_HAMED      32800

#define PDEBUG(prefix, value, type) debugger->debug(QString(#prefix " : %1").arg(value), type)
#define DEBUG(value, type) debugger->debug(QString("%1").arg(value), type)
#define DBUG(value, type) debugger->debug(value, type)
class Debugger {
public:

    void debug(const QString& text, unsigned long type, const QColor& color = QColor(0, 0, 0));

    parsian_msgs::parsian_debugs debugs;

private:
    std_msgs::ColorRGBA toColorRGBA(const QColor& _color);

};
extern Debugger* debugger;
#endif //PARSIAN_UTIL_DEBUGER_H
