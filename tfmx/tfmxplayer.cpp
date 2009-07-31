#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/endian.h"
#include "common/debug.h"

#include "sound/mixer.h"
#include "sound/mods/tfmx.h"

#if defined(TFMX_CMDLINE_TOOL)

#include "tfmx/tfmxdebug.h"

#define FILEDIR "" 

using namespace Common;

#define MUSICFILE "mdat.monkey"
#define SAMPLEFILE "smpl.monkey"
const int samplerate = 48000;

Audio::Tfmx *loadTfmxfile(const char *mdatName, const char *sampleName) {
	FSNode fileDir(FILEDIR);
	FSNode musicNode = fileDir.getChild(mdatName);
	FSNode sampleNode = fileDir.getChild(sampleName);
	SeekableReadStream *musicIn = musicNode.createReadStream();
	if (0 == musicIn) {
		debug("Couldnt load file %s", mdatName);
		return 0;
	}

	SeekableReadStream *sampleIn = sampleNode.createReadStream();
	if (0 == sampleIn) {
		debug("Couldnt load file %s", sampleName);
		delete musicIn;
		return 0;
	}

	Audio::Tfmx *player = new Audio::Tfmx(samplerate, true);
	player->load(*musicIn, *sampleIn);
	delete musicIn;
	delete sampleIn;

	return player;
}

void runFlac(int chan, int bits, int sr, const char *fileName);

void modcmdmain(const int argc, const char *const argv[]) {
	debug("Started Scumm&VM");
	debug("Sound celebrating utility for monkey melodies & Various Malfunctions");
	debug("");

	//simplePlaybacktest(argc, argv);

	Audio::Tfmx *player = loadTfmxfile(MUSICFILE, SAMPLEFILE);
	if (!player) {
		debug("couldnt create TFMX-Player");
		return;
	}

	int i = 1;
	int playflag = 1;
	bool hasCmd = false;

	
	while (i < argc && argv[i][0] == '-') {
		int param;
		if (!strcmp("-m", argv[i])) {
			if (i + 1 < argc) {
				param = atoi(argv[++i]);
				debug( "play Macro %02X", param);
				dumpMacro(*player, param);
				player->doMacro(0x1B, param);
				hasCmd = true;
			}
		} else if (!strcmp("-s", argv[i])) {
			if (i + 1 < argc) {
				param = atoi(argv[++i]);
				debug( "play Song %02X", param);
				dumpTrackstepsBySong(*player, param);
				player->doSong(param);
				hasCmd = true;
			}
		} else if (!strcmp("-c", argv[i])) {
			if (i + 1 < argc) {
				param = atoi(argv[++i]);
				debug( "play custom %02X", param);
				if (player->getSongIndex() < 0)
					player->doSong(0x18);
				player->doSfx(param);
				hasCmd = true;
			}
		} else  if (!strcmp("-flac", argv[i])) {
			playflag = 2;
		} else  if (!strcmp("-hack-patternstop", argv[i]))
			player->_playerCtx.stopWithLastPattern = true;
		++i;
	}

	if (!hasCmd) {
		player->doSong(4);
		dumpTrackstepsBySong(*player, 4);
	}



	
#if 0
	int16 buf[2 * 1024];

	while( true)
		player->readBuffer(buf, ARRAYSIZE(buf));
#endif
	int maxsecs = 10 * 60;
	if (playflag == 1) {
		// get Mixer, assume this never fails 
		Audio::Mixer *mixer = g_system->getMixer();

		Audio::SoundHandle soundH;

		mixer->playInputStream(Audio::Mixer::kMusicSoundType, &soundH, player);
		while (mixer->isSoundHandleActive(soundH) && --maxsecs)
			g_system->delayMillis(1000);
//		player->AllOff();

//		while (mixer->isSoundHandleActive(soundH));

		mixer->stopHandle(soundH);
		player = 0;
	}

	if (playflag == 2) {
		Common::FSNode file("out.raw");
		WriteStream *wav = file.createWriteStream();
		int16 buf[2 * 1024];
		int32 maxsamples = (maxsecs <= 0) ? 0 : maxsecs * samplerate;
		while (!player->endOfData() && maxsamples > 0) {
			int read = player->readBuffer(buf, ARRAYSIZE(buf));
			wav->write(buf, read * 2);
			maxsamples -= read/2;
		}
		delete wav;

		runFlac(2, 16, samplerate, "out.raw");
	}

#ifdef _MSC_VER
	printf("\npress a key");
	getc(stdin);
#endif
	
	delete player;
}

void runFlac( int chan, int bits, int sr, const char *fileName) {
	const char *format = "flac --endian="
#ifdef SCUMM_BIG_ENDIAN
			"big"
#else
			"little"
#endif
			" --channels=%d -f --bps=%d --sample-rate=%d --sign=signed --force-raw-format %s";
	char cmd[1024];
	sprintf(cmd, format, chan, bits, sr, fileName);
			debug(cmd);
	system(cmd);
}

#endif // #if defined(TFMX_CMDLINE_TOOL)
