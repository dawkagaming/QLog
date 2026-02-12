#include <QStandardPaths>
#include <QThread>
#include <QTcpSocket>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>

#include "RigctldManager.h"
#include "core/debug.h"
#include "data/SerialPort.h"

MODULE_IDENTIFICATION("qlog.rig.rigctldmanager");

RigctldManager::RigctldManager(QObject *parent)
    : QObject(parent)
{
    FCT_IDENTIFICATION;
}

RigctldManager::~RigctldManager()
{
    FCT_IDENTIFICATION;
    stop();
}

bool RigctldManager::start(const RigProfile &profile)
{
    FCT_IDENTIFICATION;

    if ( rigctldProcess && rigctldProcess->state() != QProcess::NotRunning)
    {
        qCWarning(runtime) << "rigctld is already running";
        return false;
    }

    // Find rigctld executable
    QString rigctldPath = profile.rigctldPath;
    if ( rigctldPath.isEmpty() )
    {
        rigctldPath = findRigctldPath();

        if ( rigctldPath.isEmpty() )
        {
            qCWarning(runtime) << "rigctld executable not found";
#ifdef QLOG_FLATPAK
            emit errorOccurred(tr("rigctld executable not found in /app/bin/. This should not happen in Flatpak build."));
#else
            emit errorOccurred(tr("rigctld executable not found. Please install Hamlib or specify the path in Advanced settings."));
#endif
            return false;
        }
    }

    qCDebug(runtime) << "Using rigctld at:" << rigctldPath;

    currentPort = profile.rigctldPort;

    // Create process
    rigctldProcess = new QProcess(this);
    rigctldProcess->setProcessChannelMode(QProcess::SeparateChannels);

    connect(rigctldProcess, &QProcess::started, this, &RigctldManager::onProcessStarted);
    connect(rigctldProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &RigctldManager::onProcessFinished);
    connect(rigctldProcess, &QProcess::errorOccurred, this, &RigctldManager::onProcessError);
    connect(rigctldProcess, &QProcess::readyReadStandardOutput, this, &RigctldManager::onReadyReadStdout);
    connect(rigctldProcess, &QProcess::readyReadStandardError, this, &RigctldManager::onReadyReadStderr);

    // exec arguments
    const QStringList args = buildArguments(profile);
    qCDebug(runtime) << "Starting rigctld with args:" << args;

    rigctldProcess->start(rigctldPath, args);

    if ( !rigctldProcess->waitForStarted(5000) )
    {
        qCWarning(runtime) << "Failed to start rigctld";
        emit errorOccurred(tr("Failed to start rigctld process."));
        delete rigctldProcess;
        rigctldProcess = nullptr;
        return false;
    }

    // Wait for rigctld to be ready (accepting connections)
    if ( !waitForRigctldReady() )
    {
        qCWarning(runtime) << "rigctld not responding on port" << currentPort;
        stop();
        emit errorOccurred(tr("rigctld started but not responding on port %1.").arg(currentPort));
        return false;
    }

    qCDebug(runtime) << "rigctld started successfully on port" << currentPort;
    emit started();
    return true;
}

void RigctldManager::stop()
{
    FCT_IDENTIFICATION;

    if ( !rigctldProcess ) return;

    if ( rigctldProcess->state() != QProcess::NotRunning )
    {
        qCDebug(runtime) << "Stopping rigctld";
        rigctldProcess->terminate();

        if ( !rigctldProcess->waitForFinished(3000) )
        {
            qCWarning(runtime) << "rigctld did not terminate gracefully, killing";
            rigctldProcess->kill();
            rigctldProcess->waitForFinished(1000);
        }
    }

    delete rigctldProcess;
    rigctldProcess = nullptr;

    emit stopped();
}

bool RigctldManager::isRunning() const
{
    return rigctldProcess && rigctldProcess->state() == QProcess::Running;
}

QString RigctldManager::findRigctldPath()
{
    FCT_IDENTIFICATION;

    // First priority: Check application directory (bundled rigctld)
#ifdef Q_OS_WIN
    const QString appDirPath = QCoreApplication::applicationDirPath() + "/rigctld.exe";
#else
    const QString appDirPath = QCoreApplication::applicationDirPath() + "/rigctld";
#endif
    if ( QFile::exists(appDirPath) )
    {
        qCDebug(runtime) << "Found bundled rigctld at:" << appDirPath;
        return appDirPath;
    }

    // Second priority: Platform-specific paths
    const QStringList platformPaths =
    {
#ifdef Q_OS_WIN
        "C:/Program Files/Hamlib/bin/rigctld.exe",
        "C:/Program Files (x86)/Hamlib/bin/rigctld.exe",
        QDir::homePath() + "/AppData/Local/Hamlib/bin/rigctld.exe"
#endif
#ifdef Q_OS_MACOS
        "/opt/homebrew/bin/rigctld",
        "/usr/local/bin/rigctld",
        "/opt/local/bin/rigctld"
#endif
#ifdef QLOG_FLATPAK
        "/app/bin/rigctld",  // Flatpak path
#else
        "/usr/bin/rigctld",
        "/usr/local/bin/rigctld",
        "/opt/hamlib/bin/rigctld"
#endif
    };

    for ( const QString &p : platformPaths )
    {
        if ( QFile::exists(p) )
        {
            qCDebug(runtime) << "Found rigctld at:" << p;
            return p;
        }
    }

    // Last resort: Try $PATH
    const QString path = QStandardPaths::findExecutable("rigctld");
    if ( !path.isEmpty() )
    {
        qCDebug(runtime) << "Found rigctld in PATH:" << path;
        return path;
    }

    qCWarning(runtime) << "rigctld not found";
    return QString();
}

bool RigctldManager::waitForRigctldReady(int timeoutMs)
{
    FCT_IDENTIFICATION;

    QTcpSocket testSocket;
    int elapsed = 0;
    const int checkInterval = 100;

    // test connection
    while ( elapsed < timeoutMs )
    {
        testSocket.connectToHost("127.0.0.1", currentPort);
        if ( testSocket.waitForConnected(500) )
        {
            testSocket.disconnectFromHost();
            qCDebug(runtime) << "rigctld is ready after" << elapsed << "ms";
            return true;
        }
        testSocket.abort();

        // Check if process is still running
        if ( rigctldProcess && rigctldProcess->state() != QProcess::Running )
        {
            qCWarning(runtime) << "rigctld process terminated unexpectedly";
            return false;
        }

        QThread::msleep(checkInterval);
        elapsed += checkInterval;
    }

    return false;
}

QStringList RigctldManager::buildArguments(const RigProfile &profile) const
{
    FCT_IDENTIFICATION;

    QStringList args;

    // Model
    args << "-m" << QString::number(profile.model);

    // Listen Port
    args << "-t" << QString::number(profile.rigctldPort);

    // Serial port settings
    if ( profile.getPortType() == RigProfile::SERIAL_ATTACHED )
    {
        args << "-r" << profile.portPath;

        // BandRate
        if ( profile.baudrate > 0 ) args << "-s" << QString::number(profile.baudrate);

        // Data bits
        if ( profile.databits > 0 ) args << "-C" << QString("data_bits=%1").arg(profile.databits);

        // Stop bits
        if ( profile.stopbits > 0 ) args << "-C" << QString("stop_bits=%1").arg(static_cast<int>(profile.stopbits));

        // Parity
        if ( !profile.parity.isEmpty() && profile.parity.compare(SerialPort::SERIAL_PARITY_NO, Qt::CaseInsensitive) )
        {
            QString parity = profile.parity.toLower();
            if      ( parity == SerialPort::SERIAL_PARITY_EVEN ) parity = "E";
            else if ( parity == SerialPort::SERIAL_PARITY_ODD )  parity = "O";
            else parity = "N";
            args << "-C" << QString("serial_parity=%1").arg(parity);
        }

        // Flow control
        if ( !profile.flowcontrol.isEmpty() && profile.flowcontrol.compare(SerialPort::SERIAL_FLOWCONTROL_NONE, Qt::CaseInsensitive) )
        {
            QString flow = profile.flowcontrol.toLower();
            if ( flow == SerialPort::SERIAL_FLOWCONTROL_HARDWARE )      flow = "Hardware";
            else if ( flow == SerialPort::SERIAL_FLOWCONTROL_SOFTWARE ) flow = "XONXOFF";
            args << "-C" << QString("serial_handshake=%1").arg(flow);
        }

        // CIV address for Icom
        if (profile.civAddr >= 0) args << "-C" << QString("civaddr=%1").arg(profile.civAddr, 2, 16, QChar('0'));

        // DTR signal
        if ( !profile.dtr.isEmpty() && profile.dtr.compare(SerialPort::SERIAL_SIGNAL_NONE, Qt::CaseInsensitive) )
        {
            QString dtr = profile.dtr.toLower();
            if ( dtr == SerialPort::SERIAL_SIGNAL_HIGH )     dtr = "ON";
            else if ( dtr == SerialPort::SERIAL_SIGNAL_LOW ) dtr = "OFF";
            args << "-C" << QString("dtr_state=%1").arg(dtr);
        }

        // RTS signal
        if ( !profile.rts.isEmpty() && profile.rts.compare(SerialPort::SERIAL_SIGNAL_NONE, Qt::CaseInsensitive) )
        {
            QString rts = profile.rts.toLower();
            if ( rts == SerialPort::SERIAL_SIGNAL_HIGH )     rts = "ON";
            else if ( rts == SerialPort::SERIAL_SIGNAL_LOW ) rts = "OFF";
            args << "-C" << QString("rts_state=%1").arg(rts);
        }
    }

    // Additional user-specified arguments
    if ( !profile.rigctldArgs.isEmpty() )
    {
        // Split by whitespace, respecting quotes
        QStringList extraArgs = profile.rigctldArgs.split(QRegularExpression("\\s+"), // clazy:exclude=use-static-qregularexpression
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                                                          Qt::SkipEmptyParts);
#else
                                                          QString::SkipEmptyParts);
#endif

        args << extraArgs;
    }

    qCDebug(runtime) << args;

    return args;
}

void RigctldManager::onProcessStarted()
{
    FCT_IDENTIFICATION;
    qCDebug(runtime) << "rigctld process started";
}

void RigctldManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    FCT_IDENTIFICATION;
    qCDebug(runtime) << "rigctld process finished with exit code" << exitCode
                     << "status" << exitStatus;

    if (exitStatus == QProcess::CrashExit)
    {
        emit errorOccurred(tr("rigctld process crashed."));
    }

    emit stopped();
}

void RigctldManager::onProcessError(QProcess::ProcessError error)
{
    FCT_IDENTIFICATION;

    QString errorStr;
    switch ( error )
    {
    case QProcess::FailedToStart:
        errorStr = tr("Failed to start rigctld.");
        break;
    case QProcess::Crashed:
        errorStr = tr("rigctld crashed.");
        break;
    case QProcess::Timedout:
        errorStr = tr("rigctld timed out.");
        break;
    case QProcess::WriteError:
        errorStr = tr("Write error with rigctld.");
        break;
    case QProcess::ReadError:
        errorStr = tr("Read error with rigctld.");
        break;
    default:
        errorStr = tr("Unknown rigctld error.");
        break;
    }

    qCWarning(runtime) << "rigctld process error:" << errorStr;
    emit errorOccurred(errorStr);
}

void RigctldManager::onReadyReadStdout()
{
    if (!rigctldProcess)
        return;

    const QByteArray data = rigctldProcess->readAllStandardOutput();
    if ( !data.isEmpty() )
    {
        // Split by lines and log each
        const QList<QByteArray> lines = data.split('\n');
        for ( const QByteArray &line : lines )
        {
            if ( !line.trimmed().isEmpty() )
                qCDebug(runtime) << "rigctld:" << line.trimmed();
        }
    }
}

void RigctldManager::onReadyReadStderr()
{
    if (!rigctldProcess)
        return;

    const QByteArray data = rigctldProcess->readAllStandardError();
    if ( !data.isEmpty() )
    {
        const QList<QByteArray> lines = data.split('\n');
        for ( const QByteArray &line : lines )
        {
            if ( !line.trimmed().isEmpty() )
                qCWarning(runtime) << "rigctld stderr:" << line.trimmed();
        }
    }
}
