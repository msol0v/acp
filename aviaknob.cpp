#include "aviaknob.h"

AviaKnob::AviaKnob(QWidget *parent)
    : QDial(parent)
    , m_startAngle(135.0)
    , m_spanAngle(270.0)
    , m_lightOn(false)
    , m_lightColor("#8b7cff")
{
    setRange(0, 255);
    setValue(0);

    setNotchesVisible(false);
    setWrapping(false);
    setFixedSize(78, 78);
}

void AviaKnob::setRange(int minimum, int maximum) {
    QDial::setRange(minimum, maximum);

    if (value() < minimum) {
        setValue(minimum);
    }
    if (value() > maximum) {
        setValue(maximum);
    }
    update();
}

void AviaKnob::setStartValue() {
    setValue(minimum());
    update();
}

void AviaKnob::setAngleRange(double start_angle, double span_angle) {
    m_startAngle = start_angle;
    m_spanAngle = span_angle;
    update();
}

double AviaKnob::valueRatio() const {
    int minVal = minimum();
    int maxVal = maximum();
    int val = value();

    if (maxVal == minVal) {
        return 0.0;
    }

    // Ограничиваем значение в рамках [minimum, maximum]
    val = std::max(minVal, std::min(val, maxVal));
    return static_cast<double>(val - minVal) / (maxVal - minVal);
}

void AviaKnob::on(const QColor &color) {
    m_lightOn = true;
    if (color.isValid()) {
        m_lightColor = color;
    }
    update();
}

void AviaKnob::off() {
    m_lightOn = false;
    update();
}

void AviaKnob::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();
    double size = std::min(w, h);
    double cx = w / 2.0;
    double cy = h / 2.0;

    double value_ratio = valueRatio();
    double angle = m_startAngle + m_spanAngle * value_ratio;

    double tick_radius_outer = size * 0.48;
    double tick_radius_inner = size * 0.41;

    p.setPen(QPen(QColor("#9aa6ad"), 1));

    // Отрисовка засечек (тиков)
    for (int i = 0; i < 23; ++i) {
        double a = m_startAngle + m_spanAngle * i / 22.0;
        double rad = a * M_PI / 180.0; // Перевод градусов в радианы

        double x1 = cx + tick_radius_inner * std::cos(rad);
        double y1 = cy + tick_radius_inner * std::sin(rad);
        double x2 = cx + tick_radius_outer * std::cos(rad);
        double y2 = cy + tick_radius_outer * std::sin(rad);

        p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    // Внешняя окружность ручки
    QRectF outer(cx - size * 0.39, cy - size * 0.39, size * 0.78, size * 0.78);
    p.setPen(QPen(QColor("#050607"), 4));
    p.setBrush(QBrush(QColor("#070809")));
    p.drawEllipse(outer);

    // Фаска ручки
    QRectF bevel(cx - size * 0.34, cy - size * 0.34, size * 0.68, size * 0.68);
    p.setPen(QPen(QColor("#22272b"), 3));
    p.setBrush(QBrush(QColor("#111417")));
    p.drawEllipse(bevel);

    // Основное тело ручки с градиентом объема
    QRectF body(cx - size * 0.29, cy - size * 0.29, size * 0.58, size * 0.58);
    QRadialGradient grad(QPointF(cx - size * 0.10, cy - size * 0.13), size * 0.38);
    grad.setColorAt(0.0, QColor("#3a3f43"));
    grad.setColorAt(0.55, QColor("#15191c"));
    grad.setColorAt(1.0, QColor("#050607"));

    p.setPen(QPen(QColor("#030405"), 2));
    p.setBrush(QBrush(grad));
    p.drawEllipse(body);

    // Отрисовка подсветки, если включена
    if (m_lightOn) {
        QRectF light(cx - size * 0.27, cy - size * 0.27, size * 0.54, size * 0.54);
        QRadialGradient light_grad(QPointF(cx, cy), size * 0.30);

        light_grad.setColorAt(0.0, QColor(m_lightColor.red(), m_lightColor.green(), m_lightColor.blue(), 230));
        light_grad.setColorAt(0.45, QColor(m_lightColor.red(), m_lightColor.green(), m_lightColor.blue(), 150));
        light_grad.setColorAt(0.8, QColor(m_lightColor.red(), m_lightColor.green(), m_lightColor.blue(), 70));
        light_grad.setColorAt(1.0, QColor(0, 0, 0, 0));

        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(light_grad));
        p.drawEllipse(light);
    }

    // Белая стрелка-указатель положения ручки
    double rad = angle * M_PI / 180.0;
    double r1 = size * 0.08;
    double r2 = size * 0.24;

    double x1 = cx + r1 * std::cos(rad);
    double y1 = cy + r1 * std::sin(rad);
    double x2 = cx + r2 * std::cos(rad);
    double y2 = cy + r2 * std::sin(rad);

    p.setPen(QPen(QColor("#f4f0e8"), 7, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
}
