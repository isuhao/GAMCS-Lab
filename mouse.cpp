#include <QTextStream>
#include <QList>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <gamcs/Sqlite.h>   // FIXME: check existing
#include "scene.h"
#include "config.h"
#include "mouse.h"

Mouse::Mouse(int id) : Avatar(id), storage(""), learning_mode(Agent::ONLINE)
{
    _type = MOUSE;
    _color = QColor(89, 255, 89);
    _life = 30;

    myagent = new CSOSAgent(id, 0.9, 0.01);
    myagent->setMode(learning_mode);
    connectAgent(myagent);
}

Mouse::~Mouse()
{
    delete myagent;
}

void Mouse::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    QString tips;
    QTextStream(&tips) << "id:" << id << ", life:" << _life << ", Mode" << learning_mode;
    setToolTip(tips);
    update();
}

// popup menu
void Mouse::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QAction *toggle_mark;
    if (isMarked())
        toggle_mark = menu.addAction("Unmark");
    else
        toggle_mark = menu.addAction("Mark");

    QAction *toggle_awake;
    if (isAwake())
        toggle_awake = menu.addAction("Sleep");
    else
        toggle_awake = menu.addAction("Wake up");

    QMenu *lmode = menu.addMenu("Learning Mode");
    QAction *online = lmode->addAction("Online mode");
    online->setCheckable(true);
    QAction *explore = lmode->addAction("Explore mode");
    explore->setCheckable(true);
    if (learning_mode == Agent::ONLINE)
        online->setChecked(true);
    else
        explore->setChecked(true);

    QMenu *memory = menu.addMenu("Memory");
    QAction *save = memory->addAction("Save...");
    QAction *load = memory->addAction("Load...");

    QAction *selectedAction = menu.exec(event->screenPos());

    // judge selected action
    if (selectedAction == toggle_mark)
    {
        this->setMarked(!this->isMarked());
    }
    else if (selectedAction == toggle_awake)
    {
        this->setAwake(!this->isAwake());
    }
    else if (selectedAction == online)
    {
        qDebug() << "mouse change mode to ONLINE";
        myagent->setMode(Agent::ONLINE);
        learning_mode = Agent::ONLINE;
    }
    else if (selectedAction == explore)
    {
        qDebug() << "mouse change mode to EXPLORE";
        myagent->setMode(Agent::EXPLORE);
        learning_mode = Agent::EXPLORE;
    }
    else if (selectedAction == save)
    {
        if (storage.isEmpty())
        {
            storage = QFileDialog::getSaveFileName(NULL, QObject::tr("Save memory..."), QString(),
                                                   QObject::tr("Sqlite3 database (*.slt);; All Files(*)"));
        }

        if (!storage.endsWith(".slt", Qt::CaseInsensitive))
            storage += ".slt";

        // save memory to storage
        Sqlite db(storage.toStdString());
        bool status = this->isAwake();
        this->setAwake(false);  // put to sleep for saving memory
        myagent->dumpMemoryToStorage(&db);
        this->setAwake(status);   // restore old status
    }
    else if (selectedAction == load)
    {
        storage = QFileDialog::getOpenFileName(NULL, QObject::tr("Load memory..."), QString(),
                                               QObject::tr("Sqlite3 database (*.slt);; All Files(*)"));

        if (!storage.isEmpty())
        {
            // load memory from storage
            Sqlite db(storage.toStdString());
            bool status = this->isAwake();
            this->setAwake(false);  // put to sleep for loading memory
            myagent->loadMemoryFromStorage(&db);
            this->setAwake(status);   // restore old status
        }
    }

    update();
}

void Mouse::act()
{
    Avatar::step();
}

