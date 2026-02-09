#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

#include "PlatformParameterManager.h"
#include "core/LogParam.h"
#include "core/LogDatabase.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.core.platformparametermanager");

QList<QPair<QString, QString>> PlatformParameterManager::knownParameters()
{
    FCT_IDENTIFICATION;

    // List of known platform-dependent parameters
    // Format: (LogParam key, Human-readable display name)
    //
    // Note: TQSL Path is handled specially in getParameters() because:
    // - On LinuxFlatpak target: path is fixed at /app/bin/tqsl, no need to ask
    // - From LinuxFlatpak source: path must be provided by user

    return {
        { "services/lotw/callbook/tqsl", QObject::tr("TQSL Path") }
    };
}

QList<PlatformParameter> PlatformParameterManager::getParameters(const QString &importDbPath,
                                                                  const QString &sourcePlatform)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << importDbPath << sourcePlatform;

    QList<PlatformParameter> result;

    const QString currentPlatform = LogDatabase::currentPlatformId();
    const bool platformDiffers = (sourcePlatform != currentPlatform);
    const bool targetIsFlatpak = (currentPlatform == QLatin1String("LinuxFlatpak"));
    const bool sourceIsFlatpak = (sourcePlatform == QLatin1String("LinuxFlatpak"));

    qCDebug(runtime) << "Source platform:" << sourcePlatform
                     << "Current platform:" << currentPlatform
                     << "Differs:" << platformDiffers;

    // Open the imported database as a secondary connection
    const QString connectionName = QStringLiteral("PlatformParamImport");

    {
        QSqlDatabase importDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        importDb.setDatabaseName(importDbPath);

        if ( !importDb.open() )
        {
            qWarning() << "Cannot open import database:" << importDb.lastError().text();
            QSqlDatabase::removeDatabase(connectionName);
            return result;
        }

        const QList<QPair<QString, QString>> &known = knownParameters();

        for ( const auto &param : known )
        {
            // Special handling for TQSL Path:
            // - If target is LinuxFlatpak: skip (path is fixed at /app/bin/tqsl)
            // - If source is LinuxFlatpak: don't show imported value (not useful)
            if ( param.first == QLatin1String("services/lotw/callbook/tqsl") )
            {
                if ( targetIsFlatpak )
                {
                    qCDebug(runtime) << "Skipping TQSL Path - target is Flatpak (fixed path)";
                    continue;
                }
            }

            PlatformParameter p;
            p.key = param.first;
            p.displayName = param.second;
            p.requiresChange = platformDiffers;
            p.newValue = QString();

            // Read the value from the imported database
            QSqlQuery query(importDb);
            if ( query.prepare("SELECT value FROM log_param WHERE name = :name") )
            {
                query.bindValue(":name", p.key);
                if ( query.exec() && query.first() )
                {
                    // If source is Flatpak, the path /app/bin/tqsl is not useful
                    // for other platforms, so show empty
                    if ( param.first == QLatin1String("services/lotw/callbook/tqsl") && sourceIsFlatpak )
                        p.currentValue = QObject::tr("(Flatpak internal path - not applicable)");
                    else
                        p.currentValue = query.value(0).toString();
                }
            }

            result.append(p);
        }

        importDb.close();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return result;
}

void PlatformParameterManager::applyParameters(const QList<PlatformParameter> &params)
{
    FCT_IDENTIFICATION;

    for ( const PlatformParameter &p : params )
    {
        // Only apply if user provided a new value
        if ( !p.newValue.isEmpty() )
        {
            qCDebug(runtime) << "Applying parameter:" << p.key << "=" << p.newValue;

            // Use LogParam to set the value in the current database
            if ( p.key == "services/lotw/callbook/tqsl" )
                LogParam::setLoTWTQSLPath(p.newValue);
            // Add more parameter handlers here as needed
        }
    }
}

QString PlatformParameterManager::pendingParametersPath()
{
    return LogDatabase::dbDirectory().filePath("qlog.db.pending.params");
}

QList<ProfilePortParameter> PlatformParameterManager::getProfilePortParameters(const QString &importDbPath)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << importDbPath;

    QList<ProfilePortParameter> result;

    const QString connectionName = QStringLiteral("ProfilePortParamImport");

    {
        QSqlDatabase importDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        importDb.setDatabaseName(importDbPath);

        if ( !importDb.open() )
        {
            qWarning() << "Cannot open import database:" << importDb.lastError().text();
            QSqlDatabase::removeDatabase(connectionName);
            return result;
        }

        // Define profile tables and their port columns
        struct ProfilePortDef
        {
            QString tableName;
            QString columnName;
            QString displayPrefix;
        };

        QList<ProfilePortDef> profileDefs = {
            { "rig_profiles", "port_pathname", QObject::tr("Rig") },
            { "rig_profiles", "ptt_port_pathname", QObject::tr("Rig PTT") },
            { "rot_profiles", "port_pathname", QObject::tr("Rotator") },
            { "cwkey_profiles", "port_pathname", QObject::tr("CW Keyer") }
        };

        for ( const ProfilePortDef &def : profileDefs )
        {
            QSqlQuery query(importDb);
            QString sql = QString("SELECT profile_name, %1 FROM %2 WHERE %1 IS NOT NULL AND %1 != ''")
                              .arg(def.columnName, def.tableName);

            if ( !query.exec(sql) )
            {
                qCDebug(runtime) << "Query failed for" << def.tableName << ":" << query.lastError().text();
                continue;
            }

            while ( query.next() )
            {
                ProfilePortParameter p;
                p.tableName = def.tableName;
                p.profileName = query.value(0).toString();
                p.columnName = def.columnName;
                p.currentValue = query.value(1).toString();
                p.displayName = QString("%1: %2").arg(def.displayPrefix, p.profileName);
                p.newValue = QString();
                result.append(p);
            }
        }

        importDb.close();
    }

    QSqlDatabase::removeDatabase(connectionName);

    qCDebug(runtime) << "Found" << result.size() << "profile port parameters";
    return result;
}

