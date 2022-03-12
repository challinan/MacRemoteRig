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

    gstreamerListener_p = nullptr;
    scene_p = nullptr;
    hamlib_p = nullptr;
    pTxWindow = nullptr;
    vfoB_fast_toggle = false;
    ledColor = QColor(Qt::green);
    my_split = false;

    // parent comes into this constructor as null
    ui->setupUi(this);
    setWindowTitle("Mac Remote Rig");
    // setStyleSheet("background-color: gray;");

#ifndef SKIP_CONFIG_INIT
    // Initialize our config database
    configobj_p = new ConfigObject;
    configobj_p->debug_display_map();
#endif

    // Bulk of the initialization happens here
    mwInitialize();

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

void MainWindow::mwInitialize() {

    QFont myFont("Arial", 18, QFont::Bold);
    freq_t fA, fB;
    int speed;
    QString str_tmp;

    /* Initialize the rig */
    // HamlibConnector hamlibc;
    hamlib_p = new HamlibConnector;
    qDebug() << "MainWindow::mwInitialize() constructor: hamlib init returned" << hamlib_p->get_retcode();
#ifndef SKIP_RIG_INIT
    if ( hamlib_p->get_retcode() != RIG_OK ) {
        init_failed = true;
        init_failure_code = hamlib_p->get_retcode();
        qDebug() << "MainWindow::mwInitialize(): hamlib failed to init: retcode" << init_failure_code;
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
    qDebug() << "MainWindow::mwInitialize(): Initialize Radio label";
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
    ui->vfoB_fastTune_pButton->setCheckable(1);
    ui->split_pButton->setCheckable(1);

    // Get the initial CW Speed value
#ifndef SKIP_RIG_INIT
    speed = hamlib_p->getCwSpeed();
    if ( speed != -1 )
        ui->cwSpeedValueLabel->setText(QString().setNum(speed));

    // Initialize other front panel status bits, split status, modes, etc.
    qDebug() << "MainWindow::mwInitialize(): calling getConfigIconBits()";
    getConfigIconBits();

    // Setup band combox box.
    for ( int i=0; i< (int) (sizeof(band_index) / (int) sizeof(band_index[0])); i++ ) {
        ui->band_comboBox->insertItem(band_index[i].index, band_index[i].band);
    }

    initialize_front_panel();
    ui->monLevelSpinBox->setMinimum(0);
    ui->monLevelSpinBox->setMaximum(60);

    // FIXME: this generates the valueChanged signal and doubles the network traffic on read
    ui->monLevelSpinBox->setValue( hamlib_p->mrrGetMonLevel() );

    ui->widthDial->setNotchesVisible(true);
    ui->widthDial->setWrapping(true);
    ui->widthDial->setValue(0);
    ui->freqB_Dial->setValue(0);
    ui->freqB_Dial->setNotchesVisible(true);
    ui->freqB_Dial->setWrapping(true);
    // ui->freqB_Dial->setSingleStep(10);
    ui->smeterLabel->setText("0");
    ui->vfoB_FastLabel->setText("Norm");
    ui->vfoA_RX_Label->setText("RX");
    ui->vfoA_TX_Label->setText("TX");
    ui->vfoA_TX_Label->setStyleSheet("QLabel { color : red; }");
    ui->vfoB_TX_Label->setText("");

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

void MainWindow::paintEvent(QPaintEvent *event)
{
    // This function paints a power LED on the front panel
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(QBrush(ledColor));
    painter.drawEllipse(1025,16,15,15);
}

#if 0
void MainWindow::drawPowerLED(int x, int y)
{
    QPainter painter(this);
     painter.drawArc(x,y,150,50,0,16*360);
    // item = new QGraphicsPixmapItem(QPixmap::fromImage(this));
    // this->addItem(item);

}
#endif

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
    qDebug() << "MainWindow::on_afx_pbutton_clicked not implemented";
    // Start Remote GStreamer
}

void MainWindow::on_agc_pbutton_clicked()
{
    qDebug() << "MainWindow::on_agc_pbutton_clicked not implemented";
    // Stop remote GStreamer
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
    hamlib_p->mrrGetRigIF_XCVR_Info();
}

void MainWindow::on_pre_pbutton_clicked()
{

}

void MainWindow::on_rev_vfo_pbutton_clicked()
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
    nudgeFrequency(F_UP, RIG_VFO_A);
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
            nudgeFrequency(F_UP, RIG_VFO_A);
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
    nudgeFrequency(F_DN, RIG_VFO_A);
}

void MainWindow::on_downtune_pButton_pressed() {
    // Kickoff a short timer and if the button is still pressed, continue nudging freq up/down
    down_pressed = 1;
    QTimer::singleShot(nudge_delay, this, SLOT(nudge_downtimer_fired()) );
}

void MainWindow::nudge_downtimer_fired() {
    // Instead of this implementation, spawn a single thread that lives as long as pButton is pressed
    //    We need to avoid the situation where we have multiple timer threads all trying to send freq requests to rig
    // Do we ever reset nudge_delay back to INITIAL_NUDGE_DELAY and is that important?
    // Impelement auto-down
    if ( ui->downtune_pButton->isDown() ) {
        if ( nudge_delay == INITIAL_NUDGE_DELAY ) nudge_delay = AUTONUDGE_DELAY;
        // qDebug() << "Down Button still pressed";
        if ( down_pressed ) {
            nudgeFrequency(F_DN, RIG_VFO_A);
            on_downtune_pButton_pressed();
        }
    }
}

void MainWindow::on_downtune_pButton_released() {
    down_pressed = 0;
    nudge_delay = INITIAL_NUDGE_DELAY;
}

void MainWindow::nudgeFrequency(int direction, vfo_t vfo) {

    HamlibConnector &hl = *hamlib_p;
    freq_t f;

    if ( vfo == RIG_VFO_A) {
        if ( direction == F_UP ) {
            f = hl.mrrGetCachedFreqA() + mrr_frequency_increment;
        } else {
            f = hl.mrrGetCachedFreqA() - mrr_frequency_increment;
        }

        hl.mrrSetCachedFreqA(f);  // Stores our current cached value
    // qDebug() << "MainWindow::nudgeFrequency(): current freq is " << hl.mrr_getCurrentFreq_A();
        int rc = hl.mrrSetRigFreqA(f);
        if ( rc != RIG_OK ) {
            qDebug() << "MainWindow::nudgeFreqency(): set frequency failed: " << rc << hamlib_p->getRigError(rc);
            QApplication::exit(rc);
        }
        QString str_tmp = HamlibConnector::get_display_frequency(f);
        ui->freqDisplay->display(str_tmp);
    }

    if ( vfo == RIG_VFO_B) {
        if ( direction == F_UP ) {
            f = hl.mrrGetCachedFreqB() + (vfoB_fast_toggle ? FAST_FREQ_INCREMENT : INITIAL_FREQ_INCREMENT);
        } else {
            f = hl.mrrGetCachedFreqB() - (vfoB_fast_toggle ? FAST_FREQ_INCREMENT : INITIAL_FREQ_INCREMENT);
        }

        hl.mrrSetCachedFreqB(f);  // Stores our current cached value
    // qDebug() << "MainWindow::nudgeFrequency(): current freq B is " << hl.mrrGetCachedFreqB();
        int rc = hl.mrrSetRigFreqB(f);
        if ( rc != RIG_OK ) {
            qDebug() << "MainWindow::nudgeFreqency(): set frequency B failed: " << rc << hamlib_p->getRigError(rc);
            QApplication::exit(rc);
        }
        QString str_tmp = HamlibConnector::get_display_frequency(f);
        ui->freqBDisplay->display(str_tmp);
        update();
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

    mode_t m = hamlib_p->mrrGetMode();
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
    mode_t m = hamlib_p->mrrGetMode();
    int current_bwidth = hamlib_p->mrr_get_width();
    int bw_step = current_bwidth < 1000 ? BANDWIDTH_STEP - 5 : BANDWIDTH_STEP;
    int bw = hamlib_p->mrr_get_width() + bw_step;
    // Validate max up range based on mode
    switch( m ) {
    case RIG_MODE_CW:
        if ( bw > 2800 ) return;  // Ignore the request
        break;
    case RIG_MODE_USB:
    case RIG_MODE_LSB:
    case RIG_MODE_RTTY:
    case RIG_MODE_FM:
    case RIG_MODE_AM:
    default:
        qDebug() << "MainWindow::on_upwidth_pButton_clicked(): invalid rig mode" << m;
        bw = 1200;
        break;
    }
    updateXFIL_display();
    emit bwidth_change(bw);
}


void MainWindow::on_centerWidth_pButton_clicked()
{
    mode_t m = hamlib_p->mrrGetMode();
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
        bw = 13000;
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
    mode_t m = hamlib_p->mrrGetMode();
    int current_bwidth = hamlib_p->mrr_get_width();
    int bw_step = current_bwidth < 1000 ? BANDWIDTH_STEP - 5 : BANDWIDTH_STEP;
    int bw = hamlib_p->mrr_get_width() - bw_step;

    // Validate max down range based on mode
    switch( m ) {
    case RIG_MODE_CW:
        if ( bw < 50 ) return;  // Ignore this request - min value
        break;
    case RIG_MODE_USB:
    case RIG_MODE_LSB:
    case RIG_MODE_RTTY:
    case RIG_MODE_FM:
    case RIG_MODE_AM:
    default:
        qDebug() << "MainWindow::on_downwidth_pButton_clicked(): invalid rig mode" << m;
        break;
    }

    updateXFIL_display();
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

    mode_t mode = hamlib_p->mrrGetMode();

    // Get and display current mode
    QString modeStr = hamlib_p->mrr_getModeString(mode);
    ui->mode_pbutton->setText(modeStr);

    // Setup width control and xfil value
    pbwidth_t w = hamlib_p->mrr_get_width();
    update_width_slider(w);

    // Set Icons
    // getConfigIconBits();

    // Check band and set band combobox
    band = hamlib_p->mrr_get_band();
    // qDebug() << "MainWindow::initialize_front_panel(): ***********************band" << band;
    ui->band_comboBox->setCurrentIndex(band);

    // Set XFIL Icon
    updateXFIL_display();
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
    if ( hamlib_p->mrrGetIcConfig(ic_bits) == 0 ) { // Success
        if ( ic_bits[2] & K3_ICON_VOX_ON_CW ) ui->voxLabel->setText("VOX");

        if ( ic_bits[0] & K3_ICON_TXTEST )  {
            ui->txTestLabel->setText("TXTEST");
            ui->txtest_pbutton->setChecked(true);
        } else {
            ui->txTestLabel->setText("TXNORM");
            ui->txtest_pbutton->setChecked(false);
        }
    }
}

void MainWindow::processConfigIconBits() {

}

void MainWindow::on_txtest_pbutton_clicked()
{
    // FIXME
    // Either maintain cached state or read value before toggling it
    // Make sure the pushbutton state reflects the state of TX Test mode
    // Seems redundant to read it after we write (set) it

    qDebug() << "MainWindow::on_txtest_pbutton_clicked(): entered - checked =" << ui->txtest_pbutton->isChecked();
    hamlib_p->mrr_set_tx_test();

    // Now retrieve the value from the rig
    hamlib_p->mrrGetIcConfig(ic_bits);
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

    // Unfortunately, this signal/slot is emitted even when the value
    // is changed via our own SetValue call.  We need to squelch the
    // extraneous network traffic.

    // qint64 t1 = QDateTime::currentMSecsSinceEpoch();
    hamlib_p->mrrSetMonLevel(level);
    // qint64 t2 = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "MainWindow::on_monLevelSpinBox_valueChanged(): level:" << level;
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

    int f_num = hamlib_p->mrrGetXFILValue();
    qDebug() << "MainWindow::updateXFIL_display(): f_num =" << f_num;
    QString s = hamlib_p->getXFILString(f_num);
    ui->xfilLabel->setText(s);
}

void MainWindow::uncheckSpotButton() {

    ui->spot_pbutton->setChecked(false);
}

void MainWindow::on_powerOff_pushButton_clicked() {
    static bool power_toggle = true;
#ifndef SKIP_RIG_INIT
    hamlib_p->powerOFF();
#endif
    power_toggle = power_toggle ? false : true;
    ledColor = power_toggle ? QColor(Qt::green) : QColor(Qt::red);
    update();   // Schedule a repaint
}

void MainWindow::on_freqB_Dial_valueChanged(int value)
{
    static int prev_val = 0;
#define MRR_UP_CW true
#define MRR_DOWN_CCW false
    bool direction;

    // qDebug() << "MainWindow::on_freqB_Dial_valueChanged:" << value;

    // Detect direction of rotation.  Default values go from 0 to 99 in two directions
    switch (value) {
    case 0:
        // Detect rotation direction
        if ( prev_val > 50 && prev_val <= 99 ) {
            direction = MRR_UP_CW;
        }
        else {
            direction = MRR_DOWN_CCW;
        }
        break;
    case 99:
        // Detect rotation direction
        if ( prev_val >= 0 && prev_val < 50 ) {
            direction = MRR_DOWN_CCW;
        }
        else {
            direction = MRR_UP_CW;
        }
        break;
    default:
        if ( value > prev_val ) {
            direction = MRR_UP_CW;
        }
        else {
            direction = MRR_DOWN_CCW;
        }
        break;
    }

    if ( direction == MRR_UP_CW ) {
        nudgeFrequency(F_UP, RIG_VFO_B);
    }
    else {
        // Down/CCW rotation
        nudgeFrequency(F_DN, RIG_VFO_B);
    }

    // QString s = direction ? "UP" : "DOWN";
    // qDebug() << "direction:" << s << "step:" << value - prev_val << "value:" << value << "prev_val:" << prev_val;
    prev_val = value;
}

void MainWindow::on_vfoB_fastTune_pButton_clicked()
{
    vfoB_fast_toggle = !vfoB_fast_toggle;

    if ( vfoB_fast_toggle == true )
        ui->vfoB_FastLabel->setText("Fast");
    else
        ui->vfoB_FastLabel->setText("Norm");
}

void MainWindow::on_split_pButton_clicked()
{
    int rc;
    freq_t f;
    HamlibConnector &hl = *hamlib_p;

    // Toggle split mode setting
    my_split = my_split == RIG_SPLIT_OFF ? RIG_SPLIT_ON : RIG_SPLIT_OFF;

    if ( my_split == RIG_SPLIT_ON ) {
        // First set A -> B
        hl.mrr_a_2_b();

        // Now set B to 1 KHz UP
        f = hl.mrrGetCachedFreqA();  // Get our current cached value
        qDebug() << "MainWindow::on_split_pButton_clicked(): current freq is " << f << "setting VFO_B to" << f-1e3;
        rc = hl.mrrSetRigFreqB(f-1e3);
        if ( rc != RIG_OK ) {
            qDebug() << "MainWindow::on_split_pButton_clicked(): set frequency failed: " << rc << hl.getRigError(rc);
            return;
        }

        // Update the Freq B display
        QString str_tmp = HamlibConnector::get_display_frequency(f);
        ui->freqBDisplay->display(str_tmp);

        // Enable split mode
        rc = hamlib_p->mrrRigSetSplitVfo(true);
        if ( rc != RIG_OK ) {
            qDebug() << "MainWindow::on_split_pButton_clicked(): set split mode failed: " << rc << hl.getRigError(rc);
            return;
        }

        // Update RX/TX Icons
        ui->vfoB_TX_Label->setText("TX");
        ui->vfoB_TX_Label->setStyleSheet("QLabel { color : red; }");
        ui->vfoA_TX_Label->setStyleSheet("QLabel { color : black; }");
        update();
    }

    if ( my_split == RIG_SPLIT_OFF ) {
        // ui->vfoB_TX_Label->setText("");
        ui->vfoA_TX_Label->setStyleSheet("QLabel { color : red; }");
        ui->vfoB_TX_Label->setStyleSheet("QLabel { color : black; }");
        rc = hamlib_p->mrrRigSetSplitVfo(false);
        if ( rc != RIG_OK ) {
            qDebug() << "MainWindow::on_split_pButton_clicked(): cancel split mode failed: " << rc << hl.getRigError(rc);
            return;
        }
    }
}

