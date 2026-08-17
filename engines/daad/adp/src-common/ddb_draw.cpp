#if HAS_DRAWSTRING

#include <ddb.h>
#include <ddb_data.h>
#include <ddb_vid.h>
#include <os_bito.h>
#include <os_lib.h>
#include <os_mem.h>
#include <os_file.h>
#include <stdio.h>

#define TRACE_VECTOR
// #define DISABLE_FILL
// #define STEP_CAPTURE

#ifdef STEP_CAPTURE
#include <stdio.h>
extern bool VID_SaveGraphicsSnapshot(const char* fileName);
static int stepPicno = -1;
static int stepCount = 0;
#endif

uint32_t CPC_Colors[27] = {
	0xFF000000, 0xFF000080, 0xFF0000FF, 0xFF800000, 0xFF800080, 0xFF8000FF, 0xFFFF0000, 0xFFFF0080, 0xFFFF00FF,
	0xFF008000, 0xFF008080, 0xFF0080FF, 0xFF808000, 0xFF808080, 0xFF8080FF, 0xFFFF8000, 0xFFFF8080, 0xFFFF80FF,
	0xFF00FF00, 0xFF00FF80, 0xFF00FFFF, 0xFF80FF00, 0xFF80FF80, 0xFF80FFFF, 0xFFFFFF00, 0xFFFFFF80, 0xFFFFFFFF,
};

extern void VID_SaveDebugBitmap();

static const uint8_t* vectorGraphicsRAM = 0;
static size_t vectorGraphicsSize = 0;
static DDB_Machine vectorGraphicsMachine = DDB_MACHINE_SPECTRUM;
static DDB_Version vectorGraphicsVersion = DDB_VERSION_1;

static uint16_t start;				// Start of picture data
static uint16_t spare;				// End of picture data
static uint16_t locattr;			// Table of picture attributes (PAWS)
static uint16_t table;				// Table of picture addresses
static uint16_t windefs;			// Pointer to a table of windows definitions (entry size platform-dependant)
static uint16_t unknown;			// TODO: Find out what this is
static uint16_t chset;				// Table to the 2K character set
static uint16_t coltab;
static uint16_t extra;
static uint16_t ending;
static uint16_t count;

// TODO: Move drawing functions to a separate file
extern uint8_t* bitmap;
extern uint8_t* attributes;
extern uint8_t* graphicsBuffer;
extern uint16_t screenWidth;

static const uint8_t* udgs;
static const uint8_t* shades = 0;
static bool mixShades = false;

#if HAS_PAWS
static bool ResolvePAWSPicture(uint8_t picno, const uint8_t** pageBase,
	uint8_t* localPicture, uint16_t* pageTable, uint16_t* pageAttributes)
{
	if (vectorGraphicsRAM == 0 || vectorGraphicsVersion != DDB_VERSION_PAWS)
		return false;
	uint8_t from = 0;
	const uint8_t* map = vectorGraphicsRAM + 0x9429;
	for (int n = 0; n < 6; n++)
	{
		uint8_t page = map[n * 2];
		uint8_t to = map[n * 2 + 1];
		uint16_t upper = to == 0xFF ? count : to;
		if (picno < upper)
		{
			const uint8_t* base = vectorGraphicsRAM;
			if (page != 0)
			{
				if (vectorGraphicsSize <= 0x10000 || page > 7) return false;
				base = vectorGraphicsRAM + 0x10000 + page * 0x4000 - 0xC000;
			}
			uint16_t directory = read16LE(base + 0xFFF1);
			uint16_t attributes = read16LE(base + 0xFFF3);
			uint8_t local = (uint8_t)(picno - from);
			if (directory >= attributes || directory + local * 2 + 1 >= 0x10000 ||
				attributes + local >= 0x10000) return false;
			*pageBase = base;
			*localPicture = local;
			*pageTable = directory;
			*pageAttributes = attributes;
			return true;
		}
		from = to;
		if (to == 0xFF) break;
	}
	return false;
}
#endif

// The drawing position is not clamped to the screen: pictures move and
// draw through off-screen coordinates and rely on per-pixel clipping
static int16_t cursorX;
static int16_t cursorY;

static int cpcUserX;
static int cpcUserY;
static uint8_t ink;
static uint8_t paper;
static uint8_t border;
static uint8_t bright;
static uint8_t flash;
static uint8_t attrValue = 7;
static uint8_t attrMask = 0xFF;
static uint8_t defaultPaper = 0;
static uint8_t defaultInk = 7;
static uint8_t defaultBorder = 0;
static uint8_t depth = 0;

static uint16_t scrMaxX = 255;
static uint16_t scrMaxY = 175;
static uint8_t  stride  = 32;
static bool     pixelMode = false;
static uint8_t  transparentColor = 8;

static int16_t winMinX = 0, winMinY = 0, winMaxX = 0x7FFF, winMaxY = 0x7FFF;

void VID_SetInk (uint8_t value)
{
	uint8_t mask = transparentColor - 1;

	ink = value;
	if (ink == transparentColor)
	{
		attrMask  |=  mask;
		attrValue &= ~mask;
	}
	else
	{
		attrMask &= ~mask;
		attrValue = (attrValue & ~mask) | (ink & mask);
	}
}

void VID_SetPaper (uint8_t value)
{
	uint8_t shift = transparentColor == 8 ? 3 : 4;
	uint8_t mask = (transparentColor - 1) << shift;

	paper = value;
	if (paper == transparentColor)
	{
		attrMask  |=  mask;
		attrValue &= ~mask;
	}
	else
	{
		attrMask &= ~mask;
		attrValue = (attrValue & ~mask) | ((paper << shift) & mask);
	}
}

uint8_t VID_GetInk()
{
	return ink;
}

uint8_t VID_GetDefaultInk()
{
	return defaultInk;
}

uint8_t VID_GetPaper()
{
	return paper;
}

uint8_t VID_GetDefaultPaper()
{
	return defaultPaper;

}

uint8_t VID_GetAttributes()
{
	return attrValue;
}

void VID_SetAttributes(uint8_t value)
{
	attrValue = value;

	uint8_t shift = transparentColor == 8 ? 3 : 4;
	uint8_t pmask = (transparentColor - 1) << shift;
	uint8_t imask = transparentColor - 1;
	paper  = (value & pmask) >> shift;
	ink    = value & imask;
	bright = (value & 0x40) ? 1 : 0;
	flash  = (value & 0x80) ? 1 : 0;
}

void VID_SetBright (uint8_t value)
{
	bright = value;
	if (bright == 8)
	{
		attrMask  |=  0x40;
		attrValue &= ~0x40;
	}
	else
	{
		attrMask &= ~0x40;
		attrValue = (attrValue & ~0x40) | ((bright & 0x01) << 6);
	}
}

void VID_SetFlash (uint8_t value)
{
	flash = value;
	if (flash == 8)
	{
		attrMask  |=  0x80;
		attrValue &= ~0x80;
	}
	else
	{
		attrMask &= ~0x80;
		attrValue = (attrValue & ~0x80) | ((flash & 0x01) << 7);
	}
}

void VID_MoveTo (uint16_t x, uint16_t y)
{
	// The 6502 interpreters and the CPC firmware keep an unclamped
	// position and clip each pixel individually, so off-screen positions
	// must survive here for the next stroke to start from the right place
	if (vectorGraphicsMachine != DDB_MACHINE_C64 &&
	    vectorGraphicsMachine != DDB_MACHINE_CPC)
	{
		if (x > scrMaxX) x = scrMaxX;
		if (y > scrMaxY) y = scrMaxY;
	}
	cursorX = (int16_t)x;
	cursorY = (int16_t)y;
}

// Relative move arithmetic of the Spectrum/MSX interpreters, which work
// in 8 bit registers with the Y origin at the bottom of the screen:
// X wraps modulo 256 and Y wraps modulo the 176 line drawing area, with
// a single correction applied per move (moving down past the bottom adds
// 176 back, moving up past line 175 subtracts 176 once)
static void ZXRelativeMove (int16_t dx, int16_t dy, int16_t* px, int16_t* py)
{
	uint8_t xi = (uint8_t)(*px + dx);
	uint8_t yi = (uint8_t)(175 - *py);
	int     up = -dy;					// Positive displacements go up

	if (up >= 0)
	{
		int sum = yi + up;
		yi = (uint8_t)sum;
		if (sum > 255 || yi >= 176)
			yi = (uint8_t)(yi - 176);
	}
	else
	{
		if (-up > yi)
			yi = (uint8_t)(yi + up - 80);
		else
			yi = (uint8_t)(yi + up);
	}
	*px = xi;
	*py = 175 - yi;
}

void VID_MoveBy (int16_t x, int16_t y)
{
	if (vectorGraphicsMachine == DDB_MACHINE_C64 ||
	    vectorGraphicsMachine == DDB_MACHINE_CPC)
	{
		// The 6502 interpreters and the CPC firmware keep an unclamped
		// position and clip each pixel individually
		cursorX += x;
		cursorY += y;
		return;
	}
	if (vectorGraphicsMachine == DDB_MACHINE_MSX)
	{
		// Verified against the MSX interpreter; the Spectrum ones shipped
		// with earlier revisions of this code and keep the clamping
		// behavior below, which matches all verified captures
		ZXRelativeMove(x, y, &cursorX, &cursorY);
		return;
	}
	// Unsigned comparisons preserve the historical wrap-around behavior
	// of the Z80 interpreters for off-screen positions
	if (x < 0 && (uint16_t)cursorX < (uint16_t)-x)
		cursorX = 0;
	else
		cursorX += x;
	if (y < 0 && (uint16_t)cursorY < (uint16_t)-y)
		cursorY = 0;
	else
		cursorY += y;
	if ((uint16_t)cursorX > scrMaxX)
		cursorX = scrMaxX;
	if ((uint16_t)cursorY > scrMaxY)
		cursorY = scrMaxY;
}

