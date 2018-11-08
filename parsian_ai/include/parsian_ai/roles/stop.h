#ifndef STOP_H
#define STOP_H

#include <parsian_ai/roles/role.h>

class CRoleStopInfo : public CRoleInfo {
public:
    explicit CRoleStopInfo(QString _roleName);
    void findPositions();
    Vector2D getEmptyTarget(const Vector2D& _position, const double& _radius);
    void setAgentBehindBall(int agentId);
    int inCorner;
    Vector2D TA;
    QList <Vector2D> Ps;
    QList <double> thR;
    QList <double> thP;
    QList <int> robotId;

    int agentIDBehindBall;
};

class CRoleStop : public CRole {
protected:
    GotopointavoidAction* gotopoint;
    NoAction*       noAction;
    bool switchAgent;
public:
    virtual void parse(QStringList params);

    explicit CRoleStop(Agent *_agent);
    ~CRoleStop();
    void assign(Agent* _agent) override;
    void execute();
    static CRoleStopInfo* info();
    double progress();

private:
    static CRoleStopInfo* m_info;
};



class CRoleHaltInfo : public CRoleInfo {
public:
    explicit CRoleHaltInfo(QString _roleName);
};

class CRoleHalt : public CRole {
public:
    explicit CRoleHalt(Agent *_agent);
    ~CRoleHalt();
    void execute();
    double progress();

private:
    NoAction* noAction;
};

#endif // STOP_H
