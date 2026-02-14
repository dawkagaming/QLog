#include <QScrollBar>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QApplication>
#include <QStyle>
#include <QTimeZone>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QFileDialog>

#include "QSLGalleryDialog.h"
#include "ui_QSLGalleryDialog.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.qslgallerydialog");

class QSLCardDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text.clear();
        opt.icon = QIcon();
        opt.decorationSize = QSize();
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        const QRect rect = option.rect;
        const int iconW = option.decorationSize.width();
        const int iconH = option.decorationSize.height();
        const int textMargin = 4;

        // Draw icon centered horizontally
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        const int iconX = rect.x() + (rect.width() - iconW) / 2;
        const int iconY = rect.y() + textMargin;
        icon.paint(painter, iconX, iconY, iconW, iconH);

        // Text area below icon
        const int textY = iconY + iconH + textMargin;
        const QRect textRect(rect.x() + textMargin, textY,
                             rect.width() - 2 * textMargin, rect.bottom() - textY);

        // Bold callsign
        const QString callsign = index.data(QSLGalleryDialog::CallsignRole).toString();
        QFont boldFont = option.font;
        boldFont.setBold(true);
        painter->setFont(boldFont);
        painter->setPen(option.palette.color(QPalette::Text));

        const QFontMetrics boldFm(boldFont);
        const QString elidedCallsign = boldFm.elidedText(callsign, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, elidedCallsign);

        // Normal date below callsign
        const QString dateStr = index.data(QSLGalleryDialog::DateStringRole).toString();
        QFont normalFont = option.font;
        painter->setFont(normalFont);

        const QRect dateRect(textRect.x(), textY + boldFm.height() + 1,
                             textRect.width(), textRect.height() - boldFm.height());
        const QFontMetrics normalFm(normalFont);
        const QString elidedDate = normalFm.elidedText(dateStr, Qt::ElideRight, dateRect.width());
        painter->drawText(dateRect, Qt::AlignHCenter | Qt::AlignTop, elidedDate);

        // Draw favorite star in the top-right corner of the icon
        if ( index.data(QSLGalleryDialog::FavoriteRole).toBool() )
        {
            QFont starFont = option.font;
            starFont.setPixelSize(14);
            painter->setFont(starFont);

            const QString star = QStringLiteral("\u2605");
            const int starX = iconX + iconW - 14;
            const int starY = iconY + 14;

            // Black outline
            painter->setPen(Qt::black);
            painter->drawText(starX - 1, starY, star);
            painter->drawText(starX + 1, starY, star);
            painter->drawText(starX, starY - 1, star);
            painter->drawText(starX, starY + 1, star);

            // Yellow star
            painter->setPen(QColor(0xFF, 0xD7, 0x00));
            painter->drawText(starX, starY, star);
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &) const override
    {
        const int w = option.decorationSize.width() + 20;
        const int h = option.decorationSize.height() + QFontMetrics(option.font).height() * 2 + 16;
        return QSize(w, h);
    }
};

QSLGalleryDialog::QSLGalleryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSLGalleryDialog),
    scrollTimer(new QTimer(this)),
    tempDir(nullptr)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    // tree 20%, gallery 80%
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 8);

    ui->cardListWidget->setItemDelegate(new QSLCardDelegate(ui->cardListWidget));

    connect(ui->filterTree, &QTreeWidget::currentItemChanged,
            this, [this]() { filterTreeSelectionChanged(); });
    connect(ui->cardListWidget, &QListWidget::itemDoubleClicked,
            this, &QSLGalleryDialog::cardDoubleClicked);

    ui->cardListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->cardListWidget, &QWidget::customContextMenuRequested,
            this, &QSLGalleryDialog::showContextMenu);

    // Lazy loading
    scrollTimer->setSingleShot(true);
    scrollTimer->setInterval(150);
    connect(scrollTimer, &QTimer::timeout,
            this, &QSLGalleryDialog::loadVisibleThumbnails);

    connect(ui->cardListWidget->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this]() { scrollTimer->start(); });

    connect(ui->exportFilteredButton, &QPushButton::clicked,
            this, &QSLGalleryDialog::exportFiltered);

    buildFilterTree();
}

