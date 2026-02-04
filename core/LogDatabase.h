#ifndef QLOG_CORE_LOGDATABASE_H
#define QLOG_CORE_LOGDATABASE_H

#include <QString>
#include <QDir>

class LogDatabase
{
public:

    static LogDatabase* instance()
    {
        static LogDatabase instance;
        return &instance;
    };

    static QDir dbDirectory();
    static QString dbFilename();
    static QString currentPlatformId();

    bool atomicCopy(const QString &filename);
    bool openDatabase();
    bool schemaVersionUpgrade();

private:
    LogDatabase();
    bool createSQLFunctions();
};

#endif // QLOG_CORE_LOGDATABASE_H
