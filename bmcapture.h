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
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
}


class VideoDelegate;

class BMCapture : public QObject
{
    Q_OBJECT

private:
    DisplayWindow       *displayWindow;
    int deviceId;
    VideoDelegate       *mVideoDelegate;

pthread_mutex_t sleepMutex;
pthread_cond_t sleepCond;

int g_videoModeIndex;
int g_audioChannels;
int g_audioSampleDepth;
const char *g_videoOutputFile;
const char *g_audioOutputFile;
int g_maxFrames;
unsigned long long g_memoryLimit;


AVOutputFormat *fmt;
AVFormatContext *oc;
BMDTimeValue frameRateDuration, frameRateScale;

AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id);
AVStream *add_video_stream(AVFormatContext *oc, enum CodecID codec_id, IDeckLinkDisplayMode *displayMode);
void *push_packet(void *ctx);

public:
    BMCapture();
    BMCapture(int id);

    void    print_input_modes (IDeckLink* deckLink);
    void    print_output_modes (IDeckLink* deckLink);

    void    print_input_capabilities (IDeckLink* deckLink);
    void    print_output_capabilities (IDeckLink* deckLink);

    int    print_capabilities();
    void    capture(int mode, int connection, bool tiled);

    int     GetFrameSize(int card, int mode, int *winWidth, int *winHeight);
    void    SetDisplayWindow(DisplayWindow **dispay_window);
    void    SetOutputFilename(const char *filename);
    void    SetOutputFormat(AVOutputFormat *format);

    HRESULT VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*);
    HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);


//signals:
//    void frameArrived(){};
};

class VideoDelegate : public QObject, public IDeckLinkInputCallback
{
    Q_OBJECT

private:

typedef struct AVPacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    unsigned long long size;
    int abort_request;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} AVPacketQueue;

AVPacketQueue queue;

AVPacket flush_pkt;

void avpacket_queue_init(AVPacketQueue *q);
void avpacket_queue_flush(AVPacketQueue *q);
void avpacket_queue_end(AVPacketQueue *q);
int avpacket_queue_put(AVPacketQueue *q, AVPacket *pkt);
int avpacket_queue_get(AVPacketQueue *q, AVPacket *pkt, int block);
unsigned long long avpacket_queue_size(AVPacketQueue *q);

    int32_t mRefCount;
    double  lastTime;
    int     framecount;
    bool g_verbose;
    int64_t initial_video_pts;

    public:
    AVStream *audio_st, *video_st;
void avpacket_queue_init();
void avpacket_queue_end();
int avpacket_queue_get(AVPacket *pkt, int block);
unsigned long long avpacket_queue_size();

    VideoDelegate ()
    { framecount = 0; g_verbose = false; initial_video_pts = AV_NOPTS_VALUE; };

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

signals:
    void captureFrameArrived();
};



#endif	/* _BMCAPTURE_H_ */