QSLGalleryDialog::~QSLGalleryDialog()
{
    FCT_IDENTIFICATION;

    delete tempDir;
    delete ui;
}

void QSLGalleryDialog::buildFilterTree()
{
    FCT_IDENTIFICATION;

    ui->filterTree->clear();

    // "All" root item
    QTreeWidgetItem *allItem = new QTreeWidgetItem(ui->filterTree);
    allItem->setText(0, tr("All QSL Cards"));
    allItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    allItem->setData(0, Qt::UserRole, FILTER_ALL);

    // "Favorites" item
    QTreeWidgetItem *favItem = new QTreeWidgetItem(ui->filterTree);
    favItem->setText(0, tr("Favorites"));
    favItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    favItem->setData(0, Qt::UserRole, FILTER_FAVORITE);

    // "By Country" branch
    QTreeWidgetItem *countryRoot = new QTreeWidgetItem(ui->filterTree);
    countryRoot->setText(0, tr("By Country"));
    countryRoot->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    countryRoot->setFlags(countryRoot->flags() & ~Qt::ItemIsSelectable);

    const QStringList countries = qslStorage.getDistinctCountries();
    for ( const QString &country : countries )
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(countryRoot);
        item->setText(0, country);
        item->setData(0, Qt::UserRole, FILTER_COUNTRY);
        item->setData(0, Qt::UserRole + 1, country);
    }

    // "By Year" branch
    QTreeWidgetItem *yearRoot = new QTreeWidgetItem(ui->filterTree);
    yearRoot->setText(0, tr("By Year"));
    yearRoot->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    yearRoot->setFlags(yearRoot->flags() & ~Qt::ItemIsSelectable);

    const QStringList years = qslStorage.getDistinctYears();
    for ( const QString &year : years )
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(yearRoot);
        item->setText(0, year);
        item->setData(0, Qt::UserRole, FILTER_YEAR);
        item->setData(0, Qt::UserRole + 1, year);
    }

    // Expand all branches
    ui->filterTree->expandAll();

    // Select "All" by default
    ui->filterTree->setCurrentItem(allItem);
}

void QSLGalleryDialog::filterTreeSelectionChanged()
{
    FCT_IDENTIFICATION;

    loadGallery();
}

void QSLGalleryDialog::loadGallery()
{
    FCT_IDENTIFICATION;

    QTreeWidgetItem *current = ui->filterTree->currentItem();

    if ( !current )
        return;

    const FilterType filter = static_cast<FilterType>(current->data(0, Qt::UserRole).toInt());
    QList<QSLGalleryItem> items;

    switch ( filter )
    {
    case FILTER_ALL:
        items = qslStorage.getGalleryItems();
        break;

    case FILTER_FAVORITE:
        items = qslStorage.getGalleryItemsFavorite();
        break;

    case FILTER_COUNTRY:
    {
        const QString country = current->data(0, Qt::UserRole + 1).toString();
        items = qslStorage.getGalleryItemsByCountry(country);
        break;
    }

    case FILTER_YEAR:
    {
        const QString year = current->data(0, Qt::UserRole + 1).toString();
        items = qslStorage.getGalleryItemsByYear(year);
        break;
    }
    }

    populateItems(items);
}

void QSLGalleryDialog::populateItems(const QList<QSLGalleryItem> &items)
{
    FCT_IDENTIFICATION;

    ui->cardListWidget->clear();

    // Create a placeholder pixmap
    QPixmap placeholder(150, 112);
    placeholder.fill(QColor(220, 220, 220));

    for ( const QSLGalleryItem &item : items )
    {
        const QString dateStr = item.startTime.toTimeZone(QTimeZone::utc())
                                              .toString(locale.formatDateTimeShortWithYYYY());

        QListWidgetItem *listItem = new QListWidgetItem(QIcon(placeholder), QString());
        listItem->setData(ContactIdRole, static_cast<quint64>(item.contactId));
        listItem->setData(SourceRole, static_cast<int>(item.source));
        listItem->setData(NameRole, item.name);
        listItem->setData(ThumbnailLoadedRole, false);
        listItem->setData(CallsignRole, item.callsign);
        listItem->setData(DateStringRole, dateStr);
        listItem->setData(FavoriteRole, item.favorite);

        ui->cardListWidget->addItem(listItem);
    }

    ui->statusLabel->setText(tr("%n QSL card(s)", "", items.count()));

    // initial thumbnail load
    QTimer::singleShot(0, this, &QSLGalleryDialog::loadVisibleThumbnails);
}

