#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QButtonGroup>
#include "aviabuttons.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    thread_arinc = new QThread(this);
    thread_arinc->setObjectName("ArincThread");
    handler_arinc = new ArincHandler();
    handler_arinc->moveToThread(thread_arinc);
    connect(thread_arinc, &QThread::started, handler_arinc, &ArincHandler::start);

    //Arinc Handler signals
    connect(handler_arinc, &ArincHandler::sigShowErrors, this, &MainWindow::showErrors);
    connect(handler_arinc, &ArincHandler::sigShowPN, ui->labelPN, &QLabel::setText);
    connect(handler_arinc, &ArincHandler::sigShowChanState, this, &MainWindow::showChanStates);
    connect(handler_arinc,
            &ArincHandler::sigShowIntRadio,
            ui->intRadTogglePlaceholder,
            &AviaVerticalToggle::setPosition);
    connect(handler_arinc, &ArincHandler::sigReset, ui->btnReset, &AviaTextButton::setChecked);
    connect(handler_arinc, &ArincHandler::sigVoice, ui->btnVoice, &AviaDualTextButton::topOn);
    connect(handler_arinc, &ArincHandler::sigShowTxCode, this, &MainWindow::showTxCode);

    //MainW singals
    connect(this, &MainWindow::sigChangeSelCal, handler_arinc, &ArincHandler::changeSelCal);
    connect(this, &MainWindow::sigChangePinProg, handler_arinc, &ArincHandler::changePinProg);
    connect(this, &MainWindow::sigChangeMech, handler_arinc, &ArincHandler::changeMechState);
    connect(this, &MainWindow::sigChangeAtt, handler_arinc, &ArincHandler::changeAttState);
    connect(this, &MainWindow::sigChangeAmuChannel, handler_arinc, &ArincHandler::changeAmuChannel);
    connect(ui->checkBox, &QCheckBox::toggled, handler_arinc, &ArincHandler::arincTxEnable);

    //UI slots
    btnGroup = new QButtonGroup(this);
    quint8 id = 0;
    for (auto btn : findChildren<AviaLampToggleButton *>()) {
        btnGroup->addButton(btn, id++);
    }
    btnGroup->setExclusive(false);
    connect(btnGroup, &QButtonGroup::buttonToggled, this, &MainWindow::toggledBtn);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::txComboBoxCallback);

    chan_wrappers_table = {
        {"VHF1", new ChannelWidgetWrapper(ui->dialVHF1, ui->volVHF1, 8, this)},
        {"VHF2", new ChannelWidgetWrapper(ui->dialVHF2, ui->volVHF2, 8, this)},
        {"VHF3", new ChannelWidgetWrapper(ui->dialVHF3, ui->volVHF3, 8, this)},
        {"HF1", new ChannelWidgetWrapper(ui->dialHF1, ui->volHF1, 8, this)},
        {"HF2", new ChannelWidgetWrapper(ui->dialHF2, ui->volHF2, 8, this)},
        {"INT", new ChannelWidgetWrapper(ui->dialINT, ui->volINT, 0, this)},
        {"CAB", new ChannelWidgetWrapper(ui->dialCAB, ui->volCAB, 0, this)},
        {"PA", new ChannelWidgetWrapper(ui->dialPA, ui->volPA, 0, this)},
        {"VOR1", new ChannelWidgetWrapper(ui->dialVOR1, ui->volVOR1, 0, this)},
        {"VOR2", new ChannelWidgetWrapper(ui->dialVOR2, ui->volVOR2, 0, this)},
        {"MKR", new ChannelWidgetWrapper(ui->dialMKR, ui->volMKR, 0, this)},
        {"LS", new ChannelWidgetWrapper(ui->dialLS, ui->volLS, 0, this)},
        {"MLS", new ChannelWidgetWrapper(ui->dialMLS, ui->volMLS, 0, this)},
        {"ADF1", new ChannelWidgetWrapper(ui->dialADF1, ui->volADF1, 0, this)},
        {"ADF2", new ChannelWidgetWrapper(ui->dialADF2, ui->volADF2, 0, this)},
    };

    tx_code_btns_table = {ui->btnCall1,
                          ui->btnCall2,
                          ui->btnCall3,
                          ui->btnCall4,
                          ui->btnCall5,
                          ui->btnMech,
                          ui->btnAtt,
                          ui->btnPA};

    // BLINK CALL BUTTONS
    blink_state = true;
    blink_timer = new QTimer;
    blink_timer->setInterval(250);
    connect(blink_timer, &QTimer::timeout, this, [&]() {
        blink_state = !blink_state;
        emit btnBlink(blink_state);
    });
    for (AviaCallButton *btnCall : tx_code_btns_table) {
        connect(this, &MainWindow::btnBlink, btnCall, &AviaCallButton::setBlinkVisible);
    }

    // VALPP валидация пин программ видмо 0 - ок / 1 - не ок. Включаю сразу
    //ui->lampPlaceholder_7->lampOn();

    blink_timer->start();

    thread_arinc->start();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showErrors(QVector<quint8> errors)
{
    for (char i = 0; i < 16; ++i) {
        QString lampName = QString("lampPlaceholder_%1").arg(i + 19);
        AviaRectLamp *lamp = this->findChild<AviaRectLamp *>(lampName);
        errors[i] ? lamp->on() : lamp->off();
    }

    for (char i = 16; i < 20; ++i) {
        QString lampName = QString("lampPlaceholder_%1").arg(i - 1);
        AviaLamp *lamp = this->findChild<AviaLamp *>(lampName);
        lamp->setOn(errors[i]);
    }
}

void MainWindow::showChanStates(const QString chan_name, quint8 chan_state, quint8 volume_state)
{
    ChannelWidgetWrapper *wrapper = chan_wrappers_table.value(chan_name);
    chan_state ? wrapper->knob->on() : wrapper->knob->off();
    wrapper->knob->setValue(volume_state);
    wrapper->label->setText(QString::number(volume_state));
}

void MainWindow::toggledBtn(QAbstractButton *button, bool checked)
{
    AviaLampToggleButton *btn = qobject_cast<AviaLampToggleButton *>(button);
    checked ? btn->lampOn() : btn->lampOff();
    int id_btn = btnGroup->id(btn);
    if (id_btn > 6) {                              //Calls
        // if (id_btn > 8 && id_btn < 12) {
        //     if (!(btnGroup->button(id_btn - 9))->isChecked())
        //         return; // Если пин прог этого канала не выставлен, выходим, мигание вызова на АСР не нужно
        // }
        emit sigChangeSelCal(id_btn - 7, checked);
        AviaCallButton *call_btn = tx_code_btns_table[id_btn - 7];
        checked ? call_btn->startTextBlink() : call_btn->stopTextBlink();

        if (id_btn == 12) {
            emit sigChangeMech(static_cast<quint8>(checked));
            return;
        }
        if (id_btn == 13) {
            emit sigChangeAtt(static_cast<quint8>(checked));
            return;
        }
    } else {
        emit sigChangePinProg(id_btn, checked);
    }
}
static quint8 last_code = 0;
void MainWindow::txComboBoxCallback(quint32 tx_code)
{
    if (last_code == tx_code)
        return;

    if (last_code > 0)
        tx_code_btns_table[last_code - 1]->barsOff();

    if (tx_code > 0)
        tx_code_btns_table[tx_code - 1]->barsOn();

    last_code = tx_code;
    emit sigChangeAmuChannel(tx_code);
}

void MainWindow::showTxCode(quint8 tx_code)
{

    if (last_code == tx_code)
        return;

    if (last_code > 0)
        tx_code_btns_table[last_code - 1]->barsOff();

    if (tx_code > 0)
        tx_code_btns_table[tx_code - 1]->barsOn();

    last_code = tx_code;
    ui->comboBox->setCurrentIndex(tx_code);
    emit sigChangeAmuChannel(tx_code);
}
