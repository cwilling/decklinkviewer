/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */

#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

/*
Copyright (C) 2013 Christoph Willing

This file is part of decklinkviewer.

decklinkviewer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

decklinkviewer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with decklinkviewer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui>

#include "viewer.h"


class DisplayWindow : public QMainWindow
{
    Q_OBJECT

public:
    DisplayWindow();
    DisplayWindow(int width, int height, int tiles);

    CDeckLinkGLWidget **glviewers;

    void setDesktopDimensions(int width, int height);
    void setScreenCrawl(bool yesno);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void updateFramePosition();
    void about();

private:
    int baseWidth;
    int baseHeight;
    int desktopWidth;
    int desktopHeight;
    double aspectRatio;
    int number_of_tiles;
    int mouseX;
    static const int minXsize = 100;
    bool do_crawl;

    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

signals:
    void frame_updated();

};

#endif  /* DISPLAY_WINDOW_H */

