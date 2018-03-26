#include "mark.h"

INIT_ROLE(CRoleMark, "mark");

CRoleMark::CRoleMark(Agent *_agent) : CRole(_agent) {
    gotopointavoid = new CSkillGotoPointAvoid(_agent);
    shoot = new CBehaviourKick;
    kickOffMode = false;
    toBeMarkedID = -1;
    forceMark = -1;
}

CRoleMark::~CRoleMark() {
    delete gotopointavoid;
}


int CRoleMark::chooseOppToMark() {

    QList <CRobot*> markedByDef;
    for (int i = 0 ; i < knowledge->toBeMopps.size() ; i++)
        if (knowledge->toBeMopps[i]->markedByDefense == true) {
            markedByDef.append(knowledge->toBeMopps[i]);
        }

    QList <CRobot*> freeOpps;
    for (int i = 0 ; i < knowledge->toBeMopps.size() ; i++)
        if (knowledge->toBeMopps[i]->markedByDefense == false) {
            freeOpps.append(knowledge->toBeMopps[i]);
        }

    for (int i = 0 ; i < freeOpps.count() ; i++) {
        draw(freeOpps[i]->pos);
    }

    int res = -1;
    double minDist = 100000;

    //sharte dovom tokhom?
    for (int i = 0 ; i < freeOpps.size() && i < info()->count() ; i++) {
        if (freeOpps[i]->pos.dist(agent->pos()) < minDist) {
            minDist = freeOpps[i]->pos.dist(agent->pos());
            res = freeOpps[i]->id;
        }
    }

    if (res == -1) {
        for (int i = 0 ; i < markedByDef.size() ; i++) {
            if (markedByDef[i]->markedByMark == false && markedByDef[i]->pos.dist(agent->pos()) < minDist) {
                minDist = markedByDef[i]->pos.dist(agent->pos());
                res = markedByDef[i]->id;
            }
        }
    }

    if (res != -1) {
        wm->opp[res]->markedByMark = true;
        draw(QString("%1").arg(res) , wm->opp[res]->pos + Vector2D(0.2 , -0.2) , "yellow");
    }

    return res;
}

