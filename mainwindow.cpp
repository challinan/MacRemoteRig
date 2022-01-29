#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hamlibconnector.h"
#include "genericdialog.h"
#include "gstreamerlistener.h"

// #define SKIP_RIG_INIT
// #define SKIP_CONFIG_INIT

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) , ui(new Ui::MainWindow)
{
    // parent comes into this constructor as null
    ui->setupUi(this);
    this->setWindowTitle("Mac Remote Rig");
    qDebug() << "MainWindow::MainWindow(): constructor entered: this =" << this;

#ifdef SKIP_CONFIG_INIT
    // Initialize our config database
    configobj_p = new ConfigObject;
    configobj_p->debug_display_map();
#endif

    /* Initialize the rig */
    // HamlibConnector hamlibc;
#ifndef SKIP_RIG_INIT
    hamlib_p = new HamlibConnector;
    hamlib_p->store_ui_pointer(ui);
    qDebug() << " MainWindow::MainWindow() constructor: hamlib init returned" << hamlib_p->get_retcode();

    /* Update our Main (VFO_A) display window with the current VFO_A freq */
    freq_t fA = hamlib_p->mrr_getRigFrequency(RIG_VFO_A);
    QString str_tmp = HamlibConnector::get_display_frequency(fA);

    /* Initialize the freq window */
    ui->freqDisplay->setDigitCount(8);
    ui->freqDisplay->setSmallDecimalPoint(1);
    ui->freqDisplay->display(str_tmp);

    /* Update our VFO_B display window with the current VFO_B freq */
    freq_t fB = hamlib_p->mrr_getRigFrequency(RIG_VFO_B);
    str_tmp.clear();
    str_tmp = HamlibConnector::get_display_frequency(fB);

    /* Initialize the Secondary freq (VFO_B) window */
    ui->freqBDisplay->setDigitCount(8);
    ui->freqBDisplay->setSmallDecimalPoint(1);
    ui->freqBDisplay->display(str_tmp);

    // Initialize S-Meter
    // See comments at http://hamlib.sourceforge.net/manuals/4.3/group__rig.html about rig_get_level
    ui->smeterProgressBar->setMinimum(0);
    ui->smeterProgressBar->setMaximum(15);
    ui->smeterProgressBar->reset();     // Reset to zero

    freq_polling_active = 0;
    mrr_frequency_increment = INITIAL_FREQ_INCREMENT;       // Amount used by nudge to tune up or down the band
    up_pressed = down_pressed = 0;
    nudge_delay = INITIAL_NUDGE_DELAY;          // 500 mS initially
#endif

    // Initialize the radio label
    qDebug() << "Initialize Radio label";
    QFont myFont( "Arial", 18, QFont::Bold);
    ui->radioLabel->setFont(myFont);
    ui->radioLabel->setText("Elecraft K3");

    // Initialize the width slider view
    scene_p = new QGraphicsScene;
    ui->bwidthGraphicsView->setScene(scene_p);
    ui->bwidthGraphicsView->setInteractive(false);

    // Customize the "Fast" and "PauseTX" buttons to make them "Checkable"
    ui->fast_pButton->setCheckable(1);
    ui->pauseTXpbutton->setCheckable(1);

    // Get the initial CW Speed value
    int speed = hamlib_p->getCwSpeed();
    if ( speed != -1 )
        ui->cwSpeedValueLabel->setText(QString().setNum(speed));

    // Initialize other front panel status bits
#ifndef SKIP_RIG_INIT
    initialize_front_panel();

    // Start the listener thread for audio
    gstreamerListener_p = new GstreamerListener();
    gstreamerListener_p->start();  // start() unlike run() detaches and returns immediately
#endif

    // Setup transmit window
    pTxEdit = new TransmitWindow(this, hamlib_p);
    pTxEdit->setHamlibPointer(hamlib_p);
#ifndef SKIP_RIG_INIT

    // Connect Signals & Slots
    connect(hamlib_p, &HamlibConnector::updateWidthSlider, this, &MainWindow::update_width_slider);
    connect(this, &MainWindow::bwidth_change, hamlib_p, &HamlibConnector::bwidth_change_request);
    connect(this, &MainWindow::refresh_rig_mode_bw, hamlib_p, &HamlibConnector::get_rig_mode_and_bw);
#endif
}

