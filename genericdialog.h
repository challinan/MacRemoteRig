#ifndef GENERICDIALOG_H
#define GENERICDIALOG_H
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QApplication>
#include "hamlibconnector.h"

class GenericDialog : public QDialog
{
    Q_OBJECT

public:
    GenericDialog(QWidget *parent);
    ~GenericDialog();

private:
    QDialog *pd;
    QLineEdit *le;
    QLabel *pLabel;
    QDialogButtonBox *pbox;
    QObject *p_parent;

private slots:
     void pb_ok_clicked(QAbstractButton *button);
};

#endif // GENERICDIALOG_H
