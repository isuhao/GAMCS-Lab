#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPointF>
#include <QTransform>
#include <QDebug>
#include <QList>
#include <QGraphicsSceneMouseEvent>
#include "mouse.h"
#include "scene.h"
#include "cheese.h"
#include "nail.h"
#include "cat.h"
#include "config.h"

Mouse::Mouse(int id) : Avatar(id)
{
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
    setData(0, "Mouse");

    myagent = new CSOSAgent(id, 0.8, 0.001);
    connectAgent(myagent);

    amount = new Amount(this, 30);	// bite by cat 30 times til dead
}

Mouse::~Mouse()
{
    delete myagent;
    delete amount;
}

QRectF Mouse::boundingRect() const
{
    return QRectF(0, 0, GRID_SIZE, GRID_SIZE);    // the size of mouse
}

QPainterPath Mouse::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, GRID_SIZE, GRID_SIZE);
    return path;
}

void Mouse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QColor fillColor = QColor(89, 255, 89);
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.lighter();

    painter->setBrush(fillColor);
    painter->drawRect(0, 0, GRID_SIZE, GRID_SIZE);

    return;
}

void Mouse::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Mouse::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Scene *scene = dynamic_cast<Scene *>(this->scene());
    QPoint pos = scene->gridPoint(event->scenePos());
    this->setPos(pos);
    update();
}

void Mouse::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void Mouse::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    QString tips;
    QTextStream(&tips) << "id:" << id << "amount:" << amount->amount();
    setToolTip(tips);
}

Agent::State Mouse::perceiveState()
{
    qreal st = this->x() / GRID_SIZE;
    st += (this->y() * SCENE_WIDTH) / GRID_SIZE;

    return (long) st;
}

void Mouse::performAction(Agent::Action act)
{
    qreal x = this->x();
    qreal y = this->y();

    QGraphicsScene *scene = this->scene();
    QGraphicsItem *item = NULL;

    int half_grid = GRID_SIZE / 2;

    switch (act)
    {
    case 1: // move up
        // check if there're blocks
        item = scene->itemAt(x + half_grid, y - half_grid, QTransform());
        if (item == NULL || item->data(0).toString() != "Block")    // only Block can block
            y -= GRID_SIZE;
        break;
    case 2: // move down
        item = scene->itemAt(x + half_grid, y + GRID_SIZE + half_grid, QTransform());
        if (item == NULL || item->data(0).toString() != "Block")
            y += GRID_SIZE;
        break;
    case 3: // move left
        item = scene->itemAt(x - half_grid, y + half_grid,QTransform());
        if (item == NULL || item->data(0).toString() != "Block")
            x -= GRID_SIZE;
        break;
    case 4: // move right
        item = scene->itemAt(x + GRID_SIZE + half_grid, y + half_grid,QTransform());
        if (item == NULL || item->data(0).toString() != "Block")
            x += GRID_SIZE;
        break;
    case 5: // no move
        break;
    }

    this->setPos(x, y);
}

OSpace Mouse::availableActions(Agent::State st)
{
    Q_UNUSED(st);

    OSpace acts;
    acts.add(1, 5, 1);
    return acts;
}

float Mouse::originalPayoff(Agent::State st)
{
    Q_UNUSED(st);

    float pf = 0.0;
    QList<QGraphicsItem *> colliding_items = this->collidingItems(Qt::IntersectsItemShape);

    if (colliding_items.empty())
    {
        pf = 0.0;
    }
    else
    {
        // find the first valid tool
        QString item_name = "";
        QList<QGraphicsItem *>::iterator it;
        for (it = colliding_items.begin(); it != colliding_items.end(); ++it)
        {
            item_name = (*it)->data(0).toString();
            if (item_name == "")    // not a valid tool, continue
                continue;
            else
                break;      // break if found
        }

        if (it == colliding_items.end())    // no valid tool found
        {
            pf = 0.0;
        }
        else
        {
            if (item_name == "Cheese")
            {
                qDebug() << "Mouse" << id << ": Wow! cheese!";
                Cheese *cheese = dynamic_cast<Cheese *>(*it);
                pf = 1;
                cheese->amount->decrease(1);
                this->amount->increase(0.5);
            }
            else if (item_name == "Nail")
            {
                qDebug() << "Mouse" << id << ": Oops! nail!";
                Nail *nail = dynamic_cast<Nail *>(*it);
                pf = -1;
                this->amount->decrease(0.5);
            }
            else if (item_name == "Cat")
            {
                qDebug() << "Mouse" << id << ": Cat! My GOD!";
                pf = -5.0;
            }
            else    // some tool get in the way
            {
                pf = 0.0;
            }
        }
    }

        return pf;
}
