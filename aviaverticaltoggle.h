#ifndef AVIAVERTICALTOGGLE_H
#define AVIAVERTICALTOGGLE_H

#include <QBrush>
#include <QColor>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QRect>
#include <QWidget>

class AviaVerticalToggle : public QWidget
{
    Q_OBJECT

    // Регистрируем свойство для анимации (аналог pyqtProperty)
    Q_PROPERTY(int circlePos READ circlePos WRITE setCirclePos)

public:
    explicit AviaVerticalToggle(QWidget *parent = nullptr);

    int position() const;

    // Геттер и сеттер для свойства анимации
    int circlePos() const;
    void setCirclePos(int pos);
public slots:
    void setPosition(int position);

signals:
    void positionChanged(int position);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    int m_position;  // 0=центр, 1=низ, 2=верх
    int m_circlePos; // Текущая Y-координата тумблера для отрисовки

    QMap<int, int> m_positions; // Маппинг: [Индекс позиции] -> [Y-координата]
    QPropertyAnimation *m_animation;
};

#endif // AVIAVERTICALTOGGLE_H
