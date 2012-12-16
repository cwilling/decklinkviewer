#ifndef DECKLINK_VIEWER_H
#define DECKLINK_VIEWER_H

#include "DeckLinkAPI.h"

#include <QGLWidget>
#include <QMessageBox>
#include <QWidget>
#include <QMutex>


class CDeckLinkGLWidget : public QGLWidget, public IDeckLinkScreenPreviewCallback
{
private:
	QAtomicInt refCount;
	QMutex mutex;	
	IDeckLinkInput* deckLinkInput;
	IDeckLinkGLScreenPreviewHelper* deckLinkScreenPreviewHelper;
	
public:
	CDeckLinkGLWidget(QWidget* parent);
	CDeckLinkGLWidget();
	
	// IDeckLinkScreenPreviewCallback
	virtual HRESULT QueryInterface(REFIID iid, LPVOID *ppv);
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual HRESULT DrawFrame(IDeckLinkVideoFrame* theFrame);
	
protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
};

#endif // DECKLINK_VIEWER_H //

