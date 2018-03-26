#ifndef WARMUP_H
#define WARMUP_H
#include <role.h>

class CRoleWarmupInfo : public CRoleInfo {
public:
    CRoleWarmupInfo(QString _roleName);
    void findPos1();
    void findPos2();
    Vector2D cent;
    QList <Vector2D> Ps;
    QList <int> robotId;
    QList <double> distR;
    QList <double> distP;
    void reset() {}
};

class CRoleWarmup : public CRole {
protected:
    CSkillGotoPointAvoid* gotopoint;
public:
    DEF_ROLE(CRoleWarmup);
    virtual void generateFromConfig(Agent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    virtual void parse(QStringList params);

};

#endif // WARMUP_H
