#include <parsian_ai/plans/plan.h>

Plan::Plan()
    = default;

void Plan::init(const QList<Agent*> & _agents) {
    agents.clear();
    agents.append(_agents);
}

Agent* Plan::agent(int i) {
    return agents[i];
}

int Plan::count() {
    return agents.length();
}

/*!
 * @abstract assign a skill to some agent
 * @param i the number of agent in the local agents list
 * @param skill skill that is going to be assigned
 */
void Plan::assignSkill(int i, Action* _action) {
    agent(i)->action = _action;
//    agent(i)->actionName = _action->getActionName(); // TODO : Add Action Name to Agent
}

int Plan::agentById(int id) {
    for (int i = 0; i < count(); i++) {
        if (agent(i)->id() == id) {
            return i;
        }
    }
    return -1;
}

QList<int> Plan::getAgentIdList() {
    QList<int> l;
    for (int i = 0; i < count(); i++) {
        l.append(agent(i)->id());
    }
    return l;
}

void Plan::debugAgents(QString text) {
    QString str;
    for (auto agent : agents) {
        str += QString(" %1").arg(agent->id());
    }
    DBUG(QString("%1: Size: %2 --> (%3)").arg(text).arg(agents.size()).arg(str) , D_ERROR);
}
