#include "parsian_ai/roles/playmake.h"

using namespace std;


CRolePlayMake::CRolePlayMake(Agent *_agent) : CRole(_agent) {
    justTurn = false;
    kickToTheirDefense = false;
    kick = new KickAction();
    wait = new NoAction();
    wait->setWaithere(true);
    onetouch = new OnetouchAction();
    gotopoint = new GotopointavoidAction();
    chip = slow = false;
    shadowPass = false;
    indirectKhafan = false;
    chipIndirect = false;
    through = false;
    noKick = false;
    penaltyTarget.invalidate();
    manualPassReceive = false;
    allowonetouch = false;
    allowonepass = false;
    indirectTiny = false;
    passReceiver = -1;
    timerStartFlag = true;

    //    spinPass = new CBehaviourSpinPass;
    initialPoint.invalidate();

    lastBounceDataFile.setFileName("lastBounce");
    out.setDevice(&lastBounceDataFile);
}

CRolePlayMake::~CRolePlayMake() {
    delete kick;
    delete wait;
    delete onetouch;
    delete gotopoint;
}

void CRolePlayMake::stopBehindBall(bool penalty) {
    if (penalty) {
        if (gameState->isStop()) {
            DBUG("stop, reset changeDirPenaltyStriker flag", D_FATEME);

        }

        if (gameState->penaltyShootout()) {
            gotopoint->setTargetpos(wm->ball->pos + (wm->ball->pos - wm->field->oppGoal()) * 0.03);
            gotopoint->setTargetdir(wm->ball->pos - agent->pos());
        } else {
            Vector2D direction, position;

            direction = wm->ball->pos - agent->pos();
            direction.y *= 1.2;
            position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + Vector2D(0, 0.2)).norm() * (0.13);
            gotopoint->setTargetpos(position);
            gotopoint->setTargetdir(direction);
        }

        gotopoint->setSlowmode(true);
        gotopoint->setNoavoid(false);
        gotopoint->setPenaltykick(true);
        gotopoint->setAvoidpenaltyarea(false);
        gotopoint->setAvoidcentercircle(false);

        gotopoint->setBallobstacleradius(0.2);
        agent->action = gotopoint;
        gotopoint->setNoavoid(false);
        gotopoint->setSlowmode(false);

    } else {
        Vector2D shadowPoint = wm->ball->pos + Vector2D(wm->ball->pos - wm->field->oppGoal()).norm() * 0.3;
        if (kickoffmode || kickoffWing) {
            shadowPoint = wm->ball->pos + Vector2D(wm->field->oppGoal() - wm->ball->pos).norm() * 0.3;
        }
        gotopoint->setSlowmode(true);
        gotopoint->setNoavoid(false);
        gotopoint->setAvoidpenaltyarea(true);
        gotopoint->setAvoidcentercircle(false);
        gotopoint->setBallobstacleradius(static_cast<float>(4 * CBall::radius));
        gotopoint->setTargetpos(shadowPoint);
        gotopoint->setTargetdir(Vector2D(1.0, 0.0));
        gotopoint->setLookat(wm->ball->pos);
        agent->action = gotopoint;
    }
}

