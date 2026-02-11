#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include "RigctldAdvancedDialog.h"
#include "ui_RigctldAdvancedDialog.h"
#include "rig/RigctldManager.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.rigctldadvanceddialog");

RigctldAdvancedDialog::RigctldAdvancedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RigctldAdvancedDialog)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    connect(ui->autoButton, &QPushButton::clicked, this, &RigctldAdvancedDialog::autoDetectPath);
    connect(ui->browseButton, &QPushButton::clicked, this, &RigctldAdvancedDialog::browsePath);
}

RigctldAdvancedDialog::~RigctldAdvancedDialog()
{
    FCT_IDENTIFICATION;
    delete ui;
}

void RigctldAdvancedDialog::setPath(const QString &path)
{
    ui->pathEdit->setText(path);
}

QString RigctldAdvancedDialog::getPath() const
{
    return ui->pathEdit->text().trimmed();
}

void RigctldAdvancedDialog::setArgs(const QString &args)
{
    ui->argsEdit->setText(args);
}

QString RigctldAdvancedDialog::getArgs() const
{
    return ui->argsEdit->text().trimmed();
}

void RigctldAdvancedDialog::autoDetectPath()
{
    FCT_IDENTIFICATION;

    QString detectedPath = RigctldManager::findRigctldPath();

    if (detectedPath.isEmpty())
    {
        QMessageBox::warning(this, tr("Auto Detect"),
                             tr("rigctld was not found on this system.\n"
                                "Please install Hamlib or specify the path manually."));
    }
    else
    {
        ui->pathEdit->setText(detectedPath);
        QMessageBox::information(this, tr("Auto Detect"),
                                 tr("Found rigctld at:\n%1").arg(detectedPath));
    }
}

void RigctldAdvancedDialog::browsePath()
{
    FCT_IDENTIFICATION;

#ifdef Q_OS_WIN
    QString filter = tr("Executable (*.exe);;All files (*.*)");
#else
    QString filter = tr("All files (*)");
#endif

    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select rigctld executable"),
                                                ui->pathEdit->text(),
                                                filter);
    if (!path.isEmpty())
    {
        ui->pathEdit->setText(path);
    }
}
