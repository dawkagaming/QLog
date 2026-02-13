#ifndef QLOG_CORE_QSLSTORAGE_H
#define QLOG_CORE_QSLSTORAGE_H

#include <QObject>
#include <QSqlRecord>
#include <QVariant>
#include <QList>
#include <QDateTime>

class QSLObject
{
public:
    enum SourceType
    {
        QSLFILE = 0,
        EQSL = 1,
    };

    enum BLOBFormat
    {
        BASE64FORM,
        RAWBYTES
    };

    explicit QSLObject( const qulonglong &qsoID,
                        const SourceType source,
                        const QString &qslName,
                        const QByteArray &inBlob,
                        const BLOBFormat format) :
        qsoID(qsoID),
        source(source),
        qslName(qslName),
        blob((format == RAWBYTES) ? inBlob : QByteArray::fromBase64(inBlob))
        {};

    explicit QSLObject( const QSqlRecord &qso,
                        const SourceType source,
                        const QString &qslName,
                        const QByteArray &inBlob,
                        const BLOBFormat format) :
        QSLObject(qso.value("id").toULongLong(),
                  source, qslName, inBlob, format)
    {}

    qulonglong getQSOID() const {return qsoID;};
    QSLObject::SourceType getSource() const {return source;};
    QString getQSLName() const {return qslName;};
    QByteArray getBLOB(BLOBFormat format = RAWBYTES) const {return (format == BASE64FORM) ? blob.toBase64() : blob;};

private:
    qulonglong qsoID;
    SourceType source;
    QString qslName;
    QByteArray blob;
};

struct QSLGalleryItem
{
    qulonglong contactId;
    QSLObject::SourceType source;
    QString name;
    QString callsign;
    QDateTime startTime;
    QString country;
};

class QSLStorage : public QObject
{
    Q_OBJECT

public:
    explicit QSLStorage(QObject *parent = nullptr);

    bool add(const QSLObject &);
    bool remove(const QSqlRecord &qso,
                const QSLObject::SourceType source,
                const QString &qslName);
    QStringList getAvailableQSLNames(const QSqlRecord &qso,
                                     const QSLObject::SourceType sourceFilter) const;
    QSLObject getQSL(const QSqlRecord &qso,
                     const QSLObject::SourceType source,
                     const QString &qslName) const;

    QStringList getDistinctCountries() const;
    QStringList getDistinctYears() const;

    QList<QSLGalleryItem> getGalleryItems() const;
    QList<QSLGalleryItem> getGalleryItemsByCountry(const QString &country) const;
    QList<QSLGalleryItem> getGalleryItemsByYear(const QString &year) const;

    QByteArray getQSLData(qulonglong contactId, int source, const QString &name) const;

private:
    const QString galleryBaseSQL=
        "SELECT q.contactid, q.source, q.name, c.callsign, c.start_time, translate_to_locale(c.country) "
        "FROM contacts_qsl_cards q "
        "JOIN contacts c ON q.contactid = c.id ";

};

#endif // QLOG_CORE_QSLSTORAGE_H
