
#include "viewer.h"


CDeckLinkGLWidget::CDeckLinkGLWidget(QWidget* parent) : QGLWidget(parent)
{
	refCount = 1;
	
	deckLinkInput = deckLinkInput;
	deckLinkScreenPreviewHelper = CreateOpenGLScreenPreviewHelper();
}

CDeckLinkGLWidget::CDeckLinkGLWidget() : QGLWidget()
{
	refCount = 1;
	
	deckLinkInput = deckLinkInput;
	deckLinkScreenPreviewHelper = CreateOpenGLScreenPreviewHelper();
}

void	CDeckLinkGLWidget::initializeGL ()
{
	if (deckLinkScreenPreviewHelper != NULL)
	{
		mutex.lock();
			deckLinkScreenPreviewHelper->InitializeGL();
		mutex.unlock();
	}
}

void	CDeckLinkGLWidget::paintGL ()
{
	mutex.lock();
		glLoadIdentity();
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		deckLinkScreenPreviewHelper->PaintGL();
	mutex.unlock();
}

void	CDeckLinkGLWidget::resizeGL (int width, int height)
{
	mutex.lock();	
		glViewport(0, 0, width, height);
	mutex.unlock();
}

HRESULT		CDeckLinkGLWidget::QueryInterface (REFIID iid, LPVOID *ppv)
{
	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG		CDeckLinkGLWidget::AddRef ()
{
	int		oldValue;
	
	oldValue = refCount.fetchAndAddAcquire(1);
	return (ULONG)(oldValue + 1);
}

ULONG		CDeckLinkGLWidget::Release ()
{
	int		oldValue;
	
	oldValue = refCount.fetchAndAddAcquire(-1);
	if (oldValue == 1)
	{
		delete this;
	}
	
	return (ULONG)(oldValue - 1);
}

HRESULT		CDeckLinkGLWidget::DrawFrame (IDeckLinkVideoFrame* theFrame)
{
	if (deckLinkScreenPreviewHelper != NULL)
	{
		deckLinkScreenPreviewHelper->SetFrame(theFrame);
		update();
	}
	return S_OK;
}

