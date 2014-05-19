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

#include <QApplication>

#include "bmcapture.h"
#include "displaywindow.h"
#include <getopt.h>

#define MAX_CAPTURE_TILES     (32)

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int c;
    int card = 0;
    int mode = -1;
    int connection = -1;
    int winWidth, winHeight;
    DisplayWindow *displayWindow;
    bool do_tile_inputs = false;
    bool do_slide = false;
    int tiles_requested = 0;
    int tiles_available = 0;
    BMCapture *bmCaptureDevices[MAX_CAPTURE_TILES]; /* from displaywindow.h */
    const char *video_output_filename = NULL;
    AVOutputFormat *fmt = NULL;

    av_register_all();

    BMCapture *bmCapture = new BMCapture(0);

    while ((c = getopt(argc, argv, "lhc:m:i:tsf:F:")) != EOF )
    {
        switch(c)
        {
            case 'l':
                bmCapture->print_capabilities();
                exit(0);
                break;

            case 'h':
                std::cerr << "Usage> " << argv[0] << std::endl;
                std::cerr << "\t\t\t -l list capabilities" << std::endl;
                std::cerr << "\t\t\t -c select Card to use (default 0)" << std::endl;
                std::cerr << "\t\t\t -m select input video Mode" << std::endl;
                std::cerr << "\t\t\t -i select connection Interface" << std::endl;
                std::cerr << "\t\t\t -t Tile multiple capture cards" << std::endl;
                std::cerr << "\t\t\t -s Slide across the screen" << std::endl;
                exit(0);
                break;

            case 'c':
                // The capture card to use
                card = atoi(optarg);
                break;

            case 'm':
                // The capture video mode
                mode = atoi(optarg);
                break;

            case 'i':
                // The interface to use
                connection = atoi(optarg);
                break;

            case 's':
                // Whether to slide window across the screen
                do_slide = true;
                break;

            case 't':
                // Whether to tile multiple input cards
                do_tile_inputs = true;
                break;

            case 'f':
                video_output_filename = optarg;
                break;

            case 'F':
                fmt = av_guess_format(optarg, NULL, NULL);
                break;

            default:
                std::cerr << "Decklink Viewer Program" << std::endl;
                exit(0);
                break;
        }
    }
    if ( (mode < 0) || (connection < 0) )
    {
        std::cerr << "Please specify both input mode (-m) and connection interface (-i) from the following lists" << std::endl;
        bmCapture->print_capabilities();
        exit(1);
    }

    tiles_available = bmCapture->print_capabilities();
    if ( tiles_available > MAX_CAPTURE_TILES )
    {
        std::cerr << "Too many capture devices (" << tiles_available << ")!"  << std::endl;
        tiles_available = MAX_CAPTURE_TILES;
        std::cerr << "Limiting capture devices to " << tiles_available << std::endl;
    }
    if ( tiles_available < 1 )
    {
        std::cerr << "No devices available. Exiting now ..." << std::endl;
        exit(2);
    }
    bmCaptureDevices[0] = bmCapture;
    for(int i=1;i<tiles_available;i++)
    {
        bmCaptureDevices[i] = new BMCapture(i);
    }

    // Check output file & format
    if (!video_output_filename)
    {
        std::cerr << "No output file specified. For piping to mplayer via stdin, try adding: -f pipe:1" << std::endl;
        exit(3);
    }
    if (!fmt)
    {
        fmt = av_guess_format(NULL, video_output_filename, NULL);
        if (!fmt)
        {
            std::cerr << "No output format specified. Try adding: -F nut" << std::endl;
            exit(4);
        }
    }
    std::cerr << "File " << video_output_filename << ", format " << fmt << std::endl;

    // Set up anything that applies to all tiles
    for(int i=0;i<tiles_available;i++)
    {
        bmCaptureDevices[i]->SetOutputFilename(video_output_filename);
        bmCaptureDevices[i]->SetOutputFormat(fmt);
    }

    // Assume the first device has id = 0
    if (bmCaptureDevices[0]->GetFrameSize(0, mode, &winWidth, &winHeight) < 0 )
    {
	bmCaptureDevices[0]->print_capabilities();
	exit(1);
    }

    if( do_tile_inputs )
    {
        displayWindow = new DisplayWindow(winWidth, winHeight, tiles_available);

        for(int i=0;i<tiles_available;i++)
        {
            bmCaptureDevices[i]->SetDisplayWindow(&displayWindow);
            bmCaptureDevices[i]->capture(mode, connection, do_tile_inputs);
        }
    }
    else
    {
        displayWindow = new DisplayWindow(winWidth, winHeight, 1);
        bmCaptureDevices[card]->SetDisplayWindow(&displayWindow);
        bmCaptureDevices[card]->capture(mode, connection, do_tile_inputs);
    }

    if( do_slide )
        displayWindow->setScreenCrawl(do_slide);

    return app.exec();
}

