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


#include <parsian_msgs/plan_service.h>

class Exprimental {
public:
    GotopointavoidAction *mygpa;
    KickAction *myKick;
    Exprimental(Agent** _agents) {
        agents = _agents;
        gpa = new GotopointavoidAction();
        pos  << Vector2D(1,-3) << Vector2D(1,3) << Vector2D(4.5,3) << Vector2D(1,3);
        dist << 0.3 << 1 << 0.3 << 0.3;
        agents[8]->action = gpa;
        state = 0;
        mygpa = new GotopointavoidAction();
        myKick = new KickAction;
    }
    ~Exprimental() {

    }
    void reset() {

    }
    void execute() {

    }
private:
    int state;
    QList<Vector2D> pos;
    QList<double> dist;
    GotopointavoidAction *gpa;
    Agent** agents;
};

#endif //PARSIAN_AI_EXPRIMENTAL_H