void CRolePlayMake::executeOurKickOff() {
    slow = true;
    kickoffmode = true;

    if (cyclesExecuted < cyclesToWait) {
        drawer->draw("waiting", Vector2D(0, -2));
        stopBehindBall(false);
        return;
    }

    if (kickMode == FixedShoot) {
        chipToOppGoal = true;
    }
    if (gameState->isStop()) {
        stopBehindBall(false);
    } else {
        if (chipToOppGoal) {
            kick->setTolerance(0.05);
            kick->setTarget(wm->field->oppGoal());

            kick->setTarget(wm->field->oppGoal() * (2.0 / 3.0));

            double kickSpeed;
            kickSpeed = wm->ball->pos.dist(wm->field->oppCornerL()) / 3.0;

            kick->setKickspeed(kickSpeed);
            kick->setSpin(false);
            kick->setChip(true);
            kick->setSlow(false);
            agent->action = kick;
        } else {
            if (wm->ball->inSight <= 0 || !wm->ball->pos.valid() || !wm->field->isInField(wm->ball->pos)) {
                wait->setWaithere(true);
                agent->action = wait;
                return;
            }

            Vector2D target{};

            target = pointToPass;
            int kickSpeed = 3;

            kick->setSlow(true);
            kick->setSpin(false);
            kick->setChip(false);
            kick->setTarget(target);
            kick->setKickspeed(kickSpeed);
            kick->setTolerance(0.06);
            agent->action = kick;
        }
    }
}
bool CRolePlayMake::ShootPenalty() {
    double w;
    QList<int> relax, empty;
    relax.append(agent->id());
    penaltyTarget = know->getEmptyPosOnGoal(agent->pos(), w, true, relax, empty);
    if (penaltyTarget.dist(wm->field->oppGoal()) < 0.1) {
        penaltyTarget = know->getEmptyPosOnGoalForPenalty(1.0 / 10.0, true, 0.06, agent);
    }
    DBUG(QString("goalie index :%1").arg(wm->opp.data->goalieID), D_NADIA);
    if (wm->opp[wm->opp.data->goalieID] == nullptr) {
        return false;
    }
    return Segment2D(agent->pos(), penaltyTarget).dist(wm->opp[wm->opp.data->goalieID]->pos)
           > fabs(agent->pos().x - wm->opp[wm->opp.data->goalieID]->pos.x) / 4;
}

double CRolePlayMake::lastBounce() {
    return (wm->field->oppGoal().dist(agent->pos())) - 0.23;
}

int CRolePlayMake::getPenaltychipSpeed() {
    Vector2D oppGoaliPos = wm->opp[wm->opp.data->goalieID]->pos;
    DBUG(QString("chipsepeed:%1").arg(know->chipGoalPropability(true)), D_NADIA);
    if (know->chipGoalPropability(true) > 0.1) {

        // TODO : FIX THIS
        return 2;//(knowledge->getProfile(agent->id(),(oppGoaliPos-agent->pos()).length(),false)+knowledge->getProfile(agent->id()+1,lastBounce(),false))/2;
    } else {
        return -1;
    }
}

void CRolePlayMake::firstKickInShootout(bool isChip) {

    double divation = 0;
    DBUG("first : ", D_NADIA);
    penaltyTarget = wm->field->oppGoalL() + divation * Vector2D(0, wm->field->oppGoalL().y);//tune with deviation
    kick->setTarget(penaltyTarget);
    if (isChip) { //chip first
        kick->setChip(true);
        kick->setChipdist(2.0);//tune
        if (wm->ball->vel.length() > 0.4) {
            firstKick = false;
        }
    } else { //kick first
        kick->setChip(false);
        kick->setKickspeed(6);
        if (wm->ball->vel.length() > 0.1) {
            firstKick = false;
        }
    }
}

void CRolePlayMake::kickInitialShootout() {
    penaltyTarget = wm->field->oppGoal();
    kick->setTarget(penaltyTarget);
    kick->setPenaltykick(false);
    kick->setInterceptmode(false);
    kick->setChip(false);
    kick->setVeryfine(false);
    kick->setTolerance(1);
    kick->setSpin(0);
}

void CRolePlayMake::ShootoutSwitching(bool isChip) {

    if (wm->ball->vel.length() < 0.2) {
        firstKick = true;
    }



    switch (choosePenaltyStrategy()) {

    case pgoaheadShoot:
        DBUG("pgoahead : ", D_NADIA);
        if (agent->pos().x < 1) { //agent is not ahead enough


            penaltyTarget = wm->field->oppGoalL();
            kick->setTarget(penaltyTarget);

            if (isChip) { //chip first

                kick->setKickspeed(1.5);
                kick->setChip(true);

            } else { //kick first

                kick->setChip(false);
                kick->setKickspeed(1.5);
            }
        } else { //shoot to goal

            penaltyTarget = know->getEmptyPosOnGoalForPenalty(0.13, true, 10, agent);
            kick->setChip(false);
            kick->setKickspeed(6);
            kick->setDontkick(false);
            kick->setTarget(penaltyTarget);

        }

        break;


    case pchipShoot:
        DBUG("pchipshoot", D_NADIA);
        kick->setTarget(wm->field->oppGoal());
        kick->setChipdist(getPenaltychipSpeed());
        kick->setChip(true);
        break;

    case pshootDirect:
        DBUG("pdirect : ", D_NADIA);
        penaltyTarget = know->getEmptyPosOnGoalForPenalty(0.13, true, 10, agent);
        kick->setTarget(penaltyTarget);
        kick->setChip(false);
        kick->setKickspeed(6);
        kick->setAvoidopppenaltyarea(true);
        break;
    default:
        break;
    }
}

