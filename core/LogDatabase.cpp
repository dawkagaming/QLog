#include <QSqlDatabase>
#include <QSqlDriver>
#include <sqlite3.h>
#include <QSqlError>
#include <QSqlQuery>
#include <QCoreApplication>

#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

#include "LogDatabase.h"
#include "core/debug.h"
#include "core/Migration.h"

MODULE_IDENTIFICATION("qlog.core.logdatabase");

QDir LogDatabase::dbDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
}

QString LogDatabase::dbFilename()
{
    return dbDirectory().filePath("qlog.db");
}

LogDatabase::LogDatabase()
{
    FCT_IDENTIFICATION;
}

bool LogDatabase::createSQLFunctions()
{
    FCT_IDENTIFICATION;

    QVariant v = QSqlDatabase::database().driver()->handle();

    if ( !v.isValid()
         || qstrcmp(v.typeName(), "sqlite3*") != 0 )
    {
        qCritical() << "Cannot get SQLite driver handle";
        return false;
    }

    sqlite3 *db_handle = *static_cast<sqlite3 **>(v.data());
    if ( db_handle == 0 )
    {
        qCritical() << "Cannot define new SQLite functions";
        return false;
    }

    sqlite3_initialize();
    sqlite3_create_function(db_handle,
                            "translate_to_locale",
                            1,
                            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                            nullptr,
                            [](sqlite3_context *ctx, int argc, sqlite3_value **argv) {
                                if ( argc != 1 )
                                {
                                    sqlite3_result_error(ctx, "Invalid arguments", -1);
                                    return;
                                }

                                switch ( sqlite3_value_type(argv[0]) )
                                {
                                case SQLITE_TEXT:
                                {
                                    const char *text = reinterpret_cast<const char*>(sqlite3_value_text(argv[0]));
                                    const QString &translatedText = QCoreApplication::translate("DBStrings", text);
                                    sqlite3_result_text(ctx, translatedText.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                                }
                                    break;
                                case SQLITE_NULL:
                                    sqlite3_result_null(ctx);
                                    break;
                                case SQLITE_INTEGER:
                                    sqlite3_result_int(ctx, sqlite3_value_int(argv[0]));
                                    break;
                                case SQLITE_FLOAT:
                                    sqlite3_result_double(ctx, sqlite3_value_double(argv[0]));
                                    break;
                                default:
                                    sqlite3_result_error(ctx, "Invalid arguments", -1);
                                }
                            }, nullptr, nullptr);
    sqlite3_create_collation(db_handle,
                             "LOCALEAWARE",
                             SQLITE_UTF16,
                             nullptr,
                             [](void *, int ll, const void * l, int rl, const void * r) {
                                const QString &left = QString::fromUtf16(reinterpret_cast<const char16_t *>(l), ll/2);
                                const QString &right = QString::fromUtf16(reinterpret_cast<const char16_t *>(r), rl/2);
                                return QString::localeAwareCompare(left, right); // controlled by LC_COLLATE
                             });

    return true;
}

bool LogDatabase::atomicCopy(const QString &filename)
{
    FCT_IDENTIFICATION;

    QSqlDatabase db = QSqlDatabase::database();

    if ( !db.isOpen() )
    {
        qWarning() << "Database is not opened";
        return false;
    }

    // Validate filename — must be a plain filename, no path separators
    if ( filename.isEmpty()
         || filename.contains('/')
         || filename.contains('\\')
         || filename == "."
         || filename == ".." )
    {
        qWarning() << "Invalid filename:" << filename;
        return false;
    }

    QVariant v = db.driver()->handle();
    if ( !v.isValid() || qstrcmp(v.typeName(), "sqlite3*") != 0)
    {
        qWarning() << "Cannot get Database Driver Handler";
        return false;
    }

    sqlite3 *srcHandle = *static_cast<sqlite3 **>(v.data());
    sqlite3 *dstHandle = nullptr;
    sqlite3_backup *backup = nullptr;
    bool ok = false;
    int stepSize = 5000;

    const QString newDBFilename = dbDirectory().filePath(filename);

    if (sqlite3_open_v2(newDBFilename.toUtf8().constData(), &dstHandle,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        qWarning() << "Cannot open a new database file:" << sqlite3_errmsg(dstHandle);
        goto cleanup;
    }

    backup = sqlite3_backup_init(dstHandle, "main", srcHandle, "main");
    if ( !backup )
    {
        qWarning() << "Cannot Init a backup" << sqlite3_errmsg(dstHandle);
        goto cleanup;
    }

    while (true)
    {
        int rc = sqlite3_backup_step(backup, stepSize);

        int total = sqlite3_backup_pagecount(backup);
        int remaining = sqlite3_backup_remaining(backup);

        int done = total - remaining;
        int percent = total > 0 ? (done * 100 / total) : 0;

        qInfo() << percent;

        if (rc == SQLITE_DONE)
        {
            ok = true;
            break;
        }
        else if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
        {
            sqlite3_sleep(50);
            continue;
        }
        else if (rc == SQLITE_OK)
        {
            continue;
        }
        else
        {
            qWarning() << "Backup Error" << sqlite3_errmsg(dstHandle);
            break;
        }
    }

    // sqlite3_backup_finish always releases the backup handle,
    // even on error — never call it twice
    if ( sqlite3_backup_finish(backup) != SQLITE_OK )
    {
        qWarning() << "Cannot finalize the database copy" << sqlite3_errmsg(dstHandle);
        ok = false;
    }
    backup = nullptr;

cleanup:
    if ( backup )
        sqlite3_backup_finish(backup);

    if ( dstHandle )
        sqlite3_close(dstHandle);

    if ( !ok )
        QFile::remove(newDBFilename);

    return ok;
}

bool LogDatabase::openDatabase()
{
    FCT_IDENTIFICATION;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilename());
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if ( !db.open() )
    {
        qCritical() << db.lastError();
        return false;
    }

    QSqlQuery query;
    if ( !query.exec("PRAGMA foreign_keys = ON") )
    {
        qCritical() << "Cannot set PRAGMA foreign_keys";
        return false;
    }

    if ( !query.exec("PRAGMA journal_mode = WAL") )
    {
        qCritical() << "Cannot set PRAGMA journal_mode";
        return false;
    }

    while ( query.next() )
    {
        QString pragma = query.value(0).toString();
        qCDebug(runtime) << "Pragma result:" << pragma;
    }

    return createSQLFunctions();
}

bool LogDatabase::schemaVersionUpgrade()
{
    FCT_IDENTIFICATION;

    DBSchemaMigration m;
    return m.run();
}

