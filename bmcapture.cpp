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

#include "bmcapture.h"

/* ================================================================================== */
/* Stuff (temporary) from bmdtools for streaming encoded video frames */

static enum PixelFormat pix_fmt = PIX_FMT_UYVY422;
static enum AVSampleFormat sample_fmt = AV_SAMPLE_FMT_S16;




AVStream * BMCapture::add_audio_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVCodec *codec;
    AVStream *st;

    st = avformat_new_stream(oc, NULL);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c             = st->codec;
    c->codec_id   = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    c->sample_fmt = sample_fmt;
    //    c->bit_rate = 64000;
    c->sample_rate = 48000;
    c->channels    = g_audioChannels;
    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    return st;
}

AVStream * BMCapture::add_video_stream(AVFormatContext *oc, enum CodecID codec_id, IDeckLinkDisplayMode *displayMode)
{
    AVCodecContext *c;
    AVCodec *codec;
    AVStream *st;

    st = avformat_new_stream(oc, NULL);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c             = st->codec;
    c->codec_id   = codec_id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;

    /* put sample parameters */
    //    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width  = displayMode->GetWidth();
    c->height = displayMode->GetHeight();
    /* time base: this is the fundamental unit of time (in seconds) in terms
    * of which frame timestamps are represented. for fixed-fps content,
    * timebase should be 1/framerate and timestamp increments should be
    * identically 1.*/
    displayMode->GetFrameRate(&frameRateDuration, &frameRateScale);
    c->time_base.den = frameRateScale;
    c->time_base.num = frameRateDuration;
    c->pix_fmt       = pix_fmt;

    if (codec_id == CODEC_ID_V210)
        c->bits_per_raw_sample = 10;
    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    /* open the codec */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    return st;
}

void * BMCapture::push_packet(void *ctx)
{
    AVFormatContext *s = (AVFormatContext *)ctx;
    AVPacket pkt;
    int ret;

    while (mVideoDelegate->avpacket_queue_get(&pkt, 1))
    {
        av_interleaved_write_frame(s, &pkt);
        if (mVideoDelegate->avpacket_queue_size() > g_memoryLimit)
        {
            pthread_cond_signal(&sleepCond);
        }
    }
    return NULL;
}



/* End of bmdtools stuff */
/* ================================================================================== */

