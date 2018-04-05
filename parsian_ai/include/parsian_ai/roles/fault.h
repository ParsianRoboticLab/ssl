#ifndef FAULT_H
#define FAULT_H

#include <parsian_ai/roles/role.h>

class CRoleFaultInfo : public CRoleInfo
{
public:
    explicit CRoleFaultInfo(QString _roleName);
    void findPositions();
    int inCorner;
    Vector2D TA;
    QList <Vector2D> Ps;
    QList <double> thR;
    QList <double> thP;
    QList <int> robotId;
};

class CRoleFault : public CRole
{
protected:
    GotopointavoidAction* gotopoint;
    NoAction*			  noAction;
    bool switchAgent;
public:
    virtual void parse(QStringList params);

    explicit CRoleFault(Agent *_agent);
    ~CRoleFault();
    void assign(Agent* _agent) override;
    void execute();
    static CRoleFaultInfo* info();
    double progress();

private:
    static CRoleFaultInfo* m_info;
};


#endif // FAULT_H
