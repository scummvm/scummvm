#include <ddb_scr.h>
#include <ddb_vid.h>
#include <ddb_test.h>
#include <os_mem.h>
#include <os_file.h>

#if !defined(NO_BUFFERING)

#include <ddb_wintext.h>

static bool smoothScrolling = true;

#define COMMAND_BUFFER_BLOCKS

#define MAX_BLOCKS 		   256
#define COMMANDS_PER_BLOCK 256

SCR_CommandData  firstCommandBlock[COMMANDS_PER_BLOCK];
SCR_CommandData	*commandBufferBlocks[MAX_BLOCKS] = { firstCommandBlock };
size_t			 commandBufferIndex = 0;
size_t			 commandBufferReadIndex = 0;

static inline int IndexBlock (size_t index)
{
	return index / COMMANDS_PER_BLOCK;
}
static inline int IndexOffset (size_t index)
{
	return index % COMMANDS_PER_BLOCK;
}

void SCR_ConsumeFullBuffer()
{
	// A wait may be armed immediately when presentation is caught up, with
	// subsequent logic already buffering commands behind it. Service that wait
	// before consuming any of those later commands; otherwise a forced drain
	// displays post-keypress output before the keypress has happened.
	while(waitingForKey || commandBufferReadIndex != commandBufferIndex)
	{
		if (waitingForKey)
		{
			VID_WaitForKey();
			waitingForKey = false;
		}
		if (commandBufferReadIndex != commandBufferIndex)
			SCR_ConsumeBuffer();
	}
	commandBufferReadIndex = 0;
	commandBufferIndex = 0;
}

static SCR_CommandData* SCR_AddCommandToBuffer()
{
	if(commandBufferIndex == commandBufferReadIndex)
	{
		commandBufferReadIndex = 0;
		commandBufferIndex = 0;
	}

	int block = IndexBlock(commandBufferIndex);
	if (commandBufferBlocks[block] == 0)
	{
		// DebugPrintf("Allocating command buffer block %ld\n", (long)block);
		commandBufferBlocks[block] = Allocate<SCR_CommandData>("Command buffer", COMMANDS_PER_BLOCK);
		if (commandBufferBlocks[block] == 0)
		{
			// PANIC! Unable to allocate command
			SCR_ConsumeFullBuffer();
			block = IndexBlock(commandBufferIndex);
			if (commandBufferBlocks[block] == 0)
			{
				// This shouldn't happen
				return 0;}
		}
	}

	int offset = IndexOffset(commandBufferIndex++);
	return &commandBufferBlocks[block][offset];
}

void SCR_WaitForKey()
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_WAITFORKEY;
	}
	else
	{
		waitingForKey = true;
		buffering = true;
	}
}

void SCR_DrawCharacter(int x, int y, uint8_t ch, uint8_t ink, uint8_t paper)
{
	#if HAS_WINDOWTEXT
	WinText_AddSpan(x, y, &ch, 1, charWidth[ch]);
	#endif
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_DRAWCHARACTER;
		c->x = x;
		c->y = y;
		c->n = ch;
		c->ink = ink;
		c->paper = paper;
	}
	else
	{
		VID_DrawCharacter(x, y, ch, ink, paper);
	}
}

void SCR_DrawTextSpan(int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper)
{
	#if HAS_WINDOWTEXT
	{
		int pixelWidth = 0;
		for (uint16_t n = 0; n < length; n++)
			pixelWidth += charWidth[text[n]];
		WinText_AddSpan(x, y, text, length, pixelWidth);
	}
	#endif
	if (buffering)
	{
		for (uint16_t n = 0; n < length; n++)
		{
			SCR_DrawCharacter(x, y, text[n], ink, paper);
			x += charWidth[text[n]];
		}
	}
	else
	{
		VID_DrawTextSpan(x, y, text, length, ink, paper);
	}
}

#if HAS_DRAWSTRING
void SCR_DrawVectorPicture(uint8_t picno)
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_DRAWVECTORPICTURE;
		c->n = picno;
		return;
	}

	uint8_t attributes = VID_GetAttributes();
	DDB_ExecuteVectorPicture(picno);
	VID_SetAttributes(attributes);
}
#endif

bool SCR_SampleExists(uint8_t sample)
{
	return VID_SampleExists(sample);
}

bool SCR_LoadPicture(uint8_t picno, DDB_ScreenMode screenMode)
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_LOADPICTURE;
		c->n = picno;
		c->x = screenMode;
	}
	else
	{
		VID_LoadPicture(picno, screenMode);
		bool fixed;
		int16_t x, y, w, h;
		VID_GetPictureInfo(&fixed, &x, &y, &w, &h);
		return w > 0 && h > 0;
	}

	return VID_PictureExists(picno);
}

void SCR_DisplayPicture(int x, int y, int w, int h, DDB_ScreenMode mode)
{
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("SCR_DisplayPicture(x=%d, y=%d, w=%d, h=%d, mode=%u, buffering=%d)\n",
		x, y, w, h, (unsigned)mode, buffering ? 1 : 0);
	#endif
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_DISPLAYPICTURE;
		c->x = x;
		c->y = y;
		c->w = w;
		c->h = h;
		c->n = mode;
	}
	else
	{
		VID_DisplayPicture(x, y, w, h, mode);
	}
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("SCR_DisplayPicture: done\n");
	#endif
}

bool SCR_PictureExists(uint8_t picno)
{
	return VID_PictureExists(picno);
}

void SCR_GetPictureInfo(bool* fixed, int16_t* x, int16_t* y, int16_t* w, int16_t* h)
{
	VID_GetPictureInfo(fixed, x, y, w, h);
}

void SCR_Clear(int x, int y, int w, int h, uint8_t color, VID_ClearMode mode)
{
	#if HAS_WINDOWTEXT
	WinText_ClearRect(x, y, w, h);
	#endif
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_CLEAR;
		c->x = x;
		c->y = y;
		c->w = w;
		c->h = h;
		c->n = color;
		c->ink = mode;
	}
	else
	{
		VID_Clear(x, y, w, h, color, mode);
	}
}

void SCR_SetAttributes(uint8_t attributes)
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_SETATTRIBUTES;
		c->n = attributes;
	}
	else
	{
		VID_SetAttributes(attributes);
	}
}

void SCR_Scroll(int x, int y, int w, int h, int lines, uint8_t paper, bool smooth)
{
	#if HAS_WINDOWTEXT
	WinText_Scroll(x, y, w, h, lines);
	#endif
	if(smoothScrolling && smooth)
		buffering = true;
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_SCROLL;
		c->x = x;
		c->y = y;
		c->w = w;
		c->h = h;
		c->n = lines;
		c->paper = paper;
	}
	else
	{
		VID_Scroll(x, y, w, h, lines, paper);
	}
}

void SCR_ConsumeBuffer()
{
	bool scrollPerformed = false;

	while(commandBufferReadIndex != commandBufferIndex)
	{
		int block = IndexBlock(commandBufferReadIndex);
		int offset = IndexOffset(commandBufferReadIndex);
		commandBufferReadIndex++;

		SCR_CommandData* c = &commandBufferBlocks[block][offset];
		switch(c->type)
		{
			case SCR_COMMAND_WAITFORKEY:
				waitingForKey = true;
				return;
			case SCR_COMMAND_DRAWCHARACTER:
				VID_DrawCharacter(c->x, c->y, c->n, c->ink, c->paper);
				break;
			#if HAS_DRAWSTRING
			case SCR_COMMAND_DRAWVECTORPICTURE:
			{
				uint8_t attributes = VID_GetAttributes();
				DDB_ExecuteVectorPicture(c->n);
				VID_SetAttributes(attributes);
				break;
			}
			#endif
			case SCR_COMMAND_LOADPICTURE:
				VID_LoadPicture(c->n,(DDB_ScreenMode)c->x);
				break;
			case SCR_COMMAND_DISPLAYPICTURE:
				VID_DisplayPicture(c->x, c->y, c->w, c->h,(DDB_ScreenMode)c->n);
				break;
			case SCR_COMMAND_CLEAR:
				VID_Clear(c->x, c->y, c->w, c->h, c->n, (VID_ClearMode)c->ink);
				break;
			case SCR_COMMAND_SETATTRIBUTES:
				VID_SetAttributes(c->n);
				break;
			case SCR_COMMAND_SCROLL:
				if(scrollPerformed && smoothScrolling) {
					commandBufferReadIndex--;
					return;
				}
				VID_Scroll(c->x, c->y, c->w, c->h, c->n, c->paper);
				scrollPerformed = true;
				break;
			case SCR_COMMAND_SAVE:
				VID_SaveScreen();
				break;
			case SCR_COMMAND_RESTORE:
				VID_RestoreScreen();
				break;
			case SCR_COMMAND_SWAP:
				VID_SwapScreen();
				break;
			case SCR_COMMAND_SETOPBUFFER:
				VID_SetOpBuffer((SCR_Operation)c->n, c->x);
				break;
			case SCR_COMMAND_CLEARBUFFER:
				VID_ClearBuffer(c->x);
				break;
		}
	}
	buffering = false;
}

