#ifndef PlayMake_H
#define PlayMake_H

#include <parsian_ai/roles/role.h>
#include <QMap>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QTime>
#include <parsian_ai/config.h>
#include <parsian_ai/util/worldmodel.h>
#include <parsian_ai/util/knowledge.h>
#include <parsian_ai/gamestate.h>


enum PlayMakerKickModes {FixedPass , FixedShoot};

class CRolePlayMakeInfo : public CRoleInfo {
public:
    explicit CRolePlayMakeInfo(QString _roleName);
    void reset() override {}
};

class CRolePlayMake : public CRole {
public:
    KickAction* kick;
    OnetouchAction* onetouch;
    GotopointavoidAction* gotopoint;
    NoAction* wait;
    Vector2D penaltyTarget;
    bool firstKick = true;
    Vector2D target;
    void stopBehindBall(bool penalty = false);
    Vector2D initialPoint;
    QTime changeDirPenaltyStrikerTime;
    bool timerStartFlag;

public:
    explicit CRolePlayMake(Agent *_agent);
    ~CRolePlayMake();
    void executeOurKickOff();
    void executeOurPenalty();
    void executeOurPenaltyShootout();
    void theirPenaltyPositioning();
    int choosePenaltyStrategy();
    void ShootoutSwitching(bool isChip);
    void firstKickInShootout(bool isChip);
    void kickInitialShootout();
    int getPenaltychipSpeed();
    double lastBounce();
    bool ShootPenalty();
    void parse(QStringList params);
    void resetOffPlays();
    void resetPlayMake();
    void execute();

    void kickPass(double kickSpeed);
    enum KickPassMode {KickPassFirst , KickPassSecond};
    enum penaltyStrategy {pgoaheadShoot , pchipShoot , pshootDirect};

    KickPassMode kickPassMode;
    int kickPassCyclesWait;
    Vector2D finalTarget;

    QTextStream out;
    QFile lastBounceDataFile;


    SkillProperty(CRolePlayMake, Vector2D, PointToPass, pointToPass);
    SkillProperty(CRolePlayMake, Vector2D, PointToShoot, pointToShoot);
    SkillProperty(CRolePlayMake, int, KickMode , kickMode);
    SkillProperty(CRolePlayMake, int, CyclesToWait, cyclesToWait);
    SkillPropertyNoSet(CRolePlayMake, int, CyclesExecuted, cyclesExecuted);
    SkillProperty(CRolePlayMake, bool, Slow, slow);
    SkillProperty(CRolePlayMake, bool, ChipInPenaltyArea, chipPenaltyArea);
    SkillProperty(CRolePlayMake, bool, Chip, chip);
    SkillProperty(CRolePlayMake, bool, Through, through);
    SkillProperty(CRolePlayMake, bool, IndirectGoogooli, indirectGoogooli);
    SkillProperty(CRolePlayMake, bool, IndirectTiny, indirectTiny);
    SkillProperty(CRolePlayMake, bool, IndirectKhafan, indirectKhafan);
    SkillProperty(CRolePlayMake, bool, KickoffWing, kickoffWing);
    SkillProperty(CRolePlayMake, bool, Kickoffmode, kickoffmode);
    SkillProperty(CRolePlayMake, bool, NoStop, nostop);
    SkillProperty(CRolePlayMake, bool, ChipIndirect, chipIndirect);
    SkillProperty(CRolePlayMake, bool, NoKick, noKick);
    SkillProperty(CRolePlayMake, int, PassReceiver, passReceiver);
    SkillProperty(CRolePlayMake, bool, ManualPassReceive, manualPassReceive);
    SkillProperty(CRolePlayMake, bool, AllowOneTouch, allowonetouch);
    SkillProperty(CRolePlayMake, bool, AllowOnePass, allowonepass);
    SkillProperty(CRolePlayMake, QList<int>, Passables, passables);
    SkillProperty(CRolePlayMake, QList<int>, PassSequence, passsequence);
    SkillProperty(CRolePlayMake, bool, FollowSequence, followsequence);
    SkillProperty(CRolePlayMake, bool, ShadowPass, shadowPass);
    SkillProperty(CRolePlayMake, bool, KickToTheirDefense, kickToTheirDefense);
    SkillProperty(CRolePlayMake, bool, JustTurn, justTurn);
    SkillProperty(CRolePlayMake, QString, LocalAgentPassTarget, localAgentPassTarget);
    SkillProperty(CRolePlayMake, bool, Khers, khers);
    SkillProperty(CRolePlayMake, bool, LongChip, longchip);
    SkillProperty(CRolePlayMake, bool, ChipToOppGoal, chipToOppGoal);
    SkillProperty(CRolePlayMake, bool, ShadowyPoint, shadowyPoint);
    SkillProperty(CRolePlayMake, bool, SafeIndirect , safeIndirect);
    SkillProperty(CRolePlayMake, bool, SpinBool , spinBool);
    SkillProperty(CRolePlayMake, bool, KickPower, kickPower);
};


#endif // PlayMake_H
