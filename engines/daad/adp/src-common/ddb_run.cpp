#include <ddb.h>
#include <ddb_scr.h>
#include <ddb_test.h>
#include <ddb_pal.h>
#include <ddb_data.h>
#include <ddb_vid.h>
#include <ddb_xmsg.h>
#include <ddb_paw.h>
#include <dmg.h>
#include <os_char.h>
#include <os_file.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

#if HAS_PSG
extern bool DDB_PlayExternalPSG(DDB* ddb, uint8_t soundIndex);
#endif

#define PAUSE_ON_INKEY 0
#define DEBUG_UNDO 0

File* transcriptFile = 0;
static bool transcriptAtLineStart = true;
static bool transcriptBreakPending = false;

static void MarkWindowOutput();

static bool SkipTimedPauses(const DDB_Interpreter* i)
{
	#if HAS_TESTMODE
	return i->skipTimedPauses;
	#else
	(void)i;
	return false;
	#endif
}

static void StartTimeout(DDB_Interpreter* i)
{
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
	{
		DDB_PAWSStartTimeout(i);
		return;
	}
	#endif
	i->timeout = true;
	i->timeoutRemainingMs = (int32_t)i->flags[Flag_Timeout] * 1000L;
}

static void CancelTimeout(DDB_Interpreter* i)
{
	i->timeout = false;
	i->timeoutRemainingMs = 0;
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
		i->timeoutTickRemainderMs = 0;
	#endif
}

static bool AdvanceTimeout(DDB_Interpreter* i, uint32_t elapsedMs)
{
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
		return DDB_PAWSAdvanceTimeout(i, elapsedMs);
	#endif
	i->timeoutRemainingMs -= elapsedMs;
	return i->timeoutRemainingMs <= 0;
}

#if HAS_TESTMODE
void DDB_TestSetFlag(uint8_t flag, uint8_t value)
{
	if (interpreter != 0)
		interpreter->flags[flag] = value;
}
#endif

static bool TranscriptMuted(DDB_Interpreter* i)
{
	if (i->suppressTranscript)
		return true;
	#if HAS_TESTMODE
	return DDB_TestWindowMuted(i->curwin);
	#else
	(void)i;
	return false;
	#endif
}

static bool AnyKeyForMore(bool allowCaptures = true)
{
	#if HAS_TESTMODE
	return SCR_AnyKeyForMore(allowCaptures);
	#else
	(void)allowCaptures;
	return SCR_AnyKey();
	#endif
}

static uint8_t objNameBuffer[256];

static uint8_t ToUpper(uint8_t ch);
static uint8_t ToLower(uint8_t ch);

#if TRACE_ON
static const char* DDB_MessageTypeNames[] = {
	"MSG",
	"SYSMSG",
	"OBJNAME",
	"LOCDESC"
};
static const char* DDB_StateNames[] = {
	"DDB_RUNNING",
	"DDB_PAUSED",
	"DDB_FINISHED",
	"DDB_FATAL_ERROR",
	"DDB_QUIT",
	"DDB_WAITING_FOR_KEY",
	"DDB_CHECKING_KEY",
	"DDB_VSYNC",
	"DDB_INPUT",
	"DDB_INPUT_QUIT",
	"DDB_INPUT_END",
	"DDB_INPUT_SAVE",
	"DDB_INPUT_LOAD"
};
static const char* DDB_FlowNames[] = {
	"FLOW_STARTING",
	"FLOW_DESC",
	"FLOW_AFTER_TURN",
	"FLOW_INPUT",
	"FLOW_RESPONSES",
};
#endif

#if TRACE_ON
static const char* TranslateCharForTrace(uint8_t c)
{
	static char buffer[16] = { 32, 0 };

	if (c == '\n')
		return "\\n";
	else if (c == '\r')
		return "\\r";
	else if (c == '\t')
		return "\\t";
	else if (c == '\b')
		return "\\b";
	else if (c == '\x0E')
		return "\\g";
	else if (c == '\x0F')
		return "\\t";

	if (c < 32 || c > 127)
	{
		buffer[0] = '{';
		char* ptr = LongToChar(c, buffer + 1, 10);
		*ptr++ = '}';
		*ptr = 0;
	}
	else
	{
		buffer[0] = c;
		buffer[1] = 0;
	}
	return buffer;
}
#endif

void DDB_TranscriptBreak()
{
	if (transcriptFile != 0)
		transcriptBreakPending = true;
}

void DDB_TranscriptNewLine()
{
	if (transcriptFile == 0)
		return;
	transcriptBreakPending = false;
	if (!transcriptAtLineStart)
	{
		File_Write(transcriptFile, "\n", 1);
		transcriptAtLineStart = true;
	}
	File_Flush(transcriptFile);
}

void DDB_TranscriptWrite(const void* text, size_t length)
{
	if (transcriptFile == 0 || length == 0)
		return;
	if (transcriptBreakPending)
		DDB_TranscriptNewLine();
	File_Write(transcriptFile, text, length);
	transcriptAtLineStart = false;
}

void DDB_TranscriptFlush()
{
	if (transcriptFile != 0)
		File_Flush(transcriptFile);
}

static void WriteTranscriptChar(uint8_t ch)
{
	if (transcriptFile == 0)
		return;

	if (ch < 16 || ch > 127)
	{
		char buffer[24] = "\\x";
		char* ptr = LongToChar(ch, buffer + 2, 16);
		DDB_TranscriptWrite(buffer, ptr - buffer);
	}
	else if (ch < 32)
	{
		static const char* spanishChars[] = { "º", "¡", "¿", "«", "»", "á", "é", "í", "ó", "ú", "ñ", "Ñ", "ç", "Ç", "ü", "Ü" };
		const char* ptr = spanishChars[ch - 16];
		DDB_TranscriptWrite(ptr, StrLen(ptr));
	}
	else
	{
		DDB_TranscriptWrite(&ch, 1);
	}
}

void DDB_SetupInkMap (DDB_Interpreter* i)
{
	DDB_ScreenMode logicScreenMode = i->screenMode;
	#if !defined(_DOS)
	if (i->ddb->target == DDB_MACHINE_IBMPC && logicScreenMode == ScreenMode_CGA)
		logicScreenMode = ScreenMode_EGA;
	#endif

	switch (i->ddb->target)
	{
		case DDB_MACHINE_CPC:
			for (int n = 0; n < 16; n++)
				i->inkMap[n] = n & 3;
			break;

		case DDB_MACHINE_SPECTRUM:
			for (int n = 0; n < 16; n++)
				i->inkMap[n] = n;
			if (i->ddb->version != DDB_VERSION_PAWS)
			{
				i->inkMap[0] = 0;
				i->inkMap[1] = 7;
				i->inkMap[7] = 1;
				#if HAS_DRAWSTRING
				// Logical colours are converted through an 8-entry table
				// in the graphics database, preserving the bright bit
				const uint8_t* map = DDB_GetVectorInkMap();
				if (map != 0)
				{
					for (int n = 0; n < 16; n++)
						i->inkMap[n] = (map[n & 7] & 0x07) | (n & 0x08);
				}
				#endif
			}
			break;

		case DDB_MACHINE_C64:
			for (int n = 0; n < 16; n++)
				i->inkMap[n] = n;
			#if HAS_DRAWSTRING
			{
				// Same conversion as MSX, with C64 hardware colours
				const uint8_t* map = DDB_GetVectorInkMap();
				if (map != 0)
				{
					for (int n = 0; n < 16; n++)
						i->inkMap[n] = map[n] & 0x0F;
				}
			}
			#endif
			break;

		case DDB_MACHINE_MSX:
		{
			for (int n = 0; n < 16; n++)
				i->inkMap[n] = n;
			#if HAS_DRAWSTRING
			// Logical colours are converted to TMS9918 colours through
			// a table in the graphics database
			const uint8_t* map = DDB_GetVectorInkMap();
			if (map != 0)
			{
				for (int n = 0; n < 16; n++)
					i->inkMap[n] = map[n] & 0x0F;
			}
			#endif
			break;
		}

		case DDB_MACHINE_IBMPC:
			if (logicScreenMode == ScreenMode_Text)
			{
				for (int n = 0; n < 16; n++)
					i->inkMap[n] = n;
			}
			else if (logicScreenMode == ScreenMode_CGA)
			{
				for (int n = 0; n < 16; n += 4)
				{
					i->inkMap[n] = 0;
					i->inkMap[n+1] = 3;
					i->inkMap[n+2] = 2;
					i->inkMap[n+3] = 1;
				}
			}
			#if HAS_PCX
			else if (logicScreenMode == ScreenMode_VGA && VID_HasExternalPictures())
			{
				for (int n = 0; n < 16; n++)
					i->inkMap[n] = n;
			}
			#endif
			else
			{
				i->inkMap[0] = 0;
				i->inkMap[1] = 15;
				i->inkMap[2] = 4;
				i->inkMap[3] = 2;
				i->inkMap[4] = 1;
				i->inkMap[5] = 3;
				for (int n = 6; n < 16; n++)
					i->inkMap[n] = n-1;
			}
			break;

		default:
			i->inkMap[0] = 0;
			i->inkMap[1] = 15;
			for (int n = 2; n < 16; n++)
				i->inkMap[n] = n-1;
			break;
	}
}

void DDB_SetCharset (DDB* ddb, uint8_t c)
{
	if (c == 0)
	{
		MemCopy(charset + 256, DefaultCharset + 256, 768);
		#if HAS_PAWS
		if (ddb->version == DDB_VERSION_PAWS)
			MemCopy(charset+256, ZXSpectrumCharacterSet, 768);
		#endif
	}
	else if (ddb->numCharsets >= c)
	{
		MemCopy(charset + 256, ddb->charsets + 768*(c-1), 768);
	}
	#if HAS_PAWS
	else if (DDB_IsPAWS(ddb->version))
	{
		// PAW leaves CHARS and its saved font unchanged when the requested
		// database font is not installed.
		return;
	}
	#endif

#if HAS_PAWS
	DDB_LoadUDGs();
#endif

	ddb->curCharset = c;
}

void DDB_ResetPAWSColors (DDB_Interpreter* i, DDB_Window* w)
{
	#if HAS_PAWS
	if (i->ddb->version == DDB_VERSION_PAWS)
	{
		w->flags = (w->flags & ~(Win_Inverse | Win_Over | Win_Flash | Win_Bright)) |
			i->pawsPermanentFlags;
		w->ink = i->pawsPermanentInk;
		w->paper = i->pawsPermanentPaper;
		DDB_SetCharset(i->ddb, i->pawsPermanentCharset);
	}
	#endif
}

void DDB_ResetWindows (DDB_Interpreter* i)
{
	int defaultInk = 15;
	int defaultPaper = 0;

	// TODO: Store default colors in DDB
	if (i->ddb->target == DDB_MACHINE_CPC ||
	 	i->ddb->target == DDB_MACHINE_C64)
		defaultInk = 1;

	#if HAS_PAWS
	if (i->ddb->version == DDB_VERSION_PAWS)
	{
		defaultInk = i->ddb->defaultInk;
		defaultPaper = i->ddb->defaultPaper;
	}
	#endif

	for (int n = 0; n < 8; n++)
	{
		DDB_Window* w  = &i->windef[n];
		w->x           = 0;
		w->y           = 0;
		w->width       = screenWidth;
		w->height      = screenHeight;
		w->ink         = defaultInk;
		w->paper       = defaultPaper;
		w->posX        = w->x;
		w->posY        = w->y;
		w->scrollCount = 0;
	}

	i->win = i->windef[0];
	DDB_CalculateCells(i, &i->win, &i->cellX, &i->cellW);
	DDB_SetupInkMap(i);
	DDB_ResetPAWSColors(i, &i->win);
}

// Full game restart, matching the original interpreters' START/START2 routine:
// reset the streams/windows, clear the screen ("system clear"), reset the game
// state, and continue from process 0.  Unlike the RESTART condact (which just
// re-enters the loop), this is what the original does on a part change / EXIT
// and on a failed LOAD.
void DDB_Restart (DDB_Interpreter* i)
{
	DDB_Reset(i);
	DDB_ResetWindows(i);
	DDB_ClearWindow(i, &i->win);
}

void DDB_Reset (DDB_Interpreter* i)
{
	int n;
	uint8_t topFlags[10];
	for (n = 0xF9; n <= 0xFF; n++)
		topFlags[n-0xF9] = i->flags[n];

	i->state = DDB_RUNNING;
	i->doall = false;
	i->procstackptr = 0;
	i->procstack[0].entry = 0;
	i->procstack[0].process = 0;
	i->procstack[0].offset = 0;
	i->inputBufferLength = 0;
	i->inputBuffer[0] = 0;
	i->inputBufferPtr = 0;
	i->inputCompletionX = 0;
	MemClear(i->buffer, i->saveStateSize);

	for (n = 0xF9; n <= 0xFF; n++)
		i->flags[n] = topFlags[n-0xF9];

	i->flags[Flag_CPAdjective] = 255;
	i->flags[Flag_CPNoun] = 255;
	i->flags[Flag_GraphicFlags] = 128;
	i->flags[Flag_MaxCarried] = 4;
	i->flags[Flag_Strength] = 10;

	if (i->ddb->version > 1)
	{
		DDB_ScreenMode logicScreenMode = i->screenMode;
		#if !defined(_DOS)
		if (i->ddb->target == DDB_MACHINE_IBMPC && logicScreenMode == ScreenMode_CGA)
			logicScreenMode = ScreenMode_EGA;
		#endif
		i->flags[Flag_ScreenMode] = logicScreenMode;
	}
	if (i->ddb->oldMainLoop)
	{
		i->state = DDB_FINISHED;
		i->oldMainLoopState = FLOW_STARTING;
	}

	for (n = 0; n < i->ddb->numObjects; n++)
	{
		i->objloc[n] = i->ddb->objLocTable[n];
		if (i->objloc[n] == Loc_Carried)
			i->flags[Flag_NumCarried]++;
	}

	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
		DDB_PAWSResetState(i);
	#endif

	// SCR_Clear(0, 0, screenWidth, screenHeight, 0);
}

// --------------------
//  Internal functions
// --------------------

static void TraceVocabularyWord (DDB* ddb, uint8_t type, uint8_t index)
{
	#if TRACE_ON
	uint8_t* ptr = ddb->vocabulary;

	while (*ptr != 0)
	{
		if (ptr[5] == index && ptr[6] == type)
		{
			for (int n = 0; n < 5; n++)
				TRACE("%c", DDB_Char2ISO[(ptr[n] ^ 0xFF) & 0x7F]);
			return;
		}
		ptr += 7;
	}
	const int convertibleNoun = ddb->version < 2 ? 20 : 40;
	if (type == WordType_Verb && index < convertibleNoun)
		TraceVocabularyWord(ddb, WordType_Noun, index);
	else
		TRACE("%-5d", index);
	#endif
}

static int CalculateWeight (DDB_Interpreter* i, uint8_t objno, int depth)
{
	int value = 0;

	if (objno >= i->ddb->numObjects)
		return 0;

	uint8_t attr = i->ddb->objAttrTable[objno];
	value += attr & Obj_Weight;
	if ((attr & Obj_Container) != 0 && depth < 10)
	{
		// Container
		for (int n = 0; n < i->ddb->numObjects; n++)
		{
			if (i->objloc[n] == objno)
				value += CalculateWeight(i, n, depth + 1);
		}
	}

	return value > 255 ? 255 : value;
}

static int CalculateCarriedWeight(DDB_Interpreter* i)
{
	int value = 0;
	for (int n = 0; n < i->ddb->numObjects; n++)
	{
		if (i->objloc[n] == Loc_Carried || i->objloc[n] == Loc_Worn)
			value += CalculateWeight(i, n, 0);
	}
	return value > 255 ? 255 : value;
}

void DDB_CalculateCells (DDB_Interpreter* i, DDB_Window* w, uint8_t* cellX, uint8_t* cellW)
{
	if (columnWidth == screenCellWidth)
	{
		int cellsX = w->x / screenCellWidth;
		int cellsW = w->width / screenCellWidth;

		if (w->x + w->width >= screenWidth)
			cellsW = (screenWidth - cellsX * screenCellWidth) / screenCellWidth;
		if (cellsW == 0)
			cellsW = 1;

		*cellX = cellsX;
		*cellW = cellsW;
		return;
	}

	static int inc[] = { 0,1,2,3,1,2,3,3,1,2,2,3,1,1,2,3 };
	int charsX = w->x / columnWidth;
	int charsW = w->width / columnWidth;
	int cellsX = 3*(charsX >> 2) + ((charsX & 3) != 0 ? (charsX & 3)-1 : 0);
	int cellsW = inc[((charsX & 3) << 2) | (charsW & 3)] + 3*(charsW >> 2);

	//if ((w->x & 0x07) == 0) cellsX = w->x / 8;

	// This adjustment is necessary because the original interpreter
	// calculates this using the unclipped window width
	if (w->x + w->width >= screenWidth)
		cellsW = (screenWidth - cellsX * screenCellWidth) / screenCellWidth;
	if (cellsW == 0)
		cellsW = 1;

	// fprintf(stderr, "Adjusting x=%d width=%d -> %d width %d\n", x, width, cellsX*8, cellsW*8);

	*cellX = cellsX;
	*cellW = cellsW;
}

static void ShowMorePrompt (DDB_Interpreter* i)
{
	static char more[128];

	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
	{
		if ((i->flags[Flag_PAWMode] & 0x40) == 0)
			return;

		// The B03 pagination path is a channel switch, not an overlay drawn at
		// the current cursor. Finish presentation of the page before temporarily
		// borrowing the formatter for editable system message 32.  Opening the
		// PAW output stream reloads CHARS from L880D (the permanent font), while
		// the active font is saved and restored around the prompt.
		SCR_ConsumeFullBuffer();
		uint8_t savedPending[sizeof(i->pending)];
		MemCopy(savedPending, i->pending, sizeof(savedPending));
		const uint8_t savedPendingPtr = i->pendingPtr;
		const uint8_t savedControlParams = i->pawsControlParams;
		const uint8_t savedCharset = i->ddb->curCharset;
		const uint8_t savedVideoAttributes = VID_GetAttributes();
		const bool savedSuppressTranscript = i->suppressTranscript;

		DDB_Window prompt = i->win;
		prompt.x = 0;
		prompt.y = screenHeight - 2 * lineHeight;
		prompt.width = screenWidth;
		prompt.height = 2 * lineHeight;
		prompt.posX = 0;
		prompt.posY = prompt.y;
		prompt.scrollCount = 0;
		prompt.smooth = false;

		i->pendingPtr = 0;
		i->pawsControlParams = 0;
		i->suppressTranscript = true;
		DDB_SetCharset(i->ddb, i->pawsPermanentCharset);
		DDB_OutputMessageToWindow(i, DDB_SYSMSG, 32, &prompt);
		DDB_FlushWindow(i, &prompt);
		// Character commands refer to the live glyph table when consumed. Draw
		// the prompt before restoring the interrupted font; queue its key wait
		// afterwards so presentation remains asynchronous and capturable.
		SCR_ConsumeFullBuffer();

		DDB_SetCharset(i->ddb, savedCharset);
		MemCopy(i->pending, savedPending, sizeof(savedPending));
		i->pendingPtr = savedPendingPtr;
		i->pawsControlParams = savedControlParams;
		i->suppressTranscript = savedSuppressTranscript;

		// The input timeout which was active while acquiring the command does
		// not govern this separate PAW keyboard wait.  Only TIME's explicit
		// More-prompt option may start a fresh timeout below.
		i->timeout = false;
		i->timeoutRemainingMs = 0;
		i->timeoutTickRemainderMs = 0;
		SCR_WaitForKey();

		// ROM $0D6E is CLS-LOWER: clear the two-line lower input area, not the
		// upper display, then reopen the normal PAW stream with saved state.
		uint8_t clearInk, clearPaper;
		DDB_GetCurrentColors(i->ddb, &i->win, &clearInk, &clearPaper);
		SCR_SetAttributes(savedVideoAttributes);
		SCR_Clear(0, screenHeight - 2 * lineHeight, screenWidth, 2 * lineHeight,
			clearPaper, Clear_All);

		if ((i->flags[Flag_TimeoutFlags] & Timeout_MorePrompt) != 0 &&
			i->flags[Flag_Timeout] != 0)
			StartTimeout(i);
		return;
	}
	#endif

	DDB_Window* w = &i->win;
	int x = w->x;
	int maxX = (i->cellX + i->cellW) * screenCellWidth;
	int ink = w->ink;
	int paper = w->paper;

	const char* end = DDB_GetMessage(i->ddb, DDB_SYSMSG, 32, more, sizeof(more));
	SCR_Clear(w->posX, w->posY, maxX - w->posX, lineHeight, paper);
	for (const char* ptr = more; ptr < end; ptr++) {
		if (x + charWidth[(uint8_t)*ptr] > maxX)
			break;
		SCR_DrawCharacter(x, w->posY, *ptr, ink, paper);
		x += charWidth[(uint8_t)*ptr];
	}
	SCR_WaitForKey();
	SCR_Clear(w->posX, w->posY, maxX - w->posX, lineHeight, paper);

	if (i->flags[Flag_TimeoutFlags] & Timeout_MorePrompt)
	{
		StartTimeout(i);
	}
	i->flags[Flag_TimeoutFlags] &= ~Timeout_LastFrame;
}

bool DDB_NextLineAtWindow (DDB_Interpreter* i, DDB_Window* w)
{
	#if HAS_PAWS
	const bool pawsMode = DDB_IsPAWS(i->ddb->version);
	if (pawsMode && w == &i->win)
	{
		// The PAW stream-2 region follows live flags, so the rect is
		// refreshed here before the shared newline logic runs on it
		DDB_PAWSRefreshWindow(i, w);

		// PAW paginates before scrolling, on the ROM's count-down scroll
		// counter, and only shows the prompt with the MODE page option
		if (w->posY + lineHeight >= w->y + w->height)
		{
			if (w->scrollCount == 0)
				DDB_PAWSReloadScrollCounter(i, w);
			if (--w->scrollCount == 0)
			{
				DDB_PAWSReloadScrollCounter(i, w);
				if ((i->flags[Flag_PAWMode] & 0x40) != 0)
				{
					ShowMorePrompt(i);
					w->smooth = 1;
				}
			}
		}
	}
	#endif

	int maxY = w->y + w->height - lineHeight;
	int paper = w->paper;
	#if HAS_PAWS
	if (pawsMode)
	{
		uint8_t ink, effectivePaper;
		DDB_GetCurrentColors(i->ddb, w, &ink, &effectivePaper);
		paper = effectivePaper;
		// VID_Scroll clears the newly exposed row from the current video
		// attributes. Queue them explicitly so buffered execution cannot inherit
		// attributes from an unrelated earlier screen command.
		uint8_t attributes = (ink & 0x07) | ((paper & 0x07) << 3) |
			((ink & 0x08) << 3) | ((ink & 0x10) << 3);
		SCR_SetAttributes(attributes);
	}
	#endif

	w->posX = w->x;
	w->posY += lineHeight;
	if (w->posY > maxY)
	{
		uint8_t cellX = i->cellX, cellW = i->cellW;
		int scroll = w->posY - maxY;
		if (w != &i->win)
			DDB_CalculateCells(i, w, &cellX, &cellW);
		SCR_Scroll(cellX * screenCellWidth, w->y, cellW * screenCellWidth, w->height, scroll, paper, (w->flags & Win_NoMorePrompt) ? false : w->smooth);
		w->posY -= scroll;
	}

	#if HAS_PAWS
	if (pawsMode)
		return false;
	#endif

	w->scrollCount++;

	if ((w->flags & Win_NoMorePrompt) == 0)
	{
		int scrollLines = (w->height / lineHeight) - 1;
		if (scrollLines > 2 && w->scrollCount >= scrollLines) {
			ShowMorePrompt(i);
			w->scrollCount = 0;
			w->smooth = 1;
		}
	}

	return false;
}

bool DDB_NextLine (DDB_Interpreter* i)
{
	return DDB_NextLineAtWindow(i, &i->win);
}

bool DDB_NewLineAtWindow (DDB_Interpreter* i, DDB_Window* w)
{
	int maxX = w->x + w->width;
	uint8_t cellX, cellW;
	if (w == &i->win)
	{
		cellX = i->cellX;
		cellW = i->cellW;
	}
	else
	{
		DDB_CalculateCells(i, w, &cellX, &cellW);
	}
	maxX = cellX * screenCellWidth + cellW * screenCellWidth;
	if (w->posX < maxX)
	{
		#if HAS_PAWS
		if (DDB_IsPAWS(i->ddb->version))
		{
			// PAW reaches the right edge by printing literal spaces.  This is
			// observably different from a rectangle clear when temporary INVERSE,
			// BRIGHT/FLASH, OVER, or a custom character set is active.
			uint8_t ink, paper;
			DDB_GetCurrentColors(i->ddb, w, &ink, &paper);
			int x = w->posX;
			int width = charWidth[' '];
			if (width <= 0)
				width = screenCellWidth;
			while (x + width <= maxX)
			{
				SCR_DrawCharacter(x, w->posY, ' ', ink, paper);
				x += width;
			}
		}
		else
		#endif
		{
			SCR_Clear(w->posX, w->posY, maxX - w->posX, lineHeight,
				w->paper == 255 ? 0 : w->paper);
		}
	}

	return DDB_NextLineAtWindow(i, w);
}

bool DDB_NewLine (DDB_Interpreter* i)
{
	if (!TranscriptMuted(i))
		DDB_TranscriptNewLine();
	return DDB_NewLineAtWindow(i, &i->win);
}

void DDB_ClearWindow (DDB_Interpreter* i, DDB_Window* w)
{
	DDB_TranscriptBreak();
	int x, width;
	if (w != &i->win)
	{
		uint8_t cellX, cellW;
		DDB_CalculateCells(i, w, &cellX, &cellW);
		x = cellX * screenCellWidth;
		width = cellW * screenCellWidth;
	}
	else
	{
		x = i->cellX * screenCellWidth;
		width = i->cellW * screenCellWidth;
	}

	SCR_Clear(x, w->y, width, w->height, w->paper == 255 ? 0 : w->paper, Clear_All);

	w->posX = w->x;
	w->posY = w->y;
	w->scrollCount = 0;
	w->smooth = 0;
}

static bool BufferPicture (DDB_Interpreter* i, uint8_t picno)
{
	i->currentPicture = picno;
	return SCR_LoadPicture(picno, i->screenMode);
}

static inline int AdjustX (int x, int columnWidth)
{
	int    cells = x / columnWidth;
	return cells * columnWidth;
}

static void PrepareBufferedPictureWindow(DDB_Interpreter* i, int16_t* x, int16_t* width)
{
	DDB_Window* w = &i->win;

	bool fixed = false;
	int16_t picx = 0;
	int16_t picy = 0;
	int16_t picw = 0;
	int16_t pich = 0;
	SCR_GetPictureInfo(&fixed, &picx, &picy, &picw, &pich);

	if (fixed)
	{
		*x            = (picx / screenCellWidth) * screenCellWidth;
		*width        = ((picw + screenCellWidth - 1) / screenCellWidth) * screenCellWidth;
		i->cellX	  = picx / screenCellWidth;
		i->cellW	  = *width / screenCellWidth;
		i->win.width  = AdjustX(picw, columnWidth);
		i->win.height = pich;
		int alignBias = columnWidth == 8 ? 3 : columnWidth / 2;
		i->win.x      = AdjustX(*x + alignBias, columnWidth);
		i->win.y      = picy;
		i->win.posX   = *x;
		i->win.posY   = picy;
	}
	else
	{
		*x  	          = i->cellX * screenCellWidth;
		*width 		  = i->cellW * screenCellWidth;
		i->win.posX   = i->win.x;
		i->win.posY   = i->win.y;
	}
}

static void DrawBufferedPicture (DDB_Interpreter* i)
{
	DDB_Window* w = &i->win;
	int16_t x, width;

	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DrawBufferedPicture: begin pic=%u win=(%d,%d,%d,%d) screenMode=%u\n",
		(unsigned)i->currentPicture,
		(int)w->x,
		(int)w->y,
		(int)w->width,
		(int)w->height,
		(unsigned)i->screenMode);
	#endif

	PrepareBufferedPictureWindow(i, &x, &width);

	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DrawBufferedPicture: prepared x=%d width=%d pos=(%d,%d) size=(%d,%d)\n",
		(int)x,
		(int)width,
		(int)w->x,
		(int)w->y,
		(int)w->width,
		(int)w->height);
	#endif

	SCR_DisplayPicture(x, w->y, width, w->height, i->screenMode);

	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DrawBufferedPicture: SCR_DisplayPicture returned\n");
	#endif
}

void DDB_GetCurrentColors (DDB* ddb, DDB_Window* w, uint8_t* ink, uint8_t* paper)
{
	*ink = w->ink;
	*paper = w->paper;

	#if HAS_PAWS
	if (ddb->version == DDB_VERSION_PAWS)
	{
		if (w->flags & Win_Inverse)
		{
			uint8_t tmp = *ink;
			*ink = *paper;
			*paper = tmp;
		}
		if (*paper == 9)
			*paper = *ink > 2 ? 0 : 7;
		if (*ink == 9)
			*ink = *paper > 2 ? 0 : 7;
		if (w->flags & Win_Flash)
			*ink |= 0x10;
		if (w->flags & Win_Bright)
			*ink |= 0x08;
	}
	#endif
}

void DDB_FlushWindow (DDB_Interpreter* i, DDB_Window* w)
{
	#ifdef _DEBUGPRINT
	uint32_t drawnChars = 0;
	#endif

	// TODO: This logic is incorrect. The pending buffer should know which window
	// it was written to, and the flush should only flush that window. The current
	// implementation sometimes writes garbage to the unintended window.

	// TODO: maxX calculations are wrong, as they only refer to the *current*
	// window (!!!). We should either change DDB_FlushWindow, DDB_OutputCharToWindow
	// etc. to write to the current window only, or save cellX/cellW for each window

	int maxX = (i->cellX + i->cellW) * screenCellWidth;
	bool forceGraphics = (w->flags & Win_ForceGraphics) != 0;

#if HAS_PAWS
	const bool pawsMode = i->ddb->version == DDB_VERSION_PAWS;
	if (pawsMode) forceGraphics = false;
#else
	const bool pawsMode = false;
#endif

	if (w != &i->win)
	{
		uint8_t cellX, cellW;
		DDB_CalculateCells(i, w, &cellX, &cellW);
		maxX = (cellX + cellW) * screenCellWidth;
	}

	uint8_t spanChars[sizeof(i->pending)];
	for (int n = 0; n < i->pendingPtr; )
	{
		uint8_t ch = i->pending[n];

		#if HAS_PAWS
		if (pawsMode && ch < 32)
		{
			switch (ch)
			{
				case 16:		// Ink
					// BRIGHT is independent Spectrum attribute state.  Logical
					// colour 9 is PAW's contrast colour, not a bright-bit carrier.
					w->ink = i->pending[n + 1] & 0x0F;
					n += 2;
					continue;
				case 17:		// Paper
					w->paper = i->pending[n + 1] & 0x0F;
					n += 2;
					continue;
				case 18:		// Flash
					w->flags = (w->flags & ~Win_Flash) | ((i->pending[n + 1] & 0x01) ? Win_Flash : 0);
					n += 2;
					continue;
				case 19:		// Bright
					w->flags = (w->flags & ~Win_Bright) | ((i->pending[n + 1] & 0x01) ? Win_Bright : 0);
					n += 2;
					continue;
				case 20:		// Inverse
					w->flags = (w->flags & ~Win_Inverse) | ((i->pending[n + 1] & 0x01) ? Win_Inverse : 0);
					n += 2;
					continue;
				case 21:		// Over
					w->flags = (w->flags & ~Win_Over) | ((i->pending[n + 1] & 0x01) ? Win_Over : 0);
					n += 2;
					continue;
				case 22:		// AT row,column
					w->posY = i->pending[n + 1] * lineHeight;
					w->posX = i->pending[n + 2] * screenCellWidth;
					n += 3;
					continue;
				case 23:		// Spectrum TAB; PAW uses the screen-relevant low byte
					w->posX = i->pending[n + 1] * screenCellWidth;
					n += 3;
					continue;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					DDB_SetCharset(i->ddb, ch);
					n++;
					continue;
				default:
					DebugPrintf("Unknown PAWS control code %d\n", ch);
					n++;
					break;
			}
			continue;
		}
		else
		#endif

		if (ch < 16)
		{
			switch (ch)
			{
				case 0x0E:		// Graphics on ('\g')
					w->graphics = true;
					break;

				case 0x0F:		// Graphics off ('\t')
					w->graphics = false;
					break;
			}
			n++;
			continue;
		}

		if (ch != ' ' && ch != 0xA0 && w->posX > w->x)
		{
			int wordWidth = 0;
			for (int wordIndex = n;
				 wordIndex < i->pendingPtr && (!pawsMode || wordIndex < n + 32);
				 wordIndex++)
			{
				uint8_t wordCh = i->pending[wordIndex];

				#if HAS_PAWS
				if (pawsMode && wordCh < 32)
					break;
				#endif

				if (wordCh < 16 || wordCh == ' ' || wordCh == 0xA0)
					break;

				if (w->graphics || forceGraphics)
					wordCh |= 0x80;

				wordWidth += charWidth[wordCh];
			}

			if (w->posX + wordWidth > maxX)
			{
				DDB_NewLineAtWindow(i, w);
				continue;
			}
		}

		uint8_t ink, paper;
		DDB_GetCurrentColors(i->ddb, w, &ink, &paper);
		int spanLength = 0;
		int spanWidth = 0;
		int lastBreakIndex = -1;
		int lastBreakLength = 0;
		int lastBreakWidth = 0;
		int nextIndex = n;

		while (nextIndex < i->pendingPtr)
		{
			uint8_t spanCh = i->pending[nextIndex];

			#if HAS_PAWS
			if (pawsMode && spanCh < 32)
				break;
			#endif

			if (spanCh < 16)
				break;

			if (w->graphics || forceGraphics)
				spanCh |= 0x80;

			int width = charWidth[spanCh];
			if (w->posX + spanWidth + width > maxX)
			{
				if (lastBreakIndex >= 0)
				{
					nextIndex = lastBreakIndex;
					spanLength = lastBreakLength;
					spanWidth = lastBreakWidth;
				}
				break;
			}

			spanChars[spanLength++] = spanCh;
			spanWidth += width;
			nextIndex++;

			if (spanCh == ' ' || spanCh == 0xA0)
			{
				lastBreakIndex = nextIndex;
				lastBreakLength = spanLength;
				lastBreakWidth = spanWidth;
			}
		}

		if (spanLength == 0)
		{
			uint8_t singleCh = ch;
			if (w->graphics || forceGraphics)
				singleCh |= 0x80;

			int width = charWidth[singleCh];
			if (w->posX + width > maxX)
			{
				if (singleCh == ' ' || singleCh == 0xA0)
				{
					// The renderer discards a separator at the right edge so the
					// next line does not start with it. It is still part of the
					// logical message and must remain in the transcript.
					if (transcriptFile && !TranscriptMuted(i))
						WriteTranscriptChar(ch);
					w->posX = maxX;
					n++;
					continue;
				}

				DDB_NewLineAtWindow(i, w);
				continue;
			}

			spanChars[0] = singleCh;
			spanLength = 1;
			spanWidth = width;
			nextIndex = n + 1;
		}

		SCR_DrawTextSpan(w->posX, w->posY, spanChars, (uint16_t)spanLength, ink, paper);
		w->posX += spanWidth;
		n = nextIndex;

		#ifdef _DEBUGPRINT
		drawnChars += (uint32_t)spanLength;
		#endif

		if (transcriptFile && !TranscriptMuted(i))
		{
			for (int spanIndex = 0; spanIndex < spanLength; spanIndex++)
				WriteTranscriptChar(spanChars[spanIndex]);
		}
	}
	i->pendingPtr = 0;
	DDB_TranscriptFlush();

	i->keyChecked = false;
}

void DDB_Flush (DDB_Interpreter* i)
{
	DDB_FlushWindow(i, &i->win);
}

void DDB_ResetSmoothScrollFlags(DDB_Interpreter* i)
{
	i->win.smooth = 0;
	for (int n = 0; n < 8; n++)
		i->windef[n].smooth = false;
}

void DDB_ResetScrollCounts(DDB_Interpreter* i)
{
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
	{
		DDB_PAWSReloadScrollCounter(i, &i->win);
		for (int n = 0; n < 8; n++)
			i->windef[n].scrollCount = i->win.scrollCount;
		return;
	}
	#endif
	i->win.scrollCount = 0;
	for (int n = 0; n < 8; n++)
		i->windef[n].scrollCount = 0;
}

static void OutputCharToWindow (DDB_Interpreter* i, DDB_Window* w, char c)
{
	if ((w->flags & Win_ExpectingCodeByte) != 0)
	{
		if (i->pendingPtr >= sizeof(i->pending))
			DDB_FlushWindow(i, w);
		i->pending[i->pendingPtr++] = c;
		if (i->pawsControlParams > 0)
			i->pawsControlParams--;
		if (i->pawsControlParams == 0)
			w->flags &= ~Win_ExpectingCodeByte;
		return;
	}
	else
	{
		#if HAS_PAWS
		if (i->ddb->version == DDB_VERSION_PAWS)
		{
			switch(c)
			{
				case 6:
				{
					DDB_FlushWindow(i, w);
					int maxX = (i->cellX + i->cellW) * screenCellWidth;
					int tabWidth = screenCellWidth * 16;
					int nextTab = ((w->posX + tabWidth) / tabWidth) * tabWidth;
					if (nextTab > maxX)
						nextTab = maxX;
					if (nextTab > w->posX)
					{
						uint8_t ink, paper;
						DDB_GetCurrentColors(i->ddb, w, &ink, &paper);
						int width = charWidth[' '];
						if (width <= 0)
							width = screenCellWidth;
						for (int x = w->posX; x + width <= nextTab; x += width)
							SCR_DrawCharacter(x, w->posY, ' ', ink, paper);
						w->posX = nextTab;
					}
					return;
				}

				case 7:
					DDB_FlushWindow(i, w);
					DDB_NewLineAtWindow(i, w);
					if (!TranscriptMuted(i))
						DDB_TranscriptNewLine();
					return;

				case 13:
					DDB_FlushWindow(i, w);
					DDB_NewLineAtWindow(i, w);
					DDB_ResetPAWSColors(i, w);
					if (!TranscriptMuted(i))
						DDB_TranscriptNewLine();
					return;

				case '_':
				{
					// PAW's overlay-1 noun-phrase printer ($8329 in the B03
					// image, $8321 in A17C): leading spaces are skipped, output
					// stops at a full stop. The English interpreter strips the
					// first word (the article) when substituting after object-
					// action messages; the Spanish one instead lower-cases the
					// first capital in the first word ($5C/Ñ pairs with $7C/ñ
					// through the same OR $20).
					const uint8_t* end = (const uint8_t*)DDB_GetMessage(i->ddb, DDB_OBJNAME, i->flags[Flag_Objno], (char *)objNameBuffer, sizeof(objNameBuffer));
					uint8_t* ptr = objNameBuffer;
					if (ptr == end)
						return;
					while (ptr < end) {
						if (*ptr >= 16 && *ptr <= 20)
							ptr += 2;
						else if (*ptr <= 32)
							ptr++;
						else
							break;
					}
					if (i->ddb->language == DDB_ENGLISH)
					{
						while (ptr < end && *ptr != ' ' && *ptr != '.')
							ptr++;
						while (ptr < end && *ptr == ' ')
							ptr++;
					}
					else
					{
						while (ptr < end && *ptr != ' ' && *ptr != '.') {
							uint8_t c = *ptr;
							bool lowered = (c >= 'A' && c <= 'Z') || c == 0x5C || (c >= 0x23 && c <= 0x26);
							if (lowered)
								c |= 0x20;
							OutputCharToWindow(i, w, c);
							ptr++;
							if (lowered)
								break;
						}
					}
					for (; ptr < end; ptr++) {
						if (*ptr == '.') break;
						OutputCharToWindow(i, w, *ptr);
					}
					return;
				}

				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
					w->flags |= Win_ExpectingCodeByte;
					i->pawsControlParams = c >= 22 ? 2 : 1;
					if (i->pendingPtr >= sizeof(i->pending) - 1) {
						DDB_FlushWindow(i, w);
					}
					i->pending[i->pendingPtr++] = c;
					return;
			}
		}
		else
		#endif
		{
			switch(c)
			{
				case '\x0D':		// Newline ('\n')
					DDB_FlushWindow(i, w);
					DDB_NewLineAtWindow(i, w);
					DDB_TranscriptNewLine();
					return;

				case '@':
				case '_':
				{
					int firstChar = 0;
					const void* end = DDB_GetMessage(i->ddb, DDB_OBJNAME, i->flags[Flag_Objno], (char *)objNameBuffer, sizeof(objNameBuffer));
					if (end == objNameBuffer)
						return;
					if (i->ddb->language == DDB_SPANISH)
					{
						if (objNameBuffer[1] == 'n') {
							if (objNameBuffer[0] == 'u' || objNameBuffer[0] == 'U') {
								if (objNameBuffer[3] == 's' && (objNameBuffer[2] == 'a' || objNameBuffer[2] == 'o')) {
									firstChar++;
								} else if (objNameBuffer[2] == 'a') {
									firstChar++;
									objNameBuffer[2] = 'a';
								} else {
									objNameBuffer[0] = 'e';
								}
								objNameBuffer[1] = 'l';
							}
						}
					}
					else if (i->ddb->language == DDB_ENGLISH)
					{
						// Remove the first word
						if (objNameBuffer[0] >= 'a' && objNameBuffer[1] <='z') {
							firstChar = 1;
							while (objNameBuffer[firstChar] && objNameBuffer[firstChar] != ' ')
								firstChar++;
							if (objNameBuffer[firstChar] == ' ')
								firstChar++;
							else
								firstChar = 0;
						}
					}
					if (c == '@')
						objNameBuffer[firstChar] = ToUpper(objNameBuffer[firstChar]);
					else
						objNameBuffer[firstChar] = ToLower(objNameBuffer[firstChar]);
					for (uint8_t* ptr = objNameBuffer + firstChar; ptr < end; ptr++) {
						if (*ptr == '.') break;
						OutputCharToWindow(i, w, *ptr);
					}
					return;
				}

				case '\x0B':		// Clear screen ('\b')
					if (i->ddb->version != DDB_VERSION_PAWS)
					{
						DDB_FlushWindow(i, w);
						DDB_ClearWindow(i, w);
						return;
					}
					break;

				case '\x0C':		// Wait for a keypress ('\k')
					if (i->ddb->version != DDB_VERSION_PAWS)
					{
						DDB_FlushWindow(i, w);
						SCR_WaitForKey();
						DDB_ResetScrollCounts(i);
						w->smooth = 1;
						return;
					}
					break;
			}
		}
	}

	if (i->pendingPtr >= sizeof(i->pending) - 1) {
		DDB_FlushWindow(i, w);
	}

	i->pending[i->pendingPtr++] = c;
	if (i->pendingPtr >= sizeof(i->pending) / 2 && (c < 16 || c == ' ' || (uint8_t)c == 0xA0))
		DDB_FlushWindow(i, w);
}

void DDB_OutputChar (DDB_Interpreter* i, char c)
{
	OutputCharToWindow(i, &i->win, c);
}

static void OutputTextToWindow (DDB_Interpreter* i, const char* text, DDB_Window* w)
{
	while (*text != 0)
		OutputCharToWindow(i, w, (uint8_t)*text++);
}

void DDB_OutputText (DDB_Interpreter* i, const char* text)
{
	OutputTextToWindow(i, text, &i->win);
}

static void OutputMessageContents(DDB_Interpreter* i, const uint8_t* ptr, DDB_Window* w)
{
	DDB* ddb = i->ddb;

	#if HAS_PAWS
	uint8_t eof = ddb->version == DDB_VERSION_PAWS ? 0x1F : 0x0A;
	#else
	const uint8_t eof = 0x0A;
	#endif

	uint8_t firstToken = ddb->firstToken;

	while (true)
	{
		uint8_t c = *ptr++ ^ 0xFF;
		if (c == eof)
			break;
		if (c >= firstToken)
		{
			if (!ddb->hasTokens)
			{
				DDB_Warning("Message contains token 0x%02X but DDB has no tokens!", c);
				continue;
			}
			uint8_t* token = ddb->tokensPtr[c - ddb->firstToken];
			if (token == 0)
			{
				DDB_Warning("Message contains token 0x%02X but it's not defined in the DDB!", c);
				continue;
			}
			for (;;)
			{
				OutputCharToWindow(i, w, *token & 0x7F);
				TRACE(TranslateCharForTrace(*token & 0x7F));
				if (*token >= 128)
					break;
				token++;
			}
		}
		else
		{
			OutputCharToWindow(i, w, c);
			TRACE(TranslateCharForTrace(c));
		}
	}
}

bool DDB_OutputMessageToWindow (DDB_Interpreter* i, DDB_MsgType type, uint8_t msgId, DDB_Window* w)
{
	DDB* ddb = i->ddb;
	uint8_t* ptr;

	switch (type)
	{
		case DDB_MSG:
			ptr = ddb->messages[msgId];
			break;
		case DDB_SYSMSG:
			if (msgId >= ddb->numSystemMessages)
				return false;
			ptr = ddb->data + ddb->sysMsgTable[msgId];
			break;
		case DDB_OBJNAME:
			if (msgId >= ddb->numObjects)
				return false;
			ptr = ddb->data + ddb->objNamTable[msgId];
			break;
		case DDB_LOCDESC:
			ptr = ddb->locDescriptions[msgId];
			break;
		default:
			DDB_Warning("Invalid message type %d", type);
			return false;
	}

	if (ptr <= ddb->data || ptr >= ddb->data + ddb->dataSize)
		return false;

	TRACE("%s%d: \"", DDB_MessageTypeNames[type], msgId);
	OutputMessageContents(i, ptr, w);
	TRACE("\" ");

	return true;
}


bool DDB_OutputMessage (DDB_Interpreter* i, DDB_MsgType type, uint8_t index)
{
	return DDB_OutputMessageToWindow(i, type, index, &i->win);
}

void DDB_OutputUserPrompt(DDB_Interpreter* i)
{
	DDB_Window *iw = DDB_GetInputWindow(i);
	int prompt = i->flags[Flag_Prompt];
	if (prompt == 0 || prompt >= i->ddb->numSystemMessages)
	{
		#if HAS_PAWS
		if (DDB_IsPAWS(i->ddb->version))
		{
			// PAW draws from the same $8546 stream as CHANCE/RANDOM and uses
			// deliberately uneven ranges for the four default prompts.
			uint8_t value = DDB_PAWSRandom(i);
			prompt = value < 30 ? 2 : value < 60 ? 3 : value < 90 ? 4 : 5;
		}
		else
		#endif
			prompt = RandInt(2, 5);
	}
	TRACE("\n\n");
	DDB_Flush(i);
	DDB_OutputMessageToWindow(i, DDB_SYSMSG, prompt, iw);
	DDB_FlushWindow(i, iw);
}

void DDB_OutputInputPrompt(DDB_Interpreter* i)
{
	DDB_Window *iw = DDB_GetInputWindow(i);
	DDB_OutputMessageToWindow(i, DDB_SYSMSG, 33, iw);
}

void DDB_UseTranscriptFile(const char* fileName)
{
    transcriptFile = File_Create(fileName);
	transcriptAtLineStart = true;
	transcriptBreakPending = false;
	DDB_TranscriptFlush();
}

static void PrintAt (DDB_Interpreter* i, DDB_Window* w, int line, int col)
{
	DDB_Flush(i);

	#if HAS_PAWS
	if (i->ddb->version == DDB_VERSION_PAWS)
	{
		w->posY = line * lineHeight;
		w->posX = col * columnWidth;
		w->smooth = 0;
		// DebugPrintf("PrintAt(%d,%d) in window %d (Y %d) -> %d,%d\n", line, col, i->curwin, w->posY, w->posX, w->posY);
		return;
	}
	#endif

	w->posY = w->y + line * lineHeight;
	w->posX = w->x + col * columnWidth;
	if (w->posX < w->x || w->posX > w->x + w->width ||
		w->posY < w->y || w->posY > w->y + w->height - lineHeight)
	{
		w->posX = w->x;
		w->posY = w->y;
	}
	w->scrollCount = 0;
	w->smooth = 0;

	DDB_TranscriptBreak();
}

static void WinAt (DDB_Interpreter* i, int line, int col)
{
	DDB_Window* w = &i->windef[i->curwin];

	w->y = line * lineHeight;
	w->x = col * columnWidth;

	if (w->y >= screenHeight - lineHeight)
		w->y = screenHeight - lineHeight;
	if (w->x >= screenWidth - columnWidth)
		w->x = screenWidth - columnWidth;

	if (w->height + w->y > screenHeight)
		w->height = w->y >= screenHeight ? 0 : screenHeight - w->y;
	if (w->width + w->x > screenWidth)
		w->width = w->x >= screenWidth ? 0 : screenWidth - w->x;

	#if HAS_PAWS
	if (i->ddb->version != DDB_VERSION_PAWS)
	#endif
	PrintAt(i, w, 0, 0);

	i->win.posX = w->posX;
	i->win.posY = w->posY;
	i->win.width = w->width;
	i->win.height = w->height;
	i->win.x = w->x;
	i->win.y = w->y;

	DDB_CalculateCells(i, w, &i->cellX, &i->cellW);

	// DebugPrintf("Window %d repositioned: %d,%d %dx%d\n", i->curwin, w->x, w->y, w->width, w->height);
}

static void CenterWindow (DDB_Interpreter* i, DDB_Window* w)
{
	w->posX -= w->x;
	if (w->width >= screenWidth)
	{
		w->x = 0;
		w->width = screenWidth;
	}
	else
	{
		w->x = (screenWidth - w->width) / 2;
		w->x -= w->x % columnWidth;
	}
	w->posX += w->x;

	DDB_CalculateCells(i, w, &i->cellX, &i->cellW);
}

static void WinSize (DDB_Interpreter* i, int lines, int columns)
{
	DDB_Window* w = &i->windef[i->curwin];

	w->height = lines * lineHeight;
	w->width = columns * columnWidth;
	if (w->height + w->y > screenHeight)
		w->height = w->y >= screenHeight ? 0 : screenHeight - w->y;
	if (w->width + w->x > screenWidth)
		w->width = w->x >= screenWidth ? 0 : screenWidth - w->x;

	#if HAS_PAWS
	if (i->ddb->version != DDB_VERSION_PAWS)
	#endif
	PrintAt(i, w, 0, 0);

	i->win.posX = w->posX;
	i->win.posY = w->posY;
	i->win.width = w->width;
	i->win.height = w->height;
	i->win.x = w->x;
	i->win.y = w->y;

	DDB_CalculateCells(i, w, &i->cellX, &i->cellW);

	// DebugPrintf("Window %d resized: %d,%d %dx%d\n", i->curwin, w->x, w->y, w->width, w->height);
}

static int CountObjectsAt (DDB_Interpreter* i, uint8_t locno)
{
	int count = 0;
	if (locno == 255)
		locno = i->flags[Flag_Locno];
	for (int n = 0; n < i->ddb->numObjects; n++)
	{
		if (i->objloc[n] == locno)
			count++;
	}
	return count;
}

static void ListObjectsAt (DDB_Interpreter* i, int locno)
{
	if (locno == 255) locno = i->flags[Flag_Locno];

	int count = CountObjectsAt(i, locno);
    bool newLineAtEnd = false;

	i->flags[Flag_ListFlags] &= ~ListFlag_Found;
	for (int n = 0; n < i->ddb->numObjects; n++)
	{
		if (i->objloc[n] == locno)
		{
			i->flags[Flag_ListFlags] |= ListFlag_Found;
			const void* end = DDB_GetMessage(i->ddb, DDB_OBJNAME, n, (char *)objNameBuffer, sizeof(objNameBuffer));
			if (i->flags[Flag_ListFlags] & ListFlag_Continuous)		// Continuous listing
			{
				objNameBuffer[0] = ToLower(objNameBuffer[0]);
				for (const char* ptr = (const char*)objNameBuffer; ptr < end && *ptr != '.'; ptr++)
					OutputCharToWindow(i, &i->win, (uint8_t)*ptr);
				if (count == 1)
					DDB_OutputMessage(i, DDB_SYSMSG, 48);		// .
				else if (count == 2)
					DDB_OutputMessage(i, DDB_SYSMSG, 47);		// and
				else
					DDB_OutputMessage(i, DDB_SYSMSG, 46);		// ,
			}
			else
			{
                DDB_Flush(i);
				DDB_NewLine(i);
				DDB_OutputText(i, (const char *)objNameBuffer);
                newLineAtEnd = true;
			}
			count--;
		}
	}
    if (newLineAtEnd)
    {
        DDB_Flush(i);
        DDB_NewLine(i);
    }
}

static void SetObjno (DDB_Interpreter* i, uint8_t objno)
{
	i->flags[Flag_Objno] = objno;
	i->flags[Flag_ObjLocno] = 0;
	i->flags[Flag_ObjWeight] = 0;
	i->flags[Flag_ObjContainer] = 0;
	i->flags[Flag_ObjWearable] = 0;

	if (i->ddb->version > 1)
	{
		i->flags[Flag_ObjAttributes] = 0;
		i->flags[Flag_ObjAttributes + 1] = 0;
	}

	if (objno != 255 && objno < i->ddb->numObjects)
	{
		uint8_t attr = i->ddb->objAttrTable[objno];
		i->flags[Flag_ObjLocno] = i->objloc[objno];
		i->flags[Flag_ObjWeight] = attr & Obj_Weight;
		i->flags[Flag_ObjContainer] = (attr & Obj_Container) ? 128 : 0;
		i->flags[Flag_ObjWearable] = (attr & Obj_Wearable) ? 128 : 0;
		if (attr & Obj_Container)
			i->flags[Flag_ObjWeight] = CalculateWeight(i, objno, 0);
		if (i->ddb->objExAttrTable)
		{
			uint16_t ex = i->ddb->objExAttrTable[objno];
			i->flags[Flag_ObjAttributes] = ex & 0xFF;
			i->flags[Flag_ObjAttributes + 1] = ex >> 8;
		}
	}
}

static uint8_t WhatoAt (DDB_Interpreter* i, uint8_t locno)
{
	uint8_t objno = 255;

	for (int n = 0; n < i->ddb->numObjects; n++)
	{
		uint8_t noun = i->ddb->objWordsTable[n * 2];
		uint8_t adjective = i->ddb->objWordsTable[n * 2 + 1];

		if (noun == 255)
			continue;
		if (i->flags[Flag_Noun1] == noun && (i->flags[Flag_Adjective1] == 255 || i->flags[Flag_Adjective1] == adjective))
		{
			if (locno == 255 || i->objloc[n] == locno)
				return n;
		}
	}

	// Enhancement: allow using a single adjective to refer to an object
	// but only if no unknown words where present in the sentence.

	if (objno == 255 && locno != 255 &&
		i->flags[Flag_Noun1] == 255  &&
		i->flags[Flag_Adjective1] != 255 &&
		!(i->sentenceFlags & SentenceFlag_UnknownWord))
	{
		for (int n = 0; n < i->ddb->numObjects; n++)
		{
			uint8_t noun = i->ddb->objWordsTable[n * 2];
			uint8_t adjective = i->ddb->objWordsTable[n * 2 + 1];

			if (i->objloc[n] == locno && i->flags[Flag_Adjective1] == adjective)
			{
				i->flags[Flag_Noun1] = noun;
				objno = n;
				break;
			}
		}
	}

	return objno;
}

static uint8_t WhatoAt (DDB_Interpreter* i, uint8_t locno, uint8_t locno2, uint8_t locno3)
{
	uint8_t objno = WhatoAt(i, locno);
	if (objno == 255)
		objno = WhatoAt(i, locno2);
	if (objno == 255)
		objno = WhatoAt(i, locno3);
	return objno;
}

static void AutoFailed (DDB_Interpreter* i, uint8_t msg = 28)
{
	uint8_t objno = WhatoAt(i, 255);
	if (objno != 255 && i->ddb->version != DDB_VERSION_1)
		DDB_OutputMessage(i, DDB_SYSMSG, msg);	// 28 (I can't see any of those) or custom
	else
		DDB_OutputMessage(i, DDB_SYSMSG, 8);	// I can't do that
	DDB_NewText(i);
}

static uint8_t Whato (DDB_Interpreter* i)
{
	int objno = WhatoAt(i, Loc_Carried, Loc_Worn, i->flags[Flag_Locno]);
	if (objno == 255)
		objno = WhatoAt(i, 255);
	return objno;
}

static bool DoAll (DDB_Interpreter* i, uint8_t locno, bool start)
{
	int n;

	if (locno == Loc_Here)
		locno = i->flags[Flag_Locno];

	if (start)
	{
		i->doallDepth = 0;
		i->doallLocno = locno;
		i->flags[Flag_DoAllLocNo] = locno;
		i->flags[Flag_ListFlags] &= ~ListFlag_DoallFailed;

	#if TRACE_ON
		int total = 0;
		for (int n = 0; n < i->ddb->numObjects; n++)
			if (i->objloc[n] == locno)
				total++;
		TRACE("(%d objects in loc %d)", total, locno);
	#endif
		n = 0;
	}
	else
	{
		n = i->doallObjno + 1;
	}

	for (; n < i->ddb->numObjects; n++)
	{
		if (i->objloc[n] == locno)
		{
			uint8_t noun = i->ddb->objWordsTable[2*n];
			uint8_t adjective = i->ddb->objWordsTable[2 * n + 1];
			if (i->flags[Flag_Noun2] != 255 && i->flags[Flag_Noun2] == noun)
			{
				if (i->flags[Flag_Adjective2] == 255 || i->flags[Flag_Adjective2] == adjective)
					continue;
			}
			i->flags[Flag_Noun1] = i->ddb->objWordsTable[2*n];
			i->flags[Flag_Adjective1] = i->ddb->objWordsTable[2*n + 1];

			// TODO: Verify this step, since it is not documented
			SetObjno(i, n);

			i->doall = true;
			i->doallObjno = n;
			return true;
		}
	}

	i->doall = false;
	i->doallObjno = 255;
	i->flags[Flag_DoAllLocNo] = locno;
	if (start)
		i->flags[Flag_ListFlags] |= ListFlag_DoallFailed;
	return false;
}

static bool Present(DDB_Interpreter* i, uint8_t objno)
{
	return objno < i->ddb->numObjects && (
		i->objloc[objno] == i->flags[Flag_Locno] ||
		i->objloc[objno] == Loc_Worn ||
		i->objloc[objno] == Loc_Carried);
}

static bool Absent(DDB_Interpreter* i, uint8_t objno)
{
	return objno >= i->ddb->numObjects || (
		i->objloc[objno] != i->flags[Flag_Locno] &&
		i->objloc[objno] != Loc_Carried &&
		i->objloc[objno] != Loc_Worn);
}


#if HAS_DRAWSTRING
// The original interpreters set the current text window to the drawn
// picture's window, so the game's own CLS before the next picture only
// clears the graphics area (text printed below survives)
static void AdoptVectorPictureWindow(DDB_Interpreter* i, uint8_t picno)
{
	int x, y, w, h;
	if (!DDB_GetVectorPictureWindow(picno, &x, &y, &w, &h))
		return;
	DDB_Window* win = &i->win;
	win->x = x;
	win->y = y;
	win->width = w;
	win->height = h;
	win->posX = x;
	win->posY = y;
	i->windef[i->curwin] = *win;
	DDB_CalculateCells(i, win, &i->cellX, &i->cellW);
}
#endif

void DDB_Desc (DDB_Interpreter* i, uint8_t locno)
{
	if (locno == 255)
		locno = i->flags[Flag_Locno];
	if (i->ddb->version > 1)
	{
		DDB_OutputMessage(i, DDB_LOCDESC, locno);
		return;
	}

	if (i->flags[2] > 0)
		i->flags[2]--;
	if (i->flags[3] > 0 && i->flags[Flag_Darkness] != 0)
		i->flags[3]--;

	if (i->flags[Flag_Darkness] != 0 && Absent(i, 0))
	{
		if (i->ddb->version < 2 && i->flags[4] > 0)
			i->flags[4]--;

		if (i->ddb->version == DDB_VERSION_1 && !i->ddb->drawString && SCR_PictureExists(i->flags[Flag_Locno]))
		{
			int16_t x, width;
			DDB_SetWindow(i, 0);
			BufferPicture(i, i->flags[Flag_Locno]);
			PrepareBufferedPictureWindow(i, &x, &width);
			DDB_ClearWindow(i, &i->win);
		}

		DDB_SetWindow(i, 1);
		if (i->ddb->version < 2)
		{
			if ((i->flags[Flag_GraphicFlags] & Graphics_NoClsBeforeDesc) == 0)
			{
				#if HAS_PAWS
				if (i->ddb->version == DDB_VERSION_PAWS)
				{
					WinAt(i, 0, 0);
					WinSize(i, 24, 32);
				}
				#endif
				DDB_ClearWindow(i, &i->win);
			}
		}
		DDB_OutputMessage(i, DDB_SYSMSG, 0);
	}
	else
	{
		if (i->ddb->version >= 1)
			i->flags[40] = 0;

		DDB_SetWindow(i, 0);
		bool pawsAutomaticPicture = false;
		#if HAS_DRAWSTRING
		if (i->ddb->drawString)
		{
			#if HAS_PAWS
			if (DDB_IsPAWS(i->ddb->version))
				pawsAutomaticPicture = DDB_PAWSPrepareDescription(i, locno);
			else
			#endif
			{
				DDB_ClearWindow(i, &i->win);
				if (DDB_HasVectorWindow(locno))
				{
					SCR_DrawVectorPicture(locno);
					if (DDB_HasVectorPicture(locno))
						i->flags[Flag_HasPicture] = 128;
					AdoptVectorPictureWindow(i, locno);
				}
			}
		}
		else
		#endif
		if (SCR_PictureExists(i->flags[Flag_Locno]))
		{
			if (i->ddb->version > 1)
				i->flags[40] = 255;

			BufferPicture(i, i->flags[Flag_Locno]);
			DrawBufferedPicture(i);
		}
		if (i->ddb->version != DDB_VERSION_PAWS)
		{
			DDB_SetWindow(i, 1);
			if ((i->flags[Flag_GraphicFlags] & Graphics_NoClsBeforeDesc) == 0)
			{
				DDB_Flush(i);
				DDB_ClearWindow(i, &i->win);
			}
		}

		DDB_OutputMessage(i, DDB_LOCDESC, locno == 255 ? i->flags[Flag_Locno] : locno);
		#if HAS_PAWS
		if (DDB_IsPAWS(i->ddb->version))
			DDB_PAWSFinishDescription(i, pawsAutomaticPicture);
		#endif
	}

	i->procstack[0].entry   = 0;
	i->procstack[0].process = 1;
	i->procstack[0].offset  = 0;
	i->procstackptr = 0;
	i->doall = false;
	i->oldMainLoopState = FLOW_DESC;
	i->state = DDB_RUNNING;
}

void DDB_NewText(DDB_Interpreter* i)
{
	i->inputBufferPtr = 0;
	i->inputBuffer[0] = 0;
	i->inputBufferLength = 0;
	i->inputCompletionX = 0;
}

void DDB_SetWindow(DDB_Interpreter* i, int winno)
{
	if (i->curwin != (winno & 7))
		DDB_TranscriptBreak();
	i->windef[i->curwin] = i->win;
	i->curwin = winno & 7;
	i->win = i->windef[i->curwin];
	i->flags[Flag_Window] = winno;

	DDB_CalculateCells(i, &i->win, &i->cellX, &i->cellW);

	// DebugPrintf("Window %d selected\n", i->curwin);
}

static void UpdatePos (DDB_Interpreter* i, int process, int entry, int offset)
{
	i->procstack[i->procstackptr].process = process;
	i->procstack[i->procstackptr].entry = entry;
	i->procstack[i->procstackptr].offset = offset;
}

static bool MovePlayer (DDB_Interpreter* i, uint8_t flag)
{
	// MOVE routes the location held in the given flag through the current
	// verb's connections. For flag 38 that is the player's own movement, but
	// any other flag moves an NPC: Espacial part 3 makes companions follow
	// the player with MOVE [n] over each character's location flag.
	uint8_t locno = i->flags[flag];
	if (locno >= i->ddb->numLocations)
		return false;
	uint8_t* ptr = i->ddb->locConnections[locno];
	uint8_t* end = i->ddb->data + i->ddb->dataSize;
	if (ptr == 0)
		return false;
	while (*ptr != 0xFF && ptr < end)
	{
		if (ptr[0] == i->flags[Flag_Verb])
		{
			i->flags[flag] = ptr[1];
			return true;
		}
		ptr += 2;
	}
	return false;
}

static bool FindWord (DDB_Interpreter* i, const uint8_t** textPointer, const uint8_t* end, uint8_t* type, uint8_t* code)
{
	uint8_t* word = i->ddb->vocabulary;
	const uint8_t* ptr = *textPointer;

	// TODO: Sort vocabulary in memory and use binary search

	while (*word != 0)
	{
		bool found = true;
		for (int n = 0; n < 5; n++)
		{
			uint8_t ch = word[n] ^ 0xFF;
			uint8_t cp = ptr + n < end ? ptr[n] : ' ';
			if (ch != cp)
			{
				if (ch == ToUpper(cp))
					continue;
				if (ch != ' ' || IsAlphaNumeric(ptr[n]))
					found = false;
				break;
			}
			if (ch == ' ')
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			while (ptr < end && IsAlphaNumeric(*ptr))
				ptr++;
			*textPointer = ptr;
			*type = word[6];
			*code = word[5];
			return true;
		}
		word += 7;
	}
	return false;
}

static bool ShouldCheckSpanishPronouns (DDB_Interpreter* i, int code)
{
	if (i->ddb->language == DDB_SPANISH)
	{
		if ((i->flags[Flag_ListFlags] & ListFlag_NoSuffixesHi) != 0 && code >= 240)
			return false;
		return true;
	}

	return false;
}

static bool EndsWithSpanishPronoun (const char* word, int len, int version)
{
	// Short verbs are ignored. This is probably wrong, but it is what PAWS does, and
	// fixes some words such as SOLO being used as verbs in the default database.
	if (len < 5)
		return false;

	if (ToUpper(word[len-2]) == 'L' && (ToUpper(word[len-1]) == 'A' || ToUpper(word[len-1]) == 'O'))
	{
		// This hack prevents the parser from wrongly recognizing
		// pronouns in words like HABLA or AFILA. This hack has been
		// removed for moden versions since they offer better control.

		if (version < DDB_VERSION_3)
		{
			if (len > 2 && (ToUpper(word[len-3]) == 'B' || ToUpper(word[len-3]) == 'I'))
				return false;
		}
		return true;
	}

	if (len < 3 || ToUpper(word[len-1]) != 'S')
		return false;
	if (ToUpper(word[len-3]) == 'L' && (ToUpper(word[len-2]) == 'A' || ToUpper(word[len-2]) == 'O'))
	{
		if (len > 3 && (ToUpper(word[len-4]) == 'B' || ToUpper(word[len-4]) == 'I'))
			return false;
		return true;
	}
	return false;
}

static bool Parse (DDB_Interpreter* i, bool quoted)
{
	const uint8_t* ptr;
	const uint8_t* end;
	uint8_t code;
	uint8_t type;
	uint8_t previousVerb = 255;
	int wordsFound = 0;
	uint8_t lastWordType = WordType_Unknown;

	i->flags[Flag_ListFlags] &= ~ListFlag_Preposition;
	i->flags[Flag_ListFlags] &= ~ListFlag_UnknownWord;

	if (quoted && !i->quotedString && (i->sentenceFlags & SentenceFlag_Colon) != 0) {
		ptr = i->inputBuffer + i->inputBufferPtr;
		end = i->inputBuffer + i->inputBufferLength;
		quoted = false;
	} else if (quoted && i->quotedString) {
		ptr = i->quotedString;
		end = i->quotedString + i->quotedStringLength;
	} else {
		if (quoted) return false;
		ptr = i->inputBuffer + i->inputBufferPtr;
		end = i->inputBuffer + i->inputBufferLength;
	}

	if (i->inputBufferPtr != 0)
		previousVerb = i->flags[Flag_Verb];

	i->flags[Flag_Verb]        = 255;
	i->flags[Flag_Noun1]       = 255;
	i->flags[Flag_Noun2]       = 255;
	i->flags[Flag_Adjective1]  = 255;
	i->flags[Flag_Adjective2]  = 255;
	i->flags[Flag_Preposition] = 255;
	i->flags[Flag_Adverb]      = 255;
	i->flags[Flag_Preposition] = 255;
	i->sentenceFlags           = 0;

	while (ptr < end)
	{
		const uint8_t* word;

		if (*ptr == ' ' || *ptr == '\t')
		{
			ptr++;
			continue;
		}
		if (!quoted && !i->quotedString && (*ptr == '"' || *ptr == '\'')) {
			char quote = *ptr++;
			i->quotedString = (uint8_t*)ptr;
			while (ptr < end && *ptr != quote)
				ptr++;
			i->quotedStringLength = ptr - i->quotedString;
			if (*ptr == quote) ptr++;
			continue;
		}
		if (!IsAlphaNumeric(*ptr) && !IsDelimiter(*ptr))
		{
			ptr++;
			continue;
		}
		if (IsDelimiter(*ptr))
		{
			if (*ptr == ':')
				i->sentenceFlags |= SentenceFlag_Colon;
			else if (*ptr == '?')
				i->sentenceFlags |= SentenceFlag_Question;
			ptr++;
			if (wordsFound == 0 && (i->sentenceFlags & SentenceFlag_UnknownWord) == 0)
				continue;
			break;
		}

		word = ptr;
		if (FindWord(i, &ptr, end, &type, &code))
		{
			if (type == WordType_Conjunction)
			{
				if (wordsFound == 0)
				{
					lastWordType = WordType_Unknown;
					continue;
				}
				break;
			}

			wordsFound++;

			switch (type)
			{
				case WordType_Verb:
					if (i->flags[Flag_Verb] == 255)
					{
						i->flags[Flag_Verb] = code;

						if (i->flags[Flag_Noun1] == 255 && 
							i->flags[Flag_CPNoun] != 255 &&
							ShouldCheckSpanishPronouns(i, code) &&
							EndsWithSpanishPronoun((const char*)word, ptr - word, i->ddb->version))
						{
							i->flags[Flag_Noun1] = i->flags[Flag_CPNoun];
							i->flags[Flag_Adjective1] = i->flags[Flag_CPAdjective];
						}
					}
					else
					{
						// Second verb in the sentence is ignored
						type = WordType_Unknown;
					}
					break;
				case WordType_Pronoun:
					if (i->flags[Flag_Noun1] == 255 && i->flags[Flag_CPNoun] != 255)
					{
						i->flags[Flag_Noun1] = i->flags[Flag_CPNoun];
						i->flags[Flag_Adjective1] = i->flags[Flag_CPAdjective];
					}
					else if (i->flags[Flag_Noun2] == 255 && i->flags[Flag_CPNoun] != 255)
					{
						i->flags[Flag_Noun2] = i->flags[Flag_CPNoun];
						i->flags[Flag_Adjective2] = i->flags[Flag_CPAdjective];
					}
					break;
				case WordType_Noun:
					if (i->flags[Flag_Noun1] == 255)
						i->flags[Flag_Noun1] = code;
					else if (i->flags[Flag_Noun2] == 255)
						i->flags[Flag_Noun2] = code;
					break;
				case WordType_Adjective:
					if (i->flags[Flag_Adjective1] == 255 && i->flags[Flag_Noun2] == 255)
						i->flags[Flag_Adjective1] = code;
					else if (i->flags[Flag_Adjective2] == 255)
						i->flags[Flag_Adjective2] = code;
					break;
				case WordType_Preposition:
					// If no noun yet, mark the preposition prefix flag
					// to enable the game to check for early prepositions
					if (i->flags[Flag_Noun1] == 255)
						i->flags[Flag_ListFlags] |= ListFlag_Preposition;
					if (i->flags[Flag_Preposition] == 255)
						i->flags[Flag_Preposition] = code;
					break;
				case WordType_Adverb:
					if (i->flags[Flag_Adverb] == 255)
						i->flags[Flag_Adverb] = code;
					break;
				default:
					break;
			}
			lastWordType = type;
		}
		else
		{
			if (i->flags[Flag_Verb] != 255)
				i->flags[Flag_ListFlags] |= ListFlag_UnknownWord;

			lastWordType = WordType_Unknown;
			i->sentenceFlags |= SentenceFlag_UnknownWord;
			while(ptr < end && IsAlphaNumeric(*ptr))
				ptr++;
			continue;
		}
	}

	if (quoted) {
		i->quotedString = 0;
		i->quotedStringLength = 0;
	} else {
		i->inputBufferPtr = ptr - i->inputBuffer;
	}

	const int convertibleNoun = i->ddb->version < 2 ? 20 : 40;
	if (i->flags[Flag_Verb] == 255 && i->flags[Flag_Noun1] < convertibleNoun)
		i->flags[Flag_Verb] = i->flags[Flag_Noun1];
	else if (i->flags[Flag_Verb] == 255 && previousVerb && i->flags[Flag_Noun1] != 255 && i->flags[Flag_Noun1] != previousVerb)
		i->flags[Flag_Verb] = previousVerb;

	if (i->flags[Flag_Noun1] != 255 && i->flags[Flag_Noun1] >= 50)
	{
		i->flags[Flag_CPNoun] = i->flags[Flag_Noun1];
		i->flags[Flag_CPAdjective] = i->flags[Flag_Adjective1];
	}
	else if (i->flags[Flag_Noun2] != 255 && i->flags[Flag_Noun2] >= 50)
	{
		i->flags[Flag_CPNoun] = i->flags[Flag_Noun2];
		i->flags[Flag_CPAdjective] = i->flags[Flag_Adjective2];
	}

	return wordsFound > 0;
}

static struct
{
	int x;
	int y;
	int width;
	int height;
	bool hasOutput;
}
windowClears[16];
static int windowClearCount = 0;
static const int maxWindowClears = sizeof(windowClears) / sizeof(windowClears[0]);

static void MarkWindowOutput()
{
	if (windowClearCount > 0)
		windowClears[windowClearCount - 1].hasOutput = true;
}

static bool AnyWindowOverlapsCurrent (DDB_Interpreter* i)
{
	for (int n = 0; n < windowClearCount; n++)
	{
		if (   windowClears[n].x < i->win.x + i->win.width
			&& windowClears[n].x + windowClears[n].width > i->win.x
			&& windowClears[n].y < i->win.y + i->win.height
			&& windowClears[n].y + windowClears[n].height > i->win.y
			&& windowClears[n].hasOutput)
		{
			// DebugPrintf("Window %d (%d,%d %dx%d) overlaps previously cleared window %d (%d,%d %dx%d)\n", i->curwin,
			// 	i->win.x, i->win.y, i->win.width, i->win.height,
			// 	n, windowClears[n].x, windowClears[n].y, windowClears[n].width, windowClears[n].height);
			return true;
		}
	}
	if (windowClearCount < maxWindowClears)
	{
		windowClears[windowClearCount].x = i->win.x;
		windowClears[windowClearCount].y = i->win.y;
		windowClears[windowClearCount].width = i->win.width;
		windowClears[windowClearCount].height = i->win.height;
		windowClears[windowClearCount].hasOutput = i->win.paper != 0;
		windowClearCount++;
	}
	return false;
}

static int HasAt(DDB_Interpreter* i, int param0, DDB_HasAtOp op)
{
	int last = (i->flags[Flag_ListFlags] & ListFlag_AltHasAtRange) != 0 ? 91 : 59;
	int n = last - param0/8;
	int m = (1 << (param0 & 7));
	int ok = 1;

	switch (op)
	{
		case HASAT_ISSET:
			ok = (i->flags[n] & m) != 0;
			break;
		case HASAT_ISNOTSET:
			ok = (i->flags[n] & m) == 0;
			break;
		case HASAT_CLEAR:
			i->flags[n] &= ~m;
			return 1;
		case HASAT_SET:		
			i->flags[n] |= m;
			return 1;
		case HASAT_TOGGLE:
			i->flags[n] ^= m;
			return 1;
	}
	TRACE("%sFlag %d = %d ($%02X mask $%02X)", ok != 0 ? "":"[Failed] ", n, i->flags[n], i->flags[n] & m, m);
	return ok;
}

static void ResumeOldMainLoopProcess(DDB_Interpreter* i, DDB_Flow nextState, uint8_t process)
{
	i->oldMainLoopState = nextState;
	i->state = DDB_RUNNING;
	i->procstack[0].process = process;
	i->procstack[0].entry = 0;
	i->procstack[0].offset = 0;
	i->procstackptr = 0;
}

static void HandleOldMainLoopFinished(DDB_Interpreter* i)
{
	TRACE("\nSimulating PAWS main loop (current state: %s)\n", DDB_FlowNames[i->oldMainLoopState]);
	DDB_Flush(i);

	switch (i->oldMainLoopState)
	{
		case FLOW_STARTING:
			DDB_Desc(i, i->flags[Flag_Locno]);
			break;

		case FLOW_RESPONSES:
			if (i->flags[Flag_TimeoutFlags] & 0x80)
				DDB_OutputMessage(i, DDB_SYSMSG, 35);		// Time passes...
			else if (!i->done)
			{
				if (i->flags[Flag_Verb] < 14 || (i->flags[Flag_Verb] == 255 && i->flags[Flag_Noun1] < 14))
				{
					if (MovePlayer(i, Flag_Locno))
					{
						DDB_Desc(i, i->flags[Flag_Locno]);
						break;
					}
					DDB_OutputMessage(i, DDB_SYSMSG, 7);		// I can't go that way
				}
				else
					DDB_OutputMessage(i, DDB_SYSMSG, 8);		// I don't understand that.
				DDB_NewText(i);
			}
			// Fall through

		case FLOW_DESC:
			ResumeOldMainLoopProcess(i, FLOW_AFTER_TURN, 2);
			break;

		case FLOW_AFTER_TURN:
			if (i->flags[5] > 0) i->flags[5]--;
			if (i->flags[6] > 0) i->flags[6]--;
			if (i->flags[7] > 0) i->flags[7]--;
			if (i->flags[8] > 0) i->flags[8]--;
			if (i->flags[Flag_Darkness] != 0)
			{
				if (i->flags[9] > 0) i->flags[9]--;
				if (i->flags[10] > 0 && Absent(i, 0)) i->flags[10]--;
			}
			#if HAS_PAWS
			if (DDB_IsPAWS(i->ddb->version))
			{
				if (i->flags[Flag_Turns] == 255)
				{
					i->flags[Flag_Turns] = 0;
					if (i->flags[Flag_Turns + 1] != 255)
						i->flags[Flag_Turns + 1]++;
				}
				else i->flags[Flag_Turns]++;
			}
			else
			#endif
			if (i->flags[Flag_Turns+1] == 255) {
				i->flags[Flag_Turns+1] = 0;
				if (i->flags[Flag_Turns] != 255)
					i->flags[Flag_Turns]++;
			} else {
				i->flags[Flag_Turns+1]++;
			}
			if (Parse(i, 0)) {
				i->done = false;
				ResumeOldMainLoopProcess(i, FLOW_RESPONSES, 0);
			} else {
				DDB_StartInput(i, true);
				i->oldMainLoopState = FLOW_INPUT;
				DDB_PrintInputLine(i, true);
				if (i->flags[Flag_TimeoutFlags] & Timeout_Input)
				{
					StartTimeout(i);
				}
				i->flags[Flag_TimeoutFlags] &= ~Timeout_LastFrame;
			}
			break;

		case FLOW_INPUT:
			if (!Parse(i, false))
			{
				if (i->flags[Flag_TimeoutFlags] & Timeout_LastFrame)
					DDB_OutputMessage(i, DDB_SYSMSG, 35);
				else
					DDB_OutputMessage(i, DDB_SYSMSG, 6);
				ResumeOldMainLoopProcess(i, FLOW_AFTER_TURN, 2);
				break;
			}
			i->done = false;
			ResumeOldMainLoopProcess(i, FLOW_RESPONSES, 0);
			break;
	}
}

static void StartInputWindowState(DDB_Interpreter* i, DDB_State state, uint8_t message)
{
	DDB_Window* iw = DDB_GetInputWindow(i);
	DDB_Flush(i);
	DDB_NewText(i);
	DDB_OutputMessageToWindow(i, DDB_SYSMSG, message, iw);
	DDB_FlushWindow(i, iw);
	DDB_StartInput(i, false);
	i->state = state;
	DDB_PrintInputLine(i, true);
}

static void StartFileNameInputState(DDB_Interpreter* i, DDB_State state)
{
	DDB_OutputMessage(i, DDB_SYSMSG, 60);		// Enter file name
	DDB_OutputText(i, " ");
	DDB_Flush(i);
	DDB_NewText(i);
	i->state = state;
	DDB_NewText(i);
	DDB_PrintInputLine(i, true);
}

static bool ResolveFileDialogInput(DDB_Interpreter* i, bool isLoad)
{
	i->inputBufferLength = 0;
	i->inputBufferPtr = 0;
	i->inputBuffer[0] = 0;	// clear first: a cancelled/stub dialog must not reuse leftover input
	SCR_OpenFileDialog(isLoad, (char*)i->inputBuffer, sizeof(i->inputBuffer));
	if (i->inputBuffer[0] == 0)
		return false;

	i->inputBufferLength = StrLen((const char*)i->inputBuffer);
	if (isLoad)
		DDB_ResolveInputLoad(i);
	else
		DDB_ResolveInputSave(i);
	return true;
}

static void SetExecutionPosition(DDB_Interpreter* i, uint8_t process, uint16_t entry, uint16_t offset, uint8_t** entryPtr, uint8_t** code)
{
	*entryPtr = i->ddb->data + i->ddb->processTable[process] + entry * 4;
	*code = i->ddb->data + *(uint16_t *)(*entryPtr + 2) + offset;
}

static void SetExecutionProcessStart(DDB_Interpreter* i, uint8_t process, uint16_t* entry, uint16_t* offset, uint8_t** entryPtr, uint8_t** code)
{
	*entry = 0;
	*offset = 0;
	SetExecutionPosition(i, process, 0, 0, entryPtr, code);
}

// --------------------
//   Public functions
// --------------------

void DDB_Step (DDB_Interpreter* i, int stepCount)
{
	char output[16];
	uint16_t paletteChanges = 0;
	bool repeatingDisplay = false;

	bool matchVerbNoun = true;
	if (i->ddb->version < 2 && i->oldMainLoopState != FLOW_RESPONSES)
		matchVerbNoun = false;

	uint8_t  process  = i->procstack[i->procstackptr].process;
	uint16_t entry    = i->procstack[i->procstackptr].entry;
	uint16_t offset   = i->procstack[i->procstackptr].offset;
	uint8_t* entryPtr;
	uint8_t* code;
	SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);

	uint8_t  value, locno;

	if (i->state == DDB_PAUSED)
		i->state = DDB_RUNNING;
	else if (i->state != DDB_RUNNING)
		return;

	windowClearCount = 0;

	while (stepCount-- > 0)
	{
		if (offset == 0)
		{
			if (entry == 0)
			{
				TRACE("\nEntering process %d\n\n", process);
			}
			if (*entryPtr == 0)
			{
				if (i->doall)
				{
					if (i->doallDepth > 0)
						i->doallDepth--;
					else if (DoAll(i, i->flags[Flag_DoAllLocNo], false))
					{
						TRACE("Performing next DoAll: process %d, entry %d, offset %d\n", i->doallProcess, i->doallEntry, i->doallOffset);
						entry    = i->doallEntry;
						offset   = i->doallOffset;
						process  = i->doallProcess;
						SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);
						continue;
					}
					else
					{
						TRACE("DoAll finished\n");
					}
				}
				if (i->procstackptr == 0)
				{
					DDB_Flush(i);
					i->state = DDB_FINISHED;
					return;
				}

				TRACE("\nLeaving process %d\n\n", process);

				i->procstackptr--;
				process  = i->procstack[i->procstackptr].process;
				entry    = i->procstack[i->procstackptr].entry;
				offset   = i->procstack[i->procstackptr].offset;
				SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);

				TRACE("Resuming process %d, entry %d, offset %d\n\n", process, entry, offset);
				continue;
			}
			else
			{
				uint8_t verb = entryPtr[0];
				uint8_t noun = entryPtr[1];

				#if HAS_PAWS
				// PAW stores '*' as 1 and '_' as 255; both take the wildcard path
				if (i->ddb->version == DDB_VERSION_PAWS)
				{
					if (verb == 1) verb = 255;
					if (noun == 1) noun = 255;
				}
				#endif

				if (matchVerbNoun && (
					(verb != 255 && verb != i->flags[Flag_Verb]) ||
				    (noun != 255 && noun != i->flags[Flag_Noun1])))
				{
					entry++;
					offset = 0;
					entryPtr += 4;
					continue;
				}
				else
				{
					SetExecutionPosition(i, process, entry, 0, &entryPtr, &code);
				}

				if (verb == 255)
					TRACE("_    ");
				else
					TraceVocabularyWord(i->ddb, WordType_Verb, verb);
				TRACE("       ");
				if (noun == 255)
					TRACE("_    ");
				else
					TraceVocabularyWord(i->ddb, WordType_Noun, noun);
				TRACE("       ");
			}
		}
		else
		{
			TRACE("%-24s", "");
		}

		if (*code == 0xFF) // End of entry
		{
			entry++;
			offset = 0;
			entryPtr += 4;
			TRACE("\n");
			continue;
		}

		uint8_t condactIndex = *code & 0x7F;
		uint8_t condact = i->ddb->condactMap[condactIndex].condact;
		uint8_t params = i->ddb->condactMap[condactIndex].parameters;
		uint8_t param0 = params > 0 ? code[1] : 0;
		uint8_t param1 = params > 1 ? code[2] : 0;
		if (*code & 0x80)
			param0 = i->flags[param0];

		#if TRACE_ON
		TRACE("%-12s", DDB_GetCondactName((DDB_Condact)condact));
		if (params > 0)
		{
			if (*code & 0x80)
				TRACE("[%d]%s", code[1], code[1] < 10 ? "  " : code[1] < 100 ? " " : "");
			else
				TRACE("%-5d", param0);
			if (params > 1)
				TRACE(" %-3d", param1);
			else
				TRACE("    ");
			if (params > 2)
				TRACE(" %-3d", code[3]);
		}
		else
		{
			TRACE("         ");
		}
		if (*code & 0x80)
			TRACE("    | ([%d] = %d)  ", code[1], i->flags[code[1]]);
		else
			TRACE("    | ");
		#endif

		bool finished  = false;
		bool ok = true;				// If !ok, jump to next entry

		switch (condact)
		{
			// Generic conditions

			case CONDACT_AT:
				ok = i->flags[Flag_Locno] == param0;
				TRACE("%sPlayer is at %d", ok ? "":"[Failed] ", i->flags[Flag_Locno]);
				break;
			case CONDACT_NOTAT:
				ok = i->flags[Flag_Locno] != param0;
				TRACE("%sPlayer is at %d", ok ? "":"[Failed] ", i->flags[Flag_Locno]);
				break;
			case CONDACT_ATGT:
				ok = i->flags[Flag_Locno] > param0;
				TRACE("%sPlayer is at %d", ok ? "":"[Failed] ", i->flags[Flag_Locno]);
				break;
			case CONDACT_ATLT:
				ok = i->flags[Flag_Locno] < param0;
				TRACE("%sPlayer is at %d", ok ? "":"[Failed] ", i->flags[Flag_Locno]);
				break;
			case CONDACT_PRESENT:
				ok = Present(i, param0);
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_ABSENT:
				ok = Absent(i, param0);
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_WORN:
				ok = param0 < i->ddb->numObjects && i->objloc[param0] == Loc_Worn;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_NOTWORN:
				ok = param0 < i->ddb->numObjects && i->objloc[param0] != Loc_Worn;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_CARRIED:
				ok = param0 < i->ddb->numObjects && i->objloc[param0] == Loc_Carried;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_NOTCARR:
				ok = param0 < i->ddb->numObjects && i->objloc[param0] != Loc_Carried;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_ISAT:
				if (param1 == 255) param1 = i->flags[Flag_Locno];
				ok = param0 < i->ddb->numObjects && i->objloc[param0] == param1;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_ISNOTAT:
				if (param1 == 255) param1 = i->flags[Flag_Locno];
				ok = param0 < i->ddb->numObjects && i->objloc[param0] != param1;
				TRACE("%sObj#%d \"%s\" is in %d", ok ? "":"[Failed] ", param0, DDB_GetDebugMessage(i->ddb, DDB_OBJNAME, param0), i->objloc[param0]);
				break;
			case CONDACT_HASAT:
				ok = HasAt(i, param0, HASAT_ISSET);
				break;
			case CONDACT_HASNAT:
				ok = HasAt(i, param0, HASAT_ISNOTSET);
				break;
			case CONDACT_SETAT:
				ok = HasAt(i, param0, (DDB_HasAtOp)param1);
				break;
			case CONDACT_ZERO:
				ok = i->flags[param0] == 0;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_NOTZERO:
				ok = i->flags[param0] != 0;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_EQ:
				ok = i->flags[param0] == param1;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_NOTEQ:
				ok = i->flags[param0] != param1;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_GT:
				ok = i->flags[param0] > param1;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_LT:
				ok = i->flags[param0] < param1;
				TRACE("%sFlag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0]);
				break;
			case CONDACT_SAME:
				ok = i->flags[param0] == i->flags[param1];
				TRACE("%sFlag %d = %d, flag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0], param1, i->flags[param1]);
				break;
			case CONDACT_NOTSAME:
				ok = i->flags[param0] != i->flags[param1];
				TRACE("%sFlag %d = %d, flag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0], param1, i->flags[param1]);
				break;
			case CONDACT_BIGGER:
				ok = i->flags[param0] > i->flags[param1];
				TRACE("%sFlag %d = %d, flag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0], param1, i->flags[param1]);
				break;
			case CONDACT_SMALLER:
				ok = i->flags[param0] < i->flags[param1];
				TRACE("%sFlag %d = %d, flag %d = %d", ok ? "":"[Failed] ", param0, i->flags[param0], param1, i->flags[param1]);
				break;
			case CONDACT_ADJECT1:
				ok = i->flags[Flag_Adjective1] == param0;
				TRACE("%sAdjective1 = %d", ok ? "":"[Failed] ", param0, i->flags[Flag_Adjective1]);
				break;
			case CONDACT_ADJECT2:
				ok = i->flags[Flag_Adjective2] == param0;
				TRACE("%sAdjective2 = %d", ok ? "":"[Failed] ", param0, i->flags[Flag_Adjective2]);
				break;
			case CONDACT_NOUN2:
				ok = i->flags[Flag_Noun2] == param0;
				TRACE("%sNoun2 = %d", ok ? "":"[Failed] ", param0, i->flags[Flag_Noun2]);
				break;
			case CONDACT_ADVERB:
				ok = i->flags[Flag_Adverb] == param0;
				TRACE("%sAdverb = %d", ok ? "":"[Failed] ", param0, i->flags[Flag_Adverb]);
				break;
			case CONDACT_PREP:
				ok = i->flags[Flag_Preposition] == param0;
				TRACE("%sPreposition = %d", ok ? "":"[Failed] ", param0, i->flags[Flag_Preposition]);
				break;
			case CONDACT_ISDONE:
				ok = i->done;
				TRACE("%s", ok ? "":"[Failed] ");
				break;
			case CONDACT_ISNDONE:
				ok = !i->done;
				TRACE("%s", ok ? "":"[Failed] ");
				break;

			// Parser

			case CONDACT_NEWTEXT:
				DDB_NewText(i);
				break;

			// Text output

			case_DESC:
			case CONDACT_DESC:
				UpdatePos(i, process, entry, offset + params + 1);
				if (params == 0)
					param0 = i->flags[Flag_Locno];
				DDB_Desc(i, param0);
				if (i->ddb->oldMainLoop)
				{
					TRACE("\n");
					return;
				}
				break;
			case CONDACT_MES:
				MarkWindowOutput();
				DDB_OutputMessage(i, DDB_MSG, param0);
				i->done = true;
				break;
			case CONDACT_SYSMESS:
				MarkWindowOutput();
				DDB_OutputMessage(i, DDB_SYSMSG, param0);
				i->done = true;
				break;
			case CONDACT_MESSAGE:
				MarkWindowOutput();
				DDB_OutputMessage(i, DDB_MSG, param0);
				DDB_Flush(i);
				DDB_NewLine(i);
				DDB_ResetPAWSColors(i, &i->win);
				i->done = true;
				break;
			case CONDACT_TAB:
				DDB_Flush(i);
				i->win.posX = i->win.x + param0 * columnWidth;
				if (i->win.posX > i->win.x + i->win.width - columnWidth) {
					i->win.posX = i->win.x + i->win.width - columnWidth;
				}
				i->done = true;
				break;
			case CONDACT_SPACE:
				DDB_OutputText(i, " ");
				i->done = true;
				break;
			case CONDACT_NEWLINE:
				DDB_Flush(i);
				DDB_NewLine(i);
				DDB_ResetPAWSColors(i, &i->win);
				i->done = true;
				break;
			case CONDACT_PRINT:
				LongToChar(i->flags[param0], output, 10);
				DDB_OutputText(i, output);
				break;
			case CONDACT_DPRINT:
				LongToChar(i->flags[param0] + 256*i->flags[param0+1], output, 10);
				DDB_OutputText(i, output);
				i->done = true;
				TRACE("%d", i->flags[param0] + 256*i->flags[param0+1]);
				break;
			case CONDACT_PRINTAT:
				PrintAt(i, &i->win, param0, param1);
				i->done = true;
				break;
			case CONDACT_LISTOBJ:
				i->flags[Flag_ListFlags] &= ~0x80;
				if (CountObjectsAt(i, i->flags[Flag_Locno]) == 0)
					break;
				param0 = i->flags[Flag_Locno];
				DDB_OutputMessage(i, DDB_SYSMSG, 1);
				// Fall through
			case CONDACT_LISTAT:
			{
				if (param0 == 255) param0 = i->flags[Flag_Locno];
				int count = CountObjectsAt(i, param0);
				if (count == 0)
					DDB_OutputMessage(i, DDB_SYSMSG, 53);	// Nothing.
				else
					ListObjectsAt(i, param0);
				i->done = true;
				break;
			}
			case CONDACT_INVEN:
			{
				DDB_OutputMessage(i, DDB_SYSMSG, 9); 	// I have
				int countWorn = CountObjectsAt(i, Loc_Worn);
				int countCarried = CountObjectsAt(i, Loc_Carried);
				if (countWorn == 0 && countCarried == 0)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 11);	// Nothing.
				}
				else
				{
					DDB_Flush(i);
					DDB_NewLine(i);
					for (int n = 0; n < i->ddb->numObjects; n++)
					{
						if (i->objloc[n] == Loc_Worn || i->objloc[n] == Loc_Carried)
						{
							DDB_OutputMessage(i, DDB_OBJNAME, n);
							if (i->objloc[n] == Loc_Worn)
								DDB_OutputMessage(i, DDB_SYSMSG, 10);	// (worn)
							DDB_Flush(i);
							DDB_NewLine(i);
						}
					}
				}
				i->done = true;
				break;
			}
			case CONDACT_INPUT:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					DDB_PAWSSetInput(i, param0);
				else
				#endif
				{
					i->flags[Flag_InputStream] = param0;
					i->inputFlags = param1;
				}
				break;
			case CONDACT_END:
			{
				StartInputWindowState(i, DDB_INPUT_END, 13);
				TRACE("\n");
				return;
			}
			case CONDACT_QUIT:
			{
				StartInputWindowState(i, DDB_INPUT_QUIT, 12);
				UpdatePos(i, process, entry, offset + params + 1);
				TRACE("\n");
				return;
			}

			// Flow

			case CONDACT_EXIT:
				if (param0 == 0)
				{
					i->state = DDB_QUIT;
					UpdatePos(i, process, entry, offset);
					TRACE("\n");
					return;
				}

				// A non-zero EXIT value is the DAAD AUTOLOAD feature: jump to the
				// given part number, keeping the user variables (flags + object
				// locations) live in memory rather than saving to disk. Originally
				// only the PCW interpreter supported it. When the host player can
				// reload parts (desktop), hand the request up through the
				// DDB_AUTOLOAD state; the player reloads the part and restores the
				// preserved state block. Otherwise fall back to a full reset.

				if (i->autoloadEnabled)
				{
					i->autoloadPart = (uint8_t)param0;
					i->state = DDB_AUTOLOAD;
					UpdatePos(i, process, entry, offset);
					TRACE("\n");
					return;
				}

				// Hosts without autoload support: the original interpreters did a
				// RESTART here; we perform a complete reset instead.
				DDB_Restart(i);
				process = 0;
				SetExecutionProcessStart(i, process, &entry, &offset, &entryPtr, &code);
				TRACE("\n");
				continue;

			case CONDACT_PROCESS:
				if (i->procstackptr == MAX_PROC_STACK - 1)
				{
					//fputs("\nMaximum process stack depth reached!\n", stderr);
					i->state = DDB_FATAL_ERROR;
					TRACE("\n");
					return;
				}
				TRACE("\n\nSaving state at process %d, entry %d, offset %d at stack %d\n", process, entry, offset, i->procstackptr);
				UpdatePos(i, process, entry, offset + params + 1);
				i->procstackptr++;
				process = param0;
				SetExecutionProcessStart(i, process, &entry, &offset, &entryPtr, &code);
				if (i->doall)
					i->doallDepth++;
				i->done = false;
				TRACE("\n");
				continue;

			case CONDACT_NOTDONE:
			case_NOTDONE:
				condact = CONDACT_NOTDONE;
				// Fall through
			case CONDACT_OK:
			case CONDACT_DONE:
			case_DONE:
				if (condact == CONDACT_OK)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 15);
					condact = CONDACT_DONE;
				}
				i->done = condact != CONDACT_NOTDONE;
				if (i->doall)
				{
					if (i->doallDepth > 0)
						i->doallDepth--;
					else if (DoAll(i, i->doallLocno, false))
					{
						TRACE("\nPerforming next DoAll: process %d, entry %d, offset %d\n", i->doallProcess, i->doallEntry, i->doallOffset);						entry    = i->doallEntry;
						offset   = i->doallOffset;
						process  = i->doallProcess;
						SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);
						continue;
					}
					else
					{
						TRACE("\nDoAll finished\n");
					}
				}
				if (i->procstackptr == 0)
				{
					DDB_Flush(i);
					i->state = DDB_FINISHED;
					TRACE("\n");
					return;
				}
				i->procstackptr--;
				process  = i->procstack[i->procstackptr].process;
				entry    = i->procstack[i->procstackptr].entry;
				offset   = i->procstack[i->procstackptr].offset;
				SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);
				condact  = i->ddb->condactMap[*code & 0x7F].condact;
				params   = i->ddb->condactMap[*code & 0x7F].parameters;
				TRACE("\n\nResuming process %d, entry %d, offset %d\n\n", process, entry, offset);
				continue;

			case CONDACT_REDO:
				SetExecutionProcessStart(i, process, &entry, &offset, &entryPtr, &code);
				TRACE("\n");
				continue;

			case CONDACT_RESTART:
				i->procstackptr = 0;
				i->doall = false;
				process  = 0;
				SetExecutionProcessStart(i, process, &entry, &offset, &entryPtr, &code);
				TRACE("\n");
				continue;

			case CONDACT_SKIP:
			{
				// TODO: Check overflow (going pass last entry)
				int8_t increment = (int8_t)param0;
				entry++;
				if (entry + increment < 0)
					entry = 0;
				else
					entry += increment;
				offset = 0;
				SetExecutionPosition(i, process, entry, offset, &entryPtr, &code);
				TRACE("\n");
				continue;
			}

			case CONDACT_PAUSE:
				DDB_Flush(i);
				i->done = true;
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
				{
					DDB_PAWSStartPause(i, param0);
					SCR_GetMilliseconds(&i->pauseStart);
					UpdatePos(i, process, entry, offset + params + 1);
					TRACE("\n");
					return;
				}
				#endif
				if (param0 != 0 && SkipTimedPauses(i))
				{
					TRACE("[skipped]");
					break;
				}
				#if HAS_TESTMODE
				// Scripted input is deliberately queued for the pause itself.
				if (!DDB_TestHasScriptedInput())
				#endif
					while (SCR_AnyKey())
						SCR_GetKey(0, 0, 0);
				i->state = DDB_PAUSED;
				i->pauseFrames = param0 == 0 ? 65535 : param0;
				i->saveKeyToFlags = (param0 == 0);
				SCR_GetMilliseconds(&i->pauseStart);
				UpdatePos(i, process, entry, offset + params + 1);
				if (param0 == 0)
					DDB_ResetScrollCounts(i);
				TRACE("\n");
				return;

			case CONDACT_ANYKEY:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					DDB_PAWSOutputAnyKeyMessage(i);
				else
				#endif
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 16);
					DDB_Flush(i);
				}
				i->state = DDB_WAITING_FOR_KEY;
				UpdatePos(i, process, entry, offset + params + 1);
				// ANYKEY starts an independent wait.  In particular, PAW must not
				// inherit the TIME bit-0 timer used to acquire the preceding command.
				CancelTimeout(i);
				if (i->flags[Flag_TimeoutFlags] & Timeout_AnyKey)
				{
					StartTimeout(i);
				}
				i->flags[Flag_TimeoutFlags] &= ~Timeout_LastFrame;
				DDB_ResetScrollCounts(i);
				DDB_ResetSmoothScrollFlags(i);
				i->done = true;
				TRACE("\n");
				return;

			// Flag manipulation actions

			case CONDACT_SET:
				i->flags[param0] = 255;
				TRACE("Flag %d := 255", param0);
				i->done = true;
				break;
			case CONDACT_CLEAR:
				i->flags[param0] = 0;
				TRACE("Flag %d := 0", param0);
				i->done = true;
				break;
			case CONDACT_LET:
				i->flags[param0] = param1;
				TRACE("Flag %d := %d", param0, param1);
				i->done = true;
				break;
			case CONDACT_PLUS:
				if (param1 > 255 - i->flags[param0])
					i->flags[param0] = 255;
				else
					i->flags[param0] += param1;
				TRACE("Flag %d := %d", param0, i->flags[param0]);
				i->done = true;
				break;
			case CONDACT_MINUS:
				if (param1 > i->flags[param0])
					i->flags[param0] = 0;
				else
					i->flags[param0] -= param1;
				TRACE("Flag %d := %d", param0, i->flags[param0]);
				i->done = true;
				break;
			case CONDACT_ADD:
				if (i->flags[param0] > 255 - i->flags[param1])
					i->flags[param1] = 255;
				else
					i->flags[param1] += i->flags[param0];
				TRACE("Flag %d := %d", param1, i->flags[param1]);
				i->done = true;
				break;
			case CONDACT_SUB:
				if (i->flags[param0] > i->flags[param1])
					i->flags[param1] = 0;
				else
					i->flags[param1] -= i->flags[param0];
				TRACE("Flag %d := %d", param1, i->flags[param1]);
				i->done = true;
				break;
			case CONDACT_COPYFF:
				i->flags[param1] = i->flags[param0];
				TRACE("Flag %d := Flag %d (%d)", param1, param0, i->flags[param0]);
				i->done = true;
				break;
			case CONDACT_COPYBF:
				i->flags[param0] = i->flags[param1];
				TRACE("Flag %d := Flag %d (%d)", param0, param1, i->flags[param1]);
				i->done = true;
				break;
			case CONDACT_SYNONYM:
				if (param0 != 255)
					i->flags[Flag_Verb] = param0;
				if (param1 != 255)
					i->flags[Flag_Noun1] = param1;
				if (param0 == 255)
					TRACE("_    ");
				else
					TraceVocabularyWord(i->ddb, WordType_Verb, param0);
				TRACE(" ");
				if (param1 == 255)
					TRACE("_    ");
				else
					TraceVocabularyWord(i->ddb, WordType_Noun, param1);
				if (i->ddb->version < DDB_VERSION_3 && !DDB_IsAmigaOrAtari(i->ddb->machine))
					i->done = true;
				break;

			// Objects

			case CONDACT_COPYOF:
				if (param0 < i->ddb->numObjects)
					i->flags[param1] = i->objloc[param0];
				else
					i->flags[param1] = 255;
				TRACE("Flag %d := %d", param1, i->flags[param1]);
				i->done = true;
				break;
			case CONDACT_COPYOO:
				SetObjno(i, param0);
				if (param0 < i->ddb->numObjects &&
				    param1 < i->ddb->numObjects)
				{
					i->objloc[param1] = i->objloc[param0];
					SetObjno(i, param1);
				}
				i->done = true;
				break;
			case CONDACT_COPYFO:
				if (param1 < i->ddb->numObjects)
				{
					if (i->objloc[param1] == Loc_Carried && i->flags[Flag_NumCarried] > 0)
						i->flags[Flag_NumCarried]--;
					i->objloc[param1] = i->flags[param0];
					if (i->objloc[param1] == Loc_Carried && i->flags[Flag_NumCarried] < 255)
						i->flags[Flag_NumCarried]++;
				}
				i->done = true;
				break;
			case CONDACT_WHATO:
				SetObjno(i, Whato(i));
				TRACE("Obj#%d (at %d, weight %d)", i->flags[Flag_Objno], i->flags[Flag_ObjLocno], i->flags[Flag_ObjWeight]);
				i->done = true;
				break;
			case CONDACT_SETCO:
				SetObjno(i, param0);
				TRACE("Obj#%d (at %d, weight %d)", i->flags[Flag_Objno], i->flags[Flag_ObjLocno], i->flags[Flag_ObjWeight]);
				i->done = true;
				break;
			case CONDACT_CREATE:
				SetObjno(i, param0);
				if (param0 < i->ddb->numObjects)
				{
					if (i->objloc[param0] == Loc_Carried && i->flags[Flag_NumCarried] > 0 && i->flags[Flag_NumCarried] > 0)
						i->flags[Flag_NumCarried]--;
					i->objloc[param0] = i->flags[Flag_Locno];
					if (i->objloc[param0] == Loc_Carried)
						i->flags[Flag_NumCarried]++;
				}
				i->done = true;
				break;
			case CONDACT_DESTROY:
				SetObjno(i, param0);
				if (param0 < i->ddb->numObjects)
				{
					if (i->objloc[param0] == Loc_Carried && i->flags[Flag_NumCarried] > 0)
						i->flags[Flag_NumCarried]--;
					i->objloc[param0] = 252;
				}
				i->done = true;
				break;
			case CONDACT_SWAP:
				SetObjno(i, param1);
				if (param0 < i->ddb->numObjects && param1 < i->ddb->numObjects)
				{
					uint8_t tmp = i->objloc[param0];
					i->objloc[param0] = i->objloc[param1];
					i->objloc[param1] = tmp;
				}
				i->done = true;
				break;
			case CONDACT_PLACE:
				SetObjno(i, param0);
				if (param0 < i->ddb->numObjects)
				{
					SetObjno(i, param0);
					if (i->objloc[param0] == Loc_Carried && i->flags[Flag_NumCarried] > 0)
						i->flags[Flag_NumCarried]--;
					if (param1 == 255)
						param1 = i->flags[Flag_Locno];
					i->objloc[param0] = param1;
					if (i->objloc[param0] == Loc_Carried)
						i->flags[Flag_NumCarried]++;
				}
				i->done = true;
				break;

			// Specials

			case CONDACT_WEIGH:
				i->flags[param1] = CalculateWeight(i, param0, 0);
				i->done = true;
				break;
			case CONDACT_WEIGHT:
				i->flags[param0] = CalculateCarriedWeight(i);
				i->done = true;
				break;
			case CONDACT_RESET:
				if (i->ddb->version < 2)
				{
					for (int n = 0; n < i->ddb->numObjects; n++)
					{
						if (i->objloc[n] == i->flags[Flag_Locno])
							i->objloc[n] = param0;
						else if (i->objloc[n] != Loc_Carried && i->objloc[n] != Loc_Worn)
							i->objloc[n] = i->ddb->objLocTable[n];
					}
					goto case_DESC;
				}
				else
				{
					i->flags[Flag_NumCarried] = 0;
					for (int n = 0; n < i->ddb->numObjects; n++)
					{
						i->objloc[n] = i->ddb->objLocTable[n];
						if (i->objloc[n] == Loc_Carried)
							i->flags[Flag_NumCarried]++;
					}
					i->done = true;
				}
				break;
			case CONDACT_ABILITY:
				i->flags[Flag_MaxCarried] = param0;
				i->flags[Flag_Strength]   = param1;
				i->done = true;
				break;
			case CONDACT_GOTO:
				i->flags[Flag_Locno] = param0;
				i->done = true;
				break;
			case CONDACT_TIME:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					DDB_PAWSSetTime(i, param0, param1);
				else
				#endif
				{
					i->flags[Flag_Timeout] = param0;
					i->flags[Flag_TimeoutFlags] = param1;
				}
				i->done = true;
				break;
			case CONDACT_CHANCE:
			{
				uint32_t value;
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
				{
					value = DDB_PAWSRandom(i);
					ok = value <= param0;
				}
				else
				#endif
				{
					value = RandInt(0, 100);
					ok = value < param0;
				}
				#if HAS_TESTMODE
				// A scripted override replaces the outcome but the roll above
				// still consumed its random number, so the stream stays
				// aligned with an unforced run
				if (DDB_TestChanceForced(&ok, (*code & 0x80) != 0, code[1]))
				{
					TRACE("%sCHANCE %u: [forced by script]\n", ok ? "" : "[Failed] ", param0);
					break;
				}
				#endif
				TRACE("%sCHANCE %u: %u\n", ok ? "" : "[Failed] ", param0, value);
				break;
			}
			case CONDACT_RANDOM:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					i->flags[param0] = DDB_PAWSRandom(i);
				else
				#endif
					i->flags[param0] = RandInt(0, 100) + 1;
				TRACE("RANDOM %u: %u\n", param0, i->flags[param0]);
				i->done = true;
				break;

			// Parser

			case CONDACT_PARSE:
				if (i->ddb->version > 1 && param0 != 1)
				{
					// In later version, perform INPUT here when no input is available
					while (i->inputBufferPtr < i->inputBufferLength && i->inputBuffer[i->inputBufferPtr] == ' ')
						i->inputBufferPtr++;
					if (i->inputBufferPtr == i->inputBufferLength)
					{
						DDB_StartInput(i, true);
						i->oldMainLoopState = FLOW_INPUT;
						i->state = DDB_INPUT;
						if (i->flags[Flag_TimeoutFlags] & Timeout_Input)
						{
							StartTimeout(i);
						}
						i->flags[Flag_TimeoutFlags] &= ~Timeout_LastFrame;
						DDB_PrintInputLine(i, true);
						UpdatePos(i, process, entry, offset);
						TRACE("\n");
						return;
					}
				}
				ok = !Parse(i, i->ddb->version < 2 || param0 == 1);
				if (ok) DDB_NewText(i);
				break;

			// Low level object management

			case CONDACT_DROPALL:
				for (int n = 0; n < i->ddb->numObjects; n++)
				{
					if (i->objloc[n] == Loc_Carried)
						i->objloc[n] = i->flags[Flag_Locno];
				}
				i->flags[Flag_NumCarried] = 0;
				i->done = true;
				break;

			case CONDACT_PUTO:
				if (i->flags[Flag_Objno] < i->ddb->numObjects)
				{
					if (param0 == 255)
						param0 = i->flags[Flag_Locno];
					if (i->objloc[i->flags[Flag_Objno]] == Loc_Carried && i->flags[Flag_NumCarried] > 0)
						i->flags[Flag_NumCarried]--;
					i->objloc[i->flags[Flag_Objno]] = param0;
					if (i->objloc[i->flags[Flag_Objno]] == Loc_Carried)
						i->flags[Flag_NumCarried]++;
				}
				i->done = true;
				break;

			// Movement

			case CONDACT_MOVE:
				ok = MovePlayer(i, param0);
				break;

			// Semi-automatic object management
			// TODO: Refactor into separate functions, lots of code duplication here

			case CONDACT_AUTOP:
				param1 = param0;
				param0 = WhatoAt(i, Loc_Carried, Loc_Worn, i->flags[Flag_Locno]);
				if (param0 == 255)
				{
					AutoFailed(i);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_PUTIN:
				SetObjno(i, param0);
				locno = param0 < i->ddb->numObjects ? i->objloc[param0] : 252;
				if (locno == Loc_Worn)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 24);		// I can't, I'm wearing _.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno == i->flags[Flag_Locno])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 49);		// I don't have _.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != Loc_Carried)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 28);		// I can't see _ around
					DDB_NewText(i);
					goto case_DONE;
				}
				i->objloc[param0] = param1;
				if (i->flags[Flag_NumCarried] > 0)
					i->flags[Flag_NumCarried]--;
				DDB_OutputMessage(i, DDB_SYSMSG, 44);			// _ is in
				SetObjno(i, param1);
				DDB_OutputText(i, "_");
				SetObjno(i, param0);
				DDB_OutputMessage(i, DDB_SYSMSG, 51);			// .
				i->done = true;
				break;

			case CONDACT_AUTOT:
				param1 = param0;
				param0 = WhatoAt(i, param1, Loc_Carried, Loc_Worn);
				if (param0 == 255)
					param0 = WhatoAt(i, i->flags[Flag_Locno]);
				if (param0 == 255)
				{
					AutoFailed(i);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_TAKEOUT:
					SetObjno(i, param0);
					locno = param0 < i->ddb->numObjects ? i->objloc[param0] : 252;
				if (locno == Loc_Worn || locno == Loc_Carried)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 25);		// I already have _
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno == i->flags[Flag_Locno])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 45);		// _ is not in
					DDB_OutputMessage(i, DDB_OBJNAME, param1);
					DDB_OutputMessage(i, DDB_SYSMSG, 51);		// .
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != param1 || param0 >= i->ddb->numObjects)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 52);		// I can't see _ in
					DDB_OutputMessage(i, DDB_OBJNAME, param1);
					DDB_OutputMessage(i, DDB_SYSMSG, 51);		// .
					DDB_NewText(i);
					goto case_DONE;
				}
				value = CalculateWeight(i, param0, 0);
				if (value + CalculateCarriedWeight(i) > i->flags[Flag_Strength])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 43);		// I can't carry any more weight.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (i->flags[Flag_NumCarried] >= i->flags[Flag_MaxCarried])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 27);		// I can't carry any more.
					DDB_NewText(i);
					i->doall = false;
					goto case_DONE;
				}
				i->objloc[param0] = Loc_Carried;
				i->flags[Flag_ObjLocno] = Loc_Carried;
				i->flags[Flag_NumCarried]++;
				DDB_OutputMessage(i, DDB_SYSMSG, 36);			// _ is in
				i->done = true;
				break;

			case CONDACT_AUTOD:
				param0 = WhatoAt(i, Loc_Carried, Loc_Worn, i->flags[Flag_Locno]);
				if (param0 == 255)
				{
					AutoFailed(i);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_DROP:
				SetObjno(i, param0);
				if (param0 >= i->ddb->numObjects)
				{
					ok = false;
					break;
				}
				locno = i->objloc[param0];
				if (locno == Loc_Worn)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 24);		// I can't, I'm wearing _.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno == i->flags[Flag_Locno])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 49);		// I don't have _.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != Loc_Carried)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 28);		// I don't have that.
					DDB_NewText(i);
					goto case_DONE;
				}
				i->objloc[param0] = i->flags[Flag_Locno];
				i->flags[Flag_ObjLocno] = i->flags[Flag_Locno];
				if (i->flags[Flag_NumCarried] > 0)
					i->flags[Flag_NumCarried]--;
				DDB_OutputMessage(i, DDB_SYSMSG, 39);			// I dropped _
				i->done = true;
				break;

			case CONDACT_AUTOG:
				param0 = WhatoAt(i, i->flags[Flag_Locno], Loc_Carried, Loc_Worn);
				if (param0 == 255)
				{
					AutoFailed(i, 26);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_GET:
				SetObjno(i, param0);
				if (param0 >= i->ddb->numObjects)
				{
					ok = false;
					break;
				}
				locno = i->objloc[param0];
				if (locno == Loc_Worn || locno == Loc_Carried)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 25);		// I already have _
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != i->flags[Flag_Locno])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 26);		// I can't see that around
					DDB_NewText(i);
					goto case_DONE;
				}
				value = CalculateWeight(i, param0, 0);
				if (value + CalculateCarriedWeight(i) > i->flags[Flag_Strength])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 43);		// I can't carry any more weight.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (i->flags[Flag_NumCarried] >= i->flags[Flag_MaxCarried])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 27);		// I can't carry any more.
					DDB_NewText(i);
					i->doall = false;
					goto case_DONE;
				}
				i->objloc[param0] = Loc_Carried;
				i->flags[Flag_ObjLocno] = Loc_Carried;
				i->flags[Flag_NumCarried]++;
				DDB_OutputMessage(i, DDB_SYSMSG, 36);			// I now have _
				i->done = true;
				break;

			case CONDACT_AUTOW:
				param0 = WhatoAt(i, Loc_Carried, Loc_Worn, i->flags[Flag_Locno]);
				if (param0 == 255)
				{
					AutoFailed(i, 26);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_WEAR:
				SetObjno(i, param0);
				if (param0 >= i->ddb->numObjects)
				{
					ok = false;
					break;
				}
				locno = i->objloc[param0];
				if (locno == Loc_Worn)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 29);		// I'm already wearing _
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != Loc_Carried)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 28);		// I don't have that.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (!(i->ddb->objAttrTable[param0] & Obj_Wearable))
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 40);		// I can't wear that.
					DDB_NewText(i);
					goto case_DONE;
				}
				i->objloc[param0] = Loc_Worn;
				i->flags[Flag_ObjLocno] = Loc_Worn;
				if (i->flags[Flag_NumCarried] > 0)
					i->flags[Flag_NumCarried]--;
				DDB_OutputMessage(i, DDB_SYSMSG, 37);		// I'm now wearing _
				i->done = true;
				break;

			case CONDACT_AUTOR:
				param0 = WhatoAt(i, Loc_Worn, Loc_Carried, i->flags[Flag_Locno]);
				if (param0 == 255)
				{
					AutoFailed(i, 26);
					goto case_DONE;
				}
				// Fall through
			case CONDACT_REMOVE:
				SetObjno(i, param0);
				if (param0 >= i->ddb->numObjects)
				{
					ok = false;
					break;
				}
				locno = i->objloc[param0];
				if (locno == Loc_Carried || locno == i->flags[Flag_Locno])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 50);		// I'm not wearing _
					DDB_NewText(i);
					goto case_DONE;
				}
				if (locno != Loc_Worn)
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 23);		// I am not wearing that.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (!(i->ddb->objAttrTable[param0] & Obj_Wearable))
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 41);		// I can't remove that.
					DDB_NewText(i);
					goto case_DONE;
				}
				if (i->flags[Flag_NumCarried] >= i->flags[Flag_MaxCarried])
				{
					DDB_OutputMessage(i, DDB_SYSMSG, 42);		// I can't carry any more.
					DDB_NewText(i);
					i->doall = false;
					goto case_DONE;
				}
				i->objloc[param0] = Loc_Carried;
				i->flags[Flag_ObjLocno] = Loc_Carried;
				i->flags[Flag_NumCarried]++;
				DDB_OutputMessage(i, DDB_SYSMSG, 38);		// I'm no longer wearing _
				i->done = true;
				break;

			case CONDACT_DOALL:
				i->doallProcess = process;
				i->doallEntry = entry;
				i->doallOffset = offset + params + 1;
				UpdatePos(i, process, entry, offset + params + 1);
				if (!DoAll(i, param0, true))
				{
					DDB_NewText(i);
					goto case_NOTDONE;
				}
				break;

			// Graphics and window management condacts

			case CONDACT_WINDOW:
				DDB_Flush(i);
				DDB_SetWindow(i, param0);
				repeatingDisplay = false;
				i->done = true;
				TRACE("Window %d: at %d,%d %dx%d", i->curwin, i->win.x, i->win.y, i->win.width, i->win.height);
				break;
			case CONDACT_WINAT:
				DDB_Flush(i);
				WinAt(i, param0, param1);
				i->done = true;
				TRACE("Window %d: at %d,%d %dx%d", i->curwin, i->win.x, i->win.y, i->win.width, i->win.height);
				break;
			case CONDACT_WINSIZE:
				DDB_Flush(i);
				WinSize(i, param0, param1);
				i->done = true;
				TRACE("Window %d: at %d,%d %dx%d", i->curwin, i->win.x, i->win.y, i->win.width, i->win.height);
				break;
			case CONDACT_LINE:
				#if HAS_PAWS
				DDB_PAWSSetLine(i, param0);
				#endif
				i->done = true;
				break;
			case CONDACT_PROTECT:
				#if HAS_PAWS
				DDB_PAWSProtect(i);
				#endif
				i->done = true;
				break;
			case CONDACT_CLS:
				DDB_Flush(i);
				i->done = true;
				if (!SkipTimedPauses(i) && AnyWindowOverlapsCurrent(i))
                {
					// fprintf(stderr, "WARNING: Overlap detected in CLS in process %d, entry %d, offset %d\n", process, entry, offset);
					DDB_Flush(i);
					i->state = DDB_VSYNC;
					UpdatePos(i, process, entry, offset);
					TRACE("\n");
					return;
                }
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
				{
					DDB_PAWSClear(i);
					break;
				}
				#endif
				DDB_ClearWindow(i, &i->win);
				break;
			case CONDACT_PICTURE:
				if (i->ddb->version < 2)
				{
					if (i->ddb->drawString)
					{
						#if HAS_DRAWSTRING
						SCR_DrawVectorPicture(param0);
						#if HAS_PAWS
						if (!DDB_IsPAWS(i->ddb->version))
						#endif
							AdoptVectorPictureWindow(i, param0);
						#endif

						// TODO: Did Original do this? Check
						if (i->ddb->version != DDB_VERSION_PAWS)
							i->flags[40] = 255;
					}
					else if (BufferPicture(i, param0))
						DrawBufferedPicture(i);
				}
				else
				{
					if (i->ddb->drawString)
					{
						#if HAS_DRAWSTRING
						// The original interpreters update the current picture
						// before validating the windef flag, so a failing
						// PICTURE still changes what a later DISPLAY draws
						// (Espacial 3 relies on this for its portraits)
						ok = DDB_HasVectorPicture(param0);
						if (DDB_VectorPictureInRange(param0))
							i->currentPicture = param0;
						#endif
					}
					else
					{
						ok = BufferPicture(i, param0);
						if (ok == false && SCR_SampleExists(param0))
						{
							ok = true;
							i->currentPicture = param0;
						}
						TRACE("(Picture %s)", ok ? "found" : "not found");
					}
					repeatingDisplay = false;
				}
				break;
			case CONDACT_DISPLAY:
				if (param0 == 0)
				{
					if (repeatingDisplay)
					{
						if (!SkipTimedPauses(i))
						{
							i->state = DDB_VSYNC;
							UpdatePos(i, process, entry, offset);
							TRACE("\n");
							return;
						}
						repeatingDisplay = false;
					}
					if (i->ddb->drawString)
					{
						#if HAS_DRAWSTRING
						SCR_DrawVectorPicture(i->currentPicture);
						AdoptVectorPictureWindow(i, i->currentPicture);
						#endif
					}
					else
					{
						DrawBufferedPicture(i);
					}
					repeatingDisplay = true;
				}
				else
				{
					DDB_ClearWindow(i, &i->win);
				}
				MarkWindowOutput();
				i->done = true;
				break;
			case CONDACT_MODE:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					DDB_PAWSSetMode(i, param0, param1);
				else
				#endif
					i->win.flags = param0;
				if (i->ddb->version == DDB_VERSION_1)
					i->flags[40] = param0;
				i->done = true;
				break;
			case CONDACT_GRAPHIC:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					DDB_PAWSSetGraphic(i, param0);
				else
				#endif
				{
					i->flags[Flag_GraphicFlags] &= 0x87;
					i->flags[Flag_GraphicFlags] |= ((param0 << 5) | (param1 << 3)) & 0x78;
				}
				i->done = true;
				break;
			case CONDACT_CHARSET:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
				{
					DDB_Flush(i);
					DDB_ResetPAWSColors(i, &i->win);
				}
				#endif
				DDB_SetCharset(i->ddb, param0);
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version) && param0 <= i->ddb->numCharsets)
					i->pawsPermanentCharset = param0;
				#endif
				i->done = true;
				break;
			case CONDACT_GFX:
				switch (param1)
				{
					case 0:			// copy backbuffer --> physical screen
						SCR_RestoreScreen();
						break;
					case 1:			// copy physical screen --> backbuffer
						SCR_SaveScreen();
						break;
					case 2:			// swap physical screen <-> backbuffer
						SCR_SwapScreen();
						break;
					case 3:			// set picture output to physical screen
						SCR_SetOpBuffer(SCR_OP_DRAWPICTURE, true);
						break;
					case 4:			// set picture output to backbuffer
						SCR_SetOpBuffer(SCR_OP_DRAWPICTURE, false);
						break;
					case 5:			// clear physical screen
						SCR_ClearBuffer(true);
						break;
					case 6:			// clear backbuffer
						SCR_ClearBuffer(false);
						break;
					case 7:			// set text output to physical screen
						SCR_SetOpBuffer(SCR_OP_DRAWTEXT, true);
						break;
					case 8:			// set text output to backbuffer
						SCR_SetOpBuffer(SCR_OP_DRAWTEXT, false);
						break;
					case 9:			// set palette color (param0: index of a 4-flag buffer with color#,R,G,B)
						if (paletteChanges & (1 << i->flags[param0]))
						{
							if (!SkipTimedPauses(i))
							{
								// If this color was already changed this frame, wait.
								TRACE("\n\nPalette color %d already changed this frame, waiting\n", i->flags[param0]);
								DDB_Flush(i);
								i->state = DDB_VSYNC;
								UpdatePos(i, process, entry, offset);
								TRACE("\n");
								return;
							}
							paletteChanges = 0;
						}
						TRACE("\nSetting palette color %d to %d,%d,%d\n", i->flags[param0], i->flags[param0+1], i->flags[param0+2], i->flags[param0+3]);
						SCR_SetPaletteColor(i->flags[param0], i->flags[param0+1], i->flags[param0+2], i->flags[param0+3]);
						paletteChanges |= 1 << i->flags[param0];
						break;
					case 10:		// get palette color (param0: index of a 4-flag buffer with color#,R,G,B)
						SCR_GetPaletteColor(i->flags[param0], &i->flags[param0+1], &i->flags[param0+2], &i->flags[param0+3]);
						break;
				}
				i->done = true;
				break;
			case CONDACT_INKEY:
				// HACK: prevent 'more' to be shown in Espacial
				// TODO: Check which condact resets the 'more...' counter
				DDB_ResetScrollCounts(i);
				DDB_ResetSmoothScrollFlags(i);

				// Temporary hack to make debug more manageable
				if (PAUSE_ON_INKEY)
				{
					DDB_Flush(i);
					i->state = DDB_WAITING_FOR_KEY;
					i->saveKeyToFlags = true;
					UpdatePos(i, process, entry, offset + params + 1);
					TRACE("\n");
					return;
				}

				// This is a convoluted mess because games will perform several INKEY in sequence
				// and expect the same key to be returned, but we need some way to stop sending the
				// same key back for things like menus. The way it's implemented right now is that
				// INKEY status is reset by a DDB_Flush command (essentially, printing text).
				//
				// In addition, checking for a key requires waiting a frame since the system
				// is event driven.
				if (i->keyChecked && i->keyPressed && i->keyReuseCount < 16)
				{
					ok = true;
					i->flags[Flag_Key1] = i->lastKey1;
					i->flags[Flag_Key2] = i->lastKey2;
					i->keyReuseCount++;
				}
				else
				{
					if (i->keyCheckInProgress)
					{
						ok = SCR_AnyKey();
						if (ok) {
							SCR_GetKey(&i->flags[Flag_Key1], &i->flags[Flag_Key2], 0);
						}

						i->keyCheckInProgress = false;
						i->keyChecked = true;
						i->keyPressed = ok;
						i->lastKey1 = i->flags[Flag_Key1];
						i->lastKey2 = i->flags[Flag_Key2];
						i->keyReuseCount = 0;
					}
					else
					{
						// Synchronize and wait one frame

						DDB_Flush(i);
						i->keyCheckInProgress = true;
						i->state = DDB_CHECKING_KEY;
						UpdatePos(i, process, entry, offset);
						TRACE("\n");
						return;
					}
				}
				break;
			case CONDACT_SCORE:
				DDB_OutputMessage(i, DDB_SYSMSG, 21);		// Your score is
				LongToChar(i->flags[Flag_Score], output, 10);
				DDB_OutputText(i, output);
				DDB_OutputMessage(i, DDB_SYSMSG, 22);		// %
				i->done = true;
				break;
			case CONDACT_TURNS:
			{
				uint16_t turns;
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					turns = i->flags[Flag_Turns] + 256 * i->flags[Flag_Turns + 1];
				else
				#endif
					turns = i->flags[Flag_Turns+1] + 256 * i->flags[Flag_Turns];
				DDB_OutputMessage(i, DDB_SYSMSG, 17);		// You have taken
				LongToChar(turns, output, 10);
				DDB_OutputText(i, output);
				DDB_OutputMessage(i, DDB_SYSMSG, 18);		// turn
				if (turns != 1)
					DDB_OutputMessage(i, DDB_SYSMSG, 19);	// s
				DDB_OutputMessage(i, DDB_SYSMSG, 20);		// so far.
				i->done = true;
				break;
			}
			case CONDACT_PROMPT:
				i->flags[Flag_Prompt] = param0;
				i->done = true;
				break;
			case CONDACT_PAPER:
				DDB_Flush(i);
				DDB_ResetPAWSColors(i, &i->win);
				// Transparent paper is not supported in the original
				i->win.paper = param0 == 255 ? 255 : i->inkMap[param0 & 0x0F];
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					i->pawsPermanentPaper = i->win.paper;
				#endif
				i->done = true;
				break;
			case CONDACT_INK:
				DDB_Flush(i);
				DDB_ResetPAWSColors(i, &i->win);
				i->win.ink = i->inkMap[param0 & 0x0F];
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					i->pawsPermanentInk = i->win.ink;
				#endif
				TRACE("Ink %02d (from %02d)", i->win.ink, param0);
				i->done = true;
				break;
			case CONDACT_TIMEOUT:
				ok = (i->flags[Flag_TimeoutFlags] & Timeout_LastFrame) != 0;
				break;

			// Save states

			case CONDACT_SAVE:
				if (supportsOpenFileDialog)
				{
					if (ResolveFileDialogInput(i, false))
					{
						UpdatePos(i, process, entry, offset + params + 1);
						TRACE("\n");
						return;
					}
					goto case_NOTDONE;
				}
				else
				{
					StartFileNameInputState(i, DDB_INPUT_SAVE);
					UpdatePos(i, process, entry, offset + params + 1);
					TRACE("\n");
					return;
				}
				break;

			case CONDACT_LOAD:
				if (supportsOpenFileDialog)
				{
					if (ResolveFileDialogInput(i, true))
					{
						UpdatePos(i, process, entry, offset + params + 1);
						TRACE("\n");
						return;
					}
					goto case_NOTDONE;
				}
				else
				{
					StartFileNameInputState(i, DDB_INPUT_LOAD);
					UpdatePos(i, process, entry, offset + params + 1);
					TRACE("\n");
					return;
				}
				break;

			case CONDACT_RAMSAVE:
				MemCopy(i->ramSaveArea, i->buffer, i->saveStateSize);
				i->ramSaveAvailable = true;
				break;

			case CONDACT_RAMLOAD:
				if (i->ramSaveAvailable)
				{
					// Flags are required to be first in buffer
					int flagCount = param0 + 1;
					MemCopy(i->buffer, i->ramSaveArea, flagCount);
					MemCopy(i->buffer + 256, i->ramSaveArea + 256, i->saveStateSize - 256);
				}
				else
				{
					ok = false;
				}
				break;


			case CONDACT_SAVEAT:
				DDB_Flush(i);
				i->win.saveX = i->win.posX;
				i->win.saveY = i->win.posY;
				i->done = true;
				break;

			case CONDACT_BACKAT:
				DDB_Flush(i);
				i->win.posX = i->win.saveX;
				i->win.posY = i->win.saveY;
				i->done = true;
				break;

			case CONDACT_SFX:
				if (param1 == 255) {
					int durationMs = 0;
					SCR_PlaySample(i->currentPicture, &durationMs);

					DDB_Flush(i);
					if (SkipTimedPauses(i))
					{
						// The sample keeps playing, but the interpreter
						// does not wait for it
						TRACE("[skipped]");
						i->done = true;
						break;
					}
					i->state = DDB_PAUSED;
					SCR_GetMilliseconds(&i->pauseStart);
					UpdatePos(i, process, entry, offset + params + 1);
					i->pauseFrames = durationMs / 15;
					TRACE("\n");
					return;
				} else {
					// This enables keyboard click (bit 0) and key repeat (bit 1) in original interpreter
				}
				i->done = true;
				break;

			case CONDACT_CENTRE:
				DDB_Flush(i);
				CenterWindow(i, &i->win);
				i->windef[i->curwin].x = i->win.x;
				i->windef[i->curwin].width = i->win.width;
				i->windef[i->curwin].posX = i->win.posX;
				i->done = true;
				break;

			case CONDACT_XMESSAGE:
			#if HAS_XMSG
				if (param1 == 3 && params == 3)
				{
					uint16_t offset = param0 + code[3]*256;
					const uint8_t* msg = DDB_GetXMessage(offset);
					if (msg != 0)
					{
						TRACE("XMSG %d: \"", offset);
						OutputMessageContents(i, msg, &i->win);
						TRACE("\"");
						break;
					}
					else
					{
						DebugPrintf("[Invalid XMessage %d: %s]", offset, DDB_GetErrorString());
					}
				}
				else if (params == 3)
				{
					// This is actually an EXTERN, so no third byte
					params = 2;
				}
				else if (xmsgFilePresent)
				{
					uint16_t offset = param0 + param1*256;
					const uint8_t* msg = DDB_GetXMessage(offset);
					if (msg != 0)
					{
						TRACE("XMSG %d: \"", offset);
						OutputMessageContents(i, msg, &i->win);
						TRACE("\"");
						break;
					}
					else
					{
						DebugPrintf("[Invalid XMessage %d: %s]", offset, DDB_GetErrorString());
					}
				}
			#endif
				i->done = true;
				break;

			case CONDACT_EXTERN:
				#if HAS_PCX && HAS_SPECTRUM
				if (param1 == 0 &&
					i->ddb->target == DDB_MACHINE_SPECTRUM &&
					VID_HasExternalPictures())
				{
					DDB_Window savedWindow = i->win;
					uint8_t savedCellX = i->cellX;
					uint8_t savedCellW = i->cellW;
					if (BufferPicture(i, param0))
					{
						DrawBufferedPicture(i);
						i->win = savedWindow;
						i->cellX = savedCellX;
						i->cellW = savedCellW;
					}
					else
					{
						i->win = savedWindow;
						i->cellX = savedCellX;
						i->cellW = savedCellW;
					}
					i->done = true;
					break;
				}
				#endif

				#if HAS_PSG
				if (param1 == 0 && DDB_PlayExternalPSG(i->ddb, param0))
				{
					TRACE("[PSG stream %d played]", param0);
					i->done = true;
					break;
				}
				#endif

				#if HAS_SNAPSHOTS
				// This fixes Templos & Chichen, but it is hackish to say the least
				// Unfortunately, the only way to improve it is to add a full blown
				// Z80 CPU emulator to the interpreter, which is not going to happen

				// Gate on target, not ddb->machine: the latter is the *snapshot*
				// machine, only set when the DDB is loaded from a Z80 snapshot
				// (Spectrum/CPC/MSX). The PCW build ships as a CP/M disk image, so
				// its machine stays IBMPC while target is PCW — hence the data-init
				// EXTERN never fired and its block-puzzle flags were left unset.
				if (i->ddb->externData != 0 &&
					(i->ddb->target == DDB_MACHINE_SPECTRUM ||
					 i->ddb->target == DDB_MACHINE_CPC ||
					 i->ddb->target == DDB_MACHINE_MSX ||
					 i->ddb->target == DDB_MACHINE_PCW))
				{
					static uint8_t templosRoutine[] = {
						0xC5, 				// PUSH BC
						0xEB, 				// EX DE,HL
						0x01, 0x00, 0x00, 	// LD BC,address
						0x21, 0x00, 0x00,	// LD HL,address
						0xED, 0xB0,			// LDIR
						0xC1, 				// POP BC
						0xC9,				// RET
						0xFF
					};

					// Try to detect Templos/Chichen data init routines
					uint8_t* ptr = i->ddb->externData;
					bool matchesTemplos = true;
					for (int n = 0; templosRoutine[n] != 0xFF; n++)
					{
						if (templosRoutine[n] != 0 && ptr[n] != templosRoutine[n])
						{
							matchesTemplos = false;
							break;
						}
					}
					if (matchesTemplos)
					{
						uint16_t length  = read16(ptr + 3, i->ddb->littleEndian);
						uint32_t address = 0;
						if (!DDB_DecodeStoredOffset(i->ddb, read16(ptr + 6, i->ddb->littleEndian), i->ddb->dataSize, false, &address))
							break;
						uint8_t* data    = i->ddb->data;
						if (address < i->ddb->dataSize && address + length <= i->ddb->dataSize && length < 256 - param0)
							MemCopy(i->flags + param0, data + address, length);
					}
				}
				
				#endif
				i->done = true;
				break;

			case CONDACT_INDIR:
				// TODO: Check process size to prevent out of bounds access
				// However, this is what an actual real interpreter would do, so...
				code[4] = i->flags[param0];
				break;

			// TODO

			case CONDACT_CALL:
			case CONDACT_BEEP:
			case CONDACT_MOUSE:
				i->done = true;
				break;
			case CONDACT_BORDER:
				#if HAS_PAWS
				if (DDB_IsPAWS(i->ddb->version))
					i->pawsBorder = param0 & 7;
				#endif
				i->done = true;
				break;

			default:
				DDB_Flush(i);
				#if TRACE_ON
				TRACE("Condact %s not implemented (process %d, entry %d, offset %d)\n", DDB_GetCondactName((DDB_Condact)condact), process, entry, offset);
				#endif
				i->state = DDB_FATAL_ERROR;
				TRACE("\n");
				return;
		}
		TRACE("\n");

		if (finished)
			break;
		if (!ok)
		{
			entry++;
			offset = 0;
			entryPtr += 4;
			TRACE("\n");
		}
		else
		{
			offset += params + 1;
			code   += params + 1;
		}
		if (!SCR_Synchronized())
			break;
	}

	UpdatePos(i, process, entry, offset);
}

static void StepFunction(int elapsed)
{
	DDB_Interpreter* i = interpreter;

	if (SCR_Synchronized() == false)
	{
		if (waitingForKey)
		{
			if (!AnyKeyForMore(true))
			{
				if (i->timeout)
				{
					if (AdvanceTimeout(i, elapsed))
					{
						i->timeout = false;
						#if HAS_PAWS
						if (!DDB_IsPAWS(i->ddb->version))
						#endif
							i->flags[Flag_TimeoutFlags] |= Timeout_LastFrame;
						i->state = DDB_RUNNING;
						// A buffered More prompt owns waitingForKey rather than
						// DDB_WAITING_FOR_KEY.  Release it on timeout so the
						// clear and interrupted output queued behind the wait
						// can be consumed on the following step.
						waitingForKey = false;
					}
				}
				return;
			}
			waitingForKey = false;
			SCR_GetKey(0, 0, 0);
		}
		SCR_ConsumeBuffer();
		return;
	}

	switch (i->state)
	{
		case DDB_CHECKING_KEY:
			i->state = DDB_RUNNING;
			break;

		case DDB_VSYNC:
			i->state = DDB_RUNNING;
			break;

		case DDB_RUNNING:
			DDB_Step(i, 16384);
			break;

		case DDB_INPUT:
			if (i->timeout && i->inputBufferLength == 0)
			{
				if (AdvanceTimeout(i, elapsed))
				{
					i->timeout = false;
					i->flags[Flag_TimeoutFlags] |= Timeout_LastFrame;
					if (i->ddb->oldMainLoop)
						i->state = DDB_FINISHED;
					else
						i->state = DDB_RUNNING;
					DDB_PrintInputLine(i, false);
					DDB_FinishInput(i, true);
					break;
				}
			}
			// Fall through
		case DDB_INPUT_QUIT:
		case DDB_INPUT_END:
		case DDB_INPUT_LOAD:
		case DDB_INPUT_SAVE:
			DDB_ProcessInputFrame();
			break;

		case DDB_PAUSED:
		{
			#if HAS_PAWS
			if (DDB_IsPAWS(i->ddb->version))
			{
				uint32_t advance = elapsed > 0 ? (uint32_t)elapsed : 0;
				if (SkipTimedPauses(i))
					advance = (uint32_t)i->pauseFrames * DDB_PAWS_TICK_MS;
				if (DDB_PAWSAdvancePause(i, advance))
					i->state = DDB_RUNNING;
				break;
			}
			#endif
			uint32_t current = 0;
			SCR_GetMilliseconds(&current);

			// Only the infinite ("press any key") pause is a settled input point;
			// finite pauses drive animations, so defer scripted captures there.
			// While scripted input is feeding, finite pauses do not consume
			// keys at all: fast mode skips them outright, so consuming input
			// here would make a script's behavior depend on the FAST setting.
			#if HAS_TESTMODE
			if (i->pauseFrames != 65535 && DDB_TestHasScriptedInput())
			{
				if (i->pauseFrames < 0 || current - i->pauseStart >= (uint32_t)i->pauseFrames * 16)
					i->state = DDB_RUNNING;
				break;
			}
			#endif
			if (AnyKeyForMore(i->pauseFrames == 65535))
			{
				i->state = DDB_RUNNING;
				SCR_GetKey(&i->lastKey1, &i->lastKey2, 0);
				DDB_PlayClick(i, false);
				if (i->ddb->version > 1 && i->saveKeyToFlags)
				{
					i->flags[Flag_Key1] = i->lastKey1;
					i->flags[Flag_Key2] = i->lastKey2;
					i->saveKeyToFlags = false;
				}
			}
			else
			{
				if (i->pauseFrames < 0 || current - i->pauseStart >= (uint32_t)i->pauseFrames * 16)
					i->state = DDB_RUNNING;
			}
			break;
		}

		case DDB_WAITING_FOR_KEY:
							if (AnyKeyForMore())
			{
				SCR_GetKey(&i->lastKey1, &i->lastKey2, 0);
				DDB_PlayClick(i, true);
				if (i->ddb->version > 1 && i->saveKeyToFlags)
				{
					i->flags[Flag_Key1] = i->lastKey1;
					i->flags[Flag_Key2] = i->lastKey2;
					i->saveKeyToFlags = false;
				}
				i->state = DDB_RUNNING;
				i->timeout = false;
			}
			else if (i->timeout)
			{
				if (AdvanceTimeout(i, elapsed))
				{
					i->timeout = false;
					i->flags[Flag_TimeoutFlags] |= Timeout_LastFrame;
					i->state = DDB_RUNNING;
				}
			}
			// A failed LOAD showed "I/O Error" and waited for this key; once it is
			// acknowledged, do the original's system clear + GOTO 0 + RESTART (see
			// START2). Consumed on any exit from the wait so it never lingers.
			if (i->restartPending && i->state == DDB_RUNNING)
			{
				i->restartPending = false;
				DDB_Restart(i);
			}
			break;

		case DDB_FINISHED:
			if (i->ddb->oldMainLoop)
				HandleOldMainLoopFinished(i);
			break;

		case DDB_AUTOLOAD:
			// Break the main loop so the host player can reload the requested
			// part; the transcript is left open so the playthrough is continuous
			// across the part boundary.
			SCR_Quit();
			break;

		case DDB_QUIT:
		{
			uint32_t time = 0;
			SCR_GetMilliseconds(&time);
			if (time < i->quitStart + 500)
				break;
            if (transcriptFile)
            {
                File_Close(transcriptFile);
                transcriptFile = 0;
            }
			SCR_Quit();
			break;
		}

		case DDB_FATAL_ERROR:
            if (transcriptFile)
            {
                File_Close(transcriptFile);
                transcriptFile = 0;
            }
			SCR_Quit();
			break;
	}
}