/* First, implement methods (from VideoDelegate)
*/
//void VideoDelegate::avpacket_queue_init()
//{
//    AVPacketQueue *q = &queue;
//
//    memset(q, 0, sizeof(AVPacketQueue));
//    pthread_mutex_init(&q->mutex, NULL);
//    pthread_cond_init(&q->cond, NULL);
//}
void VideoDelegate::avpacket_queue_init(AVPacketQueue *q)
{
    memset(q, 0, sizeof(AVPacketQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}
void VideoDelegate::avpacket_queue_init()
{
    avpacket_queue_init(&queue);
}

void VideoDelegate::avpacket_queue_flush(AVPacketQueue *q)
{
    AVPacketList *pkt, *pkt1;

    pthread_mutex_lock(&q->mutex);
    for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt   = NULL;
    q->first_pkt  = NULL;
    q->nb_packets = 0;
    q->size       = 0;
    pthread_mutex_unlock(&q->mutex);
}

//void VideoDelegate::avpacket_queue_end()
//{
//    AVPacketQueue *q = &queue;
//
//    avpacket_queue_flush(q);
//    pthread_mutex_destroy(&q->mutex);
//    pthread_cond_destroy(&q->cond);
//}
void VideoDelegate::avpacket_queue_end(AVPacketQueue *q)
{
    avpacket_queue_flush(q);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}
void VideoDelegate::avpacket_queue_end()
{
    avpacket_queue_end(&queue);
}

int VideoDelegate::avpacket_queue_put(AVPacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (pkt != &flush_pkt && av_dup_packet(pkt) < 0) {
        return -1;
    }

    pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
    if (!pkt1) {
        return -1;
    }
    pkt1->pkt  = *pkt;
    pkt1->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (!q->last_pkt) {
        q->first_pkt = pkt1;
    } else {
        q->last_pkt->next = pkt1;
    }

    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);

    pthread_cond_signal(&q->cond);

    pthread_mutex_unlock(&q->mutex);
    return 0;
}

int VideoDelegate::avpacket_queue_get(AVPacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret;

    pthread_mutex_lock(&q->mutex);

    for (;; ) {
        pkt1 = q->first_pkt;
        if (pkt1) {
            if (pkt1->pkt.data == flush_pkt.data) {
                ret = 0;
                break;
            }
            q->first_pkt = pkt1->next;
            if (!q->first_pkt) {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            *pkt     = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(&q->cond, &q->mutex);
        }
    }
    pthread_mutex_unlock(&q->mutex);
    return ret;
}
int VideoDelegate::avpacket_queue_get(AVPacket *pkt, int block)
{
    AVPacketQueue *q = &queue;
    return avpacket_queue_get(q, pkt, block);
}

unsigned long long VideoDelegate::avpacket_queue_size(AVPacketQueue *q)
{
    unsigned long long size;
    pthread_mutex_lock(&q->mutex);
    size = q->size;
    pthread_mutex_unlock(&q->mutex);
    return size;
}
unsigned long long VideoDelegate::avpacket_queue_size()
{
    return avpacket_queue_size(&queue);
}

HRESULT VideoDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

HRESULT VideoDelegate::VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*)
{
    void *frameBytes;
    void *audioFrameBytes;
    BMDTimeValue        frameTime, frameDuration;
    int                 hours, minutes, seconds, frames, width, height;
    HRESULT             theResult;

    /*
        Do something with the pixels if necessary
        (not needed for plain OpenGL Viewer)
        e.g.
            arrivedFrame->GetStreamTime(&frameTime, &frameDuration, 600);
    */

    framecount++;
    bool hasNoInputSource = arrivedFrame->GetFlags() & bmdFrameHasNoInputSource;
    emit captureFrameArrived();

    if (hasNoInputSource)
    {
        fprintf(stderr, "Frame received (#%d) - No signal detected ..\n", framecount);
        return S_OK;
    }
    else
    {
        AVPacket pkt;
        AVCodecContext *c;

        if ( g_verbose )
            fprintf(stderr, "Frame received (#%d) - Signal OK\n", framecount);

        av_init_packet(&pkt);
        c = video_st->codec;
        arrivedFrame->GetBytes(&frameBytes);
        arrivedFrame->GetStreamTime(&frameTime, &frameDuration, 500);
        arrivedFrame->GetStreamTime(&frameTime, &frameDuration, video_st->time_base.den);
        pkt.pts = pkt.dts = frameTime / video_st->time_base.num;

        if (initial_video_pts == AV_NOPTS_VALUE)
        {
            initial_video_pts = pkt.pts;
        }
        pkt.duration = frameDuration;
        //To be made sure it still applies
        pkt.flags       |= AV_PKT_FLAG_KEY;
        pkt.stream_index = video_st->index;
        pkt.data         = (uint8_t *)frameBytes;
        pkt.size         = arrivedFrame->GetRowBytes() * arrivedFrame->GetHeight();
        c->frame_number++;
        avpacket_queue_put(&queue, &pkt);

    }

    return S_OK;
}

void    BMCapture::SetOutputFilename(const char *filename)
{
    g_videoOutputFile = filename;
}
void    BMCapture::SetOutputFormat(AVOutputFormat *format)
{
    fmt = format;
}

BMCapture::BMCapture(int id) :
    QObject(),
    mVideoDelegate(NULL)
{
    deviceId = id;
    g_videoModeIndex = -1;
    g_audioChannels = 2;
    g_audioSampleDepth = 16;
    g_videoOutputFile = NULL;
    g_audioOutputFile = NULL;
    g_maxFrames = -1;
    g_memoryLimit = 1024 * 1024 * 1024;            // 1GByte(>50 sec)
    fmt = NULL;

}

/* Now ordinary BMCapture methods
*/
void    BMCapture::SetDisplayWindow(DisplayWindow **display_window)
{
    displayWindow = *display_window;
}

int     BMCapture::GetFrameSize(int card, int mode, int *winWidth, int *winHeight)
{
    IDeckLinkIterator* deckLinkIterator;
    IDeckLink*         deckLink;
    int                numDevices = 0;
    bool modeFound = 0;

    // Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
    deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (deckLinkIterator == NULL)
    {
        std::cerr << "A DeckLink iterator could not be created (DeckLink drivers may not be installed)." << std::endl;
        return -1;
    }

    // Try all cards in this system
    while (deckLinkIterator->Next(&deckLink) == S_OK)
    {
        // const char *            deviceNameString = NULL;

        if ( card == numDevices )
        {
            IDeckLinkInput*                         deckLinkInput = NULL;
            IDeckLinkDisplayModeIterator*           displayModeIterator = NULL;
            IDeckLinkDisplayMode*                   displayMode = NULL;
            HRESULT                                 result; 
            int mm = 0;

            // Query the DeckLink for its configuration interface
            result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
            if (result != S_OK)
            {
                std::cerr << "Could not obtain the IDeckLinkInput interface - result = " << std::hex <<result << std::endl ;
                goto bail;
            }

            // Obtain an IDeckLinkDisplayModeIterator to enumerate the display modes supported on input
            result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
            if (result != S_OK)
            {
                std::cerr << "Could not obtain the video input display mode iterator - result = " << std::hex << result << std::endl ;
                goto bail;
            }

            // List all supported input modes
            while (displayModeIterator->Next(&displayMode) == S_OK)
            {
                const char *                    displayModeString = NULL;
                
                result = displayMode->GetName(&displayModeString);
                if ( result == S_OK )
                {
                    if (mode == mm )
                    {
                        *winWidth = displayMode->GetWidth();
                        *winHeight = displayMode->GetHeight();
                        modeFound = 1;

                        displayMode->Release();
                        break;
                    }
                    mm++;
                }
                
                // Release the IDeckLinkDisplayMode object to prevent a leak
                displayMode->Release();
            }

            bail:
                // Ensure that the interfaces we obtained are released to prevent a memory leak
                if (displayModeIterator != NULL)
                        displayModeIterator->Release();

                if (deckLinkInput != NULL)
                        deckLinkInput->Release();
        }
        numDevices++;

    }

    // If no DeckLink cards were found in the system, inform the user
    if (numDevices == 0)
    {
        std::cerr << "No Blackmagic Design devices were found." << std::endl;
        return -2;
    }


    if ( modeFound )
        return 0;
    else
    {
        std::cerr << "\tNo mode number " << mode << " available on this card" << std::endl;
        std::cerr << "\tPlease choose one of the \"Supported video input display modes\" listed below." << std::endl <<std::endl;
        return -3;
    }

}

int    BMCapture::print_capabilities()
{
    IDeckLinkIterator* deckLinkIterator;
    IDeckLink*         deckLink;
    int                numDevices = 0;
    HRESULT            result;

    // Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
    deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (deckLinkIterator == NULL)
    {
        std::cerr << "A DeckLink iterator could not be created (DeckLink drivers may not be installed)." << std::endl;
        return numDevices;
    }

    // Enumerate all cards in this system
    std::cout << "Available device(s):" << std::endl;
    while (deckLinkIterator->Next(&deckLink) == S_OK)
    {
        const char *            deviceNameString = NULL;
            
        // *** Print the model name of the DeckLink card
        result = deckLink->GetModelName(&deviceNameString);
        if (result == S_OK)
        {
            std::cout << "\t" << std::setw(3) << numDevices << " =============== " << deviceNameString << " ===============" << std::endl;
        }
            
        // ** List the video output display modes supported by the card
        print_input_modes(deckLink);

        // ** List the input capabilities of the card
        print_input_capabilities(deckLink);

        // Release the IDeckLink instance when we've finished with it to prevent leaks
        deckLink->Release();

        // Increment the total number of DeckLink cards found
        numDevices++;
    }


    // If no DeckLink cards were found in the system, inform the user
    if (numDevices == 0)
        std::cout << "No Blackmagic Design devices were found." << std::endl;
    std::cout << std::endl;

    return numDevices;
}

void    BMCapture::print_input_modes(IDeckLink* deckLink)
{
    IDeckLinkInput*                         deckLinkInput = NULL;
    IDeckLinkDisplayModeIterator*           displayModeIterator = NULL;
    IDeckLinkDisplayMode*                   displayMode = NULL;
    HRESULT                                 result; 
    int mm = 0;

    // Query the DeckLink for its configuration interface
    result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the IDeckLinkInput interface - result =" << std::hex << result << std::endl;
        goto bail;
    }
        
    // Obtain an IDeckLinkDisplayModeIterator to enumerate the display modes supported on input
    result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the video input display mode iterator - result =" << std::hex << result << std::endl;
        goto bail;
    }

    // List all supported output display modes
    std::cout << "Supported video input display modes:" << std::endl;
    while (displayModeIterator->Next(&displayMode) == S_OK)
    {
        const char *                    displayModeString = NULL;
                
        result = displayMode->GetName(&displayModeString);
        if (result == S_OK)
        {
            int             modeWidth;
            int             modeHeight;
            BMDTimeValue    frameRateDuration;
            BMDTimeScale    frameRateScale;


            // Obtain the display mode's properties
            modeWidth = displayMode->GetWidth();
            modeHeight = displayMode->GetHeight();
            displayMode->GetFrameRate(&frameRateDuration, &frameRateScale);
            //printf("\t%3d) %-20s \t %d x %d \t %g FPS\n", mm, displayModeString, modeWidth, modeHeight, (double)frameRateScale / (double)frameRateDuration);
            std::cout << "\t" << std::setw(3) << mm << ") " << std::setw(20) << displayModeString << "\t" << modeWidth << "x" << modeHeight << " \t " << (double)frameRateScale / (double)frameRateDuration << " FPS" <<std::endl;
            mm++;
        }

        // Release the IDeckLinkDisplayMode object to prevent a leak
        displayMode->Release();
    }
    std::cout << std::endl;
        
  bail:
    // Ensure that the interfaces we obtained are released to prevent a memory leak
    if (displayModeIterator != NULL)
        displayModeIterator->Release();
    
    if (deckLinkInput != NULL)
        deckLinkInput->Release();

}

void    BMCapture::print_input_capabilities(IDeckLink* deckLink)
{
    IDeckLinkConfiguration*         deckLinkConfiguration = NULL;
    IDeckLinkConfiguration*         deckLinkValidator = NULL;
    int                                                     itemCount;
    HRESULT                                         result; 

    // Query the DeckLink for its configuration interface
    result = deckLink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&deckLinkConfiguration);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the IDeckLinkConfiguration interface - result = " << std::hex << result << std::endl;
        goto bail;
    }

    // Obtain a validator object from the IDeckLinkConfiguration interface.
    // The validator object implements IDeckLinkConfiguration, however, all configuration changes are ignored
    // and will not take effect.  However, you can use the returned result code from the validator object
    // to determine whether the card supports a particular configuration.

    result = deckLinkConfiguration->GetConfigurationValidator(&deckLinkValidator);
    if (result != S_OK)
    {
        std::cerr << "Could not obtain the configuration validator interface - result = " << std::hex << result << std::endl;
        goto bail;
    }

    // Use the validator object to determine which video input connections are available
    std::cout << "Supported video input connections:" << std::endl;
    itemCount = 0;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionSDI) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") SDI" << std::endl;
    }
    itemCount++;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionHDMI) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") HDMI" << std::endl;
    }
    itemCount++;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionComponent) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") Component" << std::endl;
    }
    itemCount++;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionComposite) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") Composite" << std::endl;
    }
    itemCount++;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionSVideo) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") S-Video" << std::endl;
    }
    itemCount++;
    if (deckLinkValidator->SetVideoInputFormat(bmdVideoConnectionOpticalSDI) == S_OK)
    {
        std::cout << "\t" << std::setw(3) << itemCount << ") Optical SDI" << std::endl;
    }

    std::cout << std::endl;
        
  bail:
    if (deckLinkValidator != NULL)
        deckLinkValidator->Release();

    if (deckLinkConfiguration != NULL)
        deckLinkConfiguration->Release();
}

