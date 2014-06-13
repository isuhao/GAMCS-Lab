#ifndef CHEESE_H
#define CHEESE_H

#include <QGraphicsItem>
#include <QMutex>
#include "amount.h"

class Cheese : public QGraphicsItem
{
public:
    Cheese();
    ~Cheese();

    Amount *amount;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
};

#endif // CHEESE_H
