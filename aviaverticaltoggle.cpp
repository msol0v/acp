#include "aviaverticaltoggle.h"

AviaVerticalToggle::AviaVerticalToggle(QWidget *parent)
    : QWidget(parent)
    , m_position(0)
    , m_circlePos(27) // 27 — координата центральной позиции Y
{
    setFixedSize(50, 90);
    setCursor(Qt::PointingHandCursor);

    m_positions[0] = 27; // Центр
    m_positions[1] = 50; // Низ
    m_positions[2] = 4;  // Верх

    // Настраиваем анимацию
    m_animation = new QPropertyAnimation(this, "circlePos", this);
    m_animation->setDuration(150);
}

int AviaVerticalToggle::circlePos() const
{
    return m_circlePos;
}

void AviaVerticalToggle::setCirclePos(int pos)
{
    m_circlePos = pos;
    update();
}

int AviaVerticalToggle::position() const
{
    return m_position;
}

void AviaVerticalToggle::setPosition(int position)
{
    // Ограничиваем рамками [0, 2]
    position = std::max(0, std::min(2, position));

    if (position == m_position) {
        return;
    }

    m_position = position;

    // Запуск плавной анимации переключения
    m_animation->stop();
    m_animation->setStartValue(m_circlePos);
    m_animation->setEndValue(m_positions[position]);
    m_animation->start();

    emit positionChanged(position);
    update();
}

void AviaVerticalToggle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Циклически переключаем: 0 (центр) -> 1 (низ) -> 2 (верх) -> 0 (центр)
        setPosition((m_position + 1) % 3);
    }
    QWidget::mousePressEvent(event);
}

void AviaVerticalToggle::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg_color(0x151719);
    QColor border_color(0x050607);
    QColor accent(0xd8c89f);

    // Отрисовка корпуса переключателя
    painter.setPen(QPen(border_color, 3));
    painter.setBrush(QBrush(bg_color));
    painter.drawRoundedRect(2, 2, 46, 86, 20, 20);

    // Подсветка рамки
    if (m_position != 1) {
        painter.setPen(QPen(accent, 2));
        painter.drawRoundedRect(5, 5, 40, 80, 18, 18);
    }

    // Отрисовка круглого тумблера
    painter.setPen(QPen(QColor(0x050607), 2));
    painter.setBrush(QBrush(QColor(0xe8dcc0)));
    painter.drawEllipse(QRect(7, m_circlePos, 36, 36));

    // Отрисовка горизонтальной полосы-индикатора на тумблере
    painter.setPen(QPen(QColor(0xffffff), 3, Qt::SolidLine, Qt::RoundCap));
    int y = m_circlePos + 18;
    painter.drawLine(18, y, 32, y);
}