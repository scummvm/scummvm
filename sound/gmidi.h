#if !defined(gmidi_h)
#define gmidi_h

/* General Midi header file */
#define SEQ_MIDIPUTC    5
#define SPECIAL_CHANNEL 9
#define DEVICE_NUM 0

struct MidiEvent {
	uint32 delta;
	uint32 event;
};

/* Lowlevel Abstract Midi Driver Class */
class MidiDriver {
	
public:
	/* called whenever the midi driver is in streaming mode,
	 * and more midi commands need to be generated
	 * return 0 to tell the mididriver that the end of stream was reached
	 */
	typedef int StreamCallback(void *param, MidiEvent *ev, int num);
	

	/* open modes, pass one of those to open() */
	enum {
		MO_SIMPLE = 1,
		MO_STREAMING = 2,
	};

	/* Special events that can be inserted in a MidiEvent.
	 * event = (ME_xxx<<24) | <24-bit data associated with event>
	 */
	enum {
		ME_NONE  = 0,
		ME_TEMPO = 1,
	};

	/* error codes returned by open.
	 * can be converted to a string with get_error_name()
	 */
	enum {
		MERR_CANNOT_CONNECT = 1,
		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4,
	};

	enum {
		PROP_TIMEDIV = 1,
	};

	
	/* destroy the midi object */
	virtual void destroy() = 0;

	/* open the midi driver.
	 * returns 0 if successful.
	 * otherwise an error code. */
	virtual int open(int mode) = 0;

	/* close the midi driver */
	virtual void close() = 0;

	/* output a packed midi command to the midi stream
	 * valid only if mode is MO_SIMPLE
	 */
	virtual void send(uint32 b) = 0;

	/* set callback when more streams need to be generated.
	 * valid only when mode==MO_STREAMING
	 */
	virtual void set_stream_callback(void *param, StreamCallback *sc) = 0;

	/* Pause or resume streaming MIDI */
	virtual void pause(bool pause) = 0;


	/* Get or set a property */
	virtual uint32 property(int prop, uint32 param) = 0;

	/* retrieve a string representation of an error code */
	static const char *get_error_name(int error_code);
};



/* driver types */
enum {
	MD_AUTO = 0,
	MD_NULL = 1,
	MD_WINDOWS = 2,
	MD_TIMIDITY = 3,
	MD_SEQ = 4,
	MD_QTMUSIC = 5,
	MD_AMIDI = 6,
};


/* Factory functions => no need to include the specific classes
 * in this header => faster compile */
MidiDriver *MidiDriver_NULL_create();
MidiDriver *MidiDriver_WIN_create();
MidiDriver *MidiDriver_TIMIDITY_create();
MidiDriver *MidiDriver_SEQ_create();
MidiDriver *MidiDriver_QT_create();
MidiDriver *MidiDriver_AMIDI_create();



/* Roland to General Midi patch table. Still needs much work. */
static const byte mt32_to_gmidi[128] = {
  0,   1,   2,   4,   4,   5,   5,   3,  16,  17,  18,  18,  19,
  19,  20,  21,   6,   6,   6,   7,   7,   7,   8,   8,  62,  63,
  62,  63,  38,  39,  38,  39,  88,  89,  52, 113,  97,  96,  91,
  85,  14, 101,  68,  95,  86, 103,  88,  80,  48,  49,  51,  45,
  40,  40,  42,  42,  43,  46,  46,  24,  25,  26,  27, 104,  32,
  33,  34,  39,  36,  37,  38,  35,  79,  73,  72,  72,  74,  75,
  64,  65,  66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,
  63,  60,  60,  58,  61,  61,  11,  11,  12,  88,   9,  14,  13,
  12, 107, 111,  77,  78,  78,  76, 121,  47, 117, 127, 115, 118,
 116, 118,  94, 115,   9,  55, 124, 123, 125, 126, 127
};

#ifdef __APPLE__CW
	#include <QuickTimeComponents.h>
	#include "QuickTimeMusic.h"

	NoteAllocator qtNoteAllocator;
	NoteChannel qtNoteChannel[16];
	NoteRequest simpleNoteRequest;
#endif

#ifdef WIN32
	#include <winsock.h>
#elif defined(UNIX)
	#include <sys/time.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#ifdef __MORPHOS__
	#include <exec/types.h>
	#include <devices/amidi.h>

	#define NO_PPCINLINE_STDARG
	#define NO_PPCINLINE_VARARGS
	#include <clib/alib_protos.h>
	#include <proto/exec.h>
	#undef CMD_INVALID

	extern struct IOMidiRequest *ScummMidiRequest;
#endif
#endif	/* defined(gmidi_h) */