MainWindow::~MainWindow()
{
    gstreamerListener_p->terminate();
    gstreamerListener_p->wait();
    delete gstreamerListener_p;
#ifndef SKIP_RIG_INIT
    delete scene_p;
    delete hamlib_p;
    delete ui;
    delete pTxEdit;
#endif
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    // qDebug() << "MainWindow::keyPressEvent(): entered - centralWidget" << centralWidget() << "key =" << event->key();
    QWidget::keyPressEvent(event);
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

#if 0       // Standalone debug test dialog
    // More debug code
    QDialog *pd = new QDialog();
    pd->setMinimumSize(QSize(800, 800));
    upbutton_p = new QPushButton("⬆︎", pd);
    // connect(upbutton_p, &QPushButton::pressed, this, &MainWindow::on_uptune_pButton_pressed);
    // button.show();

    pd->show();
    pd->exec();

    // int rig_get_freqs(RIG *rig, freq_t *freqA, freq_t freqB );
    QString boxStr = "RIG_VFO_A = ";
    QString vfo_number = "";
    vfo_number.setNum(RIG_VFO_A);
    boxStr.append(vfo_number);
    QMessageBox msgBox1;
    msgBox1.setText(boxStr);
    msgBox1.exec();
#endif

    bw_timer = new QTimer(this);
    connect(bw_timer, &QTimer::timeout, this, &MainWindow::do_bw_update );
    bw_timer->start(50);

}

// For debug only
void MainWindow::do_bw_update() {
    static int loop_die = 0;
    static int loop = 1;
    static int direction = 1; // Up
    const static int stop = 3000;
    update_width_slider(loop);
    if ( direction == 1 ) {
        loop += 10;
        if ( loop > stop ) { loop = 2700; direction = 0; }
    }
    else {
        loop -= 10;
        if ( loop < 1 ) { loop = 1; direction = 1; loop_die = 1; }
    }
    if ( loop_die ) { delete bw_timer; return; }
    bw_timer->start(50);
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
#ifndef SKIP_RIG_INIT
    gstreamerListener_p->terminate();
    gstreamerListener_p->wait();
#endif
    QApplication::exit(0);
}

void MainWindow::on_a_b_vfo_pbutton_clicked()
{
    hamlib_p->setSwapAB();

    // Update the displays
    freq_t fA = hamlib_p->mrr_getRigFrequency(RIG_VFO_A);
    QString str_tmp = HamlibConnector::get_display_frequency(fA);
    ui->freqDisplay->display(str_tmp);
    str_tmp.clear();
    freq_t fB = hamlib_p->mrr_getRigFrequency(RIG_VFO_B);
    str_tmp = HamlibConnector::get_display_frequency(fB);
    ui->freqBDisplay->display(str_tmp);

    emit refresh_rig_mode_bw();
    update_width_slider(hamlib_p->mrr_get_width());
    // initialize_front_panel();
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
    hamlib_p->setSpot();
}

void MainWindow::on_xfil_pbutton_clicked()
{
    qDebug() << "on_xfil_pbutton_clicked() not implemented";
}

