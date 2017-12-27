#include <QMessageBox>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <QRectF>
#include <QString>
#include <QPainter>
#include <QTime>

#include <qmath.h>
#include "gamewidget.h"
#include "keypressfilter.h"


GameWidget::GameWidget(QWidget *parent) :
    QWidget(parent),
    timer(new QTimer(this)),
    timerColor(new QTimer(this)),
    ca1(),
    universeSize(50),
    universeMode(0),
    cellMode(0),
    lifeTime(50),
    generations(-1)
    //randomMode(0)

{
    timer->setInterval(300);
    timerColor->setInterval(50);
    masterColor = "#000";
    ca1.resetWorldSize(universeSize, universeSize);
    ca1.lifeTimeUI = lifeTime;
    connect(timer, SIGNAL(timeout()), this, SLOT(newGeneration()));
    connect(timerColor, SIGNAL(timeout()), this, SLOT(newGenerationColor()));
}


GameWidget::~GameWidget() {
}


void GameWidget::startGame(const int &number) {
    /* start the game */

    emit gameStarted(universeMode, true);
    generations = number;
    timer->start();
    this->setFocus();
}


void GameWidget::stopGame() {
    /* stop the game */

    emit gameStopped(universeMode, true);
    timer->stop();
    timerColor->stop();
}


void GameWidget::clearGame() {
    /* clear the field and reset parameters if necessary */

    for (int k = 1; k <= universeSize; k++) {
        for (int j = 1; j <= universeSize; j++) {
            ca1.setValue(j, k, 0);
        }
    }
    gameEnds(universeMode, true);
    //qDebug() << "clearGame -> resetWorldsize()";
    ca1.resetWorldSize(universeSize, universeSize);

    // snake
    if (universeMode == 1) {
        ca1.putInitSnake();
        ca1.putNewFood();
    // predator-prey
    } else if (universeMode == 2) {
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                ca1.setLifetime(j, k, ca1.maxLifetime);
            }
        }
    // all modeling games
    } else if (universeMode >= 3) {
        //ca1.generateInitRandomNoise();
    }
    update();

}


int GameWidget::getUniverseSize() {
    return universeSize;
}


void GameWidget::setUniverseSize(const int &s) {
    /* set number of the cells in one row */
    universeSize = s;
    ca1.resetWorldSize(s, s);
    update();
}


int GameWidget::getUniverseMode() {
    return universeMode;
}


void GameWidget::setUniverseMode(const int &m) {
    int old_m = GameWidget::getUniverseMode();
    universeMode = m;

    if (old_m != m) GameWidget::clearGame();
    update();
}


int GameWidget::getCellMode() {
    return cellMode;
}


void GameWidget::setCellMode(const int &m) {
    cellMode = m;
}


CAbase GameWidget::getCA() {
    return ca1;
}


QString GameWidget::dumpGame(char member) {
    /* dump current universe into a string*/

    char temp;
    QString master = "";

    switch (universeMode) {

    // GAME OF LIFE
    case 0:
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                if (ca1.getValue(j, k) == 1) {
                    temp = '*';
                } else {
                    temp = 'o';
                }
                master.append(temp);
            }
            master.append("\n");
        }
        return master;
        break;

    // SNAKE
    case 1:
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                int value = ca1.getValue(j, k);
                if (value == 5) {
                    temp = 'F';
                } else if (value == 10) {
                    temp = 'H';
                } else if (value > 10) {
                    temp = 'H' + value - 10;
                } else {
                    temp = 'G';
                }
                master.append(temp);
            }
            master.append("\n");
        }
        return master;
        break;

    // PREDATOR
    case 2:
        if (member == 'v') { // world value
            for (int k = 1; k <= universeSize; k++) {
                for (int j = 1; j <= universeSize; j++) {
                    int value = ca1.getValue(j, k);
                    if (value == 1) {
                        temp = 'J';
                    } else if (value == 2) {
                        temp = 'G';
                    } else if (value == 5) {
                        temp = 'F';
                    } else {
                        temp = 'o';
                    }
                    master.append(temp);
                }
                master.append("\n");
            }
        } else if (member == 'l') { // lifetime
            for (int k = 1; k <= universeSize; k++) {
                for (int j = 1; j <= universeSize; j++) {
                    int lT = ca1.getLifetime(j, k);
                    if (lT == 0) {
                        temp = 'B';
                    } else if (lT == __INT16_MAX__) {
                        temp = 'A';
                    } else if (lT > 0 && lT < __INT16_MAX__) {
                        temp = 'B' + lT;
                    }
                    master.append(temp);
                }
                master.append("\n");
            }
        }

        return master;
        break;

    default:
        return master;
        break;
    }

}


void GameWidget::reconstructGame(const QString &data, char member) {
     /* reconstruct game from dump */

    int current;
    int ascii_H = (int) 'H';
    int ascii_B = (int) 'B';
    int ascii_Char;

    switch (universeMode) {

    // GAME OF LIFE
    case 0:
        current = 0;
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
               if (data[current] == '*') {
                   ca1.setValue(j, k, 1);
                }
                current++;
            }
            current++;
        }
        break;

    // SNAKE
    case 1:
        current = 0;
        for (int k = 1; k <= universeSize; k++) {
            for (int j = 1; j <= universeSize; j++) {
                ascii_Char = data[current].unicode();
                if (data[current] == 'F') {
                    ca1.setValue(j, k, 5);
                }
                else if (ascii_Char >= ascii_H) {
                    ca1.setValue(j, k, 10 + ascii_Char - ascii_H);
                } else {
                    ca1.setValue(j, k, 0);
                }
                current++;
            }
            current++;
        }
        break;

    // PREDATOR
    case 2:
        if (member == 'v') {
            current = 0;
            for (int k = 1; k <= universeSize; k++) {
                for (int j = 1; j <= universeSize; j++) {
                    ascii_Char = data[current].unicode();
                    if (data[current] == 'F') {
                        ca1.setValue(j, k, 5);
                    }
                    else if (data[current] == 'G') {
                        ca1.setValue(j, k, 2);
                    } else if (data[current] == 'J'){
                        ca1.setValue(j, k, 1);
                    } else {
                         ca1.setValue(j, k, 0);
                    }
                    current++;
                }
                current++;
            }
        } else if (member == 'l') {
            current = 0;
            for (int k = 1; k <= universeSize; k++) {
                for (int j = 1; j <= universeSize; j++) {
                    ascii_Char = data[current].unicode();
                    if (data[current] == 'A') {
                        ca1.setLifetime(j, k, __INT16_MAX__);
                    }
                    else if (data[current] == 'B') {
                        ca1.setLifetime(j, k, 0);
                    } else if (ascii_Char > ascii_B){
                        ca1.setLifetime(j, k, ascii_Char - ascii_B);
                    }
                    current++;
                }
                current++;
            }
        }

        break;

    default:
        break;
    }

    update();
}


int GameWidget::getInterval() {
    /* return time interval between two consecutive generations [msec]*/
    return timer->interval();
}


void GameWidget::setInterval(int msec) {
    /* set interval between generations */
    timer->setInterval(msec);
}


void GameWidget::newGeneration() {
    /* start the evolution of universe and update the game field */

    if (generations < 0)
        generations++;

    switch (universeMode) {
    // game of life
    case 0:
        ca1.worldEvolutionLife();
        break;
    // snake
    case 1:
        ca1.worldEvolutionSnake();
        break;
    // predator-prey
    case 2:
        //ca1.worldEvolutionLife();
        //qDebug() << "newGeneration -> worldEvolutionPredator()";
        ca1.worldEvolutionPredator();
        break;
    // noise
    case 3:
        ca1.worldEvolutionNoise();
        break;
    // erosion
    case 4:
        ca1.worldEvolutionErosion();
        break;
    // fluids
    case 5:
        ca1.worldEvolutionFluids();
        break;
    // gases
    case 6:
        ca1.worldEvolutionGases();
        break;

    default:
        break;
    }

    update();

    if (ca1.isNotChanged()) {
        const QString headlines[] = {"Evolution stopped!", "Game over!"};
        const QString details[] = {"All future generations will be identical to this one.",
                                   "Your snake hit an obstacle."};

        QMessageBox msgBox;
        switch (universeMode) {
        // game of life
        case 0:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;
        // snake
        case 1:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[1]);
            msgBox.setInformativeText(details[1]);
            break;
        // predator-prey
        case 2:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;
        // noise
        case 3:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;
        // erosion
        case 4:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;
        // fluids
        case 5:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;
        // gases
        case 6:
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(headlines[0]);
            msgBox.setInformativeText(details[0]);
            break;

        default:
            break;
        }
        msgBox.exec();
        stopGame();
        gameEnds(universeMode, true);
        return;
    }

    generations--;
    if (generations == 0) {
        stopGame();
        gameEnds(universeMode, true);
        QMessageBox::information(this, tr("Game finished."), tr("Iterations finished."),
                                 QMessageBox::Ok, QMessageBox::Cancel);
    }

}


void GameWidget::newGenerationColor() {
    /* start the evolution of universe and update the game field for "Cyclic cellular automata" mode */

    if (generations < 0)
        generations++;

    update();

    generations--;
    if (generations == 0) {
        stopGame();
        gameEnds(universeMode, true);
        QMessageBox::information(this, tr("Game finished."), tr("Iterations finished."),
                                 QMessageBox::Ok, QMessageBox::Cancel);
    }
}


void GameWidget::paintEvent(QPaintEvent *) {
    /* paint the grid and the universe inside ui */

    QPainter p(this);
    paintGrid(p);
    paintUniverse(p);
}


void GameWidget::mousePressEvent(QMouseEvent *e) {
    emit universeModified(universeMode, true);
    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    int k = floor(e->y() / cellHeight) + 1;
    int j = floor(e->x() / cellWidth) + 1;

    // int mode[9] = {1, 3, 6, 4, 2, 8, 9, 10, 11};

    // game of life
    if (universeMode != 2 && universeMode != 1) {
        if (ca1.getValue(j, k) != 0) {
            ca1.setValue(j, k, 0);
        }
        else {
            ca1.setValue(j, k, 1);
        }
        update();
    }

    // predator-prey
    else if (universeMode == 2) {
        switch (cellMode) {
        case 0: // predator
            if (ca1.getValue(j, k) != 1) {
                ca1.setValue(j, k, 1);
                ca1.setLifetime(j, k, lifeTime);
            } else {
                ca1.setValue(j, k, 0);
                ca1.setLifetime(j, k, ca1.maxLifetime);
            }
            break;
        case 1: // prey
            if (ca1.getValue(j, k) != 2) {
                ca1.setValue(j, k, 2);
                ca1.setLifetime(j, k, lifeTime);
            } else {
                ca1.setValue(j, k, 0);
                ca1.setLifetime(j, k, ca1.maxLifetime);
            }
            break;
        case 2: // food
            if (ca1.getValue(j, k) != 5) {
                ca1.setValue(j, k, 5);
                ca1.setLifetime(j, k, ca1.maxLifetime);
            } else {
                ca1.setValue(j, k, 0);
                ca1.setLifetime(j, k, ca1.maxLifetime);
            }
            break;
        default:
            break;
        }
        update();
    }
}


void GameWidget::mouseMoveEvent(QMouseEvent *e) {
    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    int k = floor(e->y() / cellHeight) + 1;
    int j = floor(e->x() / cellWidth) + 1;

    // int mode[9] = {1, 3, 6, 4, 2, 8, 9, 10, 11};

    // game of life
    if (universeMode != 2 && universeMode != 1) {
        if (ca1.getValue(j, k) == 0) {
            ca1.setValue(j, k, 1);
            update();
        }
    }
    // predator-prey
    else if (universeMode == 2) {
        switch (cellMode) {
        case 0: // predator
            if (ca1.getValue(j, k) != 1) {
                ca1.setValue(j, k, 1);
                ca1.setLifetime(j, k, lifeTime);
            }
            break;
        case 1: // prey
            if (ca1.getValue(j, k) != 2) {
                ca1.setValue(j, k, 2);
                ca1.setLifetime(j, k, lifeTime);
            }
            break;
        case 2: // food
            if (ca1.getValue(j, k) != 5) {
                ca1.setValue(j, k, 5);
                ca1.setLifetime(j, k, ca1.maxLifetime);
            }
            break;
        default:
            break;
        }
        update();
    }
}


void GameWidget::paintGrid(QPainter &p) {
    /* paint the grid in the ui */

    QRect borders(0, 0, width() - 1, height() - 1); // borders of the universe
    QColor gridColor = masterColor; // color of the grid
    gridColor.setAlpha(10); // must be lighter than main color
    p.setPen(gridColor);
    double cellWidth = (double) width() / universeSize; // width of the widget / number of cells at one row
    for (double k = cellWidth; k <= width(); k += cellWidth)
        p.drawLine(k, 0, k, height());
    double cellHeight = (double) height() / universeSize; // height of the widget / number of cells at one row
    for (double k = cellHeight; k <= height(); k += cellHeight)
        p.drawLine(0, k, width(), k);
    p.drawRect(borders);
}


void GameWidget::paintUniverse(QPainter &p) {
    /* paint cell values with specific colors into grid */

    double cellWidth = (double) width() / universeSize;
    double cellHeight = (double) height() / universeSize;
    for (int k = 1; k <= universeSize; k++) {
        for (int j = 1; j <= universeSize; j++) {
            if (ca1.getValue(j, k) != 0) {
                qreal left = (qreal) (cellWidth * j - cellWidth); // margin from left
                qreal top  = (qreal) (cellHeight * k - cellHeight); // margin from top
                QRectF r(left, top, (qreal) cellWidth, (qreal) cellHeight);
                // p.fillRect(r, QBrush(masterColor));
                if (universeMode != 2) {
                    p.fillRect(r, QBrush(masterColor));
                } else {
                    p.fillRect(r, getPredefinedColor(ca1.getValue(j, k))); //fill cell with brush of cell type
                }

//                if (0 && universeMode != 7) { // randomMode = 0
//                    p.fillRect(r, setColor(ca1.getColor(j, k))); //fill cell with brush from random mode
//                 }
//                else {
//                    if (ca1.getValue(j, k) == 1 || universeMode == 7) {
//                        p.fillRect(r, QBrush(masterColor)); // fill cell with brush of main color
//                    }
//                    else {
//                        p.fillRect(r, setColor(ca1.getValue(j, k))); //fill cell with brush of cell type
//                    }
//                }
            }
        }
    }
}


QColor GameWidget::getMasterColor() {
    return masterColor;
}


void GameWidget::setMasterColor(const QColor &color) {
    masterColor = color;
    update();
}


// SNAKE
void GameWidget::calcDirectionSnake(int dS) {
    /* opposing directions add up to 10 (2 + 8, 4 + 6), so past and future must NOT do so */

    if (dS + ca1.directionSnake.past == 10) {
        ca1.directionSnake.future = ca1.directionSnake.past; // continue with past direction if input is "invalid"
    } else {
        ca1.directionSnake.future = dS;
    }
}


void GameWidget::setDirectionSnake(int past, int future) {
    ca1.directionSnake.past = past;
    ca1.directionSnake.future = future;
}


void GameWidget::setSnakeLength(int l) {
    ca1.setSnakeLength(l);
}


void GameWidget::setSnakeAction(int a) {
    ca1.setSnakeAction(a);
}


void GameWidget::setPositionSnakeHead(int x, int y) {
    ca1.positionSnakeHead.x = x;
    ca1.positionSnakeHead.y = y;
}


void GameWidget::setPositionFood(int x, int y) {
    ca1.positionFood.x = x;
    ca1.positionFood.y = y;
}


int GameWidget::getLifetime() {
    return lifeTime;
}


void GameWidget::setLifetime(const int &l) {
    lifeTime = l;
}


QColor GameWidget::getPredefinedColor(const int &color) {
    QColor cellColor[12]= {Qt::red,
                           Qt::darkRed,
                           Qt::green,
                           Qt::darkGreen,
                           Qt::blue,
                           Qt::darkBlue,
                           Qt::cyan,
                           Qt::darkCyan,
                           Qt::magenta,
                           Qt::darkMagenta,
                           Qt::yellow,
                           Qt::darkYellow};

    return cellColor[color];
}

