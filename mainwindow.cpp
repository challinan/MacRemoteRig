#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hamlibconnector.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /* Initialize the rig */
    // HamlibConnector hamlibc;
    hamlib_p = new HamlibConnector;
    hamlib_p->store_ui_pointer(ui);

    /* Update our display window with the current VFO freq */
    QString s = hamlib_p->getFrequency();

    /* Populate the freq window */
    ui->FreqDisplay->setDigitCount(11);
    ui->FreqDisplay->display(s);

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
    qDebug() << "on_a_2_b_pbutton_clicked called";
    QString s = hamlib_p->getFrequency();
    ui->FreqDisplay->display(hamlib_p->getFrequency());
    qDebug() << "hamlib_p->getFrequency returning" << s;
}

void MainWindow::on_quit_pbutton_clicked() {
    qDebug() << "on_a_b_vfo_pbutton_clicked called";
    QApplication::quit();
}

void MainWindow::on_a_b_vfo_pbutton_clicked()
{

}

void MainWindow::on_afx_pbutton_clicked()
{
    qDebug() << "on_afx_pbutton_clicked called";
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

}

void MainWindow::on_xfil_button_clicked()
{

}
