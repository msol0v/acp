#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QThread>
#include <QVector>
#include <QLabel>

#include "aviaknob.h"
#include "aviabuttons.h"
#include "arinchandler.h"

class ChannelWidgetWrapper : QObject{
    Q_OBJECT
public:
    AviaKnob *knob;
    QLabel *label;

    explicit ChannelWidgetWrapper(
        AviaKnob *chan_knob, QLabel *chan_label, int start_point, QObject *parent=nullptr)
        : QObject(parent)
    {
        knob = chan_knob;
        label = chan_label;
        knob->setRange(start_point, 255);
    }
private:
    int start_point;
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /*Обработчик и поток для канала ARINC*/
    ArincHandler *handler_arinc;
    QThread *thread_arinc;

private:
    Ui::MainWindow *ui;

    QHash<const QString, ChannelWidgetWrapper*> chan_wrappers_table;
    QVector<AviaCallButton*> tx_code_btns_table;

public slots:
    void showErrors(QVector<quint8> errors);
    void showChanStates(const QString chan_name, quint8 chan_state, quint8 volume_state);
    void showTxCode(quint8 tx_code);
    void toggledBtn(bool state);

signals:
    void sigChangeAmuChannel(quint8 tx_code);
};
#endif // MAINWINDOW_H
