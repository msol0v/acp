#ifndef AVIABUTTONS_H
#define AVIABUTTONS_H

#include <QPushButton>
#include <QColor>
#include <QString>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QFont>
#include <QRectF>

// Глобальные константы цветов
const QColor GREEN_ON("#20ff4f");
const QColor ORANGE_ON("#ffb000");
const QColor ORANGE_DIM("#4a2b08");
const QColor TEXT_DIM("#4b5358");
const QColor TEXT_DIM_CHECKED("#6a737a");
const QColor TEXT_WHITE("#f2f2f2");

// =========================================================
// БАЗОВАЯ ОТРИСОВКА КОРПУСА
// =========================================================
class AviaButtonBase : public QPushButton {
    Q_OBJECT

public:
    explicit AviaButtonBase(QWidget *parent = nullptr);

protected:
    void drawBase(QPainter &p);
};

// =========================================================
// КНОПКА С ТРЕМЯ ПОЛОСКАМИ И МИГАНИЕМ
// =========================================================
class AviaCallButton : public QPushButton {
    Q_OBJECT

public:
    explicit AviaCallButton(QWidget *parent = nullptr, const QString &text = "");

    void startTextBlink();
    void stopTextBlink();
    void setBlinkVisible(bool visible);

    void barsOn();
    void barsOff();
    void setBars(bool state);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_barsOn;
    QColor m_barsColor;

    bool m_textBlinkEnabled;
    bool m_textBlinkVisible;
    QColor m_blinkColor;
};

// =========================================================
// ВАРИАНТ 1: Только нижняя надпись
// =========================================================
class AviaTextButton : public AviaButtonBase {
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    explicit AviaTextButton(QWidget *parent = nullptr, const QString &text = "CALL");
};

// =========================================================
// ВАРИАНТ 2: Верхняя светящаяся надпись + нижняя статичная
// =========================================================
class AviaDualTextButton : public AviaButtonBase {
    Q_OBJECT

public:
    explicit AviaDualTextButton(QWidget *parent = nullptr,
                                const QString &top_text = "ON",
                                const QString &bottom_text = "VOICE");

    void setTopText(const QString &text);
    void setBottomText(const QString &text);
public slots:
    void topOn(quint8 state);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_topText;
    QString m_bottomText;
    bool m_topLight;
};

#endif // AVIABUTTONS_H
