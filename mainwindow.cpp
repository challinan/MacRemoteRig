#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "hamlibconnector.h"
#include "genericdialog.h"
#include "gstreamerlistener.h"

// #define SKIP_CONFIG_INIT

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) , ui(new Ui::MainWindow)
{
    init_failed = false;
    freq_t fA, fB;
    int speed;
    QFont myFont("Arial", 18, QFont::Bold);
    QString str_tmp;

    gstreamerListener_p = nullptr;
    scene_p = nullptr;
    hamlib_p = nullptr;
    pTxWindow = nullptr;
    ledColor = QColor(Qt::green);

    // parent comes into this constructor as null
    ui->setupUi(this);
    setWindowTitle("Mac Remote Rig");
    qDebug() << "MainWindow::MainWindow(): autoFillBackground is " << autoFillBackground();
    setAutoFillBackground(true);
    // setStyleSheet("background-color: gray;");

#ifndef SKIP_CONFIG_INIT
    // Initialize our config database
    configobj_p = new ConfigObject;
    configobj_p->debug_display_map();
#endif

    /* Initialize the rig */
    // HamlibConnector hamlibc;
#ifndef SKIP_RIG_INIT
    hamlib_p = new HamlibConnector;
    qDebug() << "MainWindow::MainWindow() constructor: hamlib init returned" << hamlib_p->get_retcode();
    if ( hamlib_p->get_retcode() != RIG_OK ) {
        qDebug() << "MainWindow::MainWindow(): hamlib failed to init - exiting";
        init_failed = true;
        init_failure_code = hamlib_p->get_retcode();
        goto startupFailed;
    }
    hamlib_p->store_ui_pointer(ui);

    /* Update our Main (VFO_A) display window with the current VFO_A freq */
    fA = hamlib_p->mrr_getRigFrequency(RIG_VFO_A);
    str_tmp = HamlibConnector::get_display_frequency(fA);

    /* Initialize the freq window */
    ui->freqDisplay->setDigitCount(8);
    ui->freqDisplay->setSmallDecimalPoint(1);
    ui->freqDisplay->display(str_tmp);

    /* Update our VFO_B display window with the current VFO_B freq */
    fB = hamlib_p->mrr_getRigFrequency(RIG_VFO_B);
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
    // myFont initialized at start of constructor
    // myFont = ("Arial", 18, QFont::Bold);
    ui->radioLabel->setFont(myFont);
    ui->radioLabel->setText("Elecraft K3");

    // Initialize the width slider view
    scene_p = new QGraphicsScene;
    ui->bwidthGraphicsView->setScene(scene_p);
    ui->bwidthGraphicsView->setInteractive(false);

    // Customize the "Fast", "PauseTX" and "Tx Test" buttons to make them "Checkable"
    ui->fast_pButton->setCheckable(1);
    ui->pauseTXpbutton->setCheckable(1);
    ui->txtest_pbutton->setCheckable(1);
    ui->spot_pbutton->setCheckable(1);

    // Get the initial CW Speed value
#ifndef SKIP_RIG_INIT
    speed = hamlib_p->getCwSpeed();
    if ( speed != -1 )
        ui->cwSpeedValueLabel->setText(QString().setNum(speed));

    // Initialize other front panel status bits
    getConfigIconBits();
    qDebug() << "MainWindow::MainWindow(): sizeof band_index =" << sizeof(band_index) / sizeof(band_index[0]);
    for ( int i=0; i< (int) (sizeof(band_index) / (int) sizeof(band_index[0])); i++ ) {
        ui->band_comboBox->insertItem(band_index[i].index, band_index[i].band);
    }
    initialize_front_panel();
    ui->monLevelSpinBox->setMinimum(0);
    ui->monLevelSpinBox->setMaximum(60);
    ui->monLevelSpinBox->setValue( hamlib_p->mrrGetMonLevel() );
    ui->widthDial->setNotchesVisible(true);
    ui->widthDial->setWrapping(false);

    // Start the listener thread for audio
    gstreamerListener_p = new GstreamerListener();
    gstreamerListener_p->start();  // start() unlike run() detaches and returns immediately
#endif

    // Setup transmit window
    pTxWindow = new TransmitWindow(this, hamlib_p);
    pTxWindow->setHamlibPointer(hamlib_p);
#ifndef SKIP_RIG_INIT

    // Connect Signals & Slots
    connect(hamlib_p, &HamlibConnector::updateWidthSlider, this, &MainWindow::update_width_slider);
    connect(this, &MainWindow::bwidth_change, hamlib_p, &HamlibConnector::bwidthChangeRequest);
    connect(this, &MainWindow::refresh_rig_mode_bw, hamlib_p, &HamlibConnector::get_rig_mode_and_bw);
    connect(hamlib_p, &HamlibConnector::updateXFIL_sig, this, &MainWindow::updateXFIL_display);
    connect(hamlib_p, &HamlibConnector::spotDone, this, &MainWindow::uncheckSpotButton);
    connect(this, &MainWindow::updateCwSpeedSig, pTxWindow, &TransmitWindow::updateRigCwSpeedSlot);
#endif

startupFailed:
    return;
}

MainWindow::~MainWindow() {

#ifndef SKIP_RIG_INIT
    if ( gstreamerListener_p ) {
        gstreamerListener_p->terminate();
        gstreamerListener_p->wait();
        delete gstreamerListener_p;
    }

    if ( scene_p )
        delete scene_p;

    if ( hamlib_p )
        delete hamlib_p;

    if ( pTxWindow )
        delete pTxWindow;

    delete ui;
#endif
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(QBrush(ledColor));
    painter.drawEllipse(1025,16,15,15);
}

void MainWindow::drawLED(int x, int y)
{
    QGraphicsPixmapItem *item;
    QPainter painter(this);
     painter.drawArc(x,y,150,50,0,16*360);
    // item = new QGraphicsPixmapItem(QPixmap::fromImage(this));
    // this->addItem(item);

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
    qDebug() << "on_a_2_b_pbutton_clicked: entered";
    hamlib_p->mrr_a_2_b();
    initialize_front_panel();
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
    // str_tmp.clear();
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
    hamlib_p->mrrSetXFIL(); // Cycles through values
    hamlib_p->get_rig_mode_and_bw();

    // Setup width control
    pbwidth_t w = hamlib_p->mrr_get_width();
    update_width_slider(w);
    // qDebug() << "on_xfil_pbutton_clicked(): bw now" << w;
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


void MainWindow::on_normWidth_pButton_clicked()
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
        qDebug() << "MainWindow::on_normWidth_pButton_clicked(): invalid rig mode" << m;
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

    int band;

    // Get frequencies
    freq_t fA = hamlib_p->mrr_getRigFrequency(RIG_VFO_A);
    QString str_tmp = HamlibConnector::get_display_frequency(fA);
    ui->freqDisplay->display(str_tmp);
    freq_t fB = hamlib_p->mrr_getRigFrequency(RIG_VFO_B);
    str_tmp = HamlibConnector::get_display_frequency(fB);
    ui->freqBDisplay->display(str_tmp);

    mode_t mode = hamlib_p->mrr_get_mode();

    // Get and display current mode
    QString modeStr = hamlib_p->mrr_getModeString(mode);
    ui->mode_pbutton->setText(modeStr);

    // Setup width control
    pbwidth_t w = hamlib_p->mrr_get_width();
    update_width_slider(w);

    // Set Icons
    getConfigIconBits();

    // Check band and set band combobox
    band = hamlib_p->mrr_get_band();
    qDebug() << "MainWindow::initialize_front_panel(): ***********************band" << band;
    ui->band_comboBox->setCurrentIndex(band);
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
    pTxWindow->abortTxNow();
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
    emit updateCwSpeedSig(s);
}

void MainWindow::on_dnCwSpeedpButton_clicked()
{
    qDebug() << "MainWindow::on_dnCwSpeedpButton_clicked(): entered";
    int s = hamlib_p->bumpCwSpeed(false);
    if ( s == -1 )
        return;
    ui->cwSpeedValueLabel->setText(QString().setNum(s));

}

void MainWindow::getConfigIconBits()
{
    hamlib_p->mrr_get_ic_config(ic_bits);

    if ( ic_bits[2] & K3_ICON_VOX ) ui->voxLabel->setText("VOX");
    if ( ic_bits[0] & K3_ICON_TXTEST )  {
        ui->txTestLabel->setText("TXTEST");
        ui->txtest_pbutton->setChecked(true);
    } else {
        ui->txTestLabel->setText("TXNORM");
        ui->txtest_pbutton->setChecked(false);
    }
}


void MainWindow::on_txtest_pbutton_clicked()
{
    qDebug() << "MainWindow::on_txtest_pbutton_clicked(): entered - checked =" << ui->txtest_pbutton->isChecked();
    hamlib_p->mrr_set_tx_test();

    // Now retrieve the value from the rig
    hamlib_p->mrr_get_ic_config(ic_bits);
    if ( ic_bits[0] & K3_ICON_TXTEST )  {
        ui->txTestLabel->setText("TXTEST");
    } else {
        ui->txTestLabel->setText("TXNORM");
    }
}


void MainWindow::on_band_comboBox_activated(int band)
{
    qDebug() << "MainWindow::on_band_comboBox_activated(): entered" << band;
    hamlib_p->mrr_set_band(band);
    QThread::msleep(400);  // Band changes require minimum of 300ms
    initialize_front_panel();
}

void MainWindow::on_callSignLineEdit_returnPressed()
{

    QString callStr = ui->callSignLineEdit->text();
    qDebug() << "MainWindow::on_callSignLineEdit_returnPressed(): text = " << callStr;
    QString str = "Call Sign: " + callStr;
    ui->callSignLabel->setText(str);
}

void MainWindow::on_callSignLineEdit_textEdited(const QString &arg1)
{
    ui->callSignLineEdit->setText(arg1.toUpper());
}

bool MainWindow::failed() {
    return init_failed;
}

QString MainWindow::failedReason() {

    switch (init_failure_code) {
    case RIG_OK:
        return QString(rig_error_strings[RIG_OK].error_string);
    case -RIG_EINVAL:
        return QString(rig_error_strings[RIG_EINVAL].error_string);
    case -RIG_ECONF:
        return QString(rig_error_strings[RIG_ECONF].error_string);
    case -RIG_ENOMEM:
        return QString(rig_error_strings[RIG_ENOMEM].error_string);
    case -RIG_ENIMPL:
        return QString(rig_error_strings[RIG_ENIMPL].error_string);
    case -RIG_ETIMEOUT:
        return QString(rig_error_strings[RIG_ETIMEOUT].error_string);
    case -RIG_EIO:
        return QString(rig_error_strings[RIG_EIO].error_string);
    case -RIG_EINTERNAL:
        return QString(rig_error_strings[RIG_EINTERNAL].error_string);
    case -RIG_EPROTO:
        return QString(rig_error_strings[RIG_EPROTO].error_string);
    case -RIG_ERJCTED:
        return QString(rig_error_strings[RIG_ERJCTED].error_string);
    case -RIG_ETRUNC:
        return QString(rig_error_strings[RIG_ETRUNC].error_string);
    case -RIG_ENAVAIL:
        return QString(rig_error_strings[RIG_ENAVAIL].error_string);
    case -RIG_ENTARGET:
        return QString(rig_error_strings[RIG_ENTARGET].error_string);
    case -RIG_BUSERROR:
        return QString(rig_error_strings[RIG_BUSERROR].error_string);
    case -RIG_BUSBUSY:
        return QString(rig_error_strings[RIG_BUSBUSY].error_string);
    case -RIG_EARG:
        return QString(rig_error_strings[RIG_EARG].error_string);
    case -RIG_EVFO:
        return QString(rig_error_strings[RIG_EVFO].error_string);
    case -RIG_EDOM:
        return QString(rig_error_strings[RIG_EDOM].error_string);
    default:
        return QString("Unknown failure");
    }
}

void MainWindow::on_monLevelSpinBox_valueChanged(int level) {

    // qint64 t1 = QDateTime::currentMSecsSinceEpoch();
    hamlib_p->mrrSetMonLevel(level);
    // qint64 t2 = QDateTime::currentMSecsSinceEpoch();
    // qDebug() << "MainWindow::on_monLevelSpinBox_valueChanged(): duration - " << (t2 - t1);
}


void MainWindow::on_widthDial_valueChanged(int value)
{
    qDebug() << "MainWindow::on_widthDial_valueChanged(): value =" << value;
    // **** Down ****
    // on_upwidth_pButton_clicked()
    // int bw = hamlib_p->mrr_get_width() + BANDWIDTH_STEP;
    // Validate max up range based on mode
    // emit bwidth_change(bw);
    // **** Down ****

    // **** Up ****
    // int bw = hamlib_p->mrr_get_width() - BANDWIDTH_STEP;
    // Validate max down range based on mode
    // emit bwidth_change(bw);
    // **** Up ****
}

void MainWindow::updateXFIL_display() {
    qDebug() << "MainWindow::updateXFIL_display(): entered";
}

void MainWindow::uncheckSpotButton() {

    ui->spot_pbutton->setChecked(false);
}

void MainWindow::on_powerOff_pushButton_clicked() {
    static bool power_toggle = true;
    hamlib_p->powerOFF();
    power_toggle = power_toggle ? false : true;
    ledColor = power_toggle ? QColor(Qt::green) : QColor(Qt::red);
    update();   // Schedule a repaint
}
