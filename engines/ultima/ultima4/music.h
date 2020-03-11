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

#ifndef ULTIMA4_MUSIC_H
#define ULTIMA4_MUSIC_H

#include "ultima/ultima4/debug.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

#define musicMgr   (Music::getInstance())

#define CAMP_FADE_OUT_TIME          1000
#define CAMP_FADE_IN_TIME           0
#define INN_FADE_OUT_TIME           1000
#define INN_FADE_IN_TIME            5000
#define NLOOPS -1

struct _Mix_Music;
typedef _Mix_Music OSMusicMixer;

class Music {
public:
    enum Type {
        NONE,
        OUTSIDE,
        TOWNS,
        SHRINES,
        SHOPPING,
        RULEBRIT,
        FANFARE,
        DUNGEON,
        COMBAT,
        CASTLES,
        MAX
    };
    Music();
    ~Music();
    

    /** Returns an instance of the Music class */
    static Music *getInstance() {
        if (!instance)
            instance = new Music();
        return instance;
    }

    /** Returns true if the mixer is playing any audio. */
    static bool isPlaying() {return getInstance()->isPlaying_sys();}
    static void callback(void *);    

    void init() {}
    void play();
    void stop()         {on = false; stopMid();} /**< Stop playing music */
    void fadeOut(int msecs);
    void fadeIn(int msecs, bool loadFromMap);
    void lordBritish()  {playMid(RULEBRIT); } /**< Music when you talk to Lord British */
    void hawkwind()     {playMid(SHOPPING); } /**< Music when you talk to Hawkwind */
    void camp()         {fadeOut(1000);     } /**< Music that plays while camping */
    void shopping()     {playMid(SHOPPING); } /**< Music when talking to a vendor */
    void intro()        
    {
#ifdef IOS
        on = true; // Force iOS to turn this back on from going in the background.
#endif
        playMid(introMid);
    } /**< Play the introduction music on title loadup */
    void introSwitch(int n);
    bool toggle();

    int decreaseMusicVolume();
    int increaseMusicVolume();
    void setMusicVolume(int volume) {setMusicVolume_sys(volume);}
    int decreaseSoundVolume();
    int increaseSoundVolume();
    void setSoundVolume(int volume) {setSoundVolume_sys(volume);}


    /*
     * Static variables
     */
private:
    void create_sys();
    void destroy_sys();
    void setMusicVolume_sys(int volume);
    void setSoundVolume_sys(int volume);
    void fadeOut_sys(int msecs);
    void fadeIn_sys(int msecs, bool loadFromMap);
    bool isPlaying_sys();

    static Music *instance;
    static bool fading;
    static bool on;


    bool load_sys(const Common::String &pathname);
    void playMid(Type music);
    void stopMid();

    bool load(Type music);

public:
    static bool functional;

    /*
     * Properties
     */
    Std::vector<Common::String> filenames;
    Type introMid;
    Type current;
    OSMusicMixer *playing;
    Debug *logger;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