bool SCR_Synchronized ()
{
	return !buffering && !waitingForKey;
}

void SCR_GetKey(uint8_t* key, uint8_t* ext, uint8_t* mod)
{
	#if HAS_TESTMODE
	if (DDB_TestGetKey(key, ext, mod))
		return;
	#endif
	VID_GetKey(key, ext, mod);
}

bool SCR_AnyKey()
{
	#if HAS_TESTMODE
	if (DDB_TestIsActive())
		return DDB_TestAnyKey();
	#endif
	return VID_AnyKey();
}

#if HAS_TESTMODE
bool SCR_AnyKeyForMore(bool allowCaptures)
{
	return DDB_TestIsActive() ? DDB_TestAnyKeyForMore(allowCaptures) : SCR_AnyKey();
}
#endif

void SCR_GetPaletteColor(uint8_t color, uint8_t* r, uint8_t* g, uint8_t* b)
{
	// Should this be buffered ???
	VID_GetPaletteColor(color, r, g, b);
}

void SCR_SetPaletteColor(uint8_t color, uint8_t r, uint8_t g, uint8_t b)
{
	// Should this be buffered ???
	VID_SetPaletteColor(color, r, g, b);
}

void SCR_SaveScreen()
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_SAVE;
	}
	else
	{
		VID_SaveScreen();
	}
}

void SCR_RestoreScreen()
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_RESTORE;
	}
	else
	{
		VID_RestoreScreen();
	}
}

void SCR_SwapScreen()
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_SWAP;
	}
	else
	{
		VID_SwapScreen();
	}
}

void SCR_SetOpBuffer(SCR_Operation op, bool front)
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_SETOPBUFFER;
		c->n = op;
		c->x = front;
	}
	else
	{
		VID_SetOpBuffer(op, front);
	}
}

void SCR_ClearBuffer(bool front)
{
	if (buffering)
	{
		SCR_CommandData* c = SCR_AddCommandToBuffer();
		c->type = SCR_COMMAND_CLEARBUFFER;
		c->x = front;
	}
	else
	{
		VID_ClearBuffer(front);
	}
}

void SCR_PlaySample(uint8_t sample, int* duration)
{
	VID_PlaySample(sample, duration);
}

void SCR_PlaySampleBuffer(void* buffer, int duration, int hz, int volume)
{
	VID_PlaySampleBuffer(buffer, duration, hz, volume);
}

void SCR_GetMilliseconds(uint32_t* time)
{
	VID_GetMilliseconds(time);
}

void SCR_Quit()
{
	VID_Quit();
}

void SCR_MainLoop(DDB_Interpreter* i, MainLoopCallback callback)
{
	VID_MainLoop(i, callback);
}

void SCR_OpenFileDialog(bool existing, char* filename, size_t bufferSize)
{
	VID_OpenFileDialog(existing, filename, bufferSize);
}

void SCR_SetTextInputMode(bool enabled)
{
	VID_SetTextInputMode(enabled);
}

#if HAS_TESTMODE
void SCR_UseInputFile(const char* filename)
{
	DDB_TestLoadInput(filename);
}
#endif

#endif
