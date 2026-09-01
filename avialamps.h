#ifndef AVIALAMPS_H
#define AVIALAMPS_H

#include <QBrush>
#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRadialGradient>
#include <QRectF>
#include <QString>
#include <QWidget>

// Структура для возврата набора цветов радиального градиента круглой лампы
struct LampColors
{
    QColor c1;
    QColor c2;
    QColor c3;
    QColor glow;
    bool hasGlow;
};

// =========================================================
// КРУГЛАЯ АВИАЦИОННАЯ ЛАМПА
// =========================================================
class AviaLamp : public QWidget
{
    Q_OBJECT

public:
    explicit AviaLamp(QWidget *parent = nullptr);

    void setOn(bool state);
    bool isOn() const;
    void setColor(const QString &color); // "green" или "red"

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    LampColors getColors() const;

    bool m_on;
    QString m_color;
};

// =========================================================
// ПРЯМОУГОЛЬНАЯ СИГНАЛЬНАЯ ЛАМПА
// =========================================================
class AviaRectLamp : public QWidget
{
    Q_OBJECT

public:
    explicit AviaRectLamp(QWidget *parent = nullptr);

    void on();
    void off();
    void setState(bool state);
    bool isOn() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_on;
    QColor m_colorOn;
    QColor m_colorOff;
};

// =========================================================
// КНОПКА С ИНДИКАТОРНОЙ ЛАМПОЙ
// =========================================================
class AviaLampToggleButton : public QPushButton
{
    Q_OBJECT

public:
    explicit AviaLampToggleButton(QWidget *parent = nullptr);

    void lampOn();
    void lampOff();
    void setLamp(bool state);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_lampOn;
    QColor m_lampColor;
};

#endif // AVIALAMPS_H
