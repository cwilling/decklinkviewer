#include "displaywindow.h"

#include <iostream>


DisplayWindow::DisplayWindow(int width, int height): QMainWindow()
{
	baseWidth = width;
	baseHeight = height;

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHeightForWidth(true);
	setSizePolicy(sizePolicy);
	setFixedSize(QSize(baseWidth/4, baseHeight/4));
	aspectRatio = (1.0 * baseWidth) / (1.0 * baseHeight);
	show();

	QFrame* centralFrame = new QFrame(this);
	setCentralWidget(centralFrame);

	QGridLayout* layout = new QGridLayout(centralFrame);
	glviewer = new CDeckLinkGLWidget();
	layout->addWidget(glviewer, 0, 0, 0, 0);


	glviewer->resize(this->size());
	//glviewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//glviewer->setSizePolicy(sizePolicy);

	//createActions();
	//createMenus();


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
	fprintf(stderr, "Noted desktop size: %dx%d\n", desktopWidth, desktopHeight);
}

void DisplayWindow::emit_frame_update()
{
	emit frameUpdated();
}

void DisplayWindow::frameUpdated()
{
	QPoint currentPosition = this->pos();
//	fprintf(stdout, "current x = %d\n", currentPosition.x());

	int newX = currentPosition.x() + 2;
	/*
		This magic "23" is for window frame width etc. - should obtain it programmatically !
	*/
	if (newX > (desktopWidth - 23))
	{
		QSize currentSize = this->size();
		newX = - currentSize.width();
	}
//	this->move(newX, 20);
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

