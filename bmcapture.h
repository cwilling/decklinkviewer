/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */

#ifndef _BMCAPTURE_H_
#define _BMCAPTURE_H_

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



#include <QObject>
#include <iostream>
#include <iomanip>

#include "DeckLinkAPI.h"
#include "displaywindow.h"

class VideoDelegate : public IDeckLinkInputCallback
{

private:
    int32_t mRefCount;
    double  lastTime;
    int     framecount;

    public:
    VideoDelegate ()
    { framecount = 0; };

    virtual HRESULT     QueryInterface (REFIID iid, LPVOID *ppv)
    { return S_OK; };

    virtual ULONG       AddRef (void)
    { return mRefCount++; };

    virtual ULONG           Release (void)
    {
        int32_t         newRefValue;

        newRefValue = mRefCount--;
        if (newRefValue == 0)
        {
            delete this;
            return 0;
        }        
        return newRefValue;
    }

    virtual HRESULT VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*);
    virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
};


class BMCapture : public VideoDelegate
{

private:
    DisplayWindow       *displayWindow;

public:
    BMCapture();

    void    print_input_modes (IDeckLink* deckLink);
    void    print_output_modes (IDeckLink* deckLink);

    void    print_input_capabilities (IDeckLink* deckLink);
    void    print_output_capabilities (IDeckLink* deckLink);

    int    print_capabilities();
    void    capture(int device, int mode, int connection);

    int     GetFrameSize(int card, int mode, int *winWidth, int *winHeight);
    void    SetDisplayWindow(DisplayWindow **dispay_window);

    HRESULT VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*);
    HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
};


#endif	/* _BMCAPTURE_H_ */

