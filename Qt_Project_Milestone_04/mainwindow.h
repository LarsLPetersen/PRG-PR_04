#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include "gamewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void selectMasterColor();
    void selectRandomColor();
    void saveGame();
    void loadGame();
    void globalButtonControl(int uM);
    void enableControls(int uM, bool b);
    void disableControls(int uM, bool b);

private:
    Ui::MainWindow *ui;
    QColor currentColor;
    GameWidget *game;
};

#endif // MAINWINDOW_H
