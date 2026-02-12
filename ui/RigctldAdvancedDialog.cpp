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
#ifdef QLOG_FLATPAK
    ui->pathEdit->setEnabled(false);
    ui->browseButton->setEnabled(false);
    ui->autoButton->setEnabled(false);
    ui->pathEdit->setPlaceholderText(tr("Cannot be changed"));
#endif
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

    const QString detectedPath = RigctldManager::findRigctldPath();

    if ( detectedPath.isEmpty() )
    {
        QMessageBox::warning(this, tr("Auto Detect"),
                             tr("rigctld was not found on this system.\n"
                                "Please install Hamlib or specify the path manually."));
    }
    else
        ui->pathEdit->setText(detectedPath);
}

void RigctldAdvancedDialog::browsePath()
{
    FCT_IDENTIFICATION;

#ifdef Q_OS_WIN
    const QString filter = tr("Executable (*.exe);;All files (*.*)");
#else
    const QString filter = tr("All files (*)");
#endif
    const QString folder = (ui->pathEdit->text().isEmpty() ) ? QDir::rootPath()
                                                             : ui->pathEdit->text();

    const QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select rigctld executable"),
                                                folder,
                                                filter);
    if ( !path.isEmpty() )
    {
        ui->pathEdit->setText(path);
    }
}
