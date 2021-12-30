#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QObject>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    // ConfigDialog(QObject *pp);
    ~ConfigDialog() override;

    void set_conf_obj_pointer(QObject *p);

private slots:
    void on_config_buttonBox_clicked();
    void on_config_LineEdit_textEdited(const QString &arg1);
    void on_config_ComboBox_currentIndexChanged(int index);
    void on_config_buttonBox_accepted();
    void on_delete_key_pbutton_clicked();

private:
    QObject *config_obj_p;

public:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
