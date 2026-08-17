#include <ddb_wintext.h>

#if HAS_WINDOWTEXT

#include <os_lib.h>

// The visible screen text is kept as a flat pool of spans, exactly as drawn.
// A span dies when another span overwrites its position, when a clear covers
// it, or when a scroll pushes it out of the scrolled region.

// Generous: 8-bit ports draw decorative frames as hundreds of single-tile
// spans, which must not evict the menu texts (desktop-only memory)
#define MAX_SPANS     2048
#define MAX_SPAN_TEXT 64

struct WinTextSpan
{
	int16_t x, y;
	int16_t width;
	uint8_t length;
	char    text[MAX_SPAN_TEXT];
};

static WinTextSpan spans[MAX_SPANS];
static int spanCount = 0;

static void RemoveSpan(int n)
{
	spans[n] = spans[--spanCount];
}

void WinText_AddSpan(int x, int y, const uint8_t* text, int length, int pixelWidth)
{
	// Drop anything the new span overwrites
	for (int n = spanCount - 1; n >= 0; n--)
	{
		if (spans[n].y == y && spans[n].x < x + pixelWidth && x < spans[n].x + spans[n].width)
			RemoveSpan(n);
	}
	if (length <= 0)
		return;
	if (spanCount == MAX_SPANS)
		RemoveSpan(0);

	WinTextSpan* s = &spans[spanCount++];
	s->x = (int16_t)x;
	s->y = (int16_t)y;
	s->width = (int16_t)pixelWidth;
	if (length > MAX_SPAN_TEXT)
		length = MAX_SPAN_TEXT;
	s->length = (uint8_t)length;
	for (int n = 0; n < length; n++)
		s->text[n] = (char)(text[n] & 0x7F);
}

void WinText_Scroll(int x, int y, int w, int h, int lines)
{
	for (int n = spanCount - 1; n >= 0; n--)
	{
		WinTextSpan* s = &spans[n];
		if (s->x >= x + w || s->x + s->width <= x || s->y < y || s->y >= y + h)
			continue;
		s->y = (int16_t)(s->y - lines);
		if (s->y < y)
			RemoveSpan(n);
	}
}

void WinText_ClearRect(int x, int y, int w, int h)
{
	// A single-row, narrow clear is the erase half of a redraw cycle (menus
	// repaint each item row as clear+print every frame): keep the old span,
	// the reprint replaces it via the AddSpan overwrite rule. Dropping it
	// here makes name resolution phase-locked against redraws (the poll
	// always lands between the clear and the reprint of the same row).
	if (h <= 8 && w <= 100)
		return;
	for (int n = spanCount - 1; n >= 0; n--)
	{
		WinTextSpan* s = &spans[n];
		if (s->x < x + w && x < s->x + s->width && s->y >= y && s->y < y + h)
			RemoveSpan(n);
	}
}

static char UpperAscii(char c)
{
	// DAAD Spanish characters (codes 16-31) fold to their base letter so a
	// plain-ASCII script name matches accented on-screen text ("SULFIDRICO"
	// matches SULF{23}DRICO)
	static const char spanishBase[16] = {
		'O', '!', '?', '<', '>', 'A', 'E', 'I', 'O', 'U', 'N', 'N', 'C', 'C', 'U', 'U'
	};
	uint8_t u = (uint8_t)c;
	if (u >= 16 && u <= 31)
		return spanishBase[u - 16];
	return c >= 'a' && c <= 'z' ? (char)(c - 32) : c;
}

// Spans on the same row belong to the same text segment only when they are
// horizontally adjacent; a wide gap means a different window (menus sit side
// by side with the main text window)
#define SEGMENT_GAP 12

// True if another span on the same row ends just before this one starts
static bool HasCloseLeftNeighbor(int n)
{
	for (int m = 0; m < spanCount; m++)
	{
		if (m == n || spans[m].y != spans[n].y || spans[m].x > spans[n].x)
			continue;
		if (spans[m].x + spans[m].width + SEGMENT_GAP > spans[n].x)
			return true;
	}
	return false;
}

// Builds the text of the segment starting at span n: adjacent spans on the
// same row, left to right. Returns its length.
static int BuildSegment(int n, char* buffer, int bufferSize)
{
	int length = 0;
	int current = n;
	int end = spans[n].x;
	for (;;)
	{
		if (length > 0 && spans[current].x - end >= 3 && length < bufferSize - 1)
			buffer[length++] = ' ';
		for (int c = 0; c < spans[current].length && length < bufferSize - 1; c++)
			buffer[length++] = spans[current].text[c];
		end = spans[current].x + spans[current].width;

		int next = -1;
		for (int m = 0; m < spanCount; m++)
		{
			if (spans[m].y != spans[current].y || spans[m].x < end || spans[m].x > end + SEGMENT_GAP)
				continue;
			if (next < 0 || spans[m].x < spans[next].x)
				next = m;
		}
		if (next < 0)
			break;
		current = next;
	}
	buffer[length] = 0;
	return length;
}

// A menu line as drawn by the games: an option digit, a space, the option text
static bool IsMenuLine(const char* line)
{
	return line[0] >= '1' && line[0] <= '9' && (line[1] == ' ' || line[1] == 0);
}

