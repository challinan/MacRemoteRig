#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QList>
#include <QTimer>
#include <QThread>
#include "hamlibconnector.h"
#include "frequencypoller.h"
#include "config_object.h"
#include "genericdialog.h"
#include "tune_dialog.h"
#include "transmitwindow.h"
#include "gstreamerlistener.h"
#include "icon_defines.h"

// For debug only
#include <QLineEdit>

// #define SKIP_RIG_INIT

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define F_UP 1
#define F_DN 2
#define INITIAL_NUDGE_DELAY 400
#define AUTONUDGE_DELAY 50
#define INITIAL_FREQ_INCREMENT 10
#define FAST_FREQ_INCREMENT 100

typedef struct {
    int index;
    QString band;
} band_index_t;

static const band_index_t band_index[] = {
    {0, "160"},
    {1, "80"},
    {2, "60"},
    {3, "40"},
    {4, "30"},
    {5, "20"},
    {6, "17"},
    {7, "15"},
    {8, "12"},
    {9, "10"},
    {10, "6"}
};

class TransmitWindow;
class TuneDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    HamlibConnector *getHamlibPointer();
    bool failed();
    QString failedReason();

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
    void initialize_front_panel();
    void getConfigIconBits();

protected:
     void keyPressEvent(QKeyEvent *event) override;

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
    void on_manual_pButton_clicked();
    void on_band_pbutton_clicked();
    void on_mode_pbutton_clicked();
    void on_power_pbutton_clicked();
    void on_tune_pbutton_clicked();
    void on_upshift_pButton_clicked();
    void on_centerShift_pButton_clicked();
    void on_downshift_pButton_clicked();
    void on_upwidth_pButton_clicked();
    void on_centerWidth_pButton_clicked();
    void on_downwidth_pButton_clicked();
    void update_width_slider(int w = 0);
    void on_abortTXpbutton_clicked();
    void on_pauseTXpbutton_toggled(bool b);
    void on_dnCwSpeedpButton_clicked();
    void on_upCwSpeedpButton_clicked();
    void on_txtest_pbutton_clicked();
    void on_band_comboBox_activated(int index);
    void on_callSignLineEdit_returnPressed();
    void on_callSignLineEdit_textEdited(const QString &arg1);

private:
    void nudgeFrequency(int direction);
    void worker_thread();

private:
    Ui::MainWindow *ui;
    HamlibConnector *hamlib_p;
    FrequencyPoller *freq_poller;
    int freq_polling_active;
    ConfigObject *configobj_p;
    GenericDialog *genericdialog_p;
    TuneDialog *tuneDialog_p;
    // For debug only:
    QLineEdit *f_edit;
    QLCDNumber *lcd_p;
    QPushButton *upbutton_p;
    int mrr_frequency_increment;
    bool up_pressed;
    bool down_pressed;
    int nudge_delay;
    GstreamerListener *gstreamerListener_p;
    QGraphicsScene *scene_p;
    TransmitWindow *pTxWindow;
    char ic_bits[5];
    bool init_failed;
    int init_failure_code;

    // For Debug
    QTimer *bw_timer;
    void do_bw_update();

signals:
    void bwidth_change(int bw);
    void refresh_rig_mode_bw();
    void update_bw_slider();

};
#endif // MAINWINDOW_H