void PlatformParameterManager::applyProfilePortParameters(const QList<ProfilePortParameter> &params)
{
    FCT_IDENTIFICATION;

    for ( const ProfilePortParameter &p : params )
    {
        if ( p.newValue.isEmpty() )
            continue;

        qCDebug(runtime) << "Applying profile port parameter:" << p.tableName
                         << p.profileName << p.columnName << "=" << p.newValue;

        QSqlQuery query;
        QString sql = QString("UPDATE %1 SET %2 = :value WHERE profile_name = :profile")
                          .arg(p.tableName, p.columnName);

        if ( !query.prepare(sql) )
        {
            qWarning() << "Cannot prepare update query:" << query.lastError().text();
            continue;
        }

        query.bindValue(":value", p.newValue);
        query.bindValue(":profile", p.profileName);

        if ( !query.exec() )
            qWarning() << "Cannot update profile port:" << query.lastError().text();
    }
}

bool PlatformParameterManager::saveParametersToFile(const QList<PlatformParameter> &params,
                                                     const QList<ProfilePortParameter> &profileParams,
                                                     const QString &filePath)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << filePath;

    QJsonObject root;

    // Save log_param parameters
    QJsonArray paramArray;
    for ( const PlatformParameter &p : params )
    {
        if ( !p.newValue.isEmpty() )
        {
            QJsonObject obj;
            obj["key"] = p.key;
            obj["newValue"] = p.newValue;
            paramArray.append(obj);
        }
    }
    root["parameters"] = paramArray;

    // Save profile port parameters
    QJsonArray profileArray;
    for ( const ProfilePortParameter &p : profileParams )
    {
        if ( !p.newValue.isEmpty() )
        {
            QJsonObject obj;
            obj["tableName"] = p.tableName;
            obj["profileName"] = p.profileName;
            obj["columnName"] = p.columnName;
            obj["newValue"] = p.newValue;
            profileArray.append(obj);
        }
    }
    root["profilePorts"] = profileArray;

    if ( paramArray.isEmpty() && profileArray.isEmpty() )
    {
        qCDebug(runtime) << "No parameters to save";
        return true;
    }

    QJsonDocument doc(root);
    QFile file(filePath);

    if ( !file.open(QIODevice::WriteOnly) )
    {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();

    qCDebug(runtime) << "Saved" << paramArray.size() << "params and"
                     << profileArray.size() << "profile ports to" << filePath;
    return true;
}

QList<PlatformParameter> PlatformParameterManager::loadParametersFromFile(const QString &filePath)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << filePath;

    QList<PlatformParameter> result;

    QFile file(filePath);
    if ( !file.exists() )
    {
        qCDebug(runtime) << "No parameters file found";
        return result;
    }

    if ( !file.open(QIODevice::ReadOnly) )
    {
        qWarning() << "Cannot open parameters file:" << filePath;
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if ( error.error != QJsonParseError::NoError )
    {
        qWarning() << "JSON parse error:" << error.errorString();
        return result;
    }

    const QJsonObject root = doc.object();
    const QJsonArray array = root["parameters"].toArray();

    for ( const QJsonValue &val : array )
    {
        QJsonObject obj = val.toObject();
        PlatformParameter p;
        p.key = obj["key"].toString();
        p.newValue = obj["newValue"].toString();
        p.requiresChange = false;
        result.append(p);
    }

    qCDebug(runtime) << "Loaded" << result.size() << "parameters from" << filePath;
    return result;
}

QList<ProfilePortParameter> PlatformParameterManager::loadProfilePortParametersFromFile(const QString &filePath)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << filePath;

    QList<ProfilePortParameter> result;

    QFile file(filePath);
    if ( !file.exists() )
    {
        qCDebug(runtime) << "No parameters file found";
        return result;
    }

    if ( !file.open(QIODevice::ReadOnly) )
    {
        qWarning() << "Cannot open parameters file:" << filePath;
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if ( error.error != QJsonParseError::NoError )
    {
        qWarning() << "JSON parse error:" << error.errorString();
        return result;
    }

    const QJsonObject root = doc.object();
    const QJsonArray array = root["profilePorts"].toArray();

    for ( const QJsonValue &val : array )
    {
        QJsonObject obj = val.toObject();
        ProfilePortParameter p;
        p.tableName = obj["tableName"].toString();
        p.profileName = obj["profileName"].toString();
        p.columnName = obj["columnName"].toString();
        p.newValue = obj["newValue"].toString();
        result.append(p);
    }

    qCDebug(runtime) << "Loaded" << result.size() << "profile port parameters from" << filePath;
    return result;
}