void    BMCapture::capture(int mode, int connection, bool tiled)
{
    int         dnum, mnum, cnum, itemCount;

    IDeckLinkIterator *deckLinkIterator;
    IDeckLink         *deckLink;
    HRESULT            result;

    IDeckLinkInput*               deckLinkInput = NULL;
    IDeckLinkDisplayModeIterator* displayModeIterator = NULL;
    IDeckLinkDisplayMode*         displayMode = NULL;
    IDeckLinkConfiguration       *deckLinkConfiguration = NULL;
//    VideoDelegate                *delegate;

    std::cerr << "***** Setting up capture from device " << deviceId << ", mode " << mode << ", connection " << connection << std::endl;

    // Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
    deckLinkIterator = CreateDeckLinkIteratorInstance();
    if (deckLinkIterator == NULL)
    {
        std::cerr << "A DeckLink iterator could not be created (DeckLink drivers may not be installed)." << std::endl;
        return;
    }

    dnum = 0;
    deckLink = NULL;

    while (deckLinkIterator->Next(&deckLink) == S_OK)
    {
        if (deviceId != dnum)
        {
            dnum++;
            // Release the IDeckLink instance when we've finished with it to prevent leaks
            deckLink->Release();
            continue;
        }
        dnum++;

        const char *deviceNameString = NULL;
        // *** Print the model name of the DeckLink card
        result = deckLink->GetModelName(&deviceNameString);
        if (result == S_OK)
        {
            char deviceName[64];    
            std::cerr << "Capture device: " << deviceNameString << std::endl;

            // Query the DeckLink for its configuration interface
            result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&deckLinkInput);
            if (result != S_OK)
            {
                std::cerr << "Could not obtain the IDeckLinkInput interface - result = " << std::hex << result << std::endl;
                return;
            }

            // Obtain an IDeckLinkDisplayModeIterator to enumerate the display modes supported on input
            result = deckLinkInput->GetDisplayModeIterator(&displayModeIterator);
            if (result != S_OK)
            {
                std::cerr << "Could not obtain the video input display mode iterator - result = " << std::hex << result << std::endl;
                return;
            }

            mnum = 0;
            while (displayModeIterator->Next(&displayMode) == S_OK)
            {
                if (mode != mnum)
                {
                    mnum++;
                    // Release the IDeckLinkDisplayMode object to prevent a leak
                    displayMode->Release();
                    continue;
                }
                mnum++;

                const char *displayModeString = NULL;
                result = displayMode->GetName(&displayModeString);
                if (result == S_OK)
                {
                    BMDPixelFormat pf = bmdFormat8BitYUV;

                    std::cerr << "Stopping previous streams, if needed" << std::endl;
                    deckLinkInput->StopStreams();

                    const char *displayModeString = NULL;
                    displayMode->GetName(&displayModeString);
                    std::cerr << "Enabling video input mode: " << displayModeString << std::endl;

                    deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), pf, 0);

                    // Query the DeckLink for its configuration interface
                    result = deckLinkInput->QueryInterface(IID_IDeckLinkConfiguration, (void**)&deckLinkConfiguration);
                    if (result != S_OK)
                    {
                        std::cerr << "Could not obtain the IDeckLinkConfiguration interface: " << std::hex << result << std::endl;
                        return;
                    }

                    BMDVideoConnection conn = -1;
                    switch (connection)
                    {
                        case 0:
                            conn = bmdVideoConnectionSDI;
                            break;
                        case 1:
                            conn = bmdVideoConnectionHDMI;
                            break;
                        case 2:
                            conn = bmdVideoConnectionComponent;
                            break;
                        case 3:
                            conn = bmdVideoConnectionComposite;
                            break;
                        case 4:
                            conn = bmdVideoConnectionSVideo;
                            break;
                        case 5:
                            conn = bmdVideoConnectionOpticalSDI;
                            break;
                        default:
                            break;
                    }

                    if (deckLinkConfiguration->SetVideoInputFormat(conn) == S_OK)
                    {
                        std::cerr << "Input set to: " << connection << std::endl;
                    }

                    mVideoDelegate = new VideoDelegate();
                    deckLinkInput->SetCallback(mVideoDelegate);

                    if( tiled )
                        deckLinkInput->SetScreenPreviewCallback(displayWindow->glviewers[deviceId]);
                    else
                        deckLinkInput->SetScreenPreviewCallback(displayWindow->glviewers[0]);

                    oc = avformat_alloc_context();
                    oc->oformat = fmt;
                    snprintf(oc->filename, sizeof(oc->filename), "%s", g_videoOutputFile);
                    fmt->video_codec = (pf == bmdFormat8BitYUV ? CODEC_ID_RAWVIDEO : CODEC_ID_V210);

                    mVideoDelegate->video_st = add_video_stream(oc, fmt->video_codec, displayMode);
                    if (!(fmt->flags & AVFMT_NOFILE))
                    {
                        if (avio_open(&oc->pb, oc->filename, AVIO_FLAG_WRITE) < 0)
                        {
                            std::cerr << "Could not open " << oc->filename << std::endl;
                            exit(6);
                        }
                    }
                    avformat_write_header(oc, NULL);

                    connect(mVideoDelegate, SIGNAL(captureFrameArrived()), displayWindow, SLOT(updateFramePosition()), Qt::QueuedConnection);
                    std::cerr << "Starting capture from device " << deviceId << std::endl;
                    deckLinkInput->StartStreams();

                    mVideoDelegate->avpacket_queue_init();

                    pthread_t th;
//                    if (pthread_create(&th, NULL, BMCapture::push_packet, oc))
//                    {
//                        std::cerr << "Ahhhhhhh " << std::endl;
//                        exit(7);
//                    }
//                    // Block main thread until signal occurs
//                    pthread_mutex_lock(&sleepMutex);
//                    pthread_cond_wait(&sleepCond, &sleepMutex);
//                    pthread_mutex_unlock(&sleepMutex);
//                    deckLinkInput->StopStreams();
//                    //fprintf(stderr, "Stopping Capture\n");
//                    mVideoDelegate->avpacket_queue_end();

                }
            }
        }
    }

    // Release the IDeckLink instance when we've finished with it to prevent leaks
    if (deckLink)
        deckLink->Release();

    // Ensure that the interfaces we obtained are released to prevent a memory leak
    if (displayModeIterator != NULL)
        displayModeIterator->Release();

    return;
}

