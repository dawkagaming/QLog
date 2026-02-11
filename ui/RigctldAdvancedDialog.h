#ifndef QLOG_UI_RIGCTLDADVANCEDDIALOG_H
#define QLOG_UI_RIGCTLDADVANCEDDIALOG_H

#include <QDialog>

namespace Ui {
class RigctldAdvancedDialog;
}

class RigctldAdvancedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RigctldAdvancedDialog(QWidget *parent = nullptr);
    ~RigctldAdvancedDialog();

    void setPath(const QString &path);
    QString getPath() const;

    void setArgs(const QString &args);
    QString getArgs() const;

private slots:
    void autoDetectPath();
    void browsePath();

private:
    Ui::RigctldAdvancedDialog *ui;
};

#endif // QLOG_UI_RIGCTLDADVANCEDDIALOG_H
