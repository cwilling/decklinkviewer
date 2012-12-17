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

#include "bmcapture.h"



BMCapture::BMCapture() : VideoDelegate()
{

}

void    BMCapture::announce()
{
    std::cout << "Announcing: BMCapture instance started ..." << std::endl;
}

void    BMCapture::SetDisplayWindow(DisplayWindow **dispay_window)
{
    displayWindow = *dispay_window;
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

void    BMCapture::print_capabilities()
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
        return;
    }

    // Enumerate all cards in this system
    std::cout << "Device(s):" << std::endl;
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

void    BMCapture::capture(int device, int mode, int connection)
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

    std::cerr << "Starting> Capture on device " << device << ", mode " << mode << ", connection " << std::endl;

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
        if (device != dnum)
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
            std::cerr << "Using device " << deviceNameString << std::endl;

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
                    std::cerr << "Enable video input: " << displayModeString << std::endl;

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

                    //delegate = new VideoDelegate();
                    deckLinkInput->SetCallback(this);

                    deckLinkInput->SetScreenPreviewCallback(displayWindow->glviewer);

                    std::cerr << "Starting capture" << std::endl;
                    deckLinkInput->StartStreams();
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

HRESULT BMCapture::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

HRESULT BMCapture::VideoInputFrameArrived (IDeckLinkVideoInputFrame* arrivedFrame, IDeckLinkAudioInputPacket*)
{
    BMDTimeValue        frameTime, frameDuration;
    int                 hours, minutes, seconds, frames, width, height;
    HRESULT             theResult;

    /*
        Do something with the pixels if necessary
        (not needed for plain OpenGL Viewer)
        e.g.
            arrivedFrame->GetStreamTime(&frameTime, &frameDuration, 600);
    */

    //displayWindow->emit_frame_update();

    return S_OK;
}

