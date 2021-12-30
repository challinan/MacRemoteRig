#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hamlibconnector.h"

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
    QString s = hamlib_p->getFrequency();
    qDebug() << "GetFrequency returned QString: " << s;

    /* Populate the freq window */
    ui->freqDisplay->setDigitCount(8);
    ui->freqDisplay->setSmallDecimalPoint(1);
    ui->freqDisplay->display(s);
#endif

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
