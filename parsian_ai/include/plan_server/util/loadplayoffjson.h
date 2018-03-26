#ifndef LOADPLAYOFFJSON_H
#define LOADPLAYOFFJSON_H


#include <QVariant>

#include <plan_server/util/planloader.h>
#include <parsian_util/tools/debuger.h>
#include "../../planstruct.h"
#include <parsian_util/geom/geom.h>
#include <qjson/parser.h>

#define API_VERSION 1.2

class CLoadPlayOffJson : public CPlanLoader {

    Q_OBJECT

public:
    CLoadPlayOffJson(QObject* = 0);
    CLoadPlayOffJson(const QString& _folderDirectory, QObject* parent = 0);

    bool loadAll();
    bool load(QString _file);
    bool save(QString _file);
    bool saveAll();

    QList<NGameOff::SPlan*> getPlans() {
        return m_plans;
    }

    QString getModeStr(POMODE _mode);
protected:
    //All of Them !
    QList<NGameOff::SPlan*> m_plans;
    bool readPlan(const QVariantMap& _map, const QString& _file);
    void fillMatching(NGameOff::SMatching& _matching, const QVariantMap& _plan, bool&& _parsedOK = 0);
    void fillCommon(NGameOff::SCommon& _common, const QVariantMap& _plan, bool&& _parsedOk = 0);
    void fillExecution(NGameOff::SExecution& _execution, const QVariantMap& _plan, bool&& _parsedOk = 0);
    void fillGUI(NGameOff::SGUI& _gui, const QFileInfo& _fileInfo, bool&& _parsedOk = 0);
    POffSkills strToEnum(const QString& _str);
    QString getPackageName(QString _path);
    Vector2D findShotPos(NGameOff::SPlan* &_plan);

private:

signals:
    void plansUpdated();

public slots:
    virtual void slt_fileChanged(const QString &_file) override;

};

#endif // LOADPLAYOFFJSON_H
