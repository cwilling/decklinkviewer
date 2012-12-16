#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include <QtGui>

#include "viewer.h"


class DisplayWindow : public QMainWindow
{
	Q_OBJECT

public:
	DisplayWindow();
	DisplayWindow(int width, int height);

	CDeckLinkGLWidget *glviewer;

	void setDesktopDimensions(int width, int height);
	void emit_frame_update();

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

protected slots:
	void frameUpdated();
	void about();

private:
	int baseWidth;
	int baseHeight;
	int desktopWidth;
	int desktopHeight;
	double aspectRatio;
	int mouseX;
	static const int minXsize = 100;

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