int CRolePlayMake::choosePenaltyStrategy() {
    if (ShootPenalty()) {
        return pshootDirect;
    } else if (getPenaltychipSpeed() != -1) {
        return pchipShoot;
    } else {
        return pgoaheadShoot;
    }
}

void CRolePlayMake::executeOurPenaltyShootout() {

    // working here
    bool chipchip = false;
    DBUG("penalty Shootout : ", D_NADIA);
    if (fabs(wm->ball->pos.x) > fabs(wm->field->oppGoal().x - 0.1)) { //penalty finished
        firstKick = true;
    }
    if (gameState->ourPenaltyShootout() && !gameState->ready()/*knowledge->getGameMode()==CKnowledge::Stop*/) {
        //stop behind ball
        cyclesExecuted--;
        srand(static_cast<unsigned int>(time(nullptr)));
        stopBehindBall(true);
        setNoKick(true);
    } else {    //normal start
        //initial kick skill:
        kickInitialShootout();
        if (ShootPenalty()) {
            firstKick = false;
        }
        if (firstKick) {
            firstKickInShootout(chipchip);
        } else {
            ShootoutSwitching(chipchip);
        }
        kick->setShotemptyspot(true);
        kick->setAvoidopppenaltyarea(false);
        agent->action = kick;
        drawer->draw(penaltyTarget, "red"); // todo : is my change OK
        //drawer->draw(penaltyTarget,0,"red");
    }
}

void CRolePlayMake::executeOurPenalty() {
    Vector2D shift;
    Vector2D position;

    if (gameState->isStop()) {
        cyclesExecuted--;
        srand(static_cast<unsigned int>(time(nullptr)));
        stopBehindBall(true);
        penaltyTarget = know->getEmptyPosOnGoalForPenalty(1.0 / 8.0, true, 0.03);

        changeDirPenaltyStrikerTime.restart();
        timerStartFlag = true;
    } else {
        penaltyTarget = know->getEmptyPosOnGoalForPenalty(1.0 / 8.0, true, 0.03); //////// tune
        kick->setSpin(1);
        gotopoint->setRoller(1);
        ////////////// change robot direction before kicking //////////////
        if (timerStartFlag) {
            if (changeDirPenaltyStrikerTime.elapsed() < 2500) {
                if (penaltyTarget.y * wm->field->oppGoalL().y < 0 && penaltyTarget.dist(wm->field->oppGoal()) > 0.25) {
                    penaltyTarget.y = wm->field->oppGoalR().y * 2;
                    shift = Vector2D(0, 0.3);
                } else {
                    penaltyTarget.y = wm->field->oppGoalL().y * 2;
                    shift = Vector2D(0, -0.3);
                }
                position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + shift).norm() * (0.13);
                gotopoint->setTargetdir(penaltyTarget);
                gotopoint->setTargetpos(position);
                gotopoint->setLookat(wm->ball->pos);
            } else {
                timerStartFlag = false;
            }

        }

        gotopoint->setDivemode(true);
        gotopoint->setSlowmode(false);

        kick->setTarget(penaltyTarget);
        // TODO : Fix This Speed
        // TODO : check this mhmmd
        //        kick->setKickSpeed(knowledge->getProfile(agent->id(),7.8 ,true, false);
        //        kick->setKickSpeed(agent->kickSpeedValue(7.8,false));
        //        kick->setPenaltyKick(true);
        kick->setInterceptmode(false);
        kick->setSpin(false);
        kick->setChip(false);
        kick->setVeryfine(false);
        kick->setKickspeed(7.8);
        kick->setAvoidopppenaltyarea(false);
        kick->setTolerance(20);
        kick->setChip(false);

        if (timerStartFlag) {
            agent->action = gotopoint;
        } else {
            agent->action = kick;
        }
    }

    //    drawer->draw(penaltyTarget, 0, "cyan");

}

