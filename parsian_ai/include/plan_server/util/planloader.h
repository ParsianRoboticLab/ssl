#ifndef PLANLOADER_H
#define PLANLOADER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>

#include <parsian_util/tools/filewatcher.h>

#define Property(type,name,local) \
        public: inline type& get##name() {return local;} \
        public: inline void set##name(type val) {local = val;} \
        protected: type local

class CPlanLoader : public QObject {

    Q_OBJECT

public:
    CPlanLoader(QObject* parent = 0);
    CPlanLoader(const QString& _folderDirectory, QObject * parent = 0);

    void updateDirectory();

protected:
    FileWatcher* watcher;
    QString m_mainDirectory;
    QStringList m_dirList;


    Property(bool, AutoUpdate, autoUpdate);

private:

public slots:
    virtual void slt_fileChanged(const QString& _file);
};



#endif // PLANLOADER_H

