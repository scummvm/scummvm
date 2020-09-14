# The configuration file

## Location

The configuration file saves to different default locations depending on the platform on which you are running ScummVM. 

| Platform| Config File Path|
|-|-|
|

Most settings are also accessible directly from the Launcher. To edit global settings, click **Options**. To edit game-specific settings, highlight a game and then click **Edit Game**.
For more information, see the [global settings](using/global_settings) and [game settings](using/game_settings) pages. 

## Keywords

There are many recognized configuration keywords:

````

path               string   The path to where a game's data files are
autosave_period    number   The seconds between autosaving (default: 300)
save_slot          number   The saved game number to load on startup.
savepath           string   The path to where a game will store its
                            saved games.
screenshotpath     string   The path to where screenshots are saved.
iconpath           string   The path to where to look for icons to use as
                            overlay for the ScummVM icon in the Windows
                            taskbar or macOS X Dock when running a game.
                            The icon files should be named after the game
                            ids and be in ico format on Windows or png
                            format on macOS X.
versioninfo        string   The version of the ScummVM that created the
                            configuration file.

gameid             string   The real id of a game. Useful if you have
                            several versions of the same game, and want
                            different aliases for them. See the example.
description        string   The description of the game as it will appear
                            in the launcher.

language           string   Specify language (en, us, de, fr, it, pt, es,
                            jp, zh, kr, se, gb, hb, cz, ru)
speech_mute        bool     If true, speech is muted
subtitles          bool     Set to true to enable subtitles.
talkspeed          number   Text delay in SCUMM games, or text speed in
                            other games.

fullscreen         bool     Fullscreen mode
aspect_ratio       bool     Enable aspect ratio correction
gfx_mode           string   Graphics mode (normal, 2x, 3x, 2xsai,
                            super2xsai, supereagle, advmame2x, advmame3x,
                            hq2x, hq3x, tv2x, dotmatrix, opengl)
filtering          bool     Enable graphics filtering

confirm_exit       bool     Ask for confirmation by the user before
                            quitting (SDL backend only).
console            bool     Enable the console window (default: enabled)
                            (Windows only).
cdrom              number   Number of CD-ROM unit to use for audio. If
                            negative, don't even try to access the CD-ROM.
joystick_num       number   Number of joystick device to use for input
controller_map_db  string   A custom controller mapping file to load to
                            complete default database (SDL backend only).
                            Otherwise, file gamecontrollerdb.txt will be
                            loaded from extrapath.
music_driver       string   The music engine to use.
opl_driver         string   The AdLib (OPL) emulator to use.
output_rate        number   The output sample rate to use, in Hz. Sensible
                            values are 11025, 22050 and 44100.
audio_buffer_size  number   Overrides the size of the audio buffer. The
                            value must be one of: 256 512 1024 2048 4096
                            8192 16384 32768. The default value is
                            calculated based on the output_rate to keep
                            audio latency below 45ms.
alsa_port          string   Port to use for output when using the
                            ALSA music driver.
music_volume       number   The music volume setting (0-255)
multi_midi         bool     If true, enable combination AdLib and native
                            MIDI.
soundfont          string   The SoundFont to use for MIDI playback. (Only
                            supported by some MIDI drivers.)
native_mt32        bool     If true, disable GM emulation and assume that
                            there is a true Roland MT-32 available.
enable_gs          bool     If true, enable Roland GS-specific features to
                            enhance GM emulation. If native_mt32 is also
                            true, the GS device will select an MT-32 map
                            to play the correct instruments.
sfx_volume         number   The sfx volume setting (0-255)
tempo              number   The music tempo (50-200) (default: 100)
speech_volume      number   The speech volume setting (0-255)
midi_gain          number   The MIDI gain (0-1000) (default: 100) (Only
                            supported by some MIDI drivers.)

copy_protection    bool     Enable copy protection in certain games, in
                            those cases where ScummVM disables it by
                            default.
demo_mode          bool     Start demo in Maniac Mansion
alt_intro          bool     Use alternative intro for CD versions of
                            Beneath a Steel Sky and Flight of the Amazon
                            Queen

boot_param         number   Pass this number to the boot script
````

## Example file

````
    [scummvm]
    gfx_mode=supereagle
    fullscreen=true
    savepath=C:\saves\

    [sky]
    path=C:\games\SteelSky\

    [germansky]
    gameid=sky
    language=de
    path=C:\games\SteelSky\
    description=Beneath a Steel Sky w/ German subtitles

    [germandott]
    gameid=tentacle
    path=C:\german\tentacle\
    description=German version of DOTT

    [tentacle]
    path=C:\tentacle\
    subtitles=true
    music_volume=40
    sfx_volume=255

    [loomcd]
    cdrom=1
    path=C:\loom\
    talkspeed=5
    savepath=C:\loom\saves\

    [monkey2]
    path=C:\amiga_mi2\
    music_driver=windows

````