#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "aviabuttons.h"
#include <QButtonGroup>

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
    connect(handler_arinc, &ArincHandler::sigShowIntRadio, ui->intRadTogglePlaceholder,
            &AviaVerticalToggle::setPosition);
    connect(handler_arinc, &ArincHandler::sigReset, ui->btnReset, &AviaTextButton::setChecked);
    connect(handler_arinc, &ArincHandler::sigVoice, ui->btnVoice, &AviaDualTextButton::topOn);

    //MainW singals
    connect(this, &MainWindow::sigChangeAmuChannel, handler_arinc,&ArincHandler::changeAmuChannel);

    //UI slots
    QButtonGroup *btnGroup = new QButtonGroup(this);
    for (auto btn: findChildren<AviaLampToggleButton*>())
        btnGroup->addButton(btn);
    connect(btnGroup, &QButtonGroup::buttonToggled, this, &MainWindow::toggledBtn);

    chan_wrappers_table = {
        {"VHF1", new ChannelWidgetWrapper(ui->dialVHF1, ui->volVHF1, 8,this)},
        {"VHF2", new ChannelWidgetWrapper(ui->dialVHF2, ui->volVHF2, 8,this)},
        {"VHF3", new ChannelWidgetWrapper(ui->dialVHF3, ui->volVHF3, 8,this)},
        {"HF1", new ChannelWidgetWrapper(ui->dialHF1, ui->volHF1, 8,this)},
        {"HF2", new ChannelWidgetWrapper(ui->dialHF2, ui->volHF2, 8,this)},
        {"INT", new ChannelWidgetWrapper(ui->dialINT, ui->volINT, 0,this)},
        {"CAB", new ChannelWidgetWrapper(ui->dialCAB, ui->volCAB, 0,this)},
        {"PA", new ChannelWidgetWrapper(ui->dialPA, ui->volPA, 0,this)},
        {"VOR1", new ChannelWidgetWrapper(ui->dialVOR1, ui->volVOR1, 0,this)},
        {"VOR2", new ChannelWidgetWrapper(ui->dialVOR2, ui->volVOR2, 0,this)},
        {"MKR", new ChannelWidgetWrapper(ui->dialMKR, ui->volMKR, 0,this)},
        {"LS", new ChannelWidgetWrapper(ui->dialLS, ui->volLS, 0,this)},
        {"MLS", new ChannelWidgetWrapper(ui->dialMLS, ui->volMLS, 0,this)},
        {"ADF1", new ChannelWidgetWrapper(ui->dialADF1, ui->volADF1, 0,this)},
        {"ADF2", new ChannelWidgetWrapper(ui->dialADF2, ui->volADF2, 0,this)},
    };

    tx_code_btns_table = {
        ui->btnCall1,
        ui->btnCall2,
        ui->btnCall3,
        ui->btnCall4,
        ui->btnCall5,
        ui->btnMech,
        ui->btnAtt,
        ui->btnPA
    };

    thread_arinc->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showErrors(QVector<quint8> errors){
    for (char i = 0; i < 16; ++i){
        QString lampName = QString("lampPlaceholder_%1").arg(i+19);
        AviaRectLamp *lamp = this->findChild<AviaRectLamp*>(lampName);
        errors[i] ? lamp->on() : lamp->off();
    }

    for (char i = 16; i < 20; ++i){
        QString lampName = QString("lampPlaceholder_%1").arg(i-1);
        AviaLamp *lamp = this->findChild<AviaLamp*>(lampName);
        lamp->setOn(errors[i]);
    }
}

void MainWindow::showChanStates(const QString chan_name, quint8 chan_state, quint8 volume_state){
    ChannelWidgetWrapper *wrapper = chan_wrappers_table.value(chan_name);
    chan_state ? wrapper->knob->on() : wrapper->knob->off();
    wrapper->knob->setValue(volume_state);
    wrapper->label->setText(QString::number(volume_state));
}

void MainWindow::toggledBtn(bool state){
    qDebug() << "state" << state;
}

void MainWindow::showTxCode(quint8 tx_code){
    static quint8 last_code = 0;

    if (last_code = tx_code)
        return;

    if (last_code > 0)
        tx_code_btns_table[last_code - 1]->barsOff();

    if (tx_code > 0)
        tx_code_btns_table[tx_code - 1]->barsOn();

    last_code = tx_code;
    ui->comboBox->setCurrentIndex(tx_code);
    emit sigChangeAmuChannel(tx_code); //TODO
}

