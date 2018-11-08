//
// Created by parsian-ai on 11/8/18.
//

#ifndef PARSIAN_AI_ABSTRACTPLAYOFF_H
#define PARSIAN_AI_ABSTRACTPLAYOFF_H

#include "parsian_ai/roles/roles.h"
#include "parsian_ai/plans/plans.h"
#include <parsian_ai/gamestate.h>
#include <parsian_ai/config.h>
#include <QString>

class CAbstractPlayOff {
public:
    CAbstractPlayOff();
    virtual ~CAbstractPlayOff();
    virtual void reset() = 0;
    virtual void init(const QList<Agent *> &_agents) = 0;
    virtual void execute() = 0;
    bool getPlayonFlag();
protected:
    QList<Agent*> agents;
    bool playOnFlag;
};


#endif //PARSIAN_AI_ABSTRACTPLAYOFF_H
