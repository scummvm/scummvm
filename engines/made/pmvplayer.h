#ifndef MADE_PMVPLAYER_H
#define MADE_PMVPLAYER_H

#include "common/system.h"
#include "common/events.h"
#include "common/file.h"
#include "common/endian.h"
#include "graphics/surface.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"

#include "made/graphics.h"
#include "made/sound.h"

namespace Made {

class PmvPlayer {
public:
    PmvPlayer(OSystem *system, Audio::Mixer *mixer);
    ~PmvPlayer();
    void play(const char *filename);
protected:
    OSystem *_system;
    Audio::Mixer *_mixer;
    Common::File *_fd;
    Audio::AppendableAudioStream *_audioStream;
    Audio::SoundHandle _audioStreamHandle;
    byte _palette[768];
    Graphics::Surface *_surface;
    bool _abort;
    void readChunk(uint32 &chunkType, uint32 &chunkSize);
    void handleEvents();
    void updatePalette();
    void updateScreen();
};

}

#endif
