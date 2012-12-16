/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */
#ifndef _BMCAPTURE_H_
#define _BMCAPTURE_H_


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

    void    print_capabilities();
    void    capture(int device, int mode, int connection);

    int     GetFrameSize(int card, int mode, int *winWidth, int *winHeight);
    void    SetDisplayWindow(DisplayWindow **dispay_window);

    HRESULT VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*);
    HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
};


#endif	/* _BMCAPTURE_H_ */

