#ifndef BUILDBLASTDATABASEWORKER_H
#define BUILDBLASTDATABASEWORKER_H

#include <QObject>
#include <QProcess>

class BuildBlastDatabaseWorker : public QObject
{
    Q_OBJECT

public:
    BuildBlastDatabaseWorker(QString makeblastdbCommand, QProcess * makeblastdb);

private:
    QString m_makeblastdbCommand;
    QProcess * m_makeblastdb;

public slots:
    void buildBlastDatabase();

signals:
    void finishedBuild(QString error);
};

#endif // BUILDBLASTDATABASEWORKER_H
