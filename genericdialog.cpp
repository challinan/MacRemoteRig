#include "genericdialog.h"
#include "mainwindow.h"
#include "hamlib/rig.h"
#include <QDebug>

typedef  void (QDialogButtonBox::*QDBB_dMemFn)(QAbstractButton *);

GenericDialog::GenericDialog(QWidget *parent)

{
    p_parent = parent;
    pd = new QDialog(parent);
    pd->setModal(true);
    pd->setMinimumSize(QSize(300, 100));
    le = new QLineEdit(pd);
    le->setGeometry(10, 40, 100, 20);
    pLabel = new QLabel("Enter Frequency in MHz", pd);
    pbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, pd);
    pbox->setGeometry(50, 70, 150, 60);
    // QPushButton *QDialogButtonBox::button(QDialogButtonBox::StandardButton which) const
    QPushButton *pb_ok = pbox->button(QDialogButtonBox::Ok );
    if ( !pb_ok ) {
        qDebug() << "MainWindow::on_manual_pButton_clicked(): failed to get button pointer - exiting";
        QApplication::quit();
    }
    QDBB_dMemFn p = &QDialogButtonBox::clicked;
    connect(pbox, p, this, &GenericDialog::pb_ok_clicked);
    // pd->show();
    pd->exec();
}

void GenericDialog::pb_ok_clicked(QAbstractButton *button) {
    QString button_text = button->text();
    qDebug() << "GenericDialog::pb_ok_clicked(): slot entered" << button << "text = " << button_text;

    if ( button_text == "Cancel" ) {
        qDebug() << "Cancel pushbutton pressed";
    }
    if ( button_text == "Ok" ) {
        qDebug() << "GenericDialog::pb_ok_clicked(): OK Pushbutton pressed";
    }
    QString freq_text = le->text();
    qDebug() << "GenericDialog::pb_ok_clicked(): frequency entered = " << freq_text;
    // double QString::toDouble(bool *ok = nullptr) const
    bool ok;
    freq_t f_new = freq_text.toDouble(&ok);
    if ( ok ) {
        f_new = f_new * 1000000.0;
        qDebug() << "GenericDialog::pb_ok_clicked(): new frequency as double:" << f_new << " result = " << ok;

        MainWindow *pMain = dynamic_cast<MainWindow *>(p_parent);
        HamlibConnector *hlp = pMain->getHamlibPointer();
        hlp->set_rig_freq(f_new);
        hlp->autoupdate_frequency();
    }

    pd->close();
}

GenericDialog::~GenericDialog() {
    delete pbox; delete pLabel; delete le; delete pd;
}