void CRolePlayMake::theirPenaltyPositioning() {
    DBUG("iiiin", D_NADIA);
    gotopoint->setTargetpos(wm->field->oppCornerL());
    gotopoint->setTargetdir(wm->field->ourGoal());
    agent->action = gotopoint;
}

void CRolePlayMake::kickPass(double kickSpeed) {
    Vector2D behindTheBall = wm->ball->pos + Vector2D(wm->ball->pos - pointToPass).norm() * 0.2;
    if (kickPassMode == KickPassFirst && agent->pos().dist(behindTheBall) > 0.01) {
        finalTarget = wm->ball->pos;
        gotopoint->setTargetpos(behindTheBall);
        gotopoint->setTargetdir(Vector2D(1.0, 0.0));
        gotopoint->setLookat(pointToPass);
        gotopoint->setSlowmode(true);
        gotopoint->setNoavoid(false);
        gotopoint->setAvoidpenaltyarea(true);
        gotopoint->setAvoidcentercircle(false);
        gotopoint->setBallobstacleradius(0.2);
        agent->action = gotopoint;
    } else {
        kickPassMode = KickPassSecond;
        if (kickPassCyclesWait > 4 && agent->pos().dist(finalTarget) > 0.01) {
            gotopoint->setTargetpos(finalTarget);
            gotopoint->setTargetdir(Vector2D(1.0, 0.0));
            gotopoint->setLookat(pointToPass);
            gotopoint->setSlowmode(true);
            gotopoint->setNoavoid(true);
            gotopoint->setAvoidpenaltyarea(true);
            gotopoint->setAvoidcentercircle(false);
            gotopoint->setBallobstacleradius(0);
            agent->action = gotopoint;
        } else {
            kickPassCyclesWait++;
        }
    }
}

void CRolePlayMake::execute() {
    ROS_INFO_STREAM("shootout: gameState->ourPenaltyShootout(): " << gameState->ourPenaltyShootout());
    cyclesExecuted++;
    if (wm->ball->inSight <= 0
            || !wm->ball->pos.valid()
            || !wm->field->marginedField().contains(wm->ball->pos)) {
        wait->setWaithere(true);
        agent->action = wait;
        ROS_INFO_STREAM("shootout: in first if: ");
        return;
    }

    double region;
    QList <int> ourRelax;
    ourRelax.clear();
    ourRelax.append(wm->our.data->activeAgents);
    QList <int> oppRelax;
    Vector2D target = know->getEmptyPosOnGoal(wm->ball->pos , region , true , ourRelax, oppRelax);
    double kickSpeed = 5;

//    if (!noKick) {
//        ROS_INFO_STREAM("shootout: in second if: ");
//        return;
//    }

    if (gameState->ourKickoff()) {
        executeOurKickOff();

        return;
    } else if (gameState->theirPenaltyShootout()) {
        theirPenaltyPositioning();
        return;

    } else if (gameState->ourPenaltyShootout()) {
        ROS_INFO_STREAM("shootout: execute->playmakerol: ");
        DBUG(QString("st:%1").arg(!gameState->ourPenaltyShootout()), D_NADIA);
        executeOurPenaltyShootout();
        return;
    } else if (gameState->ourPenaltyKick()) {
        DBUG(QString("st___:%1").arg(!gameState->ourPenaltyKick()), D_NADIA);
        executeOurPenalty();
        return;

    }

    if (cyclesExecuted < cyclesToWait) {
        stopBehindBall(false);
        return;
    }

    if (noKick) {
        return;
    }

    if (kickMode == FixedPass) {
        DBUG("HERE" , D_KK);
        setThrough(false);
        target = pointToPass;
        kickSpeed = agent->pos().dist(target);
        kick->setSlow(false);
        kick->setChip(chip);
        kickPass(kickSpeed);
        return;
    } else if (kickMode == FixedShoot) {
        target = pointToShoot;
        setChip(false);
        kickSpeed = agent->pos().dist(target);
        kick->setSlow(false);
        kick->setTarget(target);
        kick->setKickspeed(kickSpeed);
        kick->setTolerance(0.06);
        kick->setAvoidpenaltyarea(true);
        agent->action = kick;
        DBUG("HERE2" , D_KK);
    }

    DBUG("HERE3" , D_KK);
    kick->setSlow(false);
    kick->setTarget(target);
    kick->setKickspeed(kickSpeed);
    kick->setTolerance(0.06);
    kick->setAvoidpenaltyarea(true);
    agent->action = kick;
}