void CRoleMark::execute() {

    chooseOppToMark();

    double w, ang, coming;
    Vector2D target = knowledge->onetouchablity(agent->id(), w, ang, coming);
    info()->oneToucher.removeAll(agent->self()->id);
    if (info()->oneToucher.isEmpty()) {
        info()->oneToucherDist2Ball = 100;
    }
    bool playmakerIsFar = true;
    if (knowledge->getPlayMaker() != NULL)
        if (knowledge->getPlayMaker()->pos().dist(agent->pos()) < 0.45) {
            playmakerIsFar = false;
        }
    if (agent->pos().dist(wm->ballCatchTarget(agent->self())) < info()->oneToucherDist2Ball
            && (agent->canOneTouch() && coming > 0.3 && ang < policy()->PlayMaker_OneTouchAngleThreshold())
            && playmakerIsFar) {
        draw(QString("Mark OneTouch"), Vector2D(-2, 1), "magenta", 18);
        shoot->agents.clear();
        shoot->agents.append(agent);
        shoot->setSlowKick(false);
        shoot->setChipToGoal(false);
        shoot->execute();
        info()->oneToucher.append(agent->self()->id);
        info()->oneToucherDist2Ball = agent->pos().dist(wm->ballCatchTarget(agent->self()));
        return;
    }


    Vector2D markPos;
    Vector2D nextPos;
    double predit_time = 0.2;

//  toBeMarkedID = chooseOppToMark();
    toBeMarkedID = info()->lastMatched[agent->id()];

    if (toBeMarkedID == -1 && info()->markedOpp.count() > 0) {
        toBeMarkedID = info()->markedOpp.at(0);

        Vector2D nextPos;
        if (wm->opp[toBeMarkedID]->vel.valid() && wm->opp[toBeMarkedID]->vel.length() > 0.3) {
            nextPos = (wm->opp[toBeMarkedID]->vel * predit_time) + wm->opp[toBeMarkedID]->pos;
        } else {
            nextPos = wm->opp[toBeMarkedID]->pos;
        }
        //this may result in push (when it enters the above else)
        markPos = ((wm->ball->pos - nextPos).norm() * (2 * Robot::robot_radius_new)) + nextPos;
    } else if (toBeMarkedID != -1 && info()->markedOpp.contains(toBeMarkedID) == true) {
        Vector2D nextPos;
        if (wm->opp[toBeMarkedID]->vel.valid() && wm->opp[toBeMarkedID]->vel.length() > 0.3) {
            nextPos = (wm->opp[toBeMarkedID]->vel * predit_time) + wm->opp[toBeMarkedID]->pos;
        } else {
            nextPos = wm->opp[toBeMarkedID]->pos;
        }
        //this may result in push (when it enters the above else)
        markPos = ((wm->ball->pos - nextPos).norm() * (2 * Robot::robot_radius_new)) + nextPos;
    } else {
        bool markFailed = false;
        if (toBeMarkedID != -1 && wm->opp[toBeMarkedID]->inSight <= 0) {
            markFailed = true;
        }

        if (toBeMarkedID == -1 || markFailed) {
//      debug( QString("There is no opponent with supporter role or supporter is not inSight.") , D_GAME | D_MASOOD);
            markPos = wm->ball->pos;
        } else {
            info()->markedOpp.append(toBeMarkedID);
            wm->opp[toBeMarkedID]->markedByMark = true;
            draw(QString("%1").arg(agent->id()) , wm->opp[toBeMarkedID]->pos + Vector2D(0.2 , -0.2) , "yellow");

            if (wm->opp[toBeMarkedID]->vel.valid() && wm->opp[toBeMarkedID]->vel.length() > 0.3) {
                nextPos = (wm->opp[toBeMarkedID]->vel * predit_time) + wm->opp[toBeMarkedID]->pos;
            } else {
                nextPos = wm->opp[toBeMarkedID]->pos;
            }
            markPos = ((wm->field->ourGoal() - nextPos).norm() * (2 * Robot::robot_radius_new)) + nextPos;
        }
    }

    double minX = 10;
    double maxDist = 10;
    for (int ii = 0 ; ii < wm->opp.activeAgentsCount() ; ii++) {
        if (minX > wm->opp.active(ii)->pos.x) {
            minX = wm->opp.active(ii)->pos.x;
            maxDist = wm->opp.active(ii)->pos.dist(wm->field->ourGoal());
        }
    }

    maxDist = max(1 , maxDist);

    if (wm->field->ourGoal().dist(markPos) > maxDist) {
        for (int i = 0 ; i < 1000 && wm->field->ourGoal().dist(markPos) > maxDist ; i++) {
            markPos = ((wm->field->ourGoal() - markPos).norm() * 0.01) + markPos;
        }
    }

    if (kickOffMode) {
        if (markPos.length() < 0.600 || markPos.x > -Robot::robot_radius_new) {
            for (int i = 0 ; i < 1000 && (markPos.x > -Robot::robot_radius_new || markPos.length() < 0.700) ; i++) {
                markPos = ((wm->field->ourGoal() - markPos).norm() * 0.01) + markPos;
            }
        }
    }

    if (wm->ball->pos.x + 1 < markPos.x) {
        for (int i = 0 ; i < 1000 && wm->ball->pos.x + 1 < markPos.x ; i++) {
            markPos = ((wm->field->ourGoal() - markPos).norm() * 0.01) + markPos;
        }
    }

    Vector2D checkMarkPos = markPos + (wm->field->ourGoal() - markPos).norm() * 0.10;
    if (wm->field->isInOurPenaltyArea(checkMarkPos)) {
        for (int i = 0 ; i < 1000 && wm->field->isInOurPenaltyArea(checkMarkPos) ; i++) {
            checkMarkPos = ((checkMarkPos - wm->field->ourGoal()).norm() * 0.01) + checkMarkPos;
        }
        markPos = checkMarkPos + (checkMarkPos - wm->field->ourGoal()).norm() * 0.10;;
    }


    if ((knowledge->getGameState() == CKnowledge::TheirIndirectKick || knowledge->getGameState() == CKnowledge::TheirDirectKick)
            && markPos.dist(wm->ball->pos) < 0.600) {
        for (int i = 0 ; i < 1000 && markPos.dist(wm->ball->pos) < 0.600 ; i++) {
            markPos = ((wm->field->ourGoal() - markPos).norm() * (0.01)) + markPos;
        }
    }

    if (kickOffMode || knowledge->getGameState() == CKnowledge::TheirIndirectKick || knowledge->getGameState() == CKnowledge::TheirDirectKick) {
        int k = 0;
        Agent* blocker;
        while ((blocker = info()->blocker(k)) != NULL) {
            if ((blocker->pos() - markPos).length() < Robot::robot_radius_new * 3.0) {
                for (int i = 0 ; i < 1000 && ((blocker->pos() - markPos).length() < Robot::robot_radius_new * 3.0) ; i++) {
                    markPos = ((wm->field->ourGoal() - markPos).norm() * 0.01) + markPos;
                }
            }
            k++;
        }
    }

    if (markPos.dist(wm->field->oppGoal()) < 1) {
        for (int i = 0 ; i < 1000 && markPos.dist(wm->field->oppGoal()) < 1 ; i++) {
            markPos = ((wm->field->ourGoal() - markPos).norm() * 0.01) + markPos;
        }
    }

    draw(Circle2D(markPos , 0.07) , 0 , 360 , "black" , true);
    draw(QString("%1").arg(agent->self()->id), markPos, QColor("red"));

    gotopointavoid->setAgent(agent);
    gotopointavoid->setAvoidPenaltyArea(true);
    gotopointavoid->setTargetLook(markPos, wm->ball->pos);

    if (knowledge->getGameState() == CKnowledge::TheirKickOff) {
        gotopointavoid->setAvoidCenterCircle(true);
    } else {
        gotopointavoid->setAvoidCenterCircle(false);
    }

    gotopointavoid->execute();
}

double CRoleMark::progress() {
    return 0.0;
}

CRoleMarkInfo::CRoleMarkInfo(QString _roleName) : CRoleInfo(_roleName) {
    oneToucher.clear();
    oneToucherDist2Ball = +100;
    mwbm.create(_MAX_NUM_PLAYERS , _MAX_NUM_PLAYERS);
}

Agent* CRoleMarkInfo::blocker(int i) {
    int z = 0;
    for (int k = 0; k < knowledge->agentCount(); k++) {
        if (knowledge->getAgent(k)->skillName == "block") {
            if (z == i) {
                return knowledge->getAgent(k);
            }
            z++;
        }
    }
    return NULL;
}

void CRoleMarkInfo::matching() {
    QList <Agent*> ourOffendersAgents;
    QList <int> oppOffenders;
    QList <int> ourOffenders;

    oppOffenders.clear();
    for (int i = 0 ; i < knowledge->toBeMopps.size() ; i++)
        if (knowledge->toBeMopps[i]->markedByDefense == false) {
            oppOffenders.append(knowledge->toBeMopps[i]->id);
        }

    ourOffendersAgents.clear();
    ourOffenders.clear();
    ourOffendersAgents.append(knowledge->roleAssignments["mark"]);
    for (int i = 0 ; i < ourOffendersAgents.size() ; i++) {
        ourOffenders.append(ourOffendersAgents[i]->id());
    }

    if (oppOffenders.size() == 0 || ourOffenders.size() > oppOffenders.size()) {
        for (int i = 0 ; i < knowledge->toBeMopps.size() && oppOffenders.size() < ourOffenders.size() ; i++)
            if (knowledge->toBeMopps[i]->markedByDefense == true) {
                oppOffenders.append(knowledge->toBeMopps[i]->id);
            }
    }

    if (ourOffenders.size() == 0 || oppOffenders.size() == 0) {
        return;
    }

    const double Dist_Mark = 3;
    const double Ang_Mark = 90;

    if (ourOffenders.size() > oppOffenders.size()) {
        QPair<double , int> weightSum[_MAX_NUM_PLAYERS];
        for (int i = 0; i < ourOffenders.count(); i++) {
            weightSum[i].first = 0;
            weightSum[i].second = i;
            for (int j = 0; j < oppOffenders.count(); j++) {
                double inertia = 0;
                if (lastMatched[ourOffenders[i]] == oppOffenders[j]) {
                    inertia = 10;
                }
                double d = (wm->our[ourOffenders[i]]->pos - wm->opp[oppOffenders[j]]->pos).length();

                double ang = AngleDeg::normalize_angle((wm->opp[oppOffenders[j]]->pos - wm->field->ourGoal()).th().degree() -
                                                       (wm->our[ourOffenders[i]]->pos - wm->field->ourGoal()).th().degree());

                //double score = 1000*wm->opp[oppOffenders[j]]->danger*exp(-d*d/(Dist_Mark*Dist_Mark)) + inertia;

                double score = 1000 * exp(-ang * ang / (Ang_Mark * Ang_Mark)) + inertia;

                weightSum[i].first += score;
            }
        }
        qSort(weightSum , weightSum + ourOffenders.size());
        QList<int> ourOffendersBak;
        ourOffendersBak.append(ourOffenders);
        bool flag = ourOffendersBak.size();
        int kk = 0;
        while (flag) {
            kk ++;
            int cnt = 0;
            ourOffenders.clear();
            for (int i = ourOffendersBak.size() - 1 ; cnt < oppOffenders.size() && i >= 0 ; i--) {
                if (ourOffendersBak[weightSum[i].second] != -1) {
                    ourOffenders.append(ourOffendersBak.at(weightSum[i].second));
                    ourOffendersBak[weightSum[i].second] = -1;
                    cnt++;
                }
            }

            mwbm.changeSize(ourOffenders.size(), oppOffenders.size());

            for (int i = 0; i < ourOffenders.count(); i++)
                for (int j = 0; j < oppOffenders.count(); j++) {
                    double inertia = 0;
                    if (lastMatched[ourOffenders[i]] == oppOffenders[j]) {
                        inertia = 10;
                    }
                    double d = (wm->our[ourOffenders[i]]->pos - wm->opp[oppOffenders[j]]->pos).length();
//          double score = 1000*wm->opp[oppOffenders[j]]->danger*exp(-d*d/(Dist_Mark*Dist_Mark)) + inertia;

                    double ang = AngleDeg::normalize_angle((wm->opp[oppOffenders[j]]->pos - wm->field->ourGoal()).th().degree() -
                                                           (wm->our[ourOffenders[i]]->pos - wm->field->ourGoal()).th().degree());

                    //double score = 1000*wm->opp[oppOffenders[j]]->danger*exp(-d*d/(Dist_Mark*Dist_Mark)) + inertia;

                    double score = 1000 * exp(-ang * ang / (Ang_Mark * Ang_Mark)) + inertia;


                    mwbm.setWeight(i, j, score);
                    Segment2D seg(wm->our[ourOffenders[i]]->pos , wm->opp[oppOffenders[j]]->pos);
                    draw(seg , "blue");
                    draw(QString("%1").arg(score) , (seg.a() + seg.b()) / 2);
                }

            for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
                lastMatched[i] = -1;
            }
            mwbm.findMatching();
            for (int i = 0; i < ourOffenders.count(); i++) {
                if (mwbm.getMatch(i) >= 0) {
                    lastMatched[ourOffenders[i]] = oppOffenders[mwbm.getMatch(i)];
                } else {
                    lastMatched[ourOffenders[i]] = -1;
                }
            }
            flag = false;
            for (int i = 0 ; i < ourOffendersBak.size() ; i++)
                if (ourOffendersBak.at(i) != -1) {
                    flag = true;
                    break;
                }
        }
    } else {
        mwbm.changeSize(ourOffenders.size(), oppOffenders.size());

        for (int i = 0; i < ourOffenders.count(); i++)
            for (int j = 0; j < oppOffenders.count(); j++) {
                double inertia = 0;
                if (lastMatched[ourOffenders[i]] == oppOffenders[j]) {
                    inertia = 10;
                }
                double d = (wm->our[ourOffenders[i]]->pos - wm->opp[oppOffenders[j]]->pos).length();
                //double score = 1000*wm->opp[oppOffenders[j]]->danger*exp(-d*d/(Dist_Mark*Dist_Mark)) + inertia;

                double ang = AngleDeg::normalize_angle((wm->opp[oppOffenders[j]]->pos - wm->field->ourGoal()).th().degree() -
                                                       (wm->our[ourOffenders[i]]->pos - wm->field->ourGoal()).th().degree());

                //double score = 1000*wm->opp[oppOffenders[j]]->danger*exp(-d*d/(Dist_Mark*Dist_Mark)) + inertia;

                double score = 1000 * exp(-ang * ang / (Ang_Mark * Ang_Mark)) + inertia;


                mwbm.setWeight(i, j, score);
                Segment2D seg(wm->our[ourOffenders[i]]->pos , wm->opp[oppOffenders[j]]->pos);
                draw(seg , "blue");
                draw(QString("%1").arg(score) , (seg.a() + seg.b()) / 2);
            }

        for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
            lastMatched[i] = -1;
        }
        mwbm.findMatching();
        for (int i = 0; i < ourOffenders.count(); i++) {
            if (mwbm.getMatch(i) >= 0) {
                lastMatched[ourOffenders[i]] = oppOffenders[mwbm.getMatch(i)];
            } else {
                lastMatched[ourOffenders[i]] = -1;
            }
        }
    }
}

void CRoleMark::parse(QStringList params) {
    forceMark = -1;
    kickOffMode = false;
    toBeMarkedRole.clear();
    for (int i = 0; i < params.length(); i++) {
        if (params[i].toLower() == "kickoffmode") {
            kickOffMode = true;
        } else {
            toBeMarkedRole.append(params[i].toLower());
        }
    }
}
