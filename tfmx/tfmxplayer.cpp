#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/endian.h"
#include "common/debug.h"

#include "sound/mixer.h"
#include "sound/mods/protracker.h"
#include "sound/mods/tfmx.h"

#include "tfmx/tfmxdebug.h"

#define FILEDIR "" 

using namespace Common;

void simplePlaybacktest(int argc, const char *const argv[]) {
	const char *modFilename = "mod.protracker";
	if (argc == 2)
		modFilename = argv[1];


	// get Mixer, assume this never fails 
	Audio::Mixer *mixer = g_system->getMixer();

	FSNode fileDir(FILEDIR);
	debug( "searching for Files in Directory: %s", fileDir.getPath().c_str());

	FSNode musicFile = fileDir.getChild(modFilename);

	SeekableReadStream *fileIn = musicFile.createReadStream();
	if (0 == fileIn) {
		debug( "cant open File %s", musicFile.getName().c_str());
		return;
	}

	Audio::AudioStream *stream = Audio::makeProtrackerStream(fileIn);
	delete fileIn;
	if (0 == stream) {
		debug( "cant open File %s as Protacker-Stream", musicFile.getName().c_str());
		return;
	}

	Audio::SoundHandle soundH;

	mixer->playInputStream(Audio::Mixer::kMusicSoundType, &soundH, stream);
	while (mixer->isSoundHandleActive(soundH))
		g_system->delayMillis(1000);
	

	//mixer->stopAll();
}

#define MUSICFILE "mdat.monkey"
#define SAMPLEFILE "smpl.monkey"

//#define MUSICFILE "mdat.tworld_1"
//#define SAMPLEFILE "smpl.tworld_1"
Audio::Tfmx *loadTfmxfile(const char *mdatName, const char *sampleName) {
	FSNode fileDir(FILEDIR);
	FSNode musicNode = fileDir.getChild(mdatName);
	FSNode sampleNode = fileDir.getChild(sampleName);
	SeekableReadStream *musicIn = musicNode.createReadStream();
	if (0 == musicIn) {
		debug("Couldnt load file %s", MUSICFILE);
		return 0;
	}

	SeekableReadStream *sampleIn = sampleNode.createReadStream();
	if (0 == sampleIn) {
		debug("Couldnt load file %s", SAMPLEFILE);
		delete musicIn;
		return 0;
	}

	Audio::Tfmx *player = new Audio::Tfmx(44100, true);
	player->load(*musicIn, *sampleIn);
	delete musicIn;
	delete sampleIn;

	return player;
}

void runFlac(int chan, int bits, int sr, const char *fileName);

void tfmxmain(const int argc, const char *const argv[]) {
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
	int playflag = 0;

	
	if (i < argc && argv[i][0] == '-' && strlen(argv[i]) == 2) {
		int param;
		switch (argv[i++][1]) {
		case 'm':
			if (i < argc) {
				param = atoi(argv[i]);
				debug( "play Macro %02X", param);
				dumpMacro(*player, param);
				playflag = 1;
				player->doMacro(param,param);
				++i;
			}
			break;
		case 's':
			if (i < argc) {
				param = atoi(argv[i]);
				debug( "play Song %02X", param);
				dumpTrackstepsBySong(*player, param);
				playflag = 1;
				player->doSong(param);
				++i;
			}
		}
	}

	if (!playflag) {
		playflag = 1;
		player->doMacro(0x17,0x1B);
		//player->doSong(4);
		//dumpTrackstepsBySong(*player, 4);
	}



	
#if 0
	int16 buf[2 * 1024];

	while( true)
		player->readBuffer(buf, ARRAYSIZE(buf));
#endif
	int maxsecs = 2 * 60;
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