void DDB_Run (DDB_Interpreter* i)
{
	SCR_MainLoop(i, StepFunction);
}

void DDB_SetAutoloadEnabled (DDB_Interpreter* i, bool enabled)
{
	if (i != 0)
		i->autoloadEnabled = enabled;
}

#if HAS_TESTMODE
void DDB_SetSkipTimedPauses (DDB_Interpreter* i, bool skip)
{
	if (i != 0)
		i->skipTimedPauses = skip;
}
#endif

DDB_Interpreter* DDB_CreateInterpreter (DDB* ddb, DDB_ScreenMode mode)
{
	DDB_Interpreter* i = Allocate<DDB_Interpreter>("DDB Interpreter");
	if (!i)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return 0;
	}

	if (interpreter == 0)
		interpreter = i;

	MemClear(i, sizeof(DDB_Interpreter));
	i->ddb = ddb;
	i->screenMode = mode;
	i->saveStateSize = 256 + ddb->numObjects;
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
		i->saveStateSize = DDB_PAWS_STATE_SIZE;
	#endif
	i->bufferSize = i->saveStateSize * 2;
	i->buffer = Allocate<uint8_t>("DDB Savestate", i->bufferSize);
	if (!i->buffer)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		Free(i);
		return 0;
	}
	MemClear(i->buffer, i->bufferSize);

	i->flags = i->buffer;
	i->objloc = i->buffer + 256;
	i->ramSaveArea = i->buffer + i->saveStateSize;
	i->keyClick = 2;
	#if HAS_PAWS
	if (DDB_IsPAWS(i->ddb->version))
		i->visited = i->buffer + DDB_PAWS_FLAG_BYTES + DDB_PAWS_OBJECT_BYTES;
	#endif

	DDB_Reset(i);
	DDB_ResetWindows(i);
	return i;
}

void DDB_CloseInterpreter (DDB_Interpreter* i)
{
	Free(i->buffer);
	Free(i);
}