void QSLGalleryDialog::loadVisibleThumbnails()
{
    FCT_IDENTIFICATION;

    QListWidget *lw = ui->cardListWidget;
    const int totalCount = lw->count();

    if ( totalCount == 0 )
        return;

    // Find visible range using viewport geometry
    const QRect viewportRect = lw->viewport()->rect();
    const QModelIndex topLeft = lw->indexAt(viewportRect.topLeft());
    const QModelIndex bottomRight = lw->indexAt(viewportRect.bottomRight());

    int firstVisible = topLeft.isValid() ? topLeft.row() : 0;
    int lastVisible = bottomRight.isValid() ? bottomRight.row() : totalCount - 1;

    // If bottomRight is invalid, scan forward to find last visible item
    if ( !bottomRight.isValid() && topLeft.isValid() )
    {
        for ( int i = firstVisible; i < totalCount; ++i )
        {
            const QRect itemRect = lw->visualItemRect(lw->item(i));
            if ( !itemRect.intersects(viewportRect) && i > firstVisible )
            {
                lastVisible = i - 1;
                break;
            }
            lastVisible = i;
        }
    }

    // Add margin for smooth scrolling
    const int margin = 10;
    firstVisible = qMax(0, firstVisible - margin);
    lastVisible = qMin(totalCount - 1, lastVisible + margin);

    qCDebug(runtime) << "Loading thumbnails" << firstVisible << "-" << lastVisible
                     << "of" << totalCount;

    for ( int i = firstVisible; i <= lastVisible; ++i )
    {
        QListWidgetItem *item = lw->item(i);

        if ( item->data(ThumbnailLoadedRole).toBool() )
            continue;

        const qulonglong contactId = item->data(ContactIdRole).toULongLong();
        const int source = item->data(SourceRole).toInt();
        const QString name = item->data(NameRole).toString();

        const QByteArray data = qslStorage.getQSLData(contactId, source, name);

        if ( !data.isEmpty() )
            item->setIcon(QIcon(createThumbnail(data, name)));

        item->setData(ThumbnailLoadedRole, true);
    }
}