Agent::State Mouse::perceiveState()
{
//    int st = grid_x;
//    st += grid_y * SCENE_WIDTH;

//    // perceive the spirits at four directions
    int stype0 = 0, stype1 = 0, stype2 = 0, stype3 = 0, stype4 = 0;

    // current pos
    QList<Spirit *> colliding_spirits = collidingSpirits();
    if (colliding_spirits.empty())
        stype0 = 0;
    else if (colliding_spirits[0]->spiritType() == BLOCK)
        stype0 = 1;
    else if (colliding_spirits[0]->spiritType() == CHEESE)
        stype0 = 2;
    else if (colliding_spirits[0]->spiritType() == NAIL)
        stype0 = 3;
    else if (colliding_spirits[0]->spiritType() == MOUSE)
        stype0 = 4;
    else if (colliding_spirits[0]->spiritType() == CAT)
        stype0 = 5;

    // above
    Spirit *spt = myscene->getSpiritAt(grid_x, grid_y - 1);
    if (spt == NULL)
        stype1 = 0;
    else if (spt->spiritType() == BLOCK)
        stype1 = 1;
    else if (spt->spiritType() == CHEESE)
        stype1 = 2;
    else if (spt->spiritType() == NAIL)
        stype1 = 3;
    else if (spt->spiritType() == MOUSE)
        stype1 = 4;
    else if (spt->spiritType() == CAT)
        stype1 = 5;

    // down
    spt = myscene->getSpiritAt(grid_x, grid_y + 1);
    if (spt == NULL)
        stype2 = 0;
    else if (spt->spiritType() == BLOCK)
        stype2 = 1;
    else if (spt->spiritType() == CHEESE)
        stype2 = 2;
    else if (spt->spiritType() == NAIL)
        stype2 = 3;
    else if (spt->spiritType() == MOUSE)
        stype2 = 4;
    else if (spt->spiritType() == CAT)
        stype2 = 5;

    // left
    spt = myscene->getSpiritAt(grid_x - 1, grid_y);
    if (spt == NULL)
        stype3 = 0;
    else if (spt->spiritType() == BLOCK)
        stype3 = 1;
    else if (spt->spiritType() == CHEESE)
        stype3 = 2;
    else if (spt->spiritType() == NAIL)
        stype3 = 3;
    else if (spt->spiritType() == MOUSE)
        stype3 = 4;
    else if (spt->spiritType() == CAT)
        stype3 = 5;

    // right
    spt = myscene->getSpiritAt(grid_x + 1, grid_y);
    if (spt == NULL)
        stype4 = 0;
    else if (spt->spiritType() == BLOCK)
        stype4 = 1;
    else if (spt->spiritType() == CHEESE)
        stype4 = 2;
    else if (spt->spiritType() == NAIL)
        stype4 = 3;
    else if (spt->spiritType() == MOUSE)
        stype4 = 4;
    else if (spt->spiritType() == CAT)
        stype4 = 5;

    int st = stype0 + stype1 * 6 + stype2 * 36 + stype3 * 216 + stype4 * 1296;

    return st;
}

void Mouse::performAction(Agent::Action act)
{
    switch (act)
    {
    case 1: // move up
        moveUp();
        break;
    case 2: // move down
        moveDown();
        break;
    case 3: // move left
        moveLeft();
        break;
    case 4: // move right
        moveRight();
        break;
    case 5: // no move
        break;
    }
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
    QList<Spirit *> colliding_spirits = collidingSpirits();
    if (colliding_spirits.empty())
    {
        pf = 0.0;
    }
    else	// integrated all the colliding spirits
    {
        for (QList<Spirit *>::iterator it = colliding_spirits.begin(); it != colliding_spirits.end(); ++it)
        {
            if ((*it)->spiritType() == CHEESE)
            {
                qDebug() << "Mouse" << id << ": Wow! cheese!";
                this->healed(0.5);
                pf += 1.0;
            }
            else if ((*it)->spiritType() == NAIL)
            {
                qDebug() << "Mouse" << id << ": Oops! nail!";
                this->injured(0.5);
                pf += -1.0;
            }
            else if ((*it)->spiritType() == CAT)
            {
                qDebug() << "Mouse" << id << ": Cat! My GOD!";
                this->injured(1);
                pf += -2.0;
            }
            else
            {
                qDebug() << "Mouse" << id << ": What's this, get out of my way!";
                pf += 0.0;
            }
        }
    }

    return pf;
}
