#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

#include "QSLStorage.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.core.qslstorage");

QSLStorage::QSLStorage(QObject *parent) : QObject(parent)
{
    FCT_IDENTIFICATION;
}

bool QSLStorage::add(const QSLObject &qslObject)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << qslObject.getQSOID()
                                 << qslObject.getSource()
                                 << qslObject.getQSLName();
    QSqlQuery insert;

    if ( !insert.prepare("REPLACE INTO contacts_qsl_cards (contactid, source, name, data) "
                         " VALUES (:contactid, :source, :name, :data)" ) )
    {
        qCDebug(runtime) << " Cannot prepare INSERT for PaperQSL " << insert.lastError();
        return false;
    }

    insert.bindValue(":contactid", qslObject.getQSOID());
    insert.bindValue(":source", qslObject.getSource());
    insert.bindValue(":name", qslObject.getQSLName());
    insert.bindValue(":data", qslObject.getBLOB(QSLObject::BASE64FORM));

    if ( !insert.exec() )
    {
        qCDebug(runtime) << "Cannot import QSL" << insert.lastError();
        return false;
    }
    return true;
}

bool QSLStorage::remove(const QSqlRecord &qso,
                        const QSLObject::SourceType source,
                        const QString &qslName)
{
    FCT_IDENTIFICATION;

    QSqlQuery query;

    if ( !query.prepare("DELETE FROM contacts_qsl_cards "
                        "WHERE source = :source "
                        "AND contactid = :contactid "
                        "AND name = :qsl_name"))
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement";
        return false;
    }

    query.bindValue(":source", source);
    query.bindValue(":contactid", qso.value("id"));
    query.bindValue(":qsl_name", qslName);

    if ( !query.exec() )
    {
        qCDebug(runtime) << "Cannot delete QSL file" << qslName;
        return false;
    }

    return true;
}

QStringList QSLStorage::getAvailableQSLNames(const QSqlRecord &qso,
                                             const QSLObject::SourceType sourceFilter) const
{
    FCT_IDENTIFICATION;

    QStringList ret;
    QSqlQuery query;

    if ( !query.prepare("SELECT name FROM contacts_qsl_cards "
                        "WHERE source = :source "
                        "AND contactid = :contactid "
                        "ORDER BY name"))
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement";
        return ret;
    }

    query.bindValue(":source", sourceFilter);
    query.bindValue(":contactid", qso.value("id"));

    if ( query.exec() )
    {
        while(query.next())
        {
            ret << query.value(0).toString();
        }
    }
    else
    {
        qCDebug(runtime) << "Error" << query.lastError();
    }
    return ret;
}

QSLObject QSLStorage::getQSL(const QSqlRecord &qso,
                             const QSLObject::SourceType source,
                             const QString &qslName) const
{
    FCT_IDENTIFICATION;

    QSqlQuery query;

    if ( !query.prepare("SELECT data FROM contacts_qsl_cards "
                        "WHERE source = :source "
                        "AND contactid = :contactid "
                        "AND name = :qsl_name "
                        "ORDER BY name LIMIT 1"))
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement";
    }
    else
    {
        query.bindValue(":source", source);
        query.bindValue(":contactid", qso.value("id"));
        query.bindValue(":qsl_name", qslName);

        if ( query.exec() && query.next() )
            return QSLObject(qso, source, qslName, query.value(0).toByteArray(), QSLObject::BASE64FORM);
    }

    return QSLObject (qso, source, qslName, QByteArray(), QSLObject::RAWBYTES);
}

