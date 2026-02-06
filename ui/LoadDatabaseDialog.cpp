#include <QFileDialog>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "LoadDatabaseDialog.h"
#include "ui_LoadDatabaseDialog.h"
#include "core/PasswordCipher.h"
#include "core/debug.h"

MODULE_IDENTIFICATION("qlog.ui.loaddatabasedialog");

const QString LoadDatabaseDialog::importConnectionName = QStringLiteral("LoadDatabaseDialogImport");

LoadDatabaseDialog::LoadDatabaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadDatabaseDialog)
{
    FCT_IDENTIFICATION;

    ui->setupUi(this);

    // Add custom Load & Restart button
    loadButton = ui->buttonBox->addButton(tr("Load && Restart"), QDialogButtonBox::AcceptRole);
    loadButton->setEnabled(false);

    // Connect buttonBox accepted signal to our accept() override
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoadDatabaseDialog::accept);

    // Initial state
    ui->decryptGroup->setEnabled(false);
}

LoadDatabaseDialog::~LoadDatabaseDialog()
{
    closeImportDatabase();
    delete ui;
}

QString LoadDatabaseDialog::getSelectedFile() const
{
    return selectedFile;
}

QString LoadDatabaseDialog::getPassword() const
{
    return ui->passwordEdit->text();
}

bool LoadDatabaseDialog::isCrossPlatform() const
{
    if ( !dbInfo.valid )
        return false;

    return dbInfo.sourcePlatform != LogDatabase::currentPlatformId();
}

bool LoadDatabaseDialog::requiresPassword() const
{
    return !encryptedPasswordsBlob.isEmpty();
}

void LoadDatabaseDialog::accept()
{
    FCT_IDENTIFICATION;

    // Verify password before accepting
    if ( !verifyPassword() ) return;

    QDialog::accept();
}

bool LoadDatabaseDialog::verifyPassword()
{
    FCT_IDENTIFICATION;

    // No password needed
    if ( encryptedPasswordsBlob.isEmpty() )
        return true;

    const QString password = ui->passwordEdit->text();

    // Try to decrypt
    QByteArray json;
    if ( !PasswordCipher::decrypt(password, encryptedPasswordsBlob, json) )
    {
        ui->passwordStatusLabel->setText(
            QString("<span style=\"color: red;\">✗ %1</span>").arg(tr("Invalid password")));
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
        return false;
    }

    // Verify JSON structure
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if ( parseError.error != QJsonParseError::NoError )
    {
        ui->passwordStatusLabel->setText(
            QString("<span style=\"color: red;\">✗ %1</span>").arg(tr("Invalid password")));
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
        return false;
    }

    return true;
}

void LoadDatabaseDialog::browseFile()
{
    FCT_IDENTIFICATION;

    const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Select Database File"),
        documentsPath,
        tr("QLog Database Export (*.dbe);;All Files (*)")
    );

    if ( filename.isEmpty() )
        return;

    // Close previous import DB if any
    closeImportDatabase();

    selectedFile = filename;
    ui->fileEdit->setText(filename);

    // Open and validate the database
    openImportDatabase();
}

void LoadDatabaseDialog::openImportDatabase()
{
    FCT_IDENTIFICATION;

    // First get basic info using LogDatabase::inspectDatabase
    dbInfo = LogDatabase::inspectDatabase(selectedFile);

    if ( !dbInfo.valid )
    {
        ui->statusLabel->setText(QString("<span style=\"color: red;\">✗ %1</span>")
                                     .arg(dbInfo.errorMessage));
        ui->decryptGroup->setEnabled(false);
        ui->passwordEdit->clear();
        encryptedPasswordsBlob.clear();
        updateLoadButtonState();
        return;
    }

    // Open the database and cache encrypted passwords blob
    {
        QSqlDatabase importDb = QSqlDatabase::addDatabase("QSQLITE", importConnectionName);
        importDb.setDatabaseName(selectedFile);

        if ( !importDb.open() )
        {
            qWarning() << "Cannot open import database:" << importDb.lastError().text();
            dbInfo.valid = false;
            dbInfo.errorMessage = tr("Cannot open database");
            ui->statusLabel->setText(QString("<span style=\"color: red;\">✗ %1</span>")
                                         .arg(dbInfo.errorMessage));
            ui->decryptGroup->setEnabled(false);
            encryptedPasswordsBlob.clear();
            updateLoadButtonState();
            return;
        }

        // Read encrypted passwords blob
        QSqlQuery query(importDb);
        if ( query.exec("SELECT value FROM log_param WHERE name = 'security/encryptedpasswords'") )
        {
            if ( query.first() && !query.value(0).toString().isEmpty() )
                encryptedPasswordsBlob = QByteArray::fromBase64(query.value(0).toByteArray());
        }
    }

    // Build status message
    QString status = QString("<span style=\"color: green;\">✓ %1</span>")
                         .arg(tr("Valid database"));

    if ( isCrossPlatform() )
    {
        status += QString("<br><span style=\"color: orange;\">⚠ %1 (%2)</span>")
                      .arg(tr("Different platform"), dbInfo.sourcePlatform);
    }

    ui->statusLabel->setText(status);

    // Enable password group if database has encrypted passwords
    if ( !encryptedPasswordsBlob.isEmpty() )
    {
        ui->decryptGroup->setEnabled(true);
        ui->passwordStatusLabel->setText(tr("Password required to import credentials"));
    }
    else
    {
        ui->decryptGroup->setEnabled(false);
        ui->passwordStatusLabel->setText(tr("No encrypted credentials in database"));
    }

    ui->passwordEdit->clear();
    updateLoadButtonState();
}

void LoadDatabaseDialog::closeImportDatabase()
{
    FCT_IDENTIFICATION;

    if ( QSqlDatabase::contains(importConnectionName) )
    {
        QSqlDatabase::database(importConnectionName).close();
        QSqlDatabase::removeDatabase(importConnectionName);
    }

    encryptedPasswordsBlob.clear();
}

void LoadDatabaseDialog::updateLoadButtonState()
{
    FCT_IDENTIFICATION;

    // Reset password status label to default text when user starts typing
    // (but not when password is cleared programmatically after invalid attempt)
    if ( !encryptedPasswordsBlob.isEmpty() && !ui->passwordEdit->text().isEmpty() )
        ui->passwordStatusLabel->setText(tr("Password required to import credentials"));

    // Load & Restart is enabled when:
    // 1. Database is valid
    // 2. Either no password is needed OR password field is not empty
    bool passwordOk = encryptedPasswordsBlob.isEmpty() || !ui->passwordEdit->text().isEmpty();
    loadButton->setEnabled(dbInfo.valid && passwordOk);
}

