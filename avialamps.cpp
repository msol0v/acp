#include "avialamps.h"

// =========================================================
// Реализация AviaLamp (Круглая лампа)
// =========================================================
AviaLamp::AviaLamp(QWidget *parent)
    : QWidget(parent)
    , m_on(false)
    , m_color("green")
{
    setFixedSize(50, 50);
}

void AviaLamp::setOn(bool state) {
    if (m_on == state) return;
    m_on = state;
    update();
}

bool AviaLamp::isOn() const { return m_on; }

void AviaLamp::setColor(const QString &color) {
    if (m_color == color) return;
    m_color = color;
    update();
}

void AviaLamp::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

LampColors AviaLamp::getColors() const {
    if (m_color == "red") {
        if (m_on) {
            return { QColor(0xff5a4f), QColor(0xa8241c), QColor(0x2a0a08), QColor(255, 80, 60, 60), true };
        } else {
            return { QColor(0x3a1a18), QColor(0x1a0a09), QColor(0x070303), QColor(), false };
        }
    } else { // green
        if (m_on) {
            return { QColor(0x6cff8f), QColor(0x1f8f4a), QColor(0x0a2a16), QColor(80, 255, 140, 60), true };
        } else {
            return { QColor(0x2a332d), QColor(0x121615), QColor(0x070909), QColor(), false };
        }
    }
}

void AviaLamp::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();
    double size = std::min(w, h);
    double cx = w / 2.0;
    double cy = h / 2.0;

    // Корпус
    QRectF outer(cx - size * 0.48, cy - size * 0.48, size * 0.96, size * 0.96);
    p.setPen(QPen(QColor(0x050607), 3));
    p.setBrush(QBrush(QColor(0x0a0c0e)));
    p.drawEllipse(outer);

    // Фаска
    QRectF bevel(cx - size * 0.40, cy - size * 0.40, size * 0.80, size * 0.80);
    p.setPen(QPen(QColor(0x22272b), 2));
    p.setBrush(QBrush(QColor(0x14181c)));
    p.drawEllipse(bevel);

    // Палитра лампы
    LampColors colors = getColors();

    // Сама линза лампы
    QRectF lamp_rect(cx - size * 0.32, cy - size * 0.32, size * 0.64, size * 0.64);
    QRadialGradient grad(cx, cy, size * 0.35);
    grad.setColorAt(0.0, colors.c1);
    grad.setColorAt(0.5, colors.c2);
    grad.setColorAt(1.0, colors.c3);

    p.setPen(QPen(QColor(0x020303), 2));
    p.setBrush(QBrush(grad));
    p.drawEllipse(lamp_rect);

    // Внешнее свечение (glow)
    if (m_on && colors.hasGlow) {
        QRadialGradient glow(cx, cy, size * 0.55);
        glow.setColorAt(0.0, colors.glow);
        glow.setColorAt(1.0, QColor(0, 0, 0, 0));

        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(glow));
        p.drawEllipse(QRectF(cx - size * 0.55, cy - size * 0.55, size * 1.10, size * 1.10));
    }
}


// =========================================================
// Реализация AviaRectLamp (Прямоугольное табло)
// =========================================================
AviaRectLamp::AviaRectLamp(QWidget *parent)
    : QWidget(parent)
    , m_on(false)
    , m_colorOn(0xff3b2f)
    , m_colorOff(0x3a0d0d)
{
    setFixedSize(59, 30);
}

void AviaRectLamp::on() { m_on = true; update(); }
void AviaRectLamp::off() { m_on = false; update(); }
void AviaRectLamp::setState(bool state) { m_on = state; update(); }
bool AviaRectLamp::isOn() const { return m_on; }