QStringList QSLStorage::getDistinctCountries() const
{
    FCT_IDENTIFICATION;

    QStringList ret;
    QSqlQuery query;

    if ( !query.prepare("SELECT DISTINCT c.country FROM contacts_qsl_cards q "
                        "JOIN contacts c ON q.contactid = c.id "
                        "WHERE c.country IS NOT NULL AND c.country != '' "
                        "ORDER BY c.country COLLATE LOCALEAWARE ASC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return ret;
    }

    if ( query.exec() )
    {
        while ( query.next() )
            ret << query.value(0).toString();
    }
    else
    {
        qCDebug(runtime) << "Error" << query.lastError();
    }

    return ret;
}

QStringList QSLStorage::getDistinctYears() const
{
    FCT_IDENTIFICATION;

    QStringList ret;
    QSqlQuery query;

    if ( !query.prepare("SELECT DISTINCT strftime('%Y', c.start_time) AS year "
                        "FROM contacts_qsl_cards q "
                        "JOIN contacts c ON q.contactid = c.id "
                        "WHERE c.start_time IS NOT NULL "
                        "ORDER BY year DESC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return ret;
    }

    if ( query.exec() )
    {
        while ( query.next() )
            ret << query.value(0).toString();
    }
    else
    {
        qCDebug(runtime) << "Error" << query.lastError();
    }

    return ret;
}

static QList<QSLGalleryItem> executeGalleryQuery(QSqlQuery &query)
{
    QList<QSLGalleryItem> ret;

    if ( query.exec() )
    {
        while ( query.next() )
        {
            QSLGalleryItem item;
            item.contactId = query.value(0).toULongLong();
            item.source = static_cast<QSLObject::SourceType>(query.value(1).toInt());
            item.name = query.value(2).toString();
            item.callsign = query.value(3).toString();
            item.startTime = query.value(4).toDateTime();
            item.country = query.value(5).toString();
            item.favorite = query.value(6).toBool();
            ret << item;
        }
    }

    return ret;
}

QList<QSLGalleryItem> QSLStorage::getGalleryItems() const
{
    FCT_IDENTIFICATION;

    QSqlQuery query;

    if ( !query.prepare(galleryBaseSQL + "ORDER BY c.start_time DESC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return QList<QSLGalleryItem>();
    }

    QList<QSLGalleryItem> ret = executeGalleryQuery(query);

    if ( ret.isEmpty() )
        qCDebug(runtime) << "No gallery items found or error" << query.lastError();

    return ret;
}

QList<QSLGalleryItem> QSLStorage::getGalleryItemsByCountry(const QString &country) const
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << country;

    QSqlQuery query;

    if ( !query.prepare(galleryBaseSQL + "WHERE c.country = :country ORDER BY c.start_time DESC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return QList<QSLGalleryItem>();
    }

    query.bindValue(":country", country);

    QList<QSLGalleryItem> ret = executeGalleryQuery(query);

    if ( ret.isEmpty() )
        qCDebug(runtime) << "No gallery items for country" << country << query.lastError();

    return ret;
}

QList<QSLGalleryItem> QSLStorage::getGalleryItemsByYear(const QString &year) const
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << year;

    QSqlQuery query;

    if ( !query.prepare(galleryBaseSQL + "WHERE strftime('%Y', c.start_time) = :year ORDER BY c.start_time DESC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return QList<QSLGalleryItem>();
    }

    query.bindValue(":year", year);

    QList<QSLGalleryItem> ret = executeGalleryQuery(query);

    if ( ret.isEmpty() )
        qCDebug(runtime) << "No gallery items for year" << year << query.lastError();

    return ret;
}

QByteArray QSLStorage::getQSLData(qulonglong contactId, int source, const QString &name) const
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << contactId << source << name;

    QSqlQuery query;

    if ( !query.prepare("SELECT data FROM contacts_qsl_cards "
                        "WHERE contactid = :contactid AND source = :source AND name = :name "
                        "LIMIT 1") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return QByteArray();
    }

    query.bindValue(":contactid", static_cast<quint64>(contactId));
    query.bindValue(":source", source);
    query.bindValue(":name", name);

    if ( query.exec() && query.next() )
        return QByteArray::fromBase64(query.value(0).toByteArray());

    qCDebug(runtime) << "QSL data not found" << query.lastError();
    return QByteArray();
}

QList<QSLGalleryItem> QSLStorage::getGalleryItemsFavorite() const
{
    FCT_IDENTIFICATION;

    QSqlQuery query;

    if ( !query.prepare(galleryBaseSQL + "WHERE q.favorite = 1 ORDER BY c.start_time DESC") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return QList<QSLGalleryItem>();
    }

    QList<QSLGalleryItem> ret = executeGalleryQuery(query);

    if ( ret.isEmpty() )
        qCDebug(runtime) << "No favorite gallery items found or error" << query.lastError();

    return ret;
}

bool QSLStorage::setFavorite(qulonglong contactId, QSLObject::SourceType source, const QString &name, bool favorite)
{
    FCT_IDENTIFICATION;

    qCDebug(function_parameters) << contactId << source << name << favorite;

    QSqlQuery query;

    if ( !query.prepare("UPDATE contacts_qsl_cards SET favorite = :fav "
                        "WHERE contactid = :contactid AND source = :source AND name = :name") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return false;
    }

    query.bindValue(":fav", favorite ? 1 : 0);
    query.bindValue(":contactid", static_cast<quint64>(contactId));
    query.bindValue(":source", static_cast<int>(source));
    query.bindValue(":name", name);

    if ( !query.exec() )
    {
        qCDebug(runtime) << "Cannot update favorite" << query.lastError();
        return false;
    }

    return true;
}

bool QSLStorage::isFavorite(qulonglong contactId, QSLObject::SourceType source, const QString &name) const
{
    FCT_IDENTIFICATION;

    QSqlQuery query;

    if ( !query.prepare("SELECT favorite FROM contacts_qsl_cards "
                        "WHERE contactid = :contactid AND source = :source AND name = :name "
                        "LIMIT 1") )
    {
        qCDebug(runtime) << "Cannot prepare SQL Statement" << query.lastError();
        return false;
    }

    query.bindValue(":contactid", static_cast<quint64>(contactId));
    query.bindValue(":source", static_cast<int>(source));
    query.bindValue(":name", name);

    if ( query.exec() && query.next() )
        return query.value(0).toBool();

    return false;
}
