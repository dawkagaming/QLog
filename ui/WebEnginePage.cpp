#include <QCoreApplication>
#include <QWebEngineProfile>
#include "WebEnginePage.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.webenginepage");

WebEnginePage::WebEnginePage(QObject *parent)
    : QWebEnginePage{parent}
{
    FCT_IDENTIFICATION;

    QString userAgent = QString("%1/%2 (+https://github.com/foldynl/QLog)")
                            .arg(QCoreApplication::applicationName(), VERSION);
    profile()->setHttpUserAgent(userAgent);
}

void WebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                             const QString &message,
                                             int lineNumber,
                                             const QString &sourceID)
{
    FCT_IDENTIFICATION;

    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);

    qCDebug(runtime)<<"level: " << level <<"; message: "<<message;
}
