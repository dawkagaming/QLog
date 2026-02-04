#ifndef QLOG_CORE_MIGRATION_H
#define QLOG_CORE_MIGRATION_H

#include "core/LOVDownloader.h"

class QProgressDialog;

class DBSchemaMigration : public QObject
{
    Q_OBJECT

public:
    DBSchemaMigration(QObject *parent = nullptr) : QObject(parent) {}
    bool run();
    static bool backupAllQSOsToADX(bool force = false);

private:
    bool functionMigration(int version);
    bool migrate(int toVersion);
    int getVersion();
    bool setVersion(int version);
    bool runSqlFile(QString filename);
    int tableRows(const QString &name);
    bool updateExternalResource();
    void updateExternalResourceProgress(QProgressDialog&,
                                        LOVDownloader&,
                                        const LOVDownloader::SourceType & sourceType,
                                        const QString &counter);
    bool fixIntlFields();
    bool insertUUID();
    bool fillMyDXCC();
    bool createTriggers();
    bool importQSLCards2DB();
    bool fillCQITUZStationProfiles();
    bool resetConfigs();
    bool profiles2DB();
    bool settings2DB();
    bool removeSettings2DB();
    bool setSelectedProfile(const QString &tablename, const QString &profileName);
    QString fixIntlField(QSqlQuery &query, const QString &columName, const QString &columnNameIntl);
    bool refreshUploadStatusTrigger();

    static const int latestVersion = 35;

    friend class MigrationSqlTest_FriendAccessor;
};

#endif // QLOG_CORE_MIGRATION_H
