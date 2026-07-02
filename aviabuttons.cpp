#include "aviabuttons.h"

// =========================================================
// Реализация AviaButtonBase
// =========================================================
AviaButtonBase::AviaButtonBase(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(105, 72);
    setCursor(Qt::PointingHandCursor);
    setCheckable(true); // Включаем режим триггера (фиксируемой кнопки) из коробки
}

void AviaButtonBase::drawBase(QPainter &p) {
    double w = width();
    double h = height();
    bool checked = isChecked(); // Используем метод встроенный в QPushButton

    QRectF outer(1.0, 1.0, w - 2.0, h - 2.0);
    QLinearGradient body_grad(0.0, 0.0, 0.0, h);

    if (checked) {
        body_grad.setColorAt(0.0, QColor(0x14191d));
        body_grad.setColorAt(0.5, QColor(0x101417));
        body_grad.setColorAt(1.0, QColor(0x090b0d));
    } else {
        body_grad.setColorAt(0.0, QColor(0x2a3136));
        body_grad.setColorAt(0.45, QColor(0x1c2226));
        body_grad.setColorAt(1.0, QColor(0x111518));
    }

    p.setPen(QPen(QColor(0x050607), 3));
    p.setBrush(QBrush(body_grad));
    p.drawRoundedRect(outer, 9.0, 9.0);

    QRectF inner(5.0, 5.0, w - 10.0, h - 10.0);

    if (checked) {
        p.setPen(QPen(QColor(0x080a0c), 2));
    } else {
        p.setPen(QPen(QColor(0x3a444b), 1));
    }

    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(inner, 7.0, 7.0);

    if (!checked) {
        p.setPen(QPen(QColor(255, 255, 255, 28), 1));
        p.drawLine(12.0, 7.0, w - 12.0, 7.0);
    }
}


// =========================================================
// Реализация AviaCallButton
// =========================================================
AviaCallButton::AviaCallButton(QWidget *parent, const QString &text)
    : QPushButton(text, parent) // Для QPushButton порядок остаётся (text, parent) — это норма
    , m_barsOn(false)
    , m_barsColor(GREEN_ON)
    , m_textBlinkEnabled(false)
    , m_textBlinkVisible(true)
    , m_blinkColor(ORANGE_ON)
{
    setFixedSize(105, 72);
    setCursor(Qt::PointingHandCursor);
    setCheckable(true);
}

void AviaCallButton::startTextBlink() { m_textBlinkEnabled = true; update(); }
void AviaCallButton::stopTextBlink() { m_textBlinkEnabled = false; m_textBlinkVisible = true; update(); }

void AviaCallButton::setBlinkVisible(bool state) {
    if (!m_textBlinkEnabled) return;
    m_textBlinkVisible = state;
    update();
}

void AviaCallButton::barsOn() { m_barsOn = true; update(); }
void AviaCallButton::barsOff() { m_barsOn = false; update(); }
void AviaCallButton::setBars(bool state) { m_barsOn = state; update(); }

void AviaCallButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();
    bool checked = isChecked();

    // ---------- Отрисовка корпуса ----------
    QRectF outer(1.0, 1.0, w - 2.0, h - 2.0);
    QLinearGradient body_grad(0.0, 0.0, 0.0, h);
    if (checked) {
        body_grad.setColorAt(0.0, QColor(0x14191d));
        body_grad.setColorAt(0.5, QColor(0x101417));
        body_grad.setColorAt(1.0, QColor(0x090b0d));
    } else {
        body_grad.setColorAt(0.0, QColor(0x2a3136));
        body_grad.setColorAt(0.45, QColor(0x1c2226));
        body_grad.setColorAt(1.0, QColor(0x111518));
    }
    p.setPen(QPen(QColor(0x050607), 3));
    p.setBrush(QBrush(body_grad));
    p.drawRoundedRect(outer, 9.0, 9.0);

    QRectF inner(5.0, 5.0, w - 10.0, h - 10.0);
    p.setPen(checked ? QPen(QColor(0x080a0c), 2) : QPen(QColor(0x3a444b), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(inner, 7.0, 7.0);

    if (!checked) {
        p.setPen(QPen(QColor(255, 255, 255, 28), 1));
        p.drawLine(12.0, 7.0, w - 12.0, 7.0);
    }

    // ---------- Три полоски ----------
    double bar_w = w * 0.42;
    double bar_h = 4.0;
    double bar_x = (w - bar_w) / 2.0;
    double bar_y0 = 13.0;
    double bar_gap = 7.0;

    if (m_barsOn) {
        QColor glow(m_barsColor.red(), m_barsColor.green(), m_barsColor.blue(), 90);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(glow));
        for (int i = 0; i < 3; ++i) {
            double y = bar_y0 + i * bar_gap;
            p.drawRoundedRect(QRectF(bar_x - 2.0, y - 1.0, bar_w + 4.0, bar_h + 2.0), 2.0, 2.0);
        }
        p.setBrush(QBrush(m_barsColor));
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0x4b5358)));
    }

    for (int i = 0; i < 3; ++i) {
        double y = bar_y0 + i * bar_gap;
        p.drawRoundedRect(QRectF(bar_x, y, bar_w, bar_h), 2.0, 2.0);
    }

    // ---------- Текст (Берем метод text() из QPushButton) ----------
    QString buttonText = text();
    if (!buttonText.trimmed().isEmpty()) {
        QFont font("Arial");
        font.setBold(true);
        font.setPointSize(15);
        p.setFont(font);

        QColor text_color;
        if (m_textBlinkEnabled) {
            text_color = m_textBlinkVisible ? m_blinkColor : QColor(0x3a2a18);
        } else {
            text_color = !checked ? QColor(0x4b5358) : QColor(0x6a737a);
        }

        p.setPen(text_color);
        QRectF text_rect(4.0, 34.0, w - 8.0, h - 36.0);
        p.drawText(text_rect, Qt::AlignCenter, buttonText);
    }
}


// =========================================================
// Реализация AviaTextButton
// =========================================================
AviaTextButton::AviaTextButton(QWidget *parent, const QString &text)
    : AviaButtonBase(parent)
{
    setText(text); // Используем стандартный метод QPushButton
}

void AviaTextButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBase(p);

    QFont font("Arial");
    font.setBold(true);
    font.setPointSize(15);
    p.setFont(font);
    p.setPen(TEXT_WHITE);

    QRectF text_rect(4.0, 34.0, width() - 8.0, height() - 36.0);
    p.drawText(text_rect, Qt::AlignCenter, text()); // Передаем встроенный метод text()
}


// =========================================================
// Реализация AviaDualTextButton
// =========================================================
AviaDualTextButton::AviaDualTextButton(QWidget *parent, const QString &top_text, const QString &bottom_text)
    : AviaButtonBase(parent)
    , m_topText(top_text)
    , m_bottomText(bottom_text)
    , m_topLight(false)
{}

void AviaDualTextButton::topOn(quint8 state) { m_topLight = bool(state); update(); }

void AviaDualTextButton::setTopText(const QString &text) { m_topText = text; update(); }
void AviaDualTextButton::setBottomText(const QString &text) { m_bottomText = text; update(); }

void AviaDualTextButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBase(p);

    double w = width();

    // ===== Верхняя надпись =====
    QFont top_font("Arial");
    top_font.setBold(true);
    top_font.setPointSize(15);
    p.setFont(top_font);

    p.setPen(m_topLight ? GREEN_ON : QColor(0x4b5358));
    QRectF top_rect(4.0, 10.0, w - 8.0, 18.0);
    p.drawText(top_rect, Qt::AlignCenter, m_topText);

    // ===== Нижняя надпись =====
    QFont bottom_font("Arial");
    bottom_font.setBold(true);
    bottom_font.setPointSize(15);
    p.setFont(bottom_font);

    p.setPen(TEXT_WHITE);
    QRectF bottom_rect(4.0, 34.0, w - 8.0, height() - 36.0);
    p.drawText(bottom_rect, Qt::AlignCenter, m_bottomText);
}