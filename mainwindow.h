#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "hamlibconnector.h"
#include "frequencypoller.h"
#include "config_object.h"
// For debug only
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define F_UP 1
#define F_DN 2
#define INITIAL_NUDGE_DELAY 400
#define AUTONUDGE_DELAY 50
#define INITIAL_FREQ_INCREMENT 10
#define FAST_FREQ_INCREMENT 100

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
    void on_xfil_pbutton_clicked();
    void on_config_pbutton_clicked();
    // For Debug:
    void on_editingFinished();
    void nudge_timer_action();

private slots:
     void on_uptune_pButton_pressed();
    void on_downtune_pButton_pressed();
    void on_uptune_pButton_released();
    void on_downtune_pButton_released();
    void on_uptune_pButton_clicked();
    void on_downtune_pButton_clicked();
    void nudge_uptimer_fired();
    void nudge_downtimer_fired();
    void on_fast_pButton_toggled(bool checked);

private:
    void nudgeFrequency(int direction);


private:
    Ui::MainWindow *ui;
    HamlibConnector *hamlib_p;
    FrequencyPoller *freq_poller;
    int freq_polling_active;
    ConfigObject *configobj_p;
    // For debug only:
    QLineEdit *f_edit;
    QLCDNumber *lcd_p;
    QPushButton *upbutton_p;
    int mrr_frequency_increment;
    bool up_pressed;
    bool down_pressed;
    int nudge_delay;

};
#endif // MAINWINDOW_H