void CRolePlayMake::parse(QStringList params) {

    setKickToTheirDefense(false);
    setJustTurn(false);
    setSlow(false);
    setChip(false);
    setIndirectKhafan(false);
    setNoStop(false);
    setKickoffWing(false);
    setIndirectGoogooli(false);
    setChipIndirect(false);
    setThrough(false);
    setManualPassReceive(false);
    setAllowOneTouch(false);
    setAllowOnePass(false);
    setFollowSequence(false);
    setKickoffmode(false);
    setIndirectTiny(false);
    setChipInPenaltyArea(false);
    setShadowPass(false);
    setKhers(false);
    setShadowyPoint(false);
    setLongChip(false);
    setPassReceiver(-1);
    setChipToOppGoal(false);
    setSafeIndirect(false);
    setLocalAgentPassTarget("");
    setPassReceiver(-1);
    kick->setInterceptmode(false);
    for (int i = 0; i < params.length(); i++) {
        bool ok = false;
        int p = params[i].toInt(&ok);
        if (params[i].toLower() == "slow") {
            setSlow(true);
        } else if (params[i].toLower() == "chip") {
            setChip(true);
        } else if (params[i].toLower() == "indirectkhafan") {
            setIndirectKhafan(true);
        } else if (params[i].toLower() == "nostop") {
            setNoStop(true);
        } else if (params[i].toLower() == "wing") {
            setKickoffWing(true);
        } else if (params[i].toLower() == "indirectgoogooli") {
            setIndirectGoogooli(true);
        } else if (params[i].toLower() == "chipindirect") {
            setChipIndirect(true);
        } else if (params[i].toLower() == "through") {
            setThrough(true);
        } else if (params[i].toLower() == "nokick") {
            setNoKick(true);
        } else if (params[i].toLower() == "allowonetouch") {
            setAllowOneTouch(true);
        } else if (params[i].toLower() == "allowonepass") {
            setAllowOnePass(true);
        } else if (params[i].toLower() == "followsequence") {
            setFollowSequence(true);
        } else if (params[i].toLower() == "kickoff") {
            setKickoffmode(true);
        } else if (params[i].toLower() == "indirecttiny") {
            setIndirectTiny(true);
        } else if (params[i].toLower() == "chipinpenalty") {
            setChipInPenaltyArea(true);
        } else if (params[i].toLower() == "shadowpass") {
            setShadowPass(true);
        } else if (params[i].toLower() == "khers") {
            setKhers(true);
        } else if (params[i].toLower() == "kicktotheirdefense") {
            setKickToTheirDefense(true);
        } else if (params[i].toLower() == "justturn") {
            setJustTurn(true);
        } else if (params[i].toLower() == "longchip") {
            setLongChip(true);
        } else if (params[i].toLower() == "chiptogoal") {
            setChipToOppGoal(true);
        } else if (params[i].toLower() == "shadowypoint") {
            setShadowyPoint(true);
        } else if (params[i].toLower() == "safe") {
            setSafeIndirect(true);
        } else if (params[i].startsWith("@")) {
            localAgentPassTarget = params[i].right(params[i].length() - 1);
        } else if (params[i].toLower() == "%") {
            setManualPassReceive(true);
            setPassReceiver(-5);
        } else if (ok) {
            setManualPassReceive(true);
            setPassReceiver(p);
        }
    }
}

CRolePlayMakeInfo::CRolePlayMakeInfo(QString _roleName) : CRoleInfo(_roleName) {

}

void CRolePlayMake::resetOffPlays() {
    setIndirectTiny(false);
    setIndirectGoogooli(false);
    setIndirectKhafan(false);
    setChipInPenaltyArea(false);
    setChipIndirect(false);
    setChipInPenaltyArea(false);
}

void CRolePlayMake::resetPlayMake() {
    cyclesToWait = 0;
    cyclesExecuted = 0;
    kickPassMode = KickPassFirst;
    kickPassCyclesWait = 0;
}
