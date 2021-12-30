#ifndef CONFIGOBJECT_H
#define CONFIGOBJECT_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QMap>
#include "configdialog.h"

class ConfigObject : public QObject
{
    Q_OBJECT
public:
    explicit ConfigObject(QObject *parent = nullptr);
    ~ConfigObject() override;  // Can delete this after testing
    void open_config_dialog();
    void key_value_changed(int index);
    void store_new_key_value();
    void delete_key_value();
    void debug_display_map();
    void write_map_to_disk();
    QString get_value_from_key(QString &s);

private:
    void get_value_from_key();

private:
    void process_line(QString s);
    // QStringList paramList;    // Used to store contents of config file line by line
    QFile file;
    ConfigDialog *pDialog;
    QMap<QString, QString> configMap;
    QString current_key;
    QString current_value;
};

#endif // CONFIGOBJECT_H
