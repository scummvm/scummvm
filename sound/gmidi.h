#if !defined(gmidi_h)
#define gmidi_h

/* General Midi header file */

#define SEQ_MIDIPUTC    5       /* For timidity */
#define SPECIAL_CHANNEL 9
#define DEVICE_NUM 0

#if defined(USE_QTMUSIC)
	#include <QuickTimeComponents.h>
	#include "QuickTimeMusic.h"

	NoteAllocator qtNoteAllocator;
	NoteChannel qtNoteChannel[16];
	NoteRequest simpleNoteRequest;
#endif

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


#endif	/* defined(gmidi_h) */
