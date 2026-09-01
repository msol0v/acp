#ifndef AVIAKNOB_H
#define AVIAKNOB_H

#include <QBrush>
#include <QColor>
#include <QDial>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRadialGradient>
#include <QRectF>
#include <cmath>

class AviaKnob : public QDial
{
    Q_OBJECT

public:
    explicit AviaKnob(QWidget *parent = nullptr);

    void setRange(int minimum, int maximum);
    void setStartValue();
    void setAngleRange(double start_angle, double span_angle);
    double valueRatio() const;

    void on(const QColor &color = QColor());
    void off();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_startAngle;
    double m_spanAngle;
    bool m_lightOn;
    QColor m_lightColor;
};

#endif // AVIAKNOB_H