void AviaRectLamp::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();

    // Внешний корпус
    QRectF outer(0.0, 0.0, w, h);
    p.setPen(QPen(QColor(0x050607), 3));
    p.setBrush(QBrush(QColor(0x0c0f12)));
    p.drawRoundedRect(outer, 10.0, 10.0);

    // Внутренняя рамка
    QRectF inner_frame(4.0, 4.0, w - 8.0, h - 8.0);
    p.setPen(QPen(QColor(0x1a1f24), 2));
    p.setBrush(QBrush(QColor(0x111417)));
    p.drawRoundedRect(inner_frame, 8.0, 8.0);

    // Подготовка прозрачности линзы
    QRectF lamp_rect(8.0, 8.0, w - 16.0, h - 16.0);
    int center_alpha = m_on ? 240 : 80;
    int mid_alpha    = m_on ? 170 : 40;
    int edge_alpha   = m_on ? 60  : 10;
    QColor base_color = m_on ? m_colorOn : m_colorOff;

    QRadialGradient grad(lamp_rect.center(), std::max(lamp_rect.width(), lamp_rect.height()) * 0.7);
    grad.setColorAt(0.0, QColor(base_color.red(), base_color.green(), base_color.blue(), center_alpha));
    grad.setColorAt(0.5, QColor(base_color.red(), base_color.green(), base_color.blue(), mid_alpha));
    grad.setColorAt(1.0, QColor(base_color.red(), base_color.green(), base_color.blue(), edge_alpha));

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(grad));
    p.drawRoundedRect(lamp_rect, 6.0, 6.0);

    // Лёгкий внешний glow табло (только в активном состоянии)
    if (m_on) {
        QRadialGradient glow(lamp_rect.center(), std::max(w, h));
        glow.setColorAt(0.0, QColor(255, 60, 40, 60));
        glow.setColorAt(1.0, QColor(0, 0, 0, 0));

        p.setBrush(QBrush(glow));
        p.drawRoundedRect(outer, 10.0, 10.0);
    }
}


// =========================================================
// Реализация AviaLampToggleButton (Кнопка со встроенной лампой)
// =========================================================
AviaLampToggleButton::AviaLampToggleButton(QWidget *parent)
    : QPushButton(parent)
    , m_lampOn(false)
    , m_lampColor(0x6cff8f)
{
    setFixedSize(80, 50);
    setCursor(Qt::PointingHandCursor);
    setCheckable(true); // Родной QPushButton берёт на себя всю работу с m_checked
}

void AviaLampToggleButton::lampOn() { m_lampOn = true; update(); }
void AviaLampToggleButton::lampOff() { m_lampOn = false; update(); }
void AviaLampToggleButton::setLamp(bool state) { m_lampOn = state; update(); }

void AviaLampToggleButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();
    bool checked = isChecked(); // Получаем состояние фиксации из QPushButton

    // ===== КОРПУС =====
    QColor bg = checked ? QColor(0x1e252b) : QColor(0x3a4650);
    QRectF outer(1.0, 1.0, w - 2.0, h - 2.0);

    p.setPen(QPen(QColor(0x050607), 3));
    p.setBrush(QBrush(bg));
    p.drawRoundedRect(outer, 12.0, 12.0);

    // Внутренняя фаска кнопочного механизма
    QRectF inner(5.0, 5.0, w - 10.0, h - 10.0);
    p.setPen(QPen(QColor(0x4a5862), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(inner, 9.0, 9.0);

    // Внутреннее радиальное затенение кнопки
    QRectF shadow(4.0, 4.0, w - 8.0, h - 8.0);
    QRadialGradient shadow_grad(shadow.center(), std::max(w, h));
    shadow_grad.setColorAt(0.0, QColor(0, 0, 0, 120));
    shadow_grad.setColorAt(1.0, QColor(0, 0, 0, 0));

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(shadow_grad));
    p.drawRoundedRect(shadow, 10.0, 10.0);

    // ===== ВСТРОЕННАЯ ЛАМПА ИНДИКАТОРА =====
    double lamp_size = 18.0;
    double lx = w - 28.0;
    double ly = (h - lamp_size) / 2.0;

    double cx = lx + lamp_size / 2.0;
    double cy = ly + lamp_size / 2.0;

    // Посадочное кольцо (ободок лампы)
    p.setPen(QPen(QColor(0x050607), 1));
    p.setBrush(QBrush(QColor(0x0a0c0e)));
    p.drawEllipse(QRectF(lx - 3.0, ly - 3.0, lamp_size + 6.0, lamp_size + 6.0));

    // Свечение/линза встроенного индикатора
    QRadialGradient lamp_grad(cx, cy, lamp_size * 0.9);
    if (m_lampOn) {
        lamp_grad.setColorAt(0.0, QColor(108, 255, 143, 255));
        lamp_grad.setColorAt(0.5, QColor(40, 180, 90, 160));
        lamp_grad.setColorAt(1.0, QColor(10, 40, 20, 20));
    } else {
        lamp_grad.setColorAt(0.0, QColor(0x2a332d));
        lamp_grad.setColorAt(0.6, QColor(0x121615));
        lamp_grad.setColorAt(1.0, QColor(0x070909));
    }

    p.setPen(QPen(QColor(0x020303), 1));
    p.setBrush(QBrush(lamp_grad));
    p.drawEllipse(QRectF(lx, ly, lamp_size, lamp_size));
}