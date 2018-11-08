#ifndef ROLE_H
#define ROLE_H

#include <parsian_util/base.h>
#include <parsian_ai/util/worldmodel.h>
#include <QString>
#include <QList>
#include <parsian_ai/util/agent.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/gamestate.h>
#include <parsian_util/action/autogenerate/gotopointaction.h>
#include <parsian_util/action/autogenerate/gotopointavoidaction.h>
#include <parsian_util/action/autogenerate/kickaction.h>
#include <parsian_util/action/autogenerate/onetouchaction.h>
#include <parsian_util/action/autogenerate/receivepassaction.h>
#include <parsian_util/action/autogenerate/noaction.h>

class CRoleInfo;

class CRole {
public:
    CRole();
    explicit CRole(Agent* _agent);

    Agent* agent;
    virtual void assign(Agent* _agent);
};

class CRoleInfo {
protected:
    QString roleName;
public:
    QString getRoleName();
    explicit CRoleInfo(QString _roleName);
    QList< Agent* > agents;
    virtual Agent* robot(int i);
    virtual int count();
    virtual int index(CRole* me);
    virtual void addAgent(Agent* agent);
    virtual void reset();
};

#endif // ROLE_H