void VID_SetAttribute()
{
	if (cursorX < 0 || cursorX > scrMaxX || cursorY < 0 || cursorY > scrMaxY)
		return;
	uint16_t offset = (cursorY >> 3) * stride + (cursorX >> 3);
	attributes[offset] = (attributes[offset] & attrMask) | attrValue;
}

int VID_GetPixel(int16_t x, int16_t y)
{
	if (x < 0 || x > scrMaxX || y < 0 || y > scrMaxY)
		return 0;

	if (pixelMode)
		return graphicsBuffer[screenWidth*y + x];

	uint8_t* ptr = bitmap + y * stride + (x >> 3);
	uint8_t mask = 0x80 >> (x & 7);
	return (*ptr & mask) != 0 ? 1 : 0;
}

int VID_GetPixel()
{
	return VID_GetPixel(cursorX, cursorY);
}

void VID_DrawPixel(uint8_t color)
{
	if (cursorX < 0 || cursorX > scrMaxX || cursorY < 0 || cursorY > scrMaxY)
		return;

	if (pixelMode)
	{
		graphicsBuffer[screenWidth*cursorY + cursorX] = color;
		return;
	}

	uint8_t* ptr = bitmap + cursorY * stride + (cursorX >> 3);
	uint8_t mask = 0x80 >> (cursorX & 7);

	if (color == 255)
		*ptr ^= mask;
	else if (color == 0)
		*ptr &= ~mask;
	else
		*ptr |= mask;

	VID_SetAttribute();
}

#define CPC_FILL_STACK 30

static int16_t* yfills = 0;
static uint32_t yfillsLines = 0;

static void GenericInnerFill (int16_t seedX, int16_t seedY, uint8_t* pattern)
{
	#ifdef DISABLE_FILL
	if (seedX >= 0 && seedX <= scrMaxX && seedY >= 0 && seedY <= scrMaxY)
		graphicsBuffer[screenWidth * seedY + seedX] = 15;
	return;
	#endif

	if (seedX < 0 || seedX > scrMaxX || seedY < 0 || seedY > scrMaxY)
		return;

	uint32_t lines = (uint32_t)scrMaxY + 1;
	if (yfillsLines < lines)
	{
		if (yfills != 0)
			Free(yfills);
		yfills = Allocate<int16_t>("Fill table", lines);
		yfillsLines = yfills != 0 ? lines : 0;
	}
	if (yfills == 0)
		return;

	bool cpcFill = vectorGraphicsMachine == DDB_MACHINE_CPC;
	int16_t clipX0 = winMinX > 0 ? winMinX : 0;
	int16_t clipY0 = winMinY > 0 ? winMinY : 0;
	int16_t clipX1 = winMaxX < scrMaxX ? winMaxX : scrMaxX;
	int16_t clipY1 = winMaxY < scrMaxY ? winMaxY : scrMaxY;
	if (cpcFill)
	{
		// The original engine bounds fills by the screen edges and the
		// 184-line picture area only, never by the current window
		clipX0 = 0;
		clipY0 = 0;
		clipX1 = scrMaxX;
		clipY1 = 183 < scrMaxY ? 183 : scrMaxY;
	}

	const uint8_t seed = graphicsBuffer[screenWidth * seedY + seedX];
	#define DS_BG(X,Y) ((X) >= clipX0 && (X) <= clipX1 && (Y) >= clipY0 && (Y) <= clipY1 && \
	                    graphicsBuffer[screenWidth * (Y) + (X)] == seed)

	if (!DS_BG(seedX, seedY))
		return;

	for (uint32_t i = 0; i < lines; i++)
		yfills[i] = 0;

	struct { int16_t x, y; } stack[CPC_FILL_STACK];

	for (int pass = 0; pass < 2; pass++)
	{
		int adjDelta = pass == 0 ? +1 : -1;
		int curX = seedX;
		int curY = pass == 0 ? seedY : seedY - 1;
		if (curY < 0 || curY > scrMaxY)
			break;
		if (!DS_BG(curX, curY))
			break;

		int  sp = 0;
		int  budget = CPC_FILL_STACK;
		bool endPass = false;

		for (;;)
		{
			int y = curY;
			int adjY = y + adjDelta;

			int rx = curX;
			while (rx + 1 <= scrMaxX && DS_BG(rx + 1, y)) rx++;
			yfills[y] = (int16_t)rx;

			int  cadj = (adjY < 0 || adjY > scrMaxY) ? (scrMaxX + 1) : yfills[adjY];
			bool fillflag = false;
			int  scanflag = 0;

			for (int x = rx; x >= 0; x--)
			{
				if (!DS_BG(x, y))
					break;
				if (cpcFill)
				{
					// The pattern rows are screen-encoded bytes: for the pixel
					// at x&3 within a screen byte, one bit selects whether the
					// low colour bit comes from the ink or the paper and
					// another selects the high bit, so a pattern can produce
					// blend colours. Rows are indexed by the fill-space y,
					// which counts upwards from screen line 183
					int n = x & 3;
					uint8_t m = pattern[(183 - y) & 7];
					uint8_t lo = ((m >> (7 - n)) & 1) != 0 ? ink : paper;
					uint8_t hi = ((m >> (3 - n)) & 1) != 0 ? ink : paper;
					graphicsBuffer[screenWidth * y + x] = (lo & 1) | (hi & 2);
				}
				else
				{
					graphicsBuffer[screenWidth * y + x] =
						(pattern[y & 7] & (0x80 >> (x & 7))) ? ink : paper;
				}
				if (x == 0)
					break;
				if (fillflag)
					continue;
				if (cadj != 0 && cadj >= x)
				{
					fillflag = true;
					continue;
				}
				bool adjBg = DS_BG(x, adjY);
				if (scanflag == 0)
				{
					if (adjBg)
					{
						if (--budget == 0) { endPass = true; break; }
						stack[sp].x = (int16_t)x;
						stack[sp].y = (int16_t)adjY;
						sp++;
						scanflag = 1;
					}
				}
				else if (!adjBg)
				{
					scanflag = 0;
				}
			}

			if (endPass || sp == 0)
				break;
			sp--;
			budget++;
			curX = stack[sp].x;
			curY = stack[sp].y;
		}
	}
	#undef DS_BG
}

static void SpectrumInnerFill (int16_t seedX, int16_t seedY, uint8_t* pattern)
{
	#ifdef DISABLE_FILL
	if (seedX >= 0 && seedX <= scrMaxX && seedY >= 0 && seedY <= scrMaxY)
		bitmap[seedY * stride + (seedX >> 3)] |= 0x80 >> (seedX & 7);
	return;
	#endif

	if (seedX < 0 || seedX > scrMaxX || seedY < 0 || seedY > scrMaxY)
		return;

	uint32_t lines = (uint32_t)scrMaxY + 1;
	if (yfillsLines < lines)
	{
		if (yfills != 0)
			Free(yfills);
		yfills = Allocate<int16_t>("Fill table", lines);
		yfillsLines = yfills != 0 ? lines : 0;
	}
	if (yfills == 0)
		return;

	// Background is a clear pixel; painting ORs the pattern bit into the
	// bitmap and refreshes the cell attribute. Pixels the pattern leaves
	// clear stay background, so termination comes from the pass structure,
	// not from the pixel state.
	#define SB_BG(X,Y) ((X) >= 0 && (X) <= scrMaxX && (Y) >= 0 && (Y) <= scrMaxY && \
	                    (bitmap[(Y) * stride + ((X) >> 3)] & (0x80 >> ((X) & 7))) == 0)

	if (!SB_BG(seedX, seedY))
		return;

	for (uint32_t i = 0; i < lines; i++)
		yfills[i] = 0;

	struct { int16_t x, y; } stack[80];
	// The 6502 interpreters keep the run queue on the CPU stack, which
	// holds far more entries than the Z80 table
	int stackLimit = vectorGraphicsMachine == DDB_MACHINE_C64 ? 74 : CPC_FILL_STACK;

	for (int pass = 0; pass < 2; pass++)
	{
		int adjDelta = pass == 0 ? +1 : -1;
		int curX = seedX;
		int curY = pass == 0 ? seedY : seedY - 1;
		if (curY < 0 || curY > scrMaxY)
			break;
		if (!SB_BG(curX, curY))
			break;

		int  sp = 0;
		int  budget = stackLimit;
		bool endPass = false;

		for (;;)
		{
			int y = curY;
			int adjY = y + adjDelta;

			int rx = curX;
			while (rx + 1 <= scrMaxX && SB_BG(rx + 1, y)) rx++;
			yfills[y] = (int16_t)rx;

			int  cadj = (adjY < 0 || adjY > scrMaxY) ? (scrMaxX + 1) : yfills[adjY];
			bool fillflag = false;
			int  scanflag = 0;

			for (int x = rx; x >= 0; x--)
			{
				if (!SB_BG(x, y))
					break;
				bitmap[y * stride + (x >> 3)] |= pattern[y & 7] & (0x80 >> (x & 7));
				uint16_t aoff = (uint16_t)((y >> 3) * stride + (x >> 3));
				attributes[aoff] = (attributes[aoff] & attrMask) | attrValue;
				if (x == 0)
					break;
				if (fillflag)
					continue;
				if (cadj != 0 && cadj >= x)
				{
					fillflag = true;
					continue;
				}
				bool adjBg = SB_BG(x, adjY);
				if (scanflag == 0)
				{
					if (adjBg)
					{
						if (--budget == 0) { endPass = true; break; }
						stack[sp].x = (int16_t)x;
						stack[sp].y = (int16_t)adjY;
						sp++;
						scanflag = 1;
					}
				}
				else if (!adjBg)
				{
					scanflag = 0;
				}
			}

			if (endPass || sp == 0)
				break;
			sp--;
			budget++;
			curX = stack[sp].x;
			curY = stack[sp].y;
		}
	}
	#undef SB_BG
}

