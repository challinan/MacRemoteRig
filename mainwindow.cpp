#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hamlibconnector.h"
#include "genericdialog.h"

// #define SKIP_RIG_INIT

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Initialize the rig */
    // HamlibConnector hamlibc;
#ifndef SKIP_RIG_INIT
    hamlib_p = new HamlibConnector;
    hamlib_p->store_ui_pointer(ui);

    /* Update our display window with the current VFO freq */
    // QString s = hamlib_p->getFrequency();
    freq_t f = hamlib_p->mrr_getFrequency(hamlib_p->mrr_getVFO());
    qDebug() << "MainWindow Constructor: mrr_getFrequency returned: " << f;
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
    mrr_frequency_increment = INITIAL_FREQ_INCREMENT;       // Amount used by nudge to tune up or down the band
    up_pressed = down_pressed = 0;
    nudge_delay = INITIAL_NUDGE_DELAY;          // 500 mS initially

    // Customize the "Fast" button to make it "Checkable"
    ui->fast_pButton->setCheckable(1);
}

MainWindow::~MainWindow()
{
    delete hamlib_p;
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
# if 0
    // For debug only
    QDialog *pd = new QDialog();
    pd->setMinimumSize(QSize(400, 200));
    f_edit = new QLineEdit(pd);
    f_edit->setGeometry(100, 20, 200, 20);
    lcd_p = new QLCDNumber(pd);
    lcd_p->setGeometry(100, 100, 200, 50);
    lcd_p->setDigitCount(8);
    lcd_p->setSmallDecimalPoint(1);

    connect(f_edit, &QLineEdit::editingFinished, this, &MainWindow::on_editingFinished)

    pd->show();
    pd->exec();
#endif

    // More debug code
    QDialog *pd = new QDialog();
    pd->setMinimumSize(QSize(800, 800));
    upbutton_p = new QPushButton("⬆︎", pd);
    // connect(upbutton_p, &QPushButton::pressed, this, &MainWindow::on_uptune_pButton_pressed);
    // button.show();

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

void MainWindow::on_config_pbutton_clicked() {
    configobj_p = new ConfigObject;
}

void MainWindow::on_uptune_pButton_clicked()
{
    nudgeFrequency(F_UP);
}

void MainWindow::on_uptune_pButton_pressed() {
    // Kickoff a short timer and if the button is still pressed, continue nudging freq up/down
    up_pressed = 1;
    QTimer::singleShot(nudge_delay, this, SLOT(nudge_uptimer_fired()) );
}

void MainWindow::nudge_uptimer_fired() {
    if ( ui->uptune_pButton->isDown() ) {
        if ( nudge_delay == INITIAL_NUDGE_DELAY ) nudge_delay = AUTONUDGE_DELAY;
        // qDebug() << "Up Button still pressed";
        if ( up_pressed ) {
            nudgeFrequency(F_UP);
            on_uptune_pButton_pressed();
        }
    }
}

void MainWindow::on_uptune_pButton_released() {
    up_pressed = 0;
    nudge_delay = INITIAL_NUDGE_DELAY;
}

void MainWindow::on_downtune_pButton_clicked() {
    // qDebug() << "MainWindow::on_downtune_pbutton_clicked(): entered";
    nudgeFrequency(F_DN);
}

void MainWindow::on_downtune_pButton_pressed() {
    // Kickoff a short timer and if the button is still pressed, continue nudging freq up/down
    down_pressed = 1;
    QTimer::singleShot(nudge_delay, this, SLOT(nudge_downtimer_fired()) );
}

void MainWindow::nudge_downtimer_fired() {
    // Impelement auto-down
    if ( ui->downtune_pButton->isDown() ) {
        if ( nudge_delay == INITIAL_NUDGE_DELAY ) nudge_delay = AUTONUDGE_DELAY;
        // qDebug() << "Down Button still pressed";
        if ( down_pressed ) {
            nudgeFrequency(F_DN);
            on_downtune_pButton_pressed();
        }
    }
}

void MainWindow::on_downtune_pButton_released() {
    down_pressed = 0;
    nudge_delay = INITIAL_NUDGE_DELAY;
}

void MainWindow::nudgeFrequency(int direction) {
    HamlibConnector &hl = *hamlib_p;
    freq_t f;
    if ( direction == F_UP ) {
        f = hl.mrr_getCurrentFreq_A() + mrr_frequency_increment;
    } else {
        f = hl.mrr_getCurrentFreq_A() - mrr_frequency_increment;
    }

    hl.mrr_setCurrentFreq_A(f);
    // qDebug() << "MainWindow::nudgeFrequency(): current freq is " << hl.mrr_getCurrentFreq_A();
    int rc = hl.set_rig_freq(f);
    if ( rc != RIG_OK ) {
        qDebug() << "MainWindow::nudgeFreqency(): set frequency failed: " << rc;
        QApplication::quit();
    }
    QString str_tmp = HamlibConnector::get_display_frequency(f);
    ui->freqDisplay->display(str_tmp);
}

void MainWindow::nudge_timer_action() {
    bool up = ui->uptune_pButton->isDown();
    if ( up ) {
        nudgeFrequency(F_UP);
    }
    else {
        nudgeFrequency(F_DN);
    }
}

void MainWindow::on_fast_pButton_toggled(bool checked)
{
    // qDebug() << "MainWindow::on_fast_pButton_toggled(): entered with " << checked;
    mrr_frequency_increment = checked ? FAST_FREQ_INCREMENT : INITIAL_FREQ_INCREMENT;
}

void MainWindow::on_manual_pButton_clicked()
{
    genericdialog_p = new GenericDialog(this);
}

