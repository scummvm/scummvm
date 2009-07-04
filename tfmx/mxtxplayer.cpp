#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/endian.h"
#include "common/debug.h"

#include "sound/mixer.h"
#include "sound/mods/maxtrax.h"

#if defined(MXTX_CMDLINE_TOOL)

#define FILEDIR "" 

using namespace Common;

#define MUSICFILE "introscr.mx"
#define SAMPLEFILE "introinst.mx"

Audio::MaxTrax *loadMtmxfile(const char *mdatName, const char *smplName) {
	FSNode fileDir(FILEDIR);
	FSNode musicNode = fileDir.getChild(mdatName);
	FSNode sampleNode = fileDir.getChild(smplName);
	SeekableReadStream *musicIn = musicNode.createReadStream();
	if (0 == musicIn) {
		debug("Couldnt load file %s", mdatName);
		return 0;
	}

	Audio::MaxTrax *mxtxPlay = new Audio::MaxTrax(44100, true);

	if (!strcmp(mdatName, smplName)) {	
		SeekableReadStream *sampleIn = sampleNode.createReadStream();
		if (0 == sampleIn) {
			debug("Couldnt load file %s", smplName);
			delete musicIn;
			return 0;
		}
		mxtxPlay->load(*musicIn, true, false);
		mxtxPlay->load(*sampleIn, false, true);
		delete sampleIn;
	} else {
		mxtxPlay->load(*musicIn, true, true);
	}

	delete musicIn;

	return mxtxPlay;
}

void runFlac(int chan, int bits, int sr, const char *fileName);

void modcmdmain(const int argc, const char *const argv[]) {
	debug("Started Scumm&VM");
	debug("Sound celebrating utility for malcoms menace & Various Malfunctions");
	debug("");

	Audio::MaxTrax *player = loadMtmxfile(MUSICFILE, SAMPLEFILE);
	if (!player) {
		debug("couldnt create MXTX-Player");
		return;
	}

		int i = 1;
	int playflag = 1;
	bool hasCmd = false;

	
	while (i < argc && argv[i][0] == '-') {
		int param;
		if (!strcmp("-s", argv[i])) {
			if (i + 1 < argc) {
				param = atoi(argv[++i]);
				debug( "play Song %02X", param);
				
				hasCmd = true;
			}
		} else  if (!strcmp("-flac", argv[i])) {
			playflag = 2;
		}
		++i;
	}

	if (!hasCmd) {
	}

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
		int32 maxsamples = (maxsecs <= 0) ? 0 : maxsecs * 44100;
		while (!player->endOfData() && maxsamples > 0) {
			int read = player->readBuffer(buf, ARRAYSIZE(buf));
			wav->write(buf, read * 2);
			maxsamples -= read/2;
		}
		delete wav;

		runFlac(2, 16, 44100, "out.raw");
	}
	delete player;

#ifdef _MSC_VER
	printf("\npress a key");
	getc(stdin);
#endif
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

#endif // #if defined(MXTX_CMDLINE_TOOL)