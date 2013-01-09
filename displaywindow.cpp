/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */

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

#include "displaywindow.h"

#include <iostream>


DisplayWindow::DisplayWindow(int width, int height, int tiles): QMainWindow()
{
    baseWidth = width;
    baseHeight = height;
    number_of_tiles = tiles;
    do_crawl = false;

    desktop = QApplication::desktop();
    number_of_screens = desktop->screenCount();
    desktopWidth = desktopHeight = 0;
    if ( number_of_screens > 1 )
    {
        std::cerr << "Multi-screen (" << number_of_screens << ")" << std::endl;

        // multiscreen
        if( desktop->isVirtualDesktop() )
        {
            // managed as a single screen
            std::cerr << "- managed as single deskop" << std::endl;

            // Assume we have a single row of all the screens
            // so add all the widths
            // and take height of tallest (probably all the same though)
            for(int i=0;i<number_of_screens;i++)
            {
                QRect g = desktop->screenGeometry(i);
                desktopWidth += g.width();
                if( g.height() > desktopHeight )
                    desktopHeight = g.height();
            }
        }
        else
        {
            // a set of individual screens
            std::cerr << "- managed as individual screens" << std::endl;

            // Is this a usable screen configuration?
        }
    }
    else
    {
        // single screen
        std::cerr << "Single screen" << std::endl;
        desktopWidth = desktop->width();
        desktopHeight = desktop->height();
    }
    std::cerr << "Desktop size calculated as: " << desktopWidth << "x" << desktopHeight <<std::endl;

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(true);
    setSizePolicy(sizePolicy);
    setFixedSize(QSize(baseWidth*tiles/4, baseHeight/4));
    aspectRatio = (1.0 * baseWidth * tiles) / (1.0 * baseHeight);
    show();

    QFrame* centralFrame = new QFrame(this);
    setCentralWidget(centralFrame);

    QHBoxLayout* layout = new QHBoxLayout(centralFrame);

    glviewers = (CDeckLinkGLWidget**)calloc(tiles, sizeof(CDeckLinkGLWidget*));
    for(int i=0;i<number_of_tiles;i++)
    {
        glviewers[i] = new CDeckLinkGLWidget();
        layout->addWidget(glviewers[i]);
        glviewers[i]->resize(this->size());
    }

}

void DisplayWindow::createActions()
{
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

}
void DisplayWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    //helpMenu->addAction(aboutQtAct);

}

void DisplayWindow::about()
{
    QMessageBox::about(this, tr("About DecklinkCaptureViewer"),
    tr("The <b>DecklinkCaptureViewer</b> application shows the "
    "video preview stream from Blackmagic-Design's Decklink capture card."));
}

void DisplayWindow::setDesktopDimensions(int width, int height)
{
    desktopWidth = width;
    desktopHeight = height;
    std::cerr << "Noted desktop size: " << desktopWidth << "x" << desktopHeight << std::endl;
}

/*
    This is just to show movement of the display window across the desktop.
    Wrapping is only rudimentary - a real implementation would have to do it properly
*/
void DisplayWindow::setScreenCrawl(bool yesno)
{
    do_crawl = yesno;
}
void DisplayWindow::updateFramePosition()
{
    if( do_crawl == false )
        return;

    //std::cout << "Slot received signal" << std:: endl;

    QPoint currentPosition = this->pos();
    //std::cout << "current x = " << currentPosition.x() << std:: endl;

    int newX = currentPosition.x() + 2;
    /*
        This magic "23" is for window frame width etc. - should obtain it programmatically !
    */
    if (newX > (desktopWidth - 23))
    {
        QSize currentSize = this->size();
        newX = - currentSize.width();
    }
    move(newX, currentPosition.y());

}


void DisplayWindow::mousePressEvent(QMouseEvent *event)
{
    mouseX = event->x();
}
void DisplayWindow::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - mouseX;
    if( deltaX == 0 )
        return;

    mouseX = mouseX + deltaX;
    int newWidth = width() + deltaX;
    if( newWidth < minXsize )
        newWidth = minXsize;
    setFixedSize(QSize(newWidth, newWidth/aspectRatio));
}

void DisplayWindow::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == QKeySequence::StandardKey(Qt::CTRL + Qt::Key_P) )
        std::cerr << "OK" << std::endl;
    else
        std::cerr << event->key() << std::endl;

    std::cerr << "--" << QKeySequence::StandardKey(Qt::CTRL + Qt::Key_P) << std::endl;
}