void VID_DrawPixel(int16_t x, int16_t y, uint8_t color)
{
	if (x < 0 || x > scrMaxX || y < 0 || y > scrMaxY)
		return;

	if (pixelMode)
	{
		graphicsBuffer[screenWidth*y + x] = color;
		return;
	}

	uint8_t* ptr = bitmap + y * stride + (x >> 3);
	uint8_t mask = 0x80 >> (x & 7);
	if (color == 0)
		*ptr &= ~mask;
	else if (color == 255)
		*ptr ^= mask;
	else
		*ptr |= mask;

	uint16_t offset = (y >> 3) * stride + (x >> 3);
	attributes[offset] = (attributes[offset] & attrMask) | attrValue;
}

void VID_PatternFill(int16_t x, int16_t y, int pattern, bool invert)
{
	uint8_t ch[8];

	if (vectorGraphicsMachine == DDB_MACHINE_MSX)
	{
		// The seed position uses the same wrapping arithmetic as
		// relative moves (the cursor itself is not updated)
		int16_t sx = cursorX, sy = cursorY;
		ZXRelativeMove(x, y, &sx, &sy);
		x = sx;
		y = sy;
		if (x < 0 || x > scrMaxX || y < 0 || y > scrMaxY)
			return;
	}
	else if (vectorGraphicsMachine == DDB_MACHINE_C64)
	{
		x += cursorX;
		y += cursorY;
		// An off-screen seed abandons the fill
		if (x < 0 || x > scrMaxX || y < 0 || y > scrMaxY)
			return;
	}
	else if (vectorGraphicsMachine == DDB_MACHINE_CPC)
	{
		x += cursorX;
		y += cursorY;
		// An off-screen seed skips the fill entirely (the original engine
		// validates the position and the 184-line picture area before
		// filling, never clamping)
		if (x < 0 || x > scrMaxX || y < 0 || y > 183)
			return;
	}
	else
	{
		x += cursorX;
		y += cursorY;
		if (x < 0) x = 0;
		if (x > scrMaxX) x = scrMaxX;
		if (y < 0) y = 0;
		if (y > scrMaxY) y = scrMaxY;
	}

	if (pattern >= 0)
	{
		if (mixShades)
		{
			const uint8_t* pattern1 = shades + 8*(pattern & 0x0F);
			const uint8_t* pattern2 = shades + 8*(pattern >> 4);
			for (int n = 0; n < 8; n++)
				ch[n] = pattern2[n] | pattern1[n];
		}
		else
		{
			MemCopy(ch, shades + 8*pattern, 8);
		}
	}
	else
	{
		for (int n = 0; n < 8; n++) ch[n] = 0xFF;
	}
	if (invert)
	{
		for (int n = 0; n < 8; n++) ch[n] ^= 0xFF;
	}

	if (pixelMode)
	{
		#ifdef STEP_CAPTURE
		char capName[64];
		int color = VID_GetPixel(x, y);
		sprintf(capName, "step_pic%02d_%03d_a_before.bmp", stepPicno, stepCount);
		VID_SaveGraphicsSnapshot(capName);
		#endif

		GenericInnerFill(x, y, ch);

		#ifdef STEP_CAPTURE
		sprintf(capName, "step_pic%02d_%03d_b_after_seed(%d,%d)_col%d_ink%d.bmp",
		        stepPicno, stepCount, x, y, color, ink);
		VID_SaveGraphicsSnapshot(capName);
		stepCount++;
		#endif
	}
	else
	{
		SpectrumInnerFill(x, y, ch);
	}
}

static void CPC_FirmwareLine(int x0p, int y0p, int x1p, int y1p, uint8_t color)
{
	// Matches the line drawer in the 6128 firmware (the 464 ROM uses a
	// different run-slicing algorithm that distributes the same number of
	// pixels with a different phase). It is a midpoint walk over pixel
	// coordinates with y growing upwards, canonicalized so the major axis
	// always increases by re-anchoring at the target endpoint and flipping
	// both step directions. Both endpoints are plotted, and the error term
	// starts at -ceil(major/2). Plotting is clipped to the 184-line
	// picture area like the original graphics window.
	int x0 = x0p, y0 = 199 - y0p;
	int x1 = x1p, y1 = 199 - y1p;

	int dx = x1 - x0, dy = y1 - y0;
	int adx = dx < 0 ? -dx : dx;
	int ady = dy < 0 ? -dy : dy;
	int sx = dx < 0 ? -1 : 1;
	int sy = dy < 0 ? -1 : 1;

	bool xMajor = ady < adx;
	int major = xMajor ? adx : ady;
	int minor = xMajor ? ady : adx;

	int px = x0, py = y0;
	if ((xMajor ? dx : dy) < 0)
	{
		px = x1;
		py = y1;
		sx = -sx;
		sy = -sy;
	}

	int err = -((major + 1) >> 1);
	for (int i = 0; i <= major; i++)
	{
		if (py >= 16 && py <= 199)
			VID_DrawPixel(px, 199 - py, color);
		err += minor;
		if (err >= 0)
		{
			err -= major;
			if (xMajor)
				py += sy;
			else
				px += sx;
		}
		if (xMajor)
			px += sx;
		else
			py += sy;
	}
}
void VID_DrawLine(int16_t incx, int16_t incy, uint8_t color)
{
	int x = cursorX;
	int y = cursorY;
    int stepx, stepy;
    int signx, signy;
    int max, inc;
    int cur, pas;

	int destx = x + incx;
	int desty = y + incy;

	if (pixelMode && vectorGraphicsMachine == DDB_MACHINE_CPC)
	{
		CPC_FirmwareLine(x, y, destx, desty, color);
		VID_MoveTo(destx < 0 ? 0 : destx, desty < 0 ? 0 : desty);
		return;
	}

	if (incx < 0)
		signx = -1, incx = -incx;
	else
		signx = 1;

	if (incy < 0)
		signy = -1, incy = -incy;
	else
		signy = 1;

	if (incy >= incx)
	{
		inc = incx;
		max = incy;
		stepx = 0;
		stepy = signy;
	}
	else
	{
		inc = incy;
		max = incx;
		stepx = signx;
		stepy = 0;
	}

	cur = max / 2;
	pas = 0;

	while (pas++ < max)
	{
		cur += inc;
		if (cur >= max)
		{
			y += signy;
			x += signx;
			cur -= max;
		}
		else
		{
			y += stepy;
			x += stepx;
		}
		VID_DrawPixel(x, y, color);
	}

	cursorX = x;
	cursorY = y;
}