QPixmap QSLGalleryDialog::createThumbnail(const QByteArray &data, const QString &name) const
{
    FCT_IDENTIFICATION;

    QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForData(data);

    if ( mimeType.name().startsWith("image/") )
    {
        QPixmap pixmap;
        if ( pixmap.loadFromData(data) )
            return pixmap.scaled(150, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Non-image file: use generic icon
    Q_UNUSED(name);
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon).pixmap(150, 112);
}

void QSLGalleryDialog::cardDoubleClicked(QListWidgetItem *item)
{
    FCT_IDENTIFICATION;

    openItem(item);
}

void QSLGalleryDialog::showContextMenu(const QPoint &pos)
{
    FCT_IDENTIFICATION;

    QListWidgetItem *item = ui->cardListWidget->itemAt(pos);

    if ( !item )
        return;

    QMenu menu(this);

    const bool isFav = item->data(FavoriteRole).toBool();
    QAction *favAction = menu.addAction(isFav ? tr("Remove from Favorites") : tr("Add to Favorites"));
    menu.addSeparator();
    QAction *openAction = menu.addAction(tr("Open"));
    QAction *saveAction = menu.addAction(tr("Save..."));

    QAction *selected = menu.exec(ui->cardListWidget->viewport()->mapToGlobal(pos));

    if ( selected == favAction )
        toggleFavorite(item);
    else if ( selected == openAction )
        openItem(item);
    else if ( selected == saveAction )
        saveItem(item);
}

void QSLGalleryDialog::openItem(QListWidgetItem *item)
{
    FCT_IDENTIFICATION;

    if ( !item )
        return;

    const qulonglong contactId = item->data(ContactIdRole).toULongLong();
    const int source = item->data(SourceRole).toInt();
    const QString name = item->data(NameRole).toString();

    const QByteArray data = qslStorage.getQSLData(contactId, source, name);

    if ( data.isEmpty() )
    {
        qCWarning(runtime) << "No QSL data for" << contactId << source << name;
        return;
    }

    // Create temp dir if needed
    if ( !tempDir )
    {
        tempDir = new QTemporaryDir();
        if ( !tempDir->isValid() )
        {
            qCWarning(runtime) << "Cannot create temporary directory";
            return;
        }
    }

    const QString filePath = tempDir->path() + "/" + name;
    QFile file(filePath);

    if ( !file.open(QIODevice::WriteOnly) )
    {
        qCWarning(runtime) << "Cannot write temporary file" << filePath;
        return;
    }

    file.write(data);
    file.close();

    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void QSLGalleryDialog::saveItem(QListWidgetItem *item)
{
    FCT_IDENTIFICATION;

    if ( !item )
        return;

    const qulonglong contactId = item->data(ContactIdRole).toULongLong();
    const int source = item->data(SourceRole).toInt();
    const QString name = item->data(NameRole).toString();

    const QByteArray data = qslStorage.getQSLData(contactId, source, name);

    if ( data.isEmpty() )
    {
        qCDebug(runtime) << "No QSL data for" << contactId << source << name;
        return;
    }

    const QString savePath = QFileDialog::getSaveFileName(this,
                                                          tr("Save QSL Card"),
                                                          QDir::homePath() + "/" + name);
    if ( savePath.isEmpty() )
        return;

    QFile file(savePath);

    if ( !file.open(QIODevice::WriteOnly) )
    {
        qCDebug(runtime) << "Cannot write file" << savePath;
        return;
    }

    file.write(data);
    file.close();

    qCDebug(runtime) << "QSL card saved to" << savePath;
}

void QSLGalleryDialog::toggleFavorite(QListWidgetItem *item)
{
    FCT_IDENTIFICATION;

    if ( !item )
        return;

    const qulonglong contactId = item->data(ContactIdRole).toULongLong();
    const auto source = static_cast<QSLObject::SourceType>(item->data(SourceRole).toInt());
    const QString name = item->data(NameRole).toString();
    const bool currentFav = item->data(FavoriteRole).toBool();

    if ( qslStorage.setFavorite(contactId, source, name, !currentFav) )
    {
        item->setData(FavoriteRole, !currentFav);
        ui->cardListWidget->update();
    }
}

void QSLGalleryDialog::exportFiltered()
{
    FCT_IDENTIFICATION;

    const int count = ui->cardListWidget->count();

    if ( count == 0 )
        return;

    const QString dir = QFileDialog::getExistingDirectory(this,
                                                           tr("Export QSL Cards"),
                                                           QDir::homePath());
    if ( dir.isEmpty() )
        return;

    int saved = 0;

    for ( int i = 0; i < count; ++i )
    {
        QListWidgetItem *item = ui->cardListWidget->item(i);

        const qulonglong contactId = item->data(ContactIdRole).toULongLong();
        const int source = item->data(SourceRole).toInt();
        const QString name = item->data(NameRole).toString();

        const QByteArray data = qslStorage.getQSLData(contactId, source, name);

        if ( data.isEmpty() )
        {
            qCDebug(runtime) << "No QSL data for" << contactId << source << name;
            continue;
        }

        const QString filePath = dir + QDir::separator() + name;

        QFile file(filePath);

        if ( !file.open(QIODevice::WriteOnly) )
        {
            qCDebug(runtime) << "Cannot write file" << filePath;
            continue;
        }

        file.write(data);
        file.close();
        ++saved;
    }

    ui->statusLabel->setText(tr("Exported %1 of %2 cards").arg(saved).arg(count));
    qCDebug(runtime) << "Exported" << saved << "of" << count << "QSL cards to" << dir;
}
