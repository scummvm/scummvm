/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
 
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/intro.h"
#include "ultima/ultima4/music.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/progress_bar.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/sound.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/utils.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

bool verbose = false;
bool quit = false;
bool useProfile = false;
Common::String profileName = "";

Performance perf("debug/performance.txt");

using namespace std;



int main(int argc, char *argv[]) {
    Debug::initGlobal("debug/global.txt");
    
#if defined(MACOSX)
    osxInit(argv[0]);
#endif

    if (!u4fopen("AVATAR.EXE"))
	{
        errorFatal(	"xu4 requires the PC version of Ultima IV to be present. "
        			"It must either be in the same directory as the xu4 executable, "
        			"or in a subdirectory named \"ultima4\"."
        			"\n\nThis can be achieved by downloading \"UltimaIV.zip\" from www.ultimaforever.com"
        			"\n - Extract the contents of UltimaIV.zip"
        			"\n - Copy the \"ultima4\" folder to your xu4 executable location."
        			"\n\nVisit the XU4 website for additional information.\n\thttp://xu4.sourceforge.net/");
	}

	unsigned int i;
    int skipIntro = 0;


    /*
     * if the -p or -profile arguments are passed to the application,
     * they need to be identified before the settings are initialized.
     */
    for (i = 1; i < (unsigned int)argc; i++) {
        if (((strcmp(argv[i], "-p") == 0)
          || (strcmp(argv[i], "-profile") == 0)
          || (strcmp(argv[i], "--profile") == 0))
                && (unsigned int)argc > i + 1) {
            // when grabbing the profile name:
            // 1. trim leading whitespace
            // 2. truncate the Common::String at 20 characters
            // 3. then strip any trailing whitespace
            profileName = argv[i+1];
            profileName.erase(0,profileName.findFirstNotOf(' '));
			profileName = Common::String(profileName.c_str(), 20);
            profileName.erase(profileName.findLastNotOf(' ') + 1);

            // verify that profileName is valid, otherwise do not use the profile
            if (!profileName.empty()) {
                useProfile = true;
            }
            i++;
            break;
        }
    }

    /* initialize the settings */
    settings.init(useProfile, profileName);

    /* update the settings based upon command-line arguments */
    for (i = 1; i < (unsigned int)argc; i++) {
        if (strcmp(argv[i], "-filter") == 0
          || strcmp(argv[i], "--filter") == 0)
        {
            if ((unsigned int)argc > i + 1)
            {
                settings.filter = argv[i+1];
                i++;
            }
            else
                errorFatal("%s is invalid alone: Requires a Common::String for input. See --help for more detail.\n", argv[i]);

        }
        else if (strcmp(argv[i], "-s") == 0
               || strcmp(argv[i], "-scale") == 0
               || strcmp(argv[i], "--scale") == 0)
        {
            if ((unsigned int)argc > i + 1)
            {
                settings.scale = strtoul(argv[i+1], NULL, 0);
                i++;
            }
            else
                errorFatal("%s is invalid alone: Requires a number for input. See --help for more detail.\n", argv[i]);


        }
        else if ( strcmp(argv[i], "-p") == 0
                || strcmp(argv[i], "-profile") == 0
                || strcmp(argv[i], "--profile") == 0)
        {
            // do nothing
            if ((unsigned int)argc > i + 1)
                i++;
            else
                errorFatal("%s is invalid alone: Requires a Common::String as input. See --help for more detail.\n", argv[i]);

        }
        else if (strcmp(argv[i], "-i") == 0
              || strcmp(argv[i], "-skipintro") == 0
              || strcmp(argv[i], "--skip-intro") == 0)
        {
                skipIntro = 1;
        }
        else if (strcmp(argv[i], "-v") == 0
              || strcmp(argv[i], "-verbose") == 0
              || strcmp(argv[i], "--verbose") == 0)
        {
            verbose = true;
        }
        else if (strcmp(argv[i], "-f") == 0
              || strcmp(argv[i], "-fullscreen") == 0
              || strcmp(argv[i], "--fullscreen") == 0)
        {
            settings.fullscreen = 1;
        }
        else if (strcmp(argv[i], "-q") == 0
              || strcmp(argv[i], "-quiet") == 0
              || strcmp(argv[i], "--quiet") == 0)
        {
            settings.musicVol = 0;
            settings.soundVol = 0;
        }
        else if (strcmp(argv[i], "-h") == 0
              || strcmp(argv[i], "-help") == 0
              || strcmp(argv[i], "--help") == 0)
        {
            debug("xu4: Ultima IV Recreated\n");

            debug("-v, --verbose		Runs xu4 in verbose mode. Increased console output.\n");
            debug("-q, --quiet		Sets all audio volume to zero.\n");
            debug("-f, --fullscreen	Runs xu4 in fullscreen mode.\n");
            debug("-i, --skip-intro	Skips the intro and loads the last savegame.\n");

            debug("\n-s <int>,\n");
            debug("--scale <int>		Used to specify scaling options.\n");
            debug("-p <Common::String>,\n");
            debug("--profile <Common::String>	Used to pass extra arguements to the program.\n");
            debug("--filter <Common::String>	Used to specify filtering options.\n");

            debug("\n-h, --help		Prints this message.\n");

            debug("\nHomepage: http://xu4.sourceforge.com\n");

            return 0;
        }
        else
            errorFatal("Unrecognized argument: %s\n\nUse --help for a list of supported arguments.", argv[i]);

    }

    xu4_srandom();

    perf.start();
    screenInit();
    ProgressBar pb((320/2) - (200/2), (200/2), 200, 10, 0, (skipIntro ? 4 : 7));
    pb.setBorderColor(240, 240, 240);
    pb.setColor(0, 0, 128);
    pb.setBorderWidth(1);

    screenTextAt(15, 11, "Loading...");
    screenRedrawScreen();
    perf.end("Screen Initialization");
    ++pb;

    perf.start();
    soundInit();
    perf.end("Misc Initialization");
    ++pb;

    perf.start();
    Tileset::loadAll();
    perf.end("Tileset::loadAll()");
    ++pb;

    perf.start();
    creatureMgr->getInstance();
    perf.end("creatureMgr->getInstance()");
    ++pb;

    intro = new IntroController();
    if (!skipIntro)
    {
        /* do the intro */
        perf.start();
        intro->init();
        perf.end("introInit()");
        ++pb;

        perf.start();
        intro->preloadMap();
        perf.end("intro->preloadMap()");
        ++pb;

        perf.start();
        musicMgr->init();
        perf.end("musicMgr->init()");
        ++pb;

        /* give a performance report */
        if (settings.debug)
            perf.report();

        eventHandler->pushController(intro);
        eventHandler->run();
        eventHandler->popController();
        intro->deleteIntro();
    }

    eventHandler->setControllerDone(false);
    if (quit)
        return 0;

    perf.reset();

    /* play the game! */
    perf.start();
    game = new GameController();
    game->init();
    perf.end("gameInit()");
    
    /* give a performance report */
    if (settings.debug)
        perf.report("\n===============================\n\n");

    eventHandler->pushController(game);
    eventHandler->run();
    eventHandler->popController();

    Tileset::unloadAll();

    delete musicMgr;
    soundDelete();
    screenDelete();

    return 0;
}

} // End of namespace Ultima4
} // End of namespace Ultima