void VID_AttributeFill (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	if (pixelMode)
		return;

	if (x1 < x0) {
		uint8_t tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y1 < y0) {
		uint8_t tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	uint8_t maxrow = scrMaxY/8;
	uint8_t maxcol = scrMaxX/8;
	if (x0 > maxcol || y0 > maxrow)
		return;
	if (x1 > maxcol)
		x1 = maxcol;
	if (y1 > maxrow)
		y1 = maxrow;

	uint8_t* ptr = attributes + y0 * stride + x0;
	uint8_t* end = attributes + y1 * stride + x1;
	for (int y = y0; y <= y1; y++)
	{
		uint8_t* line = ptr;
		for (int x = x0; x <= x1; x++, ptr++)
			*ptr = (*ptr & attrMask) | attrValue;
		ptr = line + stride;
	}
}

static void VID_Draw8x8Glyph (int x, int y, const uint8_t* glyph, uint8_t ink, uint8_t paper,
                              bool over = false, bool inverse = false)
{
	if (!attributes)
		return;

	uint8_t* out = bitmap + y * stride + (x >> 3);
	uint8_t  rot = x & 7;
	uint8_t* attr = attributes + (y >> 3) * stride + (x >> 3);
	uint8_t xattr = 0;
	uint8_t width = 8;
	uint8_t paperShift = 4;

	if (screenMachine == DDB_MACHINE_SPECTRUM)
	{
		xattr = (ink & 0x30) << 2;
		ink &= 7;
		paper &= 7;
		paperShift = 3;
	}

	uint8_t curAttr = ink | (paper << paperShift) | xattr;

	if (paper == 255)
	{
		*attr = (*attr & 0x37) | ink | xattr;
		if (rot > 8-width)
			attr[1] = (attr[1] & 0x37) | ink | xattr;
	}
	else
	{
		paper &= 7;
		*attr = curAttr;
		if (rot > 8-width)
			attr[1] = *attr;
	}

	for (int line = 0; line < 8; line++)
	{
		uint8_t* sav = out;
		uint8_t mask = 0x80 >> rot;
		// INVERSE complements the glyph; OVER combines it with the screen
		// by exclusive OR, leaving the other pixels untouched (the Spectrum
		// print semantics selected by the PAW TEXT command bits)
		uint8_t bits = inverse ? (uint8_t)~glyph[line] : glyph[line];
		for (int col = 0; col < 8; col++)
		{
			if ((bits & (0x80 >> col)))
			{
				if (over)
					*out ^= mask;
				else
					*out |= mask;
			}
			else if (!over && paper != 255)
				*out &= ~mask;
			mask >>= 1;
			if (mask == 0)
			{
				mask = 0x80;
				out++;
			}
		}
		out = sav + stride;
	}
}

void VID_Draw8x8Character (int x, int y, uint8_t ch, uint8_t ink, uint8_t paper)
{
	VID_Draw8x8Glyph(x, y, charset + (ch << 3), ink, paper);
}

bool DrawVectorSubroutine (uint8_t picno, int scale, bool flipX, bool flipY)
{
	if (vectorGraphicsRAM == 0 || picno >= count)
		return false;

	if (scale == 0)
		scale = 8;

	if (++depth == 10)
	{
		depth--;
		return false;
	}

	#ifdef TRACE_VECTOR
	TRACE("\nEntering subroutine %d\n", picno);
	#endif

	const uint8_t* drawingRAM = vectorGraphicsRAM;
	uint8_t localPicture = picno;
	uint16_t drawingTable = table;
	#if HAS_PAWS
	if (vectorGraphicsVersion == DDB_VERSION_PAWS)
	{
		uint16_t drawingAttributes;
		if (!ResolvePAWSPicture(picno, &drawingRAM, &localPicture, &drawingTable, &drawingAttributes))
		{
			depth--;
			return false;
		}
	}
	#endif
	uint16_t offset = read16LE(drawingRAM + drawingTable + localPicture * 2);
	const uint8_t* ptr = drawingRAM + offset;

	switch (vectorGraphicsMachine)
	{
		default:
			return false;

		case DDB_MACHINE_C64:
			if ((*ptr & 0x07) == 0x02)
			{
				// Balance the depth counter bumped at entry
				depth--;
				return false;
			}
			for (;;)
			{
				switch (*ptr & 0x07)
				{
					case 0:		// INK (bit 7 selects transparent)
					{
						uint8_t value = (*ptr & 0x80) ? 16 : (*ptr >> 3) & 0x0F;
						#ifdef TRACE_VECTOR
						TRACE("%02X INK %d\n", ptr[0], value);
						#endif
						VID_SetInk(value);
						ptr += 1;
						break;
					}

					case 1:		// PAPER (bit 7 selects transparent)
					{
						uint8_t value = (*ptr & 0x80) ? 16 : (*ptr >> 3) & 0x0F;
						#ifdef TRACE_VECTOR
						TRACE("%02X PAPER %d\n", ptr[0], value);
						#endif
						VID_SetPaper(value);
						ptr += 1;
						break;
					}

					case 2:		// RETURN
						#ifdef TRACE_VECTOR
						TRACE("%02X RETURN\n\n", ptr[0]);
						#endif

						depth--;
						return true;

					case 3: // GOSUB
					{
						int scale = (*ptr >> 3) & 0x7;
						bool sflipX = (*ptr & 0x80) != 0;
						bool sflipY = (*ptr & 0x40) != 0;

						#ifdef TRACE_VECTOR
						TRACE("%02X GOSUB   %02X            (scale: %d, flipX: %d, flipY: %d)\n", ptr[0], ptr[1], scale, sflipX, sflipY);
						fflush(stdout);
						#endif

						if (flipX) sflipX = !sflipX;
						if (flipY) sflipY = !sflipY;
						DrawVectorSubroutine(ptr[1], scale, sflipX, sflipY);
						ptr += 2;
						break;
					}

					case 4:		// PLOT (absolute; flips do not apply)
					{
						int x = ptr[1] | ((ptr[0] & 0x08) << 5);
						int y = ptr[2];

						#ifdef TRACE_VECTOR
						TRACE("%02X PLOT    %02X %02X         (%d, %d)\n", ptr[0], ptr[1], ptr[2], x, y);
						#endif
						VID_MoveTo(x, y);
						switch (*ptr & 0xC0)		// Over + Inverse flags
						{
							case 0x00: VID_DrawPixel(1); break;
							case 0x80: VID_DrawPixel(255); break;
							case 0x40: VID_DrawPixel(0); break;
							case 0xC0: break;
						}
						ptr += 3;
						break;
					}

					case 5: // LINE
					{
						int x = ptr[1] | ((ptr[0] & 0x08) << 5);
						int y = ptr[2];

						#ifdef TRACE_VECTOR
						if ((*ptr & 0xC0) == 0xC0)
							TRACE("%02X MOVE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
						else
							TRACE("%02X LINE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
						#endif

						if (*ptr & 0x20) x = -x;
						if (*ptr & 0x10) y = -y;
						if (flipX) x = -x;
						if (flipY) y = -y;
						y = y * scale / 8;
						x = x * scale / 8;

						#ifdef TRACE_VECTOR
						TRACE("       (%d, %d)\n", x, y);
						#endif
						switch (*ptr & 0xC0)		// Over + Inverse flags
						{
							case 0x00: VID_DrawLine(x, y, 1); break;
							case 0x80: VID_DrawLine(x, y, 255); break;
							case 0x40: VID_DrawLine(x, y, 0); break;
							case 0xC0: VID_MoveBy(x, y); break;
						}
						ptr += 3;
						break;
					}

					case 6:							// FILL/SHADE
					{
						int x = ptr[1] | ((ptr[0] & 0x08) << 5);
						int y = ptr[2];
						int p = (*ptr & 0x80) ? ptr[3] : -1;
						bool invert = (*ptr & 0x80) != 0 && (*ptr & 0x40) != 0;
						x = x * scale / 8;
						y = y * scale / 8;
						if (flipX) x = -x;
						if (flipY) y = -y;
						if (*ptr & 0x20) x = -x;
						if (*ptr & 0x10) y = -y;

						#ifdef TRACE_VECTOR
						if (p != -1)
							TRACE("%02X SHADE   %02X %02X %02X      (%d, %d, %d)\n", ptr[0], ptr[1], ptr[2], ptr[3], x, y, p);
						else
							TRACE("%02X FILL    %02X %02X         (%d, %d)\n", ptr[0], ptr[1], ptr[2], x, y);
						#endif
						VID_PatternFill(x, y, p, invert);
						ptr += p == -1 ? 3 : 4;
						break;
					}

					case 7:		// BLOCK / CHAR
					{
						if ((*ptr & 0x80) == 0)
						{
							// Stamp an 8x8 pattern from the shade table at a
							// character cell; bit 6 = over (XOR), bit 5 = inverse
							uint8_t line = ptr[1];
							uint8_t col  = ptr[2];
							uint8_t form = ptr[3];
							#ifdef TRACE_VECTOR
							TRACE("%02X CHAR    %02X %02X %02X      cell(%d,%d) form %d\n", ptr[0], ptr[1], ptr[2], ptr[3], col, line, form);
							#endif
							if (line*8 < scrMaxY && col*8 < scrMaxX)
							{
								const uint8_t* src = shades + 8*form;
								uint8_t over = (*ptr & 0x40) ? 0xFF : 0x00;
								uint8_t inv  = (*ptr & 0x20) ? 0xFF : 0x00;
								for (int row = 0; row < 8; row++)
								{
									uint8_t* out = bitmap + (line*8 + row) * stride + col;
									*out = ((*out & over) ^ src[row]) ^ inv;
								}
								uint16_t aoff = (uint16_t)(line * stride + col);
								attributes[aoff] = (attributes[aoff] & attrMask) | attrValue;
							}
							ptr += 4;
						}
						else
						{
							uint8_t y0 = ptr[1];
							uint8_t x0 = ptr[2];
							uint8_t y1 = y0+ptr[3];
							uint8_t x1 = x0+ptr[4];

							#ifdef TRACE_VECTOR
							TRACE("%02X BLOCK   %02X %02X %02X %02X   (%d,%d)-(%d,%d)\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], x0, y0, x1, y1);
							#endif
							VID_AttributeFill(x0, y0, x1, y1);
							ptr += 5;
						}
						break;
					}
				}
			}

		case DDB_MACHINE_CPC:
			if (*ptr == 0x40)
			{
				// Balance the depth counter bumped at entry
				depth--;
				return false;
			}
			for (;;)
			{
				switch ((*ptr & 0x0E) >> 1)
				{
					case 0:		// SET PEN / SET PAPER / RETURN
						if ((*ptr & 0x40) != 0)
						{
							#ifdef TRACE_VECTOR
							TRACE("%02X RETURN\n\n", ptr[0]);
							#endif
							cpcUserX &= ~1;
							cpcUserY &= ~1;
							depth--;
							return true;
						}
						else
						{
							int color = ((ptr[0] >> 7) & 0x01) | ((ptr[0] << 1) & 0x02);
							if ((*ptr & 0x10) != 0)
							{
								#ifdef TRACE_VECTOR
								TRACE("%02X PAPER               (paper set to %d)\n", *ptr, color);
								#endif
								VID_SetPaper(color);
							}
							else
							{
								#ifdef TRACE_VECTOR
								TRACE("%02X PEN                 (pen set to %d)\n", *ptr, color);
								#endif
								VID_SetInk(color);
							}
							ptr++;
						}
						break;

					case 1:		// TEXT
					{
						uint16_t x = ptr[1] | ((ptr[0] & 0x80) << 1);
						uint16_t y = ptr[2];
						uint16_t c = ptr[3];

						#ifdef TRACE_VECTOR
						TRACE("%02X TEXT                  (char %d at %d,%d)\n\n", ptr[0], c, x, y);
						#endif
						// Stamp an 8x8 character from the graphics charset at
						// the given position (the coordinates address the
						// glyph's top-left pixel), drawing set bits in the
						// current ink over the current paper
						{
							const uint8_t* glyph = vectorGraphicsRAM + chset + c * 8;
							int px = x;
							int py = scrMaxY - y;
							for (int row = 0; row < 8; row++)
							{
								for (int col = 0; col < 8; col++)
									VID_DrawPixel(px + col, py + row,
										(glyph[row] & (0x80 >> col)) != 0 ? ink : paper);
							}
						}
						ptr += 4;
						break;
					}

					case 2:		// GOSUB
					{
						int  scale  = ((ptr[0] >> 6) & 0x03) | ((ptr[0] << 2) & 0x04);
						bool sflipX = (*ptr & 0x20) != 0;
						bool sflipY = (*ptr & 0x10) != 0;

						#ifdef TRACE_VECTOR
						TRACE("%02X GOSUB   %02X            (scale: %d, flipX: %d, flipY: %d)\n", ptr[0], ptr[1], scale, sflipX, sflipY);
						#endif

						if (flipX) sflipX = !sflipX;
						if (flipY) sflipY = !sflipY;
						DrawVectorSubroutine(ptr[1], scale, sflipX, sflipY);
						ptr += 2;
						break;
					}

					case 3:		// PLOT
					{
						uint16_t dataX = ptr[1] | ((ptr[0] & 0x80) << 1);
						uint16_t dataY = ptr[2];
						cpcUserX = 2 * dataX;
						cpcUserY = 2 * dataY;

						uint16_t x = cpcUserX >> 1;
						uint16_t y = scrMaxY - (cpcUserY >> 1);

						#ifdef TRACE_VECTOR
						TRACE("%02X PLOT    %02X %02X         (%d,%d)\n", ptr[0], ptr[1], ptr[2], x, y);
						#endif
						VID_MoveTo(x, y);
						if ((*ptr & 0x40) != 0)
							VID_DrawPixel(ink);
						ptr += 3;
						break;
					}
					case 4: // LINE
					{
						uint8_t c = *ptr;
						int x, y;
						if ((c & 0x01) != 0)
						{
							// The high bit of the opcode extends the X offset the
							// same way as in the long form (adding 256), even
							// though the short form only carries a nibble
							x = ((ptr[1] >> 4) & 0x0F) | ((ptr[0] & 0x80) << 1);
							y = ptr[1] & 0x0F;

							#ifdef TRACE_VECTOR
							if ((c & 0x40) != 0x40)
								TRACE("%02X MOVE    %02X     ", ptr[0], ptr[1]);
							else
								TRACE("%02X LINE    %02X     ", ptr[0], ptr[1]);
							#endif
							ptr += 2;
						}
						else
						{
							x = ptr[1] | ((ptr[0] & 0x80) << 1);
							y = ptr[2];

							#ifdef TRACE_VECTOR
							if ((c & 0x40) != 0x40)
								TRACE("%02X MOVE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
							else
								TRACE("%02X LINE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
							#endif
							ptr += 3;
						}
						if (flipX) x = -x;
						if (!flipY) y = -y;
						if (c & 0x20) x = -x;
						if (c & 0x10) y = -y;

						{
							int dx2 = 2 * x;
							int dy2 = -2 * y;
							dx2 = (dx2 * scale) >> 3;
							dy2 = (dy2 * scale) >> 3;

							int x0 = cpcUserX >> 1;
							int y0 = scrMaxY - (cpcUserY >> 1);
							cpcUserX += dx2;
							cpcUserY += dy2;
							int x1 = cpcUserX >> 1;
							int y1 = scrMaxY - (cpcUserY >> 1);

							#ifdef TRACE_VECTOR
							TRACE("       (%d, %d)\n", x1 - x0, y1 - y0);
							#endif
							VID_MoveTo(x0, y0);
							switch (c & 0x40)
							{
								case 0x40: VID_DrawLine(x1 - x0, y1 - y0, ink); break;
								case 0x00: VID_MoveBy(x1 - x0, y1 - y0); break;
							}
						}
						break;
					}
					case 5:		// FILL
					{
						int x = ptr[1] | ((ptr[0] & 0x80) << 1);
						int y = ptr[2];
						if (flipX) x = -x;
						if (!flipY) y = -y;
						if (*ptr & 0x20) x = -x;
						if (*ptr & 0x10) y = -y;

						int dx2 = (2 * x * scale) >> 3;
						int dy2 = (-2 * y * scale) >> 3;
						int seedX = (cpcUserX + dx2) >> 1;
						int seedY = scrMaxY - ((cpcUserY + dy2) >> 1);

						#ifdef TRACE_VECTOR
						TRACE("%02X FILL    %02X %02X         seed(%d,%d)\n", ptr[0], ptr[1], ptr[2], seedX, seedY);
						#endif
						VID_PatternFill(seedX - cursorX, seedY - cursorY, -1);
						ptr += 3;
						break;
					}
					case 6:		// SHADE
					{
						int cink  = ink;
						int sink  = ink;
						int spaper = ((ptr[0] << 1) & 0x02) | ((ptr[0] >> 6) & 0x01);
						int x = ptr[1] | ((ptr[0] & 0x80) << 1);
						int y = ptr[2];
						if (flipX) x = -x;
						if (!flipY) y = -y;
						if (*ptr & 0x20) x = -x;
						if (*ptr & 0x10) y = -y;
						int dx2 = (2 * x * scale) >> 3;
						int dy2 = (-2 * y * scale) >> 3;
						int seedX = (cpcUserX + dx2) >> 1;
						int seedY = scrMaxY - ((cpcUserY + dy2) >> 1);
						#ifdef TRACE_VECTOR
						TRACE("%02X SHADE   %02X %02X %02X      seed(%d,%d)\n", ptr[0], ptr[1], ptr[2], ptr[3], seedX, seedY);
						#endif
						VID_SetInk(sink);
						VID_SetPaper(spaper);
						VID_PatternFill(seedX - cursorX, seedY - cursorY, ptr[3]);
						VID_SetInk(cink);
						ptr += 4;
						break;
					}
					case 7:		// BLOCK
					{
						// Fills a rectangle with the current ink. The operands
						// give the top-left corner as a raw screen address in
						// the standard CPC layout, then the height in scan
						// lines and the width in bytes (4 mode-1 pixels each)
						uint16_t address = ptr[1] | (ptr[2] << 8);
						uint8_t  height  = ptr[3];
						uint8_t  width   = ptr[4];

						uint16_t offset = address - 0xC000;
						int y = (offset / 0x800) + ((offset % 0x800) / 80) * 8;
						int x = ((offset % 0x800) % 80) * 4;

						#ifdef TRACE_VECTOR
						TRACE("%02X BLOCK   %02X %02X %02X %02X   (%d,%d) %dx%d\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], x, y, width*4, height);
						#endif
						for (int row = 0; row < height; row++)
						{
							for (int cx = 0; cx < width*4; cx++)
								VID_DrawPixel(x + cx, y + row, ink);
						}
						ptr += 5;
						break;
					}
				}
			}
			return true;

		case DDB_MACHINE_SPECTRUM:
		case DDB_MACHINE_MSX:
			if (*ptr == 7)
			{
				// Empty subroutine (immediate RETURN). Balance the depth counter
				// bumped at entry -- otherwise every GOSUB into an empty image
				// leaks a level and, once the limit is hit, all further vector
				// graphics silently stop rendering for the rest of the session.
				depth--;
				return false;
			}
			for (;;)
			{
				switch (*ptr & 7)
				{
					case 0:	// PLOT
					{
						#ifdef TRACE_VECTOR
						TRACE("%02X PLOT    %02X %02X\n", ptr[0], ptr[1], ptr[2]);
						#endif

						VID_MoveTo(flipX ? scrMaxX - ptr[1] : ptr[1], flipY ? ptr[2] : scrMaxY - ptr[2]);
						switch (*ptr & 0x18)		// Inverse + Over flags
						{
							case 0x00: VID_DrawPixel(1); break;
							case 0x08: VID_DrawPixel(255); break;
							case 0x10: VID_DrawPixel(0); break;
							case 0x18: VID_SetAttribute(); break;
						}
						ptr += 3;
						break;
					}
					case 1: // LINE
					{
						uint8_t c = *ptr;
						int x, y;
						if ((c & 0x20) != 0)
						{
							x = (ptr[1] >> 4) & 0x0F;
							y = ptr[1] & 0x0F;

							#ifdef TRACE_VECTOR
							if ((c & 0x18) == 0x18)
								TRACE("%02X MOVE    %02X     ", ptr[0], ptr[1]);
							else
								TRACE("%02X LINE    %02X     ", ptr[0], ptr[1]);
							#endif
							ptr += 2;
						}
						else
						{
							x = ptr[1];
							y = ptr[2];

							#ifdef TRACE_VECTOR
							if ((c & 0x18) == 0x18)
								TRACE("%02X MOVE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
							else
								TRACE("%02X LINE    %02X %02X  ", ptr[0], ptr[1], ptr[2]);
							#endif
							ptr += 3;
						}
						if (flipX) x = -x;
						if (!flipY) y = -y;
						if (c & 0x40) x = -x;
						if (c & 0x80) y = -y;
						y = y * scale / 8;
						x = x * scale / 8;

						#ifdef TRACE_VECTOR
						TRACE("       (%d, %d)\n", x, y);
						#endif
						switch (c & 0x18)		// Inverse + Over flags
						{
							case 0x00: VID_DrawLine(x, y, 1); break;
							case 0x08: VID_DrawLine(x, y, 255); break;
							case 0x10: VID_DrawLine(x, y, 0); break;
							case 0x18: VID_MoveBy(x, y); break;
						}
						break;
					}
					case 2:
					{
						#if HAS_PAWS
						const bool pawsGfx = vectorGraphicsVersion == DDB_VERSION_PAWS;
						#else
						const bool pawsGfx = false;
						#endif
						// PAW's BLOCK opcode is exactly $12; other bit-4 values
						// are FILL/SHADE variants there
						if (pawsGfx ? (*ptr == 0x12) : ((*ptr & 0x30) == 0x10))	// BLOCK
						{
							uint8_t x0 = ptr[3];
							uint8_t y0 = ptr[4];
							uint8_t x1 = x0+ptr[2];
							uint8_t y1 = y0+ptr[1];

							#ifdef TRACE_VECTOR
							TRACE("%02X BLOCK   %02X %02X %02X %02X   (%d,%d)-(%d,%d)\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], x0, y0, x1, y1);
							#endif
							VID_AttributeFill(x0, y0, x1, y1);
							ptr += 5;
						}
						else if ((*ptr & 0x20) != 0)	// SHADE
						{
							#ifdef TRACE_VECTOR
							TRACE("%02X SHADE   %02X %02X %02X\n", ptr[0], ptr[1], ptr[2], ptr[3]);
							#endif

							int x = ptr[1];
							int y = ptr[2];
							uint8_t shade = ptr[3];
							if (pawsGfx)
							{
								// The seed displacement is scaled like a relative
								// move; opcode bit 4 complements the pattern
								x = x * scale / 8;
								y = y * scale / 8;
							}
							if (flipX) x = -x;
							if (!flipY) y = -y;
							if (*ptr & 0x40) x = -x;
							if (*ptr & 0x80) y = -y;
							VID_PatternFill(x, y, shade, pawsGfx && (*ptr & 0x10) != 0);
							ptr += 4;
						}
						else							// FILL
						{
							#ifdef TRACE_VECTOR
							TRACE("%02X FILL    %02X %02X\n", ptr[0], ptr[1], ptr[2]);
							#endif

							int x = ptr[1];
							int y = ptr[2];
							if (pawsGfx)
							{
								x = x * scale / 8;
								y = y * scale / 8;
							}
							if (flipX) x = -x;
							if (!flipY) y = -y;
							if (*ptr & 0x40) x = -x;
							if (*ptr & 0x80) y = -y;
							VID_PatternFill(x, y, -1);
							ptr += 3;
						}
						break;
					}
					case 3: // GOSUB
					{
						int scale = (*ptr >> 3) & 0x7;
						bool sflipX = (*ptr & 0x40) != 0;
						bool sflipY = (*ptr & 0x80) != 0;

						#ifdef TRACE_VECTOR
						TRACE("%02X GOSUB   %02X            (scale: %d, flipX: %d, flipY: %d)\n", ptr[0], ptr[1], scale, sflipX, sflipY);
						fflush(stdout);
						#endif

						if (flipX) sflipX = !sflipX;
						if (flipY) sflipY = !sflipY;
						DrawVectorSubroutine(ptr[1], scale, sflipX, sflipY);
						ptr += 2;
						break;
					}
					case 4: // TEXT
					{
						#ifdef TRACE_VECTOR
						TRACE("%02X TEXT    %02X %02X %02X\n", ptr[0], ptr[1], ptr[2], ptr[3]);
						#endif

						int code = ptr[1];
						int col  = ptr[2];
						int row  = ptr[3];
						const uint8_t* glyph = 0;

						#if HAS_PAWS
						if (vectorGraphicsVersion == DDB_VERSION_PAWS && code >= 32 && code < 128)
						{
							// PAW TEXT embeds its own charset selector in bits 7-5.
							// Font zero is the Spectrum ROM; installed database fonts
							// contain the 96 printable glyphs consecutively.
							uint8_t font = ptr[0] >> 5;
							if (font == 0)
								glyph = ZXSpectrumCharacterSet + (code - 32) * 8;
							else if (font <= vectorGraphicsRAM[0x9449])
							{
								uint16_t fonts = read16LE(vectorGraphicsRAM + 0x944A);
								glyph = vectorGraphicsRAM + fonts + (font - 1) * 768 + (code - 32) * 8;
							}
						}
						#endif

						bool over = false, inverse = false;
						#if HAS_PAWS
						if (vectorGraphicsVersion == DDB_VERSION_PAWS)
						{
							// PAW TEXT: bit 3 = OVER, bit 4 = INVERSE
							over    = (ptr[0] & 0x08) != 0;
							inverse = (ptr[0] & 0x10) != 0;
						}
						#endif
						if (glyph == 0)
							glyph = charset + (code << 3);
						VID_Draw8x8Glyph(col*8, row*8, glyph, ink, paper, over, inverse);

						ptr += 4;
						break;
					}
					case 5: // PAPER
					{
						#ifdef TRACE_VECTOR
						TRACE("%02X PAPER                 ", ptr[0]);
						#endif

						if (transparentColor == 8)
						{
							if (*ptr & 0x80)
							{
								#ifdef TRACE_VECTOR
								TRACE("(bright set to %d)\n", (*ptr >> 3) & 0x0F);
								#endif
								VID_SetBright((*ptr >> 3) & 0x0F);
							}
							else
							{
								#ifdef TRACE_VECTOR
								TRACE("(paper set to %d)\n", (*ptr >> 3) & 0x0F);
								#endif
								VID_SetPaper((*ptr >> 3) & 0x0F);
							}
						}
						else
						{
							#ifdef TRACE_VECTOR
							TRACE("(paper set to %d)\n", (*ptr >> 3) & 0x1F);
							#endif
							VID_SetPaper((*ptr >> 3) & 0x1F);
						}
						ptr++;
						break;
					}
					case 6: // INK
					{
						#ifdef TRACE_VECTOR
						TRACE("%02X INK                   ", ptr[0]);
						#endif

						if (transparentColor == 8)
						{
							if (*ptr & 0x80)
							{
								#ifdef TRACE_VECTOR
								TRACE("(flash set to %d)\n", (*ptr >> 3) & 0x0F);
								#endif
								VID_SetFlash((*ptr >> 3) & 0x0F);
							}
							else
							{
								#ifdef TRACE_VECTOR
								TRACE("(ink set to %d)\n", (*ptr >> 3) & 0x0F);
								#endif
								VID_SetInk((*ptr >> 3) & 0x0F);
							}
						}
						else
						{
							#ifdef TRACE_VECTOR
							TRACE("(ink set to %d)\n", (*ptr >> 3) & 0x1F);
							#endif
							VID_SetInk((*ptr >> 3) & 0x1F);
						}
						ptr++;
						break;
					}
					case 7:
					{
						#ifdef TRACE_VECTOR
						TRACE("%02X RETURN\n\n", ptr[0]);
						#endif

						depth--;
						return true;
					}
				}
			}
			return true;
	}
}

bool DDB_HasVectorPicture (uint8_t picno)
{
	if (picno >= count)
		return false;

	if (locattr)
	{
		// Check if picture is a picture (attribute bit 7 set) instead of a subroutine
		const uint8_t* att = vectorGraphicsRAM + locattr + picno;
		#if HAS_PAWS
		const uint8_t* page = 0;
		uint8_t local = 0;
		uint16_t pageTable = 0, pageAttributes = 0;
		if (vectorGraphicsVersion == DDB_VERSION_PAWS)
		{
			if (!ResolvePAWSPicture(picno, &page, &local, &pageTable, &pageAttributes)) return false;
			att = page + pageAttributes + local;
		}
		#endif
		return (*att & 0x80) != 0;
	}

	uint16_t offset = read16LE(vectorGraphicsRAM + table + picno * 2);
	const uint8_t *ptr = vectorGraphicsRAM + offset;

	switch (vectorGraphicsMachine)
	{
		default:
			return (*ptr != 2);
		case DDB_MACHINE_C64:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			return (*win == 0) && (*ptr & 0x07) != 0x02;
		}
		case DDB_MACHINE_CPC:
		{
			// Bit 7 marks a subroutine entry (inverted from the Spectrum
			// convention) in version 1 databases too: subroutines never take
			// part in location display, but an explicit draw still renders
			// them "as they stand"
			const uint8_t* win = vectorGraphicsRAM + windefs + 8*picno;
			if ((*win & 0x80) != 0)
				return false;
			return (*win != 0) && (*ptr != 0x40);
		}
		case DDB_MACHINE_MSX:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			return (*win & 0x80) != 0;
		}
		case DDB_MACHINE_SPECTRUM:
		{
			// The original interpreter validates only the count and the
			// windef location bit; entries without it fail the condact
			// (but still become the current picture, see CONDACT_PICTURE)
			const uint8_t* win = vectorGraphicsRAM + windefs + 5*picno;
			return (*win & 0x80) != 0;
		}
	}
}

// True if the picture takes part in location display: unlike
// DDB_HasVectorPicture, entries with an empty drawstring still count,
// because their window setup (palette, ink, clear) must be applied
bool DDB_HasVectorWindow (uint8_t picno)
{
	if (picno >= count)
		return false;

	if (locattr)
	{
		const uint8_t* att = vectorGraphicsRAM + locattr + picno;
		#if HAS_PAWS
		const uint8_t* page = 0;
		uint8_t local = 0;
		uint16_t pageTable = 0, pageAttributes = 0;
		if (vectorGraphicsVersion == DDB_VERSION_PAWS)
		{
			if (!ResolvePAWSPicture(picno, &page, &local, &pageTable, &pageAttributes)) return false;
			att = page + pageAttributes + local;
		}
		#endif
		return (*att & 0x80) != 0;
	}

	switch (vectorGraphicsMachine)
	{
		default:
			return true;
		case DDB_MACHINE_C64:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			return *win == 0;
		}
		case DDB_MACHINE_CPC:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 8*picno;
			if ((*win & 0x80) != 0)
				return false;
			return *win != 0;
		}
		case DDB_MACHINE_MSX:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			return (*win & 0x80) != 0;
		}
		case DDB_MACHINE_SPECTRUM:
		{
			if (windefs == 0)
				return true;
			const uint8_t* win = vectorGraphicsRAM + windefs + 5*picno;
			return (*win & 0x80) != 0;
		}
	}
}

// The window rectangle a picture display applies (pixel coordinates): the
// original interpreters redefine the current text window to the picture's
// window from the graphics database, so a later CLS clears the picture
// area rather than the whole screen. Returns false for subroutine entries
// and machines where this does not apply.
bool DDB_GetVectorPictureWindow (uint8_t picno, int* x, int* y, int* w, int* h)
{
	if (vectorGraphicsRAM == 0 || picno >= count)
		return false;
	switch (vectorGraphicsMachine)
	{
		case DDB_MACHINE_CPC:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 8*picno;
			if ((win[0] & 0x80) != 0 || win[0] == 0)
				return false;
			*x = win[5] * 8;
			*y = win[4] * 8;
			*w = win[7] * 8;
			*h = win[6] * 8;
			return true;
		}
		case DDB_MACHINE_MSX:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			if ((win[0] & 0x80) != 0x80)
				return false;
			// Columns come in 6-pixel units; round out to whole cells like
			// the clear does
			int col8 = (win[3]*6) / 8;
			int width8 = (win[3]*6 + win[5]*6 + 7) / 8 - col8;
			*x = col8 * 8;
			*y = win[2] * 8;
			*w = width8 * 8;
			*h = win[4] * 8;
			return true;
		}
		case DDB_MACHINE_SPECTRUM:
		{
			if (windefs == 0)
				return false;
			const uint8_t* win = vectorGraphicsRAM + windefs + 5*picno;
			if ((win[0] & 0x80) != 0x80)
				return false;
			int col8 = (win[2]*6) / 8;
			int width8 = (win[2]*6 + win[4]*6 + 7) / 8 - col8;
			*x = col8 * 8;
			*y = win[1] * 8;
			*w = width8 * 8;
			*h = win[3] * 8;
			return true;
		}
		default:
			return false;
	}
}

// Logical-to-real colour conversion table from the graphics database,
// stored after a one byte prefix (global bright flag on Spectrum,
// border colour on MSX/C64); NULL when no graphics database is loaded.
// Spectrum tables have 8 entries, MSX and C64 tables have 16.
const uint8_t* DDB_GetVectorInkMap ()
{
	if (vectorGraphicsRAM == 0 || coltab == 0)
		return 0;
	switch (vectorGraphicsMachine)
	{
		case DDB_MACHINE_SPECTRUM:
		case DDB_MACHINE_MSX:
		case DDB_MACHINE_C64:
			return vectorGraphicsRAM + coltab + 1;
		default:
			return 0;
	}
}

bool DDB_ExecuteVectorPicture (uint8_t picno)
{
	if (picno >= count)
	{
		printf("DDB_ExecuteVectorPicture: picno %d out of range (0-%d)\n", picno, count-1);
		return false;
	}

	// This is the top of the recursion, so the depth counter must start at 0.
	// Reset it defensively so any stray leak in a subroutine cannot accumulate
	// across pictures and permanently disable graphics.
	depth = 0;

	#ifdef TRACE_VECTOR
	TRACE("\n===== TOP-LEVEL PICTURE %d =====\n", picno);
	#endif

	winMinX = 0;
	winMinY = 0;
	winMaxX = scrMaxX;
	winMaxY = scrMaxY;

	#ifdef STEP_CAPTURE
	stepPicno = picno;
	stepCount = 0;
	#endif

	switch (vectorGraphicsMachine)
	{
		case DDB_MACHINE_C64:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			// A zero flag byte marks a location picture; bit 7 marks
			// a subroutine (both versions)
			if (win[0] == 0)
			{
				int row    = win[2];
				int col    = win[3];
				int height = win[4];
				int width  = win[5];

				// The window color byte holds ink in the high nibble
				VID_SetInk((win[1] >> 4) & 0x0F);
				VID_SetPaper(win[1] & 0x0F);
				VID_Clear(col*8, row*8, width*8, height*8, 0);
				if (vectorGraphicsVersion != DDB_VERSION_1)
				{
					// Version 2 window clears also paint the cell colors
					VID_AttributeFill(col, row, col+width-1, row+height-1);
				}
			}
			// Overlay entries (nonzero window byte) are drawn as they
			// stand: no palette, window or clear, composing over the
			// current screen
			cursorX = 0;
			cursorY = scrMaxY;

			bool result = DrawVectorSubroutine(picno, 0, false, false);
			// VID_SaveDebugBitmap();
			return result;
		}

		case DDB_MACHINE_CPC:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 8*picno;
			if ((win[0] & 0x80) != 0)
			{
				// A subroutine entry is drawn "as it stands": no palette,
				// window or clear is applied, the drawing composes over the
				// current screen state (e.g. the intro portrait)
				cursorX = 0;
				cursorY = scrMaxY;
				cpcUserX = 0;
				cpcUserY = 0;
				return DrawVectorSubroutine(picno, 0, false, false);
			}
			if (win[0] == 0)
			{
				// No window: nothing to draw
				return false;
			}
			else
			{
				if (win[1] < 27) VID_SetPaletteColor32(1, CPC_Colors[win[1] & 0x1F]);
				if (win[2] < 27) VID_SetPaletteColor32(2, CPC_Colors[win[2] & 0x1F]);
				if (win[3] < 27) VID_SetPaletteColor32(3, CPC_Colors[win[3] & 0x1F]);

				int row    = win[4];
				int col    = win[5];
				int height = win[6];
				int width  = win[7];
VID_Clear(col*8, row*8, width*8, height*8, 0);
				winMinX = col*8;
				winMinY = row*8;
				winMaxX = col*8 + width*8 - 1;
				winMaxY = row*8 + height*8 - 1;
				if (winMaxX > scrMaxX) winMaxX = scrMaxX;
				if (winMaxY > scrMaxY) winMaxY = scrMaxY;
			}
			cursorX = 0;
			cursorY = scrMaxY;
			cpcUserX = 0;
			cpcUserY = 0;
			ink     = 1;

			bool result = DrawVectorSubroutine(picno, 0, false, false);
			// VID_SaveDebugBitmap();
			return result;
		}

		case DDB_MACHINE_MSX:
		{
			const uint8_t* win = vectorGraphicsRAM + windefs + 6*picno;
			if ((win[0] & 0x80) == 0x80)
			{
				VID_SetInk(win[1] & 0x0F);
				VID_SetPaper((win[1] >> 4) & 0x0F);
				VID_SetBright(0);
				VID_SetFlash(0);

				int row = win[2];
				int col = win[3];
				int height = win[4];
				int width = win[5];
				// The window is defined in 6 pixel columns, but attributes
				// work in 8x8 cells; clear whole cells so no stale pixels
				// survive at the right edge of the window
				int col8 = (col*6) / 8;
				int width8 = (col*6 + width*6 + 7) / 8 - col8;
				VID_Clear(col8*8, row*8, width8*8, height*8, 0);
				VID_AttributeFill(col8, row, col8+width8-1, row+height-1);
			}
			// Entries without the location bit are overlays drawn as they
			// stand: no palette, window or clear, composing over the
			// current screen
			cursorX = 0;
			cursorY = scrMaxY;

			bool result = DrawVectorSubroutine(picno, 0, false, false);
			// VID_SaveDebugBitmap();
			return result;
		}

		case DDB_MACHINE_SPECTRUM:
		{
			if (windefs != 0)
			{
				const uint8_t* win = vectorGraphicsRAM + windefs + 5*picno;
				if ((win[0] & 0x80) == 0x80)
				{
					VID_SetInk(win[0] & 0x07);
					VID_SetPaper((win[0] >> 3) & 0x07);
					VID_SetBright(0);
					VID_SetFlash(0);

					int row = win[1];
					int col = win[2];
					int height = win[3];
					int width = win[4];
					// The window is defined in 6 pixel columns, but attributes
					// work in 8x8 cells; clear whole cells so no stale pixels
					// survive at the right edge of the window
					int col8 = (col*6) / 8;
					int width8 = (col*6 + width*6 + 7) / 8 - col8;
					VID_Clear(col8*8, row*8, width8*8, height*8, 0);
					VID_AttributeFill(col8, row, col8+width8-1, row+height-1);
				}
			}
			else if (locattr != 0)
			{
				uint8_t attr = vectorGraphicsRAM[locattr + picno];
				#if HAS_PAWS
				if (vectorGraphicsVersion == DDB_VERSION_PAWS)
				{
					const uint8_t* page = 0;
					uint8_t local = 0;
					uint16_t pageTable = 0, pageAttributes = 0;
					if (!ResolvePAWSPicture(picno, &page, &local, &pageTable, &pageAttributes)) return false;
					attr = page[pageAttributes + local];
				}
				#endif
				if ((attr & 0x80) == 0x80)
				{
					VID_SetInk(attr & 0x07);
					VID_SetPaper((attr >> 3) & 0x07);
					VID_SetBright(0);
					VID_SetFlash(0);
				}
			}

			cursorX = 0;
			cursorY = scrMaxY;

			bool result = DrawVectorSubroutine(picno, 0, false, false);
			// VID_SaveDebugBitmap();
			return result;
		}

		default:
			break;
	}

	return false;
}

