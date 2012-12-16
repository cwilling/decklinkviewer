/* ex:set ai shiftwidth=4 inputtab=spaces smarttab noautotab: */
#include <QApplication>

#include "bmcapture.h"
#include "displaywindow.h"
#include <getopt.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int c;
    int card = 0;
    int mode = -1;
    int connexion = -1;
    int winWidth, winHeight;
    DisplayWindow *displayWindow;

    BMCapture bmCapture;
    bmCapture.announce();

    while ((c = getopt(argc, argv, "lhc:m:i:")) != EOF )
    {
        switch(c)
        {
            case 'l':
                bmCapture.print_capabilities();
                exit(0);
                break;

            case 'h':
//                fprintf(stderr, "Usage> %s \n", argv[0]);
                std::cerr << "Usage> " << argv[0] << std::endl;
                std::cerr << "\t\t\t -l list capabilities" << std::endl;
                std::cerr << "\t\t\t -c select Card to use (default 0)" << std::endl;
                std::cerr << "\t\t\t -m select input video Mode" << std::endl;
                std::cerr << "\t\t\t -i select connection Interface" << std::endl;
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
                connexion = atoi(optarg);
                break;

            default:
                fprintf(stderr, "Decklink Capture Program\n");
                exit(0);
                break;
        }
    }
    if ( (mode < 0) || (connexion < 0) )
    {
        std::cerr << "Please specify both input mode (-m) and connection interface (-i) from the following lists" << std::endl;
        bmCapture.print_capabilities();
        exit(1);
    }

    if (bmCapture.GetFrameSize(card, mode, &winWidth, &winHeight) < 0 )
    {
	bmCapture.print_capabilities();
	exit(1);
    }

    displayWindow = new DisplayWindow(winWidth, winHeight);
    bmCapture.SetDisplayWindow(&displayWindow);
    bmCapture.capture(card, mode, connexion);

//    displayWindow = DisplayWindow(winWidth, winHeight);
//    displayWindow.show();
    return app.exec();
}

