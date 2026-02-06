#ifndef QLOG_CORE_PLATFORMPARAMETERMANAGER_H
#define QLOG_CORE_PLATFORMPARAMETERMANAGER_H

#include <QString>
#include <QList>
#include <QPair>

struct PlatformParameter
{
    QString key;           // LogParam key (e.g. "services/lotw/callbook/tqsl")
    QString displayName;   // Human-readable name (e.g. "TQSL Path")
    QString currentValue;  // Value from imported DB
    QString newValue;      // User-provided value for current platform
    bool requiresChange;   // true if platform differs
};

// Structure for profile-based port paths (rig, rotator, cwkey)
struct ProfilePortParameter
{
    QString tableName;     // e.g. "rig_profiles"
    QString profileName;   // e.g. "My Rig"
    QString columnName;    // e.g. "port_pathname"
    QString displayName;   // e.g. "Rig: My Rig - Serial Port"
    QString currentValue;  // Value from imported DB
    QString newValue;      // User-provided value for current platform
};

class PlatformParameterManager
{
public:
    // Get list of platform-dependent parameters from imported DB
    // importDbPath: path to the imported database file
    // sourcePlatform: platform from which the DB was exported
    // Returns list of parameters that may need adjustment
    static QList<PlatformParameter> getParameters(const QString &importDbPath,
                                                  const QString &sourcePlatform);

    // Apply user-provided values to the current database
    // params: list of parameters with newValue set by user
    static void applyParameters(const QList<PlatformParameter> &params);

    // Get list of profile port parameters from imported DB
    static QList<ProfilePortParameter> getProfilePortParameters(const QString &importDbPath);

    // Apply profile port parameters to the current database
    static void applyProfilePortParameters(const QList<ProfilePortParameter> &params);

    // Save parameters to a JSON file for later application after restart
    static bool saveParametersToFile(const QList<PlatformParameter> &params,
                                     const QList<ProfilePortParameter> &profileParams,
                                     const QString &filePath);

    // Load parameters from JSON file
    static QList<PlatformParameter> loadParametersFromFile(const QString &filePath);

    // Load profile port parameters from JSON file
    static QList<ProfilePortParameter> loadProfilePortParametersFromFile(const QString &filePath);

    // Get path to pending parameters file
    static QString pendingParametersPath();

private:
    // Template list of known platform-dependent parameters
    // Returns list of (key, displayName) pairs
    static QList<QPair<QString, QString>> knownParameters();
};

#endif // QLOG_CORE_PLATFORMPARAMETERMANAGER_H
