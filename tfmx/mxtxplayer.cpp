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

// #include "tfmx/tfmxdebug.h"

#define FILEDIR "" 

using namespace Common;

#define MUSICFILE "introscr.mx"

bool load(Common::SeekableReadStream &musicData) {
	bool res = false;

	char buf[2 * 1024];
	uint16 tempo, flags;
	uint16 numScores;

	// 0x0000: 4 Bytes Header "MXTX"
	// 0x0004: uint16 tempo
	// 0x0006: uint16 flags. bit0 = lowpassfilter, bit1 = attackvolume, bit15 = microtonal
	musicData.read(buf, 4);
	tempo = musicData.readUint16BE();
	flags = musicData.readUint16BE();
	buf[4] = '\0';
	debug("Header: %s %02X %02X", buf, tempo, flags);

	if (flags & (1 << 15)) {
		// uint16 microtonal[128]
		musicData.skip(128 * 2);
	}

	// uint16 number of Scores
	numScores = musicData.readUint16BE();
	debug("#Scores: %d", numScores);
	int scoresLoaded = 0;
	byte *scorePtr; // array of scorestructures
	for (int i = 0; i < numScores; ++i) {
		uint32 numEvents = musicData.readUint32BE();
		uint32 dataLength = numEvents * 6;
		const int scoremax = 128; // some variable which is set upon initialisation of player
		if (scoresLoaded < scoremax) {
			// allocate dataLength zeroed bytes
			// increase _globaldata+glob_TotalScores and _maxtrax+mxtx_TotalScores
			// load events data
			debug("score %i: %i Events", scoresLoaded, numEvents);
			for (int j = 0; j < numEvents; ++j) {
				byte command, data;
				uint16 startTime, stopTime;
				command = musicData.readByte();
				data = musicData.readByte();
				startTime = musicData.readUint16BE();
				stopTime = musicData.readUint16BE();
				debug("cmd, data, start, stop: %02X, %02X, %04X, %04X", command, data, startTime, stopTime);

			}
			debug("");
			// store pointer to events and # events in scorePtr, then increase scorePtr by structsize
			scoresLoaded++;
		} else
			musicData.skip(dataLength);
	}

	uint16 numSamples;
	// uint16 number of Samples
	numSamples = musicData.readUint16BE();
	for (int i = 0; i < numSamples; ++i) {
		// load disksample structure
		uint16 number = musicData.readUint16BE();
		uint16 tune = musicData.readUint16BE();
		uint16 volume = musicData.readUint16BE();
		uint16 octaves = musicData.readUint16BE();
		uint32 attackLen = musicData.readUint32BE();
		uint32 sustainLen = musicData.readUint32BE();
		uint16 attackCount = musicData.readUint16BE();
		uint16 releaseCount = musicData.readUint16BE();

		byte *samplePtr = 0; // samplestructure ptrs
		samplePtr += number;

		byte *patchPtr = 0; // array of patchstructs
		patchPtr += number;

		// Tune and Volume Info
		// copy tune, volume to patch_Tune, patch_Volume

		// Attack Segment
		int attacksize = attackCount * 4;
		// allocate attacksize bytes
		// store allocated Ptr in patch_Attack
		// store attackCount in patch_AttackCount

		// read attack segment
		for (int j = 0; j < attackCount; ++j) {
			uint16 envDuration = musicData.readUint16BE();
			uint16 envVolume = musicData.readUint16BE();
			// store into patch_Attack
		}

		// Release Segment
		int releasesize = releaseCount * 4;
		// allocate releasesize bytes
		// store allocated Ptr in patch_Release
		// store attackCount in patch_ReleaseCount

		// read release segment
		for (int j = 0; j < releaseCount; ++j) {
			uint16 envDuration = musicData.readUint16BE();
			uint16 envVolume = musicData.readUint16BE();
			// store into patch_Release
		}













	}

	/*		

		STRUCTURE	PatchData,0
			APTR	patch_Sample				; Amiga sample data		
			APTR	patch_Attack				; array of env. segments	
			APTR	patch_Release				; array of env. segments	
			WORD	patch_AttackCount			; number of attack env.	
			WORD	patch_ReleaseCount			; number of release env.	
			WORD	patch_Volume				; sample volume 			
			WORD	patch_Tune					; sample tuning			
			BYTE	patch_Number				; self-identifing			
			BYTE	patch_pad
			LABEL	patch_sizeof
			
		STRUCTURE	DiskSample,0
			WORD	dsamp_Number
			WORD	dsamp_Tune
			WORD	dsamp_Volume
			WORD	dsamp_Octaves
			LONG	dsamp_AttackLength
			LONG	dsamp_SustainLength
			WORD	dsamp_AttackCount
			WORD	dsamp_ReleaseCount
			LABEL	dsamp_sizeof

		STRUCTURE	CookedEvent,0
			BYTE	cev_Command
			BYTE	cev_Data
			WORD	cev_StartTime
			WORD	cev_StopTime
			LABEL	cev_sizeof

		STRUCTURE	EnvelopeData,0
			WORD	env_Duration				; duration in milliseconds	
			WORD	env_Volume					; volume of envelope		
			LABEL	env_sizeof

		STRUCTURE	SampleData,0
			APTR	samp_NextSample
			APTR	samp_Waveform
			LONG	samp_AttackSize
			LONG	samp_SustainSize
			LABEL	samp_sizeof
			*/
	return res;
}

void *loadMtmxfile(const char *mdatName) {
	FSNode fileDir(FILEDIR);
	FSNode musicNode = fileDir.getChild(mdatName);
	SeekableReadStream *musicIn = musicNode.createReadStream();
	if (0 == musicIn) {
		debug("Couldnt load file %s", mdatName);
		return 0;
	}

	load(*musicIn);


	delete musicIn;

	return 0;
}

void modcmdmain(const int argc, const char *const argv[]) {
	debug("Started Scumm&VM");
	debug("Sound celebrating utility for malcoms menace & Various Malfunctions");
	debug("");

	loadMtmxfile(MUSICFILE);

#ifdef _MSC_VER
	printf("\npress a key");
	getc(stdin);
#endif
}

#endif // #if defined(MXTX_CMDLINE_TOOL)