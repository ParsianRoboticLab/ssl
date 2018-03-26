#include <plan_server/util/loadplayoffjson.h>

CLoadPlayOffJson::CLoadPlayOffJson(QObject* parent) : CPlanLoader(parent) {
}

CLoadPlayOffJson::CLoadPlayOffJson(const QString &_folderDirectory, QObject *parent)
    : CPlanLoader(_folderDirectory, parent) {

    ROS_INFO("Plan Loader -> PlayOff Json ");

    Q_FOREACH (QString dir, m_dirList) {
        load(dir);
    }

    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}

bool CLoadPlayOffJson::loadAll() {
    m_plans.clear();
    updateDirectory();
    Q_FOREACH (QString dir, m_dirList) {
        load(dir);
    }
}

bool CLoadPlayOffJson::load(QString _file) {
    qDebug() << "Loading ..." << _file;
    bool readable = false;
    QFile file(_file);

    if (!file.open(QIODevice::ReadOnly)) {
        DBUG("Couldn't open Plan file.", D_MAHI);
        return false;
    }
    QByteArray input = file.readAll();
    file.close();

    QJson::Parser parser;
    QVariantMap dataBase = parser.parse(input, &readable).toMap();
    if (readable) {
        readPlan(dataBase, _file);
    } else {
        qWarning() << "Can't Parse Plan File : " << _file;
        return false;
    }
    qDebug() << "Plan Loaded";
}

bool CLoadPlayOffJson::readPlan(const QVariantMap &_map, const QString& _file) {

    bool parsedOk = false;

    double mapVersion = _map.value("apiVersion").toDouble(&parsedOk);
    if (!parsedOk) {
        qWarning() << "Can't Parse Version";
        return false;
    }

    if (mapVersion != API_VERSION) {
        qWarning() << "Version of plan and Application have Confilict."
                   << "Your Version : " << API_VERSION
                   << "File Version : " << mapVersion;
        return false;
    }



    QVariantList plans = _map.value("plans").toList();
    Q_FOREACH (QVariant plan, plans) {
        NGameOff::SPlan* tempPlan = new NGameOff::SPlan();

        NGameOff::SGUI      & tempGui       = tempPlan->gui;
        NGameOff::SCommon   & tempCommon    = tempPlan->common;
        NGameOff::SMatching & tempMatching  = tempPlan->matching;
        NGameOff::SExecution& tempExecution = tempPlan->execution;

        QVariantMap planMap = plan.toMap();
        QFileInfo fileInfo(_file);
        fillCommon(tempCommon, planMap, std::move(parsedOk));
        fillMatching(tempMatching, planMap, std::move(parsedOk));
        fillExecution(tempExecution, planMap, std::move(parsedOk));
        fillGUI(tempGui, fileInfo, std::move(parsedOk));


        tempMatching.shotPos = findShotPos(tempPlan);

        m_plans.append(tempPlan);
    }
}

void CLoadPlayOffJson::fillCommon(NGameOff::SCommon& _common, const QVariantMap& _plan, bool&& _parsedOk) {

    _common.tags       = _plan.value("tags").toStringList();
    _common.chance     = _plan.value("chance").toDouble(&_parsedOk);
    _common.lastDist   = _plan.value("lastDist").toDouble(&_parsedOk);
    _common.agentSize  = _plan.value("agents").toList().size();
    _common.planRepeat = 0;

    QString planMode = _plan.value("planMode").toString();
    if (planMode == "KICKOFF") {
        _common.planMode = KICKOFF;
    } else if (planMode == "DIRECT") {
        _common.planMode = DIRECT;
    } else if (planMode == "INDIRECT") {
        _common.planMode = INDIRECT;
    }
}

void CLoadPlayOffJson::fillMatching(NGameOff::SMatching& _matching, const QVariantMap& _plan, bool&& _parsedOk) {

    _matching.initPos.ball = Vector2D(_plan.value("ballInitPos").toMap().value("x").toDouble(&_parsedOk),
                                      _plan.value("ballInitPos").toMap().value("y").toDouble(&_parsedOk));
    if (!_parsedOk) {
        qWarning() << "Ball Init Pos did NOT parsed okey!";
        return;
    }

    Q_FOREACH (QVariant initPos, _plan.value("agentInitPos").toList()) {
        QVariantMap initPosMap = initPos.toMap();
        _matching.initPos.agents.append(Vector2D(initPosMap.value("x").toDouble(&_parsedOk),
                                        initPosMap.value("y").toDouble(&_parsedOk)));
    }

    if (!_parsedOk) {
        qWarning() << "Agent Init Pos did NOT parsed okey!";
        return;
    }
}

void CLoadPlayOffJson::fillExecution(NGameOff::SExecution &_execution, const QVariantMap &_plan, bool&& _parsedOk) {

    qDebug() << "PLAN" << _plan.value("agents").toList().size();
    Q_FOREACH (QVariant agent, _plan.value("agents").toList()) {
        QList<playOffRobot> tempRobots;
        QVariantList positionList = agent.toMap().value("positions").toList();
        Q_FOREACH (QVariant position, positionList) {
            QVariantMap positionMap = position.toMap();
            playOffRobot tempRobot;
            tempRobot.tolerance = positionMap.value("tolerance").toDouble(&_parsedOk);
            tempRobot.angle = AngleDeg(positionMap.value("angel").toDouble(&_parsedOk));
            tempRobot.pos = Vector2D(positionMap.value("pos-x").toDouble(&_parsedOk),
                                     positionMap.value("pos-y").toDouble(&_parsedOk));
            if (!_parsedOk) {
                qWarning() << "Position did NOT parsed okey!";
                return;
            }


            Q_FOREACH (QVariant skill, positionMap.value("skills").toList()) {
                QVariantMap skillMap = skill.toMap();
                playOffSkill tempSkill;
                tempSkill.name = strToEnum(skillMap.value("name").toString());
                tempSkill.data[0] = skillMap.value("primary").toInt(&_parsedOk);
                tempSkill.data[1] = skillMap.value("secondary").toInt(&_parsedOk);
                tempSkill.targetAgent = skillMap.value("target").toMap().value("agent").toInt(&_parsedOk);
                tempSkill.targetIndex = skillMap.value("target").toMap().value("index").toInt(&_parsedOk);
                if (!_parsedOk) {
                    tempSkill.targetAgent = -1;
                    tempSkill.targetIndex = -1;
                }

                tempRobot.skill.append(tempSkill);
            }
            tempRobots.append(tempRobot);
        }
        _execution.AgentPlan.append(tempRobots);
    }
}

void CLoadPlayOffJson::fillGUI(NGameOff::SGUI &_gui, const QFileInfo& _fileInfo, bool&& _parsedOk) {
    _gui.master = false;
    _gui.active = true;
    _gui.planFile = _fileInfo.baseName();
    _gui.package = getPackageName(_fileInfo.absolutePath());
    _gui.name = QString("Mahi");
}


QString CLoadPlayOffJson::getPackageName(QString _path) {
    QString packageName = QFileInfo(m_mainDirectory).baseName();
    _path.remove(0, m_mainDirectory.length());
    _path.replace('/', '.');
    packageName.append(_path);
    //    qDebug() << packageName;
    return packageName;
}

Vector2D CLoadPlayOffJson::findShotPos(NGameOff::SPlan *&_plan) {
    QList<POffSkills> finisher;
    finisher.append(ShotToGoalSkill);
    finisher.append(ChipToGoalSkill);
    finisher.append(OneTouchSkill);

    Q_FOREACH (QList<playOffRobot> agentPlan, _plan->execution.AgentPlan) {
        Q_FOREACH (playOffRobot agentTask, agentPlan) {
            Q_FOREACH (playOffSkill agentSkill, agentTask.skill) {
                if (finisher.contains(agentSkill.name)) {
                    return agentTask.pos;
                }
            }
        }
    }
}

POffSkills CLoadPlayOffJson::strToEnum(const QString& _str) {
    if (_str == "NoSkill") {
        return NoSkill;
    } else if (_str == "Mark") {
        return Mark;
    } else if (_str == "Goalie") {
        return Goalie;
    } else if (_str == "Support") {
        return Support;
    } else if (_str == "Defense") {
        return Defense;
    } else if (_str == "Position") {
        return Position;
    } else if (_str == "MoveSkill") {
        return MoveSkill;
    } else if (_str == "PassSkill") {
        return PassSkill;
    } else if (_str == "OneTouchSkill") {
        return OneTouchSkill;
    } else if (_str == "ChipToGoalSkill") {
        return ChipToGoalSkill;
    } else if (_str == "ShotToGoalSkill") {
        return ShotToGoalSkill;
    } else if (_str == "ReceivePassSkill") {
        return ReceivePassSkill;
    } else if (_str == "ReceivePassIASkill") {
        return ReceivePassIASkill;
    } else {
        return NoSkill;
    }
}

QString CLoadPlayOffJson::getModeStr(POMODE _mode) {

    QString mode = "Indirect";
    if (_mode == KICKOFF) {
        mode = "Kick Off";
    } else if (_mode == DIRECT) {
        mode = "Direct";
    } else if (_mode == INDIRECT) {
        mode = "Indirect";
    }
    return mode;

}

//Public slots
void CLoadPlayOffJson::slt_fileChanged(const QString &_file) {
    if (autoUpdate) {
        load(_file);
        emit plansUpdated();
    } else {
        qDebug() << "File Changed But not Updated";
    }
}
