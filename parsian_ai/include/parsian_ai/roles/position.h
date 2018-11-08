#ifndef POSITION_H
#define POSITION_H

#include <role.h>
#include "roles/playmake.h"
#include <formation/edit_data.h>
#include <formation/formation_ssl.h>
#include <QTimer>
#include <QObject>

class CRolePositionInfo : public CRoleInfo {
    int playMakerID;
    int lastFrameCalculated;
public:
    QList<int> posAgents;
    CRolePositionInfo(QString _roleName);
    double lastError;
    QList<int> lastAgents;

    int indices[_MAX_NUM_PLAYERS];
    bool matched;
    void matchPositions();
    int findPlayMaker();
    void reset();
    QList<int> oneToucher;
    double oneToucherDist2Ball;
};

struct PositionRegion {
    QString name;
    bool avoid, center, mirror, sameside;
};

class CRolePosition : public CRole {
private:
    Vector2D targets[_MAX_NUM_PLAYERS];//not by agent id (just some points)

private:
    CSkillGotoPointAvoid *gotopoint;
    CBehaviourKick* shoot;
public:
    DEF_ROLE(CRolePosition);
    QList<PositionRegion> regions;
    virtual void generateFromConfig(Agent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    SkillProperty(CRolePosition, QList<Rect2D>, SearchRects, searchRects);
    SkillProperty(CRolePosition, QList<Rect2D>, AvoidRects,  avoidRects);
    SkillProperty(CRolePosition, Vector2D, Position, position);
    SkillProperty(CRolePosition, double, Direction, direction);
    SkillProperty(CRolePosition, Vector2D*, LookAt, lookat);
    SkillProperty(CRolePosition, Vector2D, PositioningPos, positioningPos);
    SkillProperty(CRolePosition, bool, AntiKhafan, antiKhafan);
    SkillProperty(CRolePosition, bool, IndirectKhafan, indirectKhafan);
    SkillProperty(CRolePosition, bool, IndirectGoogooli, indirectGoogooli);
    SkillProperty(CRolePosition, bool, IndirectTiny, indirectTiny);
    SkillProperty(CRolePosition, bool, Penalty, penalty);
    SkillProperty(CRolePosition, bool, CornerLeft, cornerL);
    SkillProperty(CRolePosition, bool, CornerRight, cornerR);
    SkillProperty(CRolePosition, bool, CornerLeftShirje, cornerLShirje);
    SkillProperty(CRolePosition, bool, CornerRightShirje, cornerRShirje);
    SkillProperty(CRolePosition, bool, Disturb, disturb);
    SkillProperty(CRolePosition, bool, Stay, stay);
    SkillProperty(CRolePosition, bool, FixedPassPoint, fixedPassPoint);
    SkillProperty(CRolePosition, bool, Clearing, clearing);
    SkillProperty(CRolePosition, bool, ClearingState, clearingState);
    SkillProperty(CRolePosition, QString, Orientation, orientation);
    SkillProperty(CRolePosition, QString, MLPName, mlpname);
    SkillProperty(CRolePosition, bool, IndirectKhafanhigh, indirectKhafanhigh);
    SkillProperty(CRolePosition, bool, FixedPointDef, fixedPointDef);
    SkillProperty(CRolePosition, bool, FixedPointws, fixedPointws);
    SkillProperty(CRolePosition, bool, DefaultPositioning, defaultPositioning);
    SkillProperty(CRolePosition, int, FixedPositionIndex, fixedPositionIndex);
    SkillProperty(CRolePosition, bool, Farib, farib);
    SkillProperty(CRolePosition, bool, Centre, centre);
    SkillProperty(CRolePosition, bool, Stop, stop);

public:
    static QMap<QString, EditData*> editData; //Contains Formations
};

#endif // Position_H
