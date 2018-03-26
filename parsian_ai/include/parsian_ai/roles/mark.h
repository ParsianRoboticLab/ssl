#ifndef Mark_H
#define Mark_H

#include <role.h>
#include <behaviours/offensive.h>
#include "mathtools.h"

class CRoleMarkInfo : public CRoleInfo {
public:
    QList <int> markedOpp;
    CRoleMarkInfo(QString _roleName);
    Agent* blocker(int i);
    QList<int> oneToucher;
    double oneToucherDist2Ball;
    void matching();
    MWBM mwbm;
    int lastMatched[_MAX_NUM_PLAYERS];
};

class CRoleMark : public CRole {
private:
    CBehaviourKick* shoot;
    CSkillGotoPointAvoid* gotopointavoid;
    int chooseOppToMark();
    int toBeMarkedID;

public:
    DEF_ROLE(CRoleMark)
    virtual void generateFromConfig(Agent *a);
    virtual void parse(QStringList params);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    QStringList toBeMarkedRole;
    SkillProperty(CRoleMark, bool, KickOffMode, kickOffMode);
    SkillProperty(CRoleMark, int, ForceMark, forceMark);
};

#endif // Mark_H
