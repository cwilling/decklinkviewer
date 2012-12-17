/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */

/*
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

