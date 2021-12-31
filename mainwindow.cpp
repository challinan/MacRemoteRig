#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hamlibconnector.h"
// for debug only
#include <QFormLayout>

// #define SKIP_RIG_INIT

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialized our config object
    // my_config_p = new ConfigObject;

    /* Initialize the rig */
    // HamlibConnector hamlibc;
#ifndef SKIP_RIG_INIT
    hamlib_p = new HamlibConnector;
    hamlib_p->store_ui_pointer(ui);

    /* Update our display window with the current VFO freq */
    // QString s = hamlib_p->getFrequency();
    freq_t f = hamlib_p->getFrequency();
    qDebug() << "GetFrequency returned: " << f;
    QString str_tmp = HamlibConnector::get_display_frequency(f);

    /* Initialize the freq window */
    ui->freqDisplay->setDigitCount(8);
    ui->freqDisplay->setSmallDecimalPoint(1);
    ui->freqDisplay->display(str_tmp);
#endif

    // Initialize S-Meter
    // See comments at http://hamlib.sourceforge.net/manuals/4.3/group__rig.html about rig_get_level
    ui->smeterProgressBar->setMinimum(0);
    ui->smeterProgressBar->setMaximum(15);
    ui->smeterProgressBar->reset();     // Reset to zero

    freq_polling_active = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

HamlibConnector *MainWindow::getHamlibPointer() {
    return hamlib_p;
}

void MainWindow::on_poll_freq_pbutton_clicked() {
    if ( freq_polling_active ) {
        delete freq_poller;
        freq_polling_active = 0;
    }
    else {
        freq_poller = new FrequencyPoller(this);
        freq_polling_active = 1;
    }
}

void MainWindow::on_a_2_b_pbutton_clicked()
{
    qDebug() << "on_a_2_b_pbutton_clicked unimplemented";
    // For debug only
    QDialog *pd = new QDialog();
    pd->setMinimumSize(QSize(400, 200));
    f_edit = new QLineEdit(pd);
    f_edit->setGeometry(100, 20, 200, 20);
    lcd_p = new QLCDNumber(pd);
    lcd_p->setGeometry(100, 100, 200, 50);
    lcd_p->setDigitCount(8);
    lcd_p->setSmallDecimalPoint(1);

    connect(f_edit, &QLineEdit::editingFinished, this, &MainWindow::on_editingFinished);
    pd->show();
    pd->exec();
}

// For debug only
void MainWindow::on_editingFinished() {
    QString str = f_edit->text();
    freq_t f = str.toDouble();
    qDebug() << "Editing done" << f;
    int fill=0;
    QString freq_display = QString(str).arg(f, fill, 'f', 5);
    // qsizetype QString::indexOf(QChar ch, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    qsizetype dot_index = freq_display.indexOf(QChar('.'));
    QString fd = freq_display.left(dot_index+3);    // Print the decimal point + 2 digits after
    lcd_p->display(fd);

}

void MainWindow::on_quit_pbutton_clicked() {
    qDebug() << "on_quit_pbutton_clicked() called";
    QApplication::quit();
}

void MainWindow::on_a_b_vfo_pbutton_clicked()
{

}

void MainWindow::on_afx_pbutton_clicked()
{
    qDebug() << "on_afx_pbutton_clicked not implemented";
}

void MainWindow::on_agc_pbutton_clicked()
{

}

void MainWindow::on_bset_pbutton_clicked()
{

}

void MainWindow::on_cwt_pbutton_clicked()
{

}

void MainWindow::on_nb_pbutton_clicked()
{

}

void MainWindow::on_nr_pbutton_clicked()
{

}

void MainWindow::on_ntch_pbutton_clicked()
{

}

void MainWindow::on_pre_pbutton_clicked()
{

}

void MainWindow::on_rev_vfo_pbutton_clicked()
{

}

void MainWindow::on_split_pbutton_clicked()
{

}

void MainWindow::on_spot_pbutton_clicked()
{
    qDebug() << "on_spot_pbutton_clicked() not implemented";
}

void MainWindow::on_xfil_pbutton_clicked()
{
    qDebug() << "on_xfil_pbutton_clicked() not implemented";
}

void MainWindow::on_config_pbutton_clicked()
{
    configobj_p = new ConfigObject;
}