bool DDB_HasVectorDatabase()
{
	return vectorGraphicsRAM != 0;
}

// True if the picture number is inside the vector database's table: the
// original interpreters make a failing PICTURE condact update the current
// picture anyway as long as the number is in range
bool DDB_VectorPictureInRange (uint8_t picno)
{
	return vectorGraphicsRAM != 0 && picno < count;
}

bool DDB_WriteVectorDatabase(const char* filename)
{
	if (vectorGraphicsRAM == 0)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	File* file = File_Create(filename);
	if (file == 0)
	{
		DDB_SetError(DDB_ERROR_CREATING_FILE);
		return false;
	}

	TRACE("Writing vector RAM 0x%04X to 0x%04X\n", start, ending);
	if (File_Write(file, vectorGraphicsRAM + start, ending - start + 1) != ending - start + 1)
	{
		File_Close(file);
		DDB_SetError(DDB_ERROR_WRITING_FILE);
		return false;
	}
	File_Close(file);
	return true;
}

#if HAS_PAWS

static const uint8_t systemUDGs[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00,
	0XF0, 0XF0, 0XF0, 0XF0, 0X00, 0X00, 0X00, 0X00,
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F,
	0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F,
	0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0,
	0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0,
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

bool DDB_LoadPAWSGraphics (const uint8_t* data, size_t size)
{
	vectorGraphicsMachine = DDB_MACHINE_SPECTRUM;
	vectorGraphicsVersion = DDB_VERSION_PAWS;
	vectorGraphicsRAM     = data;
	vectorGraphicsSize    = size;
	pixelMode             = false;

	start   = read16LE(data + 65533);		// FFFD
	table   = read16LE(data + 65521);		// FFF1
	locattr = read16LE(data + 65523);		// FFF3
	windefs = 0;
	chset   = 0;
	coltab  = 0;
	ending  = 0;
	count   = size > 0x10000 ? data[0x9445] : (locattr - table) / 2;
	ending  = 0xFFFF;

	if (start != 0x9300 || table < locattr - 512 || table >= locattr)
		return false;

	scrMaxX          = 255;
	scrMaxY          = 175;
	stride           = 32;
	transparentColor = 8;
	udgs             = data + start;
	shades           = data + start + 19*8;
	mixShades        = true;

	defaultInk       = data[start + 39*8]     & 0x0F;
	defaultBorder    = data[start + 39*8 + 1] & 0x0F;
	defaultPaper     = data[start + 39*8 + 2] & 0x0F;

	VID_SetPaper(defaultPaper);
	VID_SetInk(defaultInk);

	MemCopy(charset + 0x80*8, systemUDGs, 16*8);
	MemCopy(charset + 0x90*8, udgs, 21*8);
	return true;
}

void DDB_LoadUDGs()
{
	if (udgs != 0)
	{
		MemCopy(charset + 0x80*8, systemUDGs, 16*8);
		MemCopy(charset + 0x90*8, udgs, 21*8);
	}
}

#endif

bool DDB_LoadVectorGraphics (DDB_Machine target, DDB_Version version, const uint8_t* data, size_t size)
{
	vectorGraphicsMachine = target;
	vectorGraphicsVersion = version;
	vectorGraphicsSize = size;
	pixelMode = false;

	// A failed validation must not leave a half-initialized database behind:
	// DDB_HasVectorDatabase() would report one with garbage bounds
	vectorGraphicsRAM = 0;

	mixShades = false;

	switch (target)
	{
		case DDB_MACHINE_C64:
		{
			if (size < 65536)
				return false;

			start   = read16LE(data + 0xCBEF);
			table   = read16LE(data + 0xCBF1);
			windefs = read16LE(data + 0xCBF3);
			unknown = read16LE(data + 0xCBF5);
			chset   = read16LE(data + 0xCBF7);;
			coltab  = read16LE(data + 0xCBF9);
			ending  = read16LE(data + 0xCBFB);
			count   = data[0xCBFD];

			if (table < start || chset < start || coltab < start)
				return false;
			if (windefs + 6*count > size)
				return false;
			if (data[windefs + 6*count] != 0xFF)
				return false;
			if (ending != 0xFFFF)
				return false;

			ending  = 0xCBFF;
			scrMaxX = 319;
			scrMaxY = 199;
			stride  = 40;
			transparentColor = 16;

			// Version 2 databases use a 192 line drawing area
			if (version != DDB_VERSION_1)
				scrMaxY = 191;
			vectorGraphicsRAM = data;
			VID_SetCharset(data + chset);
			shades = vectorGraphicsRAM + chset;

			return true;
		}

		case DDB_MACHINE_CPC:
		{
			if (size < 65536)
				return false;

			pixelMode = true;

			start   = read16LE(data + 0x9DEF);
			table   = read16LE(data + 0x9DF1);
			windefs = read16LE(data + 0x9DF3);
			unknown = read16LE(data + 0x9DF5);
			chset   = 0x9E00;
			coltab  = read16LE(data + 0x9DF9);
			ending  = read16LE(data + 0x9DFB);
			count   = data[0x9DFD];

			if (table < start || chset < start || coltab < start)
				return false;
			if (windefs + 8*count > size)
				return false;
			if (data[windefs + 8*count] != 0xFF)
				return false;
			if (ending != 0xFFFF)
				return false;

			ending  = chset + 2048 - 1;
			scrMaxX = 319;
			scrMaxY = 183;
			vectorGraphicsRAM = data;
			VID_SetCharset(data + chset);
			shades = vectorGraphicsRAM + chset;

			return true;
		}

		case DDB_MACHINE_MSX:
		{
			if (size < 65536)
				return false;


			spare   = read16LE(data + 0xAFED);
			start   = read16LE(data + 0xAFEF);
			table   = read16LE(data + 0xAFF1);
			windefs = read16LE(data + 0xAFF3);
			unknown = read16LE(data + 0xAFF5);
			chset   = read16LE(data + 0xAFF7);
			coltab  = read16LE(data + 0xAFF9);
			ending  = read16LE(data + 0xAFFB);
			count   = data[0xAFFD];

			if (table < start || chset < start || coltab < start)
				return false;
			if (windefs + 6*count > size)
				return false;
			if (data[windefs + 6*count] != 0xFF)
				return false;
			if (ending != 0xFFFF)
				return false;

			// The footer lives at 0xAFED-0xAFFF: include it so a written
			// .mdg file is a valid, reloadable database
			ending  = 0xAFFF;
			scrMaxX = 255;
			scrMaxY = 175;
			stride  = 32;
			transparentColor = 16;

			vectorGraphicsRAM = data;
			VID_SetCharset(data + chset);
			shades = vectorGraphicsRAM + chset;

			return true;
		}

		case DDB_MACHINE_SPECTRUM:
		{
			if (size < 65536)
				return false;


			spare   = read16LE(data + 0xFFED);
			start   = read16LE(data + 0xFFEF);
			table   = read16LE(data + 0xFFF1);
			windefs = read16LE(data + 0xFFF3);
			unknown = read16LE(data + 0xFFF5);
			chset   = read16LE(data + 0xFFF7);
			coltab  = read16LE(data + 0xFFF9);
			ending  = read16LE(data + 0xFFFB);
			count   = data[0xFFFD];

			if (table < start || chset < start || coltab < start)
				return false;
			if (windefs + 5*count > size)
				return false;
			if (data[windefs + 5*count] != 0xFF)
			{
				// This is mangled in Templos
				// The reason is not understood yet (?)
				windefs = (windefs & 0xFF) | (data[0xFFF2] << 8);
				if (windefs + 5*count > size)
					return false;
				if (data[windefs + 5*count] != 0xFF)
					return false;
			}

			ending  = 0xFFFF;
			scrMaxX = 255;
			scrMaxY = 175;
			stride  = 32;
			transparentColor = 8;

			vectorGraphicsRAM = data;
			VID_SetCharset(data + chset);
			shades = vectorGraphicsRAM + chset;

			return true;
		}

		default:
			return false;
	}
}

#endif
