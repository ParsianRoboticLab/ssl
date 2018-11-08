#ifndef PLAN_H
#define PLAN_H

#include <QList>
#include "parsian_util/mathtools.h"
#include <parsian_util/action/action.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/util/agent.h>
//#include <skills.h> // TODO : Actions

class Plan {
protected:
    QList<Agent*> agents;
public:
    Plan();
    Agent* agent(int i);
    int count();
    void init(const QList<Agent*> & _agents);
    void assignSkill(int i, Action* skill);
    int agentById(int id);
    QList<int> getAgentIdList();
    void debugAgents(QString text);

//abstract functions
    virtual void execute() = 0;
};

#endif // PLAN_H
