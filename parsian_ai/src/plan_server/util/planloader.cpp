#include <plan_server/util/planloader.h>


CPlanLoader::CPlanLoader(QObject* parent) : QObject(parent) {
    watcher = new FileWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}

CPlanLoader::CPlanLoader(const QString &_folderDirectory, QObject *parent) : QObject(parent) {

    watcher = new FileWatcher(this);

    qDebug() << "A Plan Loader Started";

    m_mainDirectory = _folderDirectory;

    //Fill Directory List
    QDirIterator it(m_mainDirectory, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString temp = it.next();
        if (temp.contains("ignore")) {
            continue;
        }
        if (temp.endsWith(".json")) {
            if (it.fileInfo().isSymLink()) {
                temp = it.fileInfo().symLinkTarget();
            }
            m_dirList << temp;
        }
    }
    //Add Files to FileWatcher and remove Files that can't be added.
    Q_FOREACH (QString file, m_dirList) {
        if (file.endsWith(".json")) {
            if (watcher->addFile(file)) {
                qDebug() << file << "Added Succecfully and is be watching";
            } else {
                qDebug() << file << "founded but can't be wathced :( so it's removed !";
                m_dirList.removeOne(file);
            }
        }
    }

    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}

//Public Slots

void CPlanLoader::slt_fileChanged(const QString &_file) {
    qDebug() << _file << "Changed.";
    if (autoUpdate) {
        qDebug() << "Updated automaticly";
    }
}

void CPlanLoader::updateDirectory() {
    disconnect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));

    m_dirList.clear();
    delete watcher;
    watcher = new FileWatcher(this);

    //Fill Directory List
    QDirIterator it(m_mainDirectory, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString temp = it.next();
        if (temp.contains("ignore")) {
            continue;
        }
        if (temp.endsWith(".json")) {
            if (it.fileInfo().isSymLink()) {
                temp = it.fileInfo().symLinkTarget();
            }
            m_dirList << temp;
        }
    }
    //Add Files to FileWatcher and remove Files that can't be added.
    Q_FOREACH (QString file, m_dirList) {
        if (file.endsWith(".json")) {
            if (watcher->addFile(file)) {
                qDebug() << file << "Added Succecfully and is be watching";
            } else {
                qDebug() << file << "founded but can't be wathced :( so it's removed !";
                m_dirList.removeOne(file);
            }
        }
    }
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}
