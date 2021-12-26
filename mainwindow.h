#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "hamlibconnector.h"
#include "frequencypoller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    HamlibConnector *getHamlibPointer();

public slots:
    void on_quit_pbutton_clicked();
    void on_poll_freq_pbutton_clicked();
    void on_a_2_b_pbutton_clicked();
    void on_a_b_vfo_pbutton_clicked();
    void on_afx_pbutton_clicked();
    void on_agc_pbutton_clicked();
    void on_bset_pbutton_clicked();
    void on_cwt_pbutton_clicked();
    void on_nb_pbutton_clicked();
    void on_nr_pbutton_clicked();
    void on_ntch_pbutton_clicked();
    void on_pre_pbutton_clicked();
    void on_rev_vfo_pbutton_clicked();
    void on_split_pbutton_clicked();
    void on_spot_pbutton_clicked();
    void on_xfil_button_clicked();

private:
    Ui::MainWindow *ui;
    HamlibConnector *hamlib_p;
    FrequencyPoller *freq_poller;
    int freq_polling_active;
};
#endif // MAINWINDOW_H
