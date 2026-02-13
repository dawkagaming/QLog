#ifndef QLOG_UI_QSLGALLERYDIALOG_H
#define QLOG_UI_QSLGALLERYDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QTemporaryDir>
#include <QListWidget>

#include "core/QSLStorage.h"
#include "core/LogLocale.h"

namespace Ui {
class QSLGalleryDialog;
}

class QSLGalleryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QSLGalleryDialog(QWidget *parent = nullptr);
    ~QSLGalleryDialog();

    // Exposed for delegate access
    enum ItemDataRole
    {
        ContactIdRole = Qt::UserRole,
        SourceRole = Qt::UserRole + 1,
        NameRole = Qt::UserRole + 2,
        ThumbnailLoadedRole = Qt::UserRole + 3,
        CallsignRole = Qt::UserRole + 4,
        DateStringRole = Qt::UserRole + 5
    };

private slots:
    void filterTreeSelectionChanged();
    void cardDoubleClicked(QListWidgetItem *item);
    void showContextMenu(const QPoint &pos);
    void loadVisibleThumbnails();

private:
    enum FilterType
    {
        FILTER_ALL = 0,
        FILTER_COUNTRY,
        FILTER_YEAR
    };

    void buildFilterTree();
    void loadGallery();
    void populateItems(const QList<QSLGalleryItem> &items);
    QPixmap createThumbnail(const QByteArray &data, const QString &name) const;
    void openItem(QListWidgetItem *item);
    void saveItem(QListWidgetItem *item);

    Ui::QSLGalleryDialog *ui;
    QSLStorage qslStorage;
    QTimer *scrollTimer;
    QTemporaryDir *tempDir;
    LogLocale locale;
};

#endif // QLOG_UI_QSLGALLERYDIALOG_H
