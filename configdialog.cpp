#include "configdialog.h"
#include "ui_configdialog.h"
#include "config_object.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    qDebug() << "ConfigDialog constructor entered - parent pointer = " << parent;
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::set_conf_obj_pointer(QObject *p) {
    config_obj_p = p;
}

void ConfigDialog::on_config_buttonBox_clicked()
{
    // This is the "Cancel" button in the config dialog
    qDebug() << "on_config_buttonBox_clicked() - exiting";
    ConfigObject *cop = static_cast<ConfigObject *>(config_obj_p);
    cop->debug_display_map();
}

void ConfigDialog::on_config_LineEdit_textEdited(const QString &arg1)
{
    // Generated when any text is edited in the line edit widget
    qDebug() << "on_config_LineEdit_textEdited(const QString &arg1) entered: " << arg1;
}


void ConfigDialog::on_config_ComboBox_currentIndexChanged(int index)
{
    // Selection has been made in the config dialog "Key" Combo Box
    ConfigObject *cop = static_cast<ConfigObject *>(config_obj_p);
    cop->key_value_changed(index);
}


void ConfigDialog::on_config_buttonBox_accepted()
{
    // Store the new value to this map (key) entry
    qDebug() << "User pressed OK";
    ConfigObject *cop = static_cast<ConfigObject *>(config_obj_p);
    cop->store_new_key_value();
}


void ConfigDialog::on_delete_key_pbutton_clicked()
{
    ConfigObject *cop = static_cast<ConfigObject *>(config_obj_p);
    cop->delete_key_value();
}