char WinText_FindMenuOption(const char* name, int nameLength)
{
	char line[128];
	for (int n = 0; n < spanCount; n++)
	{
		if (HasCloseLeftNeighbor(n))
			continue;
		int length = BuildSegment(n, line, sizeof(line));
		if (!IsMenuLine(line))
			continue;
		// Compare against the option text (after the digit), anchored at its
		// start: substring matching is wrong here ("ESTE" is contained in
		// "OESTE" and would select the wrong exit)
		int optLength = length - 2;
		while (optLength > 0 && line[2 + optLength - 1] == ' ')
			optLength--;
		int c = 0;
		while (c < nameLength && c < optLength && UpperAscii(line[2 + c]) == UpperAscii(name[c]))
			c++;
		if (c == nameLength && (c == optLength || line[2 + c] == ' '))
		{
			// The name is the option text, or its first word(s)
			return line[0];
		}
		if (c == optLength && optLength >= 3 && optLength < nameLength)
		{
			// The window was too narrow: the visible (truncated) option
			// text is a prefix of the requested name
			return line[0];
		}
	}

	// Bar-cursor menus (8-bit style): bare item names stacked one per
	// 8-pixel row with an animated selection bar (]]] /// \\\\\\ ---) whose
	// home row sits just below the 8-item page, and a digit key selects by
	// ordinal within the page. The ordinal follows from the row distance to
	// the bar's home position, which survives items whose span tracking was
	// lost to redraws.
	int barX = -1, barY = -1;
	for (int b = 0; b < spanCount && barX < 0; b++)
	{
		char g = spans[b].text[0];
		bool isBar = spans[b].length >= 2 &&
			(g == ']' || g == '\\' || g == '/' || g == '-' || g == '|');
		for (int c = 0; c < spans[b].length && isBar; c++)
			isBar = spans[b].text[c] == g;
		if (isBar)
		{
			barX = spans[b].x;
			barY = spans[b].y;
		}
	}
	if (barX >= 0)
	{
		int best = -1;
		for (int n = 0; n < spanCount; n++)
		{
			if (HasCloseLeftNeighbor(n))
				continue;
			// Items live above the bar's home row, within one page
			int ordinal = (spans[n].y - barY) / 8 + 9;
			if (ordinal < 1 || ordinal > 8)
				continue;
			int length = BuildSegment(n, line, sizeof(line));
			// Skip window-frame tiles merged in front of the item text
			int start = 0;
			while (start < length && !((line[start] >= 'A' && line[start] <= 'Z') ||
			                           (line[start] >= '0' && line[start] <= '9')))
				start++;
			int optLength = length - start;
			while (optLength > 0 && line[start + optLength - 1] == ' ')
				optLength--;
			if (optLength <= 0)
				continue;
			int c = 0;
			while (c < nameLength && c < optLength && UpperAscii(line[start + c]) == UpperAscii(name[c]))
				c++;
			// A frame tile can follow the item text with no gap: any non-word
			// character ends the option name (hyphens and '+' are part of
			// object names like UA-E-VEECO and XI-KA+)
			char next = c < optLength ? line[start + c] : ' ';
			bool boundary = !((next >= 'A' && next <= 'Z') || (next >= 'a' && next <= 'z') ||
			                  (next >= '0' && next <= '9') || next == '-' || next == '+');
			bool match = (c == nameLength && (c == optLength || boundary)) ||
			             (c == optLength && optLength >= 3 && optLength < nameLength);
			if (match && (best < 0 || spans[n].x < spans[best].x))
				best = n;
		}
		if (best >= 0)
			return (char)('0' + (spans[best].y - barY) / 8 + 9);
	}
	return 0;
}

bool WinText_FindText(const char* text, int textLength)
{
	char line[128];
	if (textLength <= 0)
		return false;
	for (int n = 0; n < spanCount; n++)
	{
		if (HasCloseLeftNeighbor(n))
			continue;
		int length = BuildSegment(n, line, sizeof(line));
		for (int start = 0; start + textLength <= length; start++)
		{
			int c = 0;
			while (c < textLength && UpperAscii(line[start + c]) == UpperAscii(text[c]))
				c++;
			if (c == textLength)
				return true;
		}
	}
	return false;
}

int WinText_ListMenuOptions(char* buffer, int bufferSize)
{
	char line[128];
	int written = 0, found = 0;
	for (int n = 0; n < spanCount; n++)
	{
		if (HasCloseLeftNeighbor(n))
			continue;
		int length = BuildSegment(n, line, sizeof(line));
		found++;
		{
			char coords[24];
			char* out = coords;
			*out++ = '(';
			out = LongToChar(spans[n].x, out, 10);
			*out++ = ',';
			out = LongToChar(spans[n].y, out, 10);
			*out++ = ')';
			*out++ = ' ';
			for (char* p2 = coords; p2 < out && written < bufferSize - 2; p2++)
				buffer[written++] = *p2;
		}
		for (int c = 0; c < length && written < bufferSize - 2; c++)
			buffer[written++] = line[c];
		if (written < bufferSize - 1)
			buffer[written++] = '\n';
	}
	buffer[written] = 0;
	return found;
}

#endif
