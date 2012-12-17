/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */

#ifndef DECKLINK_VIEWER_H
#define DECKLINK_VIEWER_H

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

