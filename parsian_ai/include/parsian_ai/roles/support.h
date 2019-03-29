#ifndef Support_H
#define Support_H

#include <parsian_ai/roles/role.h>

class CRoleSupportInfo : public CRoleInfo {
public:
    CRoleSupportInfo(QString _roleName);

    void reset() {}
};

class CRoleSupport : public CRole {
protected:
    GotopointavoidAction* gotopoint;
    KickAction* kick;
    Vector2D supportPosition;
public:
    //DEF_ROLE(CRoleSupport);
    void findPos();
    virtual void execute();
    explicit CRoleSupport(Agent* agent);
    ~CRoleSupport();
    virtual void parse(QStringList params);
};

#endif // Support_H
