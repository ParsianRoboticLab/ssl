//
// Created by parsian-ai on 3/23/18.
//

#ifndef PARSIAN_AI_EXPRIMENTAL_H
#define PARSIAN_AI_EXPRIMENTAL_H


#include <QStringList>

#include <algorithm>

#include <QtCore/QMap>
#include <parsian_util/core/agent.h>
#include <QtCore/QTime>
#include <QtCore/QFile>
#include <QPair>
#include <QTextStream>
#include "parsian_util/base.h"
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/plans/plans.h>
#include <parsian_ai/plays/plays.h>
#include <parsian_ai/roles/stop.h>

#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/geom/geom.h>
#include <behavior/move_forward/move_forward.h>

#include <parsian_msgs/plan_service.h>

enum class State{
    SIMPLE_MOVE = 0,
    WAITFORIT = 1,
    CHIP = 2,
    GOCURVE = 3,
    CATCHBALL = 4
};

class Exprimental {
public:
    Exprimental(Agent** _agent) {

    }
    ~Exprimental() {

    }
    void reset() {

    }
    void execute() {
    }
private:

};
#endif //PARSIAN_AI_EXPRIMENTAL_H