void MainWindow::on_config_pbutton_clicked() {
    configobj_p->open_config_dialog();
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
        QApplication::exit(4);
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

void MainWindow::on_band_pbutton_clicked()
{
    qDebug() << "MainWindow::on_band_pbutton_clicked()";
}

void MainWindow::on_mode_pbutton_clicked()
{
    unsigned long i;
    mode_t new_mode;
    pbwidth_t width;
    static unsigned long index = 0;
    const static mode_t supported_modes [] = {
        RIG_MODE_NONE,
        RIG_MODE_AM,
        RIG_MODE_CW,
        RIG_MODE_USB,
        RIG_MODE_LSB,
        RIG_MODE_RTTY,
        RIG_MODE_FM
    };

    mode_t m = hamlib_p->mrr_get_mode();
    for ( i=1; i<sizeof(supported_modes); i++ ) {
        if ( m == supported_modes[i] ) {
            index = i;
            break;
        }
    }
    if ( supported_modes[index] == RIG_MODE_FM ) {
        index = 1;  // Roll to top of list, neglecting the invalid mode RIG_MODE_NONE
    } else {
        index += 1;
    }
    new_mode = supported_modes[index];
    int rc = hamlib_p->mrr_set_mode(new_mode);
    if ( rc != RIG_OK ) {
        qDebug() << "MainWindow::on_mode_pbutton_clicked(): mrr_set_mode() failed" << rc;
    }

    QString modeStr = hamlib_p->mrr_getModeString(new_mode);
    ui->mode_pbutton->setText(modeStr);
    width = hamlib_p->mrr_get_width();
    update_width_slider(width);
}

void MainWindow::on_power_pbutton_clicked()
{
    qDebug() << "MainWindow::on_power_pbutton_clicked()";
    // int 	rig_set_powerstat (RIG *rig, powerstat_t status)
}

void MainWindow::on_tune_pbutton_clicked()
{
    tuneDialog_p = new TuneDialog(this);

}

void MainWindow::on_upshift_pButton_clicked()
{
    qDebug() << "MainWindow::on_upshift_pButton_clicked()";
}


void MainWindow::on_centerShift_pButton_clicked()
{
    qDebug() << "MainWindow::on_centerShift_pButton_clicked()";
}


void MainWindow::on_downshift_pButton_clicked()
{
    qDebug() << "MainWindow::on_downshift_pButton_clicked()";
}


void MainWindow::on_upwidth_pButton_clicked()
{
    int bw = hamlib_p->mrr_get_width() + BANDWIDTH_STEP;
    // Validate max up range based on mode
    emit bwidth_change(bw);
}


void MainWindow::on_centerWidth_pButton_clicked()
{
    mode_t m = hamlib_p->mrr_get_mode();
    pbwidth_t bw;
    switch( m ) {
    case RIG_MODE_CW:
        bw = 800;
        break;
    case RIG_MODE_USB:
    case RIG_MODE_LSB:
    case RIG_MODE_RTTY:
        bw = 2700;
        break;
    case RIG_MODE_FM:
        bw = 5000;
        break;
    case RIG_MODE_AM:
        bw = 5000;
        break;
    default:
        qDebug() << "MainWindow::on_centerWidth_pButton_clicked(): invalid rig mode" << m;
        bw = 1200;
        break;
    }
    emit bwidth_change(bw);    // Center for CW
}


void MainWindow::on_downwidth_pButton_clicked()
{
    int bw = hamlib_p->mrr_get_width() - BANDWIDTH_STEP;
    // Validate max down range based on mode
    emit bwidth_change(bw);
}

void MainWindow::initialize_front_panel() {


    // Get frequencies

    mode_t mode = hamlib_p->mrr_get_mode();

    // Get and display current mode
    QString modeStr = hamlib_p->mrr_getModeString(mode);
    ui->mode_pbutton->setText(modeStr);

    // Setup width control
    pbwidth_t w = hamlib_p->mrr_get_width();
    update_width_slider(w);
}

void MainWindow::update_width_slider(int w) {

    float barcount;
    int view_width_midpoint, offset;

#define MAX_BARS 22
#define BAR_WIDTH 8
    if ( w > 2700 )
        barcount = MAX_BARS;
    else
        barcount = ((float) w / 2700e0f) * MAX_BARS;
    barcount = round(barcount / 2) * 2;         // Keep it an even number

    // qDebug() << "Width = " << w << "Barcount = " << barcount;
    scene_p->clear();
    scene_p->setSceneRect(0, 0, 220, 30);
    view_width_midpoint = ui->bwidthGraphicsView->rect().width() / 2;
    offset = view_width_midpoint - ((barcount / 2) * 8);
    // () << "MainWindow::update_width_slider(): barcount = " << barcount << "offset = " << offset;
    for ( int i=0; i<(int)barcount; i++ ) {
        // QRect r = QRect(((i+offset)*BAR_WIDTH), 0, 5, 20);
        QRect r = QRect((i*8)+offset, 4, 5, 20);
        scene_p->addRect(r, QPen(), QBrush(Qt::blue, Qt::SolidPattern));
    }
    QString wStr = "";
    wStr.setNum(w);
    ui->bwLabel->setText(wStr);
}

void MainWindow::on_abortTXpbutton_clicked()
{
    hamlib_p->abortTX();
}

void MainWindow::on_pauseTXpbutton_toggled(bool checked)
{
    qDebug() << "MainWindow::on_pauseTXpbutton_toggled(): checked =" << checked;
    hamlib_p->setPauseTx(checked);
}

void MainWindow::on_upCwSpeedpButton_clicked()
{

    qDebug() << "MainWindow::on_upCwSpeedpButton_clicked(): entered";
    int s = hamlib_p->bumpCwSpeed(true);
    if ( s == -1 )
        return;
    ui->cwSpeedValueLabel->setText(QString().setNum(s));
}

void MainWindow::on_dnCwSpeedpButton_clicked()
{
    qDebug() << "MainWindow::on_dnCwSpeedpButton_clicked(): entered";
    int s = hamlib_p->bumpCwSpeed(false);
    if ( s == -1 )
        return;
    ui->cwSpeedValueLabel->setText(QString().setNum(s));

}
