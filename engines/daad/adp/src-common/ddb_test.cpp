#include <ddb_test.h>

#if HAS_TESTMODE

#include <ddb.h>
#include <ddb_wintext.h>
#include <ddb_vid.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

static const char* input = 0;
static const char* inputBegin = 0;
static const char* inputEnd = 0;
static DDB_ScreenshotCallback screenshotCallback = 0;
static bool inputError = false;
static const char* inputErrorMessage = 0;
static bool interactiveInput = false;
static bool inputStopped = false;
static bool inputBarrier = false;
static int partSelection = 0;
static char partScreenshotFile[FILE_MAX_PATH] = { 0 };
enum WaitKey { WaitKey_None, WaitKey_Explicit, WaitKey_Implicit, WaitKey_Synthetic };
static WaitKey waitKey = WaitKey_None;
static uint8_t waitKeyValue = 0x0D;
static int menuKeyPos = -1;

static bool IsDirective(const char* directive)
{
	size_t length = StrLen(directive);
	return input != 0 && input + length <= inputEnd &&
		MemComp((void*)input, directive, length) == 0 &&
		(input + length == inputEnd || input[length] == '\r' || input[length] == '\n');
}

// "@waitkey" presses Return only when the game waits for a key (a cutscene
// ANYKEY, a pause, a more prompt); menu INKEY polls do not see it. The
// mirror image of @menu keys, which only INKEY sees.
static bool IsWaitKeyDirective()
{
	return IsDirective("@waitkey");
}

static bool IsKeyDirective()
{
	if (IsDirective("@key"))
		return true;
	// "@key X" presses a single literal key (e.g. "@key 1" for menu-driven
	// games) instead of the Return implied by a bare "@key"
	return input != 0 && input + 5 <= inputEnd && MemComp((void*)input, "@key ", 5) == 0;
}

// Value of the key pressed by the directive at the input cursor: the argument
// character of "@key X", or Return for a bare "@key". "space" and "enter" are
// accepted as named arguments.
static uint8_t KeyArgumentValue(const char* arg)
{
	if (arg >= inputEnd || *arg == '\r' || *arg == '\n')
		return 0x0D;
	arg++;
	const char* end = arg;
	while (end < inputEnd && *end != '\r' && *end != '\n') end++;
	size_t length = (size_t)(end - arg);
	if (length == 1)
		return (uint8_t)*arg;
	if (length == 5 && MemComp((void*)arg, "space", 5) == 0)
		return 0x20;
	if (length == 5 && MemComp((void*)arg, "enter", 5) == 0)
		return 0x0D;
	inputError = true;
	inputErrorMessage = "invalid @key argument";
	return 0x0D;
}

static uint8_t KeyDirectiveValue()
{
	return KeyArgumentValue(input + 4);
}

// "@inkey [X]" is the dual of @waitkey: only a direct key read (INKEY) consumes
// it, and any wait for a key reached while it is pending gets a synthetic
// Return without consuming the script. This feeds animations that poll INKEY
// in a loop (e.g. an ink-cycling ending sequence) without having to count the
// story pauses in front of them.
static bool IsInkeyDirective()
{
	if (IsDirective("@inkey"))
		return true;
	return input != 0 && input + 7 <= inputEnd && MemComp((void*)input, "@inkey ", 7) == 0;
}

static uint8_t InkeyDirectiveValue()
{
	return KeyArgumentValue(input + 6);
}

static int waitTextTicks = 0;
static void ConsumeLine();

// "@waittext TEXT" feeds no input at all until TEXT is visible on screen:
// lets a script sit through a self-running sequence of unknown length (e.g.
// the autopilot approach after a hyperspace jump) without pressing keys that
// the game would interpret as commands.
static bool IsWaitTextDirective()
{
	return input != 0 && input + 10 <= inputEnd && MemComp((void*)input, "@waittext ", 10) == 0;
}

// True while the awaited text is still absent; consumes the directive (and
// resets the stall counter) the moment the text shows up
static bool WaitTextPending()
{
	if (!IsWaitTextDirective())
		return false;
	const char* arg = input + 10;
	const char* end = arg;
	while (end < inputEnd && *end != '\r' && *end != '\n') end++;
	while (end > arg && end[-1] == ' ') end--;
	if (end > arg && WinText_FindText(arg, (int)(end - arg)))
	{
		ConsumeLine();
		waitTextTicks = 0;
		return false;
	}
	// Generous: a long self-running sequence (an autopilot approach) polls
	// thousands of times; the scenario timeout is the hard backstop
	if (++waitTextTicks > 100000)
	{
		static char message[256];
		char* out = message;
		const char* prefix = "@waittext text never appeared: ";
		while (*prefix) *out++ = *prefix++;
		for (const char* p = arg; p < end && out < message + sizeof(message) - 1; p++) *out++ = *p;
		{
			const char* mid = "; visible text:\n";
			while (*mid && out < message + sizeof(message) - 1) *out++ = *mid++;
			char items[120];
			WinText_ListMenuOptions(items, sizeof(items));
			for (const char* p = items; *p && out < message + sizeof(message) - 1; p++) *out++ = *p;
		}
		*out = 0;
		inputError = true;
		inputErrorMessage = message;
		VID_Quit();
	}
	return true;
}

static void ConsumeLine()
{
	while (input < inputEnd && *input != '\r' && *input != '\n') input++;
	if (input < inputEnd && *input == '\r') input++;
	if (input < inputEnd && *input == '\n') input++;
}

// "@menu 112" presses each character of the argument as a menu keystroke.
// Menu keys are only consumed by direct key reads (INKEY and friends); any
// intervening wait for a key (a "more..." prompt, ANYKEY, a pause) receives
// a synthetic Return instead, so it can't swallow the queued selections.
static bool IsMenuDirective()
{
	return input != 0 && input + 6 <= inputEnd && MemComp((void*)input, "@menu ", 6) == 0;
}

static uint8_t menuResolvedKey = 0;
static int     menuRetries = 0;
static int     menuWaitTicks = 0;
static uint8_t mutedWindows = 0;
static int     inkeyWaitTicks = 0;

// "@chance fail" / "@chance pass" force every CHANCE condact to fail/succeed
// until "@chance random" restores normal rolls. The forced roll still
// consumes its random number so the stream stays aligned. An optional operand restricts the rule to matching CHANCE condacts
// only: "@chance fail [74]" affects just CHANCE [74] (indirect through flag
// 74, e.g. an escalating random-encounter roll) and leaves every other roll
// alone, so game logic that runs on CHANCE (autopilot drift, animations)
// keeps working. "@chance fail 25" matches the literal operand instead.
enum ChanceMode { Chance_Random, Chance_Fail, Chance_Pass };
enum ChanceFilter { ChanceFilter_All, ChanceFilter_Literal, ChanceFilter_Indirect };
struct ChanceRule
{
	ChanceMode   mode;
	ChanceFilter filter;
	uint8_t      operand;
};
#define MAX_CHANCE_RULES 8
static ChanceRule chanceRules[MAX_CHANCE_RULES];
static int        chanceRuleCount = 0;

bool DDB_TestChanceForced(bool* result, bool indirect, uint8_t rawParam)
{
	if (!DDB_TestIsActive())
		return false;
	for (int n = 0; n < chanceRuleCount; n++)
	{
		const ChanceRule* rule = &chanceRules[n];
		if (rule->filter == ChanceFilter_Literal && (indirect || rawParam != rule->operand))
			continue;
		if (rule->filter == ChanceFilter_Indirect && (!indirect || rawParam != rule->operand))
			continue;
		*result = rule->mode == Chance_Pass;
		return true;
	}
	return false;
}

// "@mute 2" stops text printed to the given window (0-7) from reaching the
// transcript; "@unmute 2" restores it. Menu-driven games redraw their menu
// windows constantly, drowning the story text in the transcript.
bool DDB_TestWindowMuted(int winno)
{
	return (mutedWindows >> (winno & 7)) & 1;
}

// The token at the @menu cursor: literal keys pass through one character at a
// time; anything longer that isn't a number is a menu option name, resolved
// against the text currently visible on screen
static void MenuToken(const char** start, const char** end)
{
	const char* p = input + (menuKeyPos < 0 ? 6 : menuKeyPos);
	while (p < inputEnd && *p == ' ') p++;
	*start = p;
	while (p < inputEnd && *p != ' ' && *p != '\r' && *p != '\n') p++;
	*end = p;
}

static bool MenuTokenIsName(const char* start, const char* end)
{
	if (end - start < 2)
		return false;
	// Tokens made only of menu control keys (digits, navigation letters) are
	// literal key sequences; anything else is an option name to resolve
	// against the visible screen text
	for (const char* p = start; p < end; p++)
	{
		char c = *p;
		if (c >= 'A' && c <= 'Z')
			c += 32;
		if ((c < '0' || c > '9') && c != 'q' && c != 'a' && c != 'o' && c != 'p' && c != 'i' && c != 'm')
			return true;
	}
	return false;
}

// True when a key for the pending @menu token is available. Name tokens poll
// the visible screen text; while the name is not on screen yet, no key is
// reported so the game keeps running (menus redraw between polls)
static bool MenuKeyAvailable()
{
	const char *start, *end;
	MenuToken(&start, &end);
	if (!MenuTokenIsName(start, end))
		return true;
	if (menuResolvedKey == 0)
		menuResolvedKey = (uint8_t)WinText_FindMenuOption(start, (int)(end - start));
	if (menuResolvedKey != 0)
	{
		menuRetries = 0;
		menuWaitTicks = 0;
		return true;
	}
	if (++menuRetries > 300)
	{
		static char message[512];
		char items[384];
		WinText_ListMenuOptions(items, sizeof(items));
		char* out = message;
		const char* prefix = "menu option not found: ";
		while (*prefix) *out++ = *prefix++;
		for (const char* p = start; p < end && out < message + 80; p++) *out++ = *p;
		const char* suffix = "; visible options:\n";
		while (*suffix) *out++ = *suffix++;
		for (const char* p = items; *p && out < message + sizeof(message) - 1; p++) *out++ = *p;
		*out = 0;
		inputError = true;
		inputErrorMessage = message;
		VID_Quit();
	}
	return false;
}

static uint8_t NextMenuKey()
{
	const char *start, *end;
	MenuToken(&start, &end);

	uint8_t value;
	const char* next;
	if (MenuTokenIsName(start, end))
	{
		if (menuResolvedKey == 0)
			menuResolvedKey = (uint8_t)WinText_FindMenuOption(start, (int)(end - start));
		if (menuResolvedKey == 0)
		{
			// Never consume an unresolved name: report "no key" so the
			// consumer keeps waiting (the retry limits produce an error
			// if it never appears)
			return 0;
		}
		value = menuResolvedKey;
		menuResolvedKey = 0;
		next = end;
	}
	else
	{
		// Literal keys: one character per call
		value = (uint8_t)*start;
		next = start + 1;
	}
	while (next < inputEnd && *next == ' ') next++;
	menuKeyPos = (int)(next - input);
	if (next >= inputEnd || *next == '\r' || *next == '\n')
	{
		menuKeyPos = -1;
		ConsumeLine();
	}
	return value;
}

static void ConsumeStopDirective()
{
	ConsumeLine();
	interactiveInput = true;
	inputStopped = true;
}

static bool IsCaptureDirective()
{
	return (input + 9 <= inputEnd && MemComp((void*)input, (void*)"@capture ", 9) == 0) ||
	       (input + 6 <= inputEnd && MemComp((void*)input, (void*)"@save ", 6) == 0);
}

// allowCaptures is false while a finite (animation-driving) PAUSE is running, so a
// pending @capture/@save is deferred until the game reaches its next settled input
// point (command prompt or infinite PAUSE) — the screenshot then shows the final
// frame instead of a mid-animation one.
static void ProcessDirectives(bool allowCaptures)
{
	while (input != 0 && input < inputEnd)
	{
		if (IsDirective("@exit"))
		{
			// End the run cleanly at this input point: take no further input,
			// let the player loop exit via exitGame. Used by headless screenshot
			// tests to bail out after a capture without finishing (or hanging in)
			// the game.
			ConsumeLine();
			input = inputEnd;
			VID_Quit();
			return;
		}
		if (IsDirective("@interactive") || IsDirective("@stop"))
		{
			ConsumeStopDirective();
			return;
		}
		if (input + 9 <= inputEnd && MemComp((void*)input, "@reseed ", 8) == 0)
		{
			// Re-seed the random number generator mid-script: lets a script
			// steer past a deterministically lost CHANCE roll without
			// disturbing the random stream of everything before this point
			uint32_t seed = 0;
			for (const char* p = input + 8; p < inputEnd && *p >= '0' && *p <= '9'; p++)
				seed = seed * 10 + (uint32_t)(*p - '0');
			RandSeed(seed == 0 ? 1 : seed);
			ConsumeLine();
			continue;
		}
		if (input + 7 <= inputEnd && MemComp((void*)input, "@mute ", 6) == 0)
		{
			mutedWindows |= (uint8_t)(1 << ((*(input + 6) - '0') & 7));
			ConsumeLine();
			continue;
		}
		if (input + 9 <= inputEnd && MemComp((void*)input, "@unmute ", 8) == 0)
		{
			mutedWindows &= (uint8_t)~(1 << ((*(input + 8) - '0') & 7));
			ConsumeLine();
			continue;
		}
		if (input + 6 <= inputEnd && MemComp((void*)input, "@let ", 5) == 0)
		{
			// "@let FLAG VALUE" pokes an interpreter flag, mirroring the DAAD
			// LET condact: lets a script neutralize a game state that only a
			// platform-specific action sequence could reach legitimately
			// (e.g. disarm a story-armed combat that this test skips)
			const char* p = input + 5;
			uint32_t flag = 0, value = 0;
			while (p < inputEnd && *p >= '0' && *p <= '9')
				flag = flag * 10 + (uint32_t)(*p++ - '0');
			while (p < inputEnd && *p == ' ') p++;
			while (p < inputEnd && *p >= '0' && *p <= '9')
				value = value * 10 + (uint32_t)(*p++ - '0');
			DDB_TestSetFlag((uint8_t)flag, (uint8_t)value);
			ConsumeLine();
			continue;
		}
		if (input + 9 <= inputEnd && MemComp((void*)input, "@assert ", 8) == 0)
		{
			// "@assert FLAG VALUE" verifies interpreter state at a settled
			// input point without mutating it.
			const char* p = input + 8;
			uint32_t flag = 0, value = 0;
			while (p < inputEnd && *p >= '0' && *p <= '9')
				flag = flag * 10 + (uint32_t)(*p++ - '0');
			while (p < inputEnd && *p == ' ') p++;
			while (p < inputEnd && *p >= '0' && *p <= '9')
				value = value * 10 + (uint32_t)(*p++ - '0');
			if (interpreter == 0 || interpreter->flags[(uint8_t)flag] != (uint8_t)value)
			{
				DebugPrintf("Script assertion failed: flag %u is %u, expected %u\n",
					(unsigned)flag,
					interpreter == 0 ? 0u : (unsigned)interpreter->flags[(uint8_t)flag],
					(unsigned)value);
				inputError = true;
				inputErrorMessage = "scripted flag assertion failed";
				VID_Quit();
				return;
			}
			ConsumeLine();
			continue;
		}
		if (input + 8 <= inputEnd && MemComp((void*)input, "@chance ", 8) == 0)
		{
			const char* arg = input + 8;
			if (input + 12 <= inputEnd && MemComp((void*)arg, "fail", 4) == 0 ||
			    input + 12 <= inputEnd && MemComp((void*)arg, "pass", 4) == 0)
			{
				// Each "@chance fail/pass [N]" line adds a rule; earlier rules
				// take precedence. "@chance random" clears them all.
				ChanceRule rule;
				rule.mode = *arg == 'f' ? Chance_Fail : Chance_Pass;
				rule.filter = ChanceFilter_All;
				rule.operand = 0;
				const char* p = arg + 4;
				while (p < inputEnd && *p == ' ') p++;
				if (p < inputEnd && *p == '[')
				{
					rule.filter = ChanceFilter_Indirect;
					p++;
				}
				else if (p < inputEnd && *p >= '0' && *p <= '9')
					rule.filter = ChanceFilter_Literal;
				uint32_t value = 0;
				while (p < inputEnd && *p >= '0' && *p <= '9')
					value = value * 10 + (uint32_t)(*p++ - '0');
				rule.operand = (uint8_t)value;
				if (chanceRuleCount < MAX_CHANCE_RULES)
					chanceRules[chanceRuleCount++] = rule;
			}
			else
				chanceRuleCount = 0;
			ConsumeLine();
			continue;
		}

		if (!IsCaptureDirective())
			return;
		if (!allowCaptures)
			return;

		const char* prefix =
			(input + 9 <= inputEnd && MemComp((void*)input, (void*)"@capture ", 9) == 0)
				? "@capture " : "@save ";

		const char* name = input + StrLen(prefix);
		const char* end = name;
		while (end < inputEnd && *end != '\r' && *end != '\n') end++;
		char fileName[FILE_MAX_PATH];
		size_t length = (size_t)(end - name);
		if (screenshotCallback == 0 || length == 0 || length >= sizeof(fileName))
		{
			inputError = true;
			inputErrorMessage = "invalid scripted screenshot directive";
			VID_Quit();
			return;
		}
		MemCopy(fileName, name, length);
		fileName[length] = 0;
		input = end;
		ConsumeLine();
		if (!screenshotCallback(fileName))
		{
			inputError = true;
			inputErrorMessage = "scripted screenshot failed";
			VID_Quit();
			return;
		}
		inputBarrier = true;
		return;
	}
}

bool DDB_TestIsActive()
{
	return input != 0;
}

bool DDB_TestGetKey(uint8_t* key, uint8_t* ext, uint8_t* mod)
{
	if (!DDB_TestIsActive())
		return false;
	if (waitKey != WaitKey_None)
	{
		WaitKey keyType = waitKey;
		uint8_t value = waitKeyValue;
		waitKey = WaitKey_None;
		waitKeyValue = 0x0D;
		if (keyType == WaitKey_Explicit)
			ConsumeLine();
		if (keyType == WaitKey_Synthetic)
			value = 0x0D;
		if (key) *key = value;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (input < inputEnd && *input == '\r') input++;
	ProcessDirectives(true);
	if (inputStopped)
	{
		VID_GetKey(key, ext, mod);
		return true;
	}
	if (IsWaitKeyDirective())
	{
		// Only waits consume this; a direct read reports no key available
		if (key) *key = 0;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (WaitTextPending())
	{
		if (key) *key = 0;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (IsMenuDirective())
	{
		uint8_t value = NextMenuKey();
		if (value == 0 && ++menuRetries > 300)
		{
			inputError = true;
			inputErrorMessage = "menu option not visible at a direct key read";
			VID_Quit();
		}
		if (key) *key = value;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (IsInkeyDirective())
	{
		uint8_t value = InkeyDirectiveValue();
		ConsumeLine();
		inkeyWaitTicks = 0;
		if (key) *key = value;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (IsKeyDirective())
	{
		uint8_t value = KeyDirectiveValue();
		ConsumeLine();
		if (key) *key = value;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (input < inputEnd)
	{
		uint8_t value = *input++;
		if (value == '\n')
		{
			value = 0x0D;
			inputBarrier = true;
		}
		if (key) *key = value;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	if (!interactiveInput)
	{
		if (key) *key = 0;
		if (ext) *ext = 0;
		if (mod) *mod = 0;
		return true;
	}
	VID_GetKey(key, ext, mod);
	return true;
}

bool DDB_TestAnyKey()
{
	if (!DDB_TestIsActive())
		return false;
	if (inputStopped)
		return VID_AnyKey();
	if (inputBarrier)
	{
		inputBarrier = false;
		return false;
	}
	ProcessDirectives(true);
	if (inputBarrier)
	{
		inputBarrier = false;
		return false;
	}
	if (WaitTextPending())
		return false;
	if (IsWaitKeyDirective())
	{
		if (++menuWaitTicks > 300)
		{
			inputError = true;
			inputErrorMessage = "@waitkey pending but the game is polling the menu, not waiting for a key";
			VID_Quit();
			return true;
		}
		return false;
	}
	if (IsMenuDirective())
		return MenuKeyAvailable();
	if (IsKeyDirective() || input < inputEnd)
		return true;
	// Scripted input exhausted: in interactive mode hand off to live input so the
	// player regains keyboard control once the script finishes (matching
	// DDB_TestGetKey and DDB_TestAnyKeyForWait).
	if (interactiveInput) return VID_AnyKey();
	return false;
}

// The "more..." prompt: with any @menu keys pending, feed it a synthetic
// Return without consuming the script — the prompt discards its key, so
// this is always safe, and menu scripts must be able to ride over text
// pagination between menus
bool DDB_TestAnyKeyForMore(bool allowCaptures)
{
	if (!DDB_TestIsActive())
		return false;
	ProcessDirectives(allowCaptures);
	if (!inputStopped && input < inputEnd && IsMenuDirective())
	{
		waitKey = WaitKey_Synthetic;
		return true;
	}
	return DDB_TestAnyKeyForWait(allowCaptures);
}

bool DDB_TestAnyKeyForWait(bool allowCaptures)
{
	if (!DDB_TestIsActive())
		return false;
	ProcessDirectives(allowCaptures);
	if (inputStopped)
		return VID_AnyKey();
	if (inputBarrier)
	{
		inputBarrier = false;
		return false;
	}
	if (input >= inputEnd)
	{
		if (interactiveInput) return VID_AnyKey();
		return false;
	}
	// A capture was deferred (finite pause in progress): report no key so the
	// pause runs to its end rather than being consumed as a keypress.
	if (!allowCaptures && IsCaptureDirective())
		return false;
	if (WaitTextPending())
	{
		// Ride over the wait without consuming the script: stalling it would
		// freeze the game and the awaited text could never appear
		waitKey = WaitKey_Synthetic;
		return true;
	}
	if (IsWaitKeyDirective())
	{
		waitKey = WaitKey_Explicit;
		waitKeyValue = 0x0D;
		return true;
	}
	if (IsInkeyDirective())
	{
		// Reserved for a direct key read: feed this wait a synthetic Return
		// instead. A script whose @inkey never meets a direct read is stuck:
		// report it instead of riding over waits forever
		if (++inkeyWaitTicks > 300)
		{
			inputError = true;
			inputErrorMessage = "@inkey pending but the game never reads a key directly (use @key for waits)";
			VID_Quit();
			return true;
		}
		waitKey = WaitKey_Synthetic;
		return true;
	}
	if (IsMenuDirective())
	{
		const char *start, *end;
		MenuToken(&start, &end);
		if (MenuTokenIsName(start, end) && menuResolvedKey == 0)
		{
			menuResolvedKey = (uint8_t)WinText_FindMenuOption(start, (int)(end - start));
			if (menuResolvedKey == 0)
			{
				// The option is not on screen yet: report no key so menu
				// animations run their course and the next menu gets drawn.
				// Never feed synthetic keys while a name is unresolved: a
				// Return would silently select whatever is highlighted and
				// the script would keep "playing" by defaults. If the game
				// stays waiting for a key here, the script is missing an
				// explicit @key for this wait: abort with a diagnostic
				if (++menuWaitTicks > 300)
				{
					static char message[512];
					char items[384];
					WinText_ListMenuOptions(items, sizeof(items));
					char* out = message;
					const char* prefix = "menu option pending while the game waits for a key (add @key before it): ";
					while (*prefix) *out++ = *prefix++;
					for (const char* p = start; p < end && out < message + 120; p++) *out++ = *p;
					const char* suffix = "; visible text:\n";
					while (*suffix) *out++ = *suffix++;
					for (const char* p = items; *p && out < message + sizeof(message) - 1; p++) *out++ = *p;
					*out = 0;
					inputError = true;
					inputErrorMessage = message;
					VID_Quit();
					return true;
				}
				return false;
			}
		}
		// Feed the wait without consuming the queued menu keys
		waitKey = WaitKey_Synthetic;
		return true;
	}
	if (IsKeyDirective())
	{
		waitKey = WaitKey_Explicit;
		waitKeyValue = KeyDirectiveValue();
		return true;
	}
	if (*input == '\r' || *input == '\n')
	{
		waitKey = WaitKey_Explicit;
		return true;
	}
	waitKey = WaitKey_Implicit;
	return true;
}

bool DDB_TestHasScriptedInput()
{
	return !inputStopped && input != 0 && input < inputEnd;
}

void DDB_TestSetScreenshotCallback(DDB_ScreenshotCallback callback)
{
	screenshotCallback = callback;
	inputError = false;
	inputErrorMessage = 0;
}

void DDB_TestEnableInteractiveInput()
{
	interactiveInput = true;
}

void DDB_TestSetPartSelection(int part, const char* screenshotFileName)
{
	partSelection = part;
	partScreenshotFile[0] = 0;
	if (screenshotFileName != 0)
		StrCopy(partScreenshotFile, sizeof(partScreenshotFile), screenshotFileName);
}

int DDB_TestGetPartSelection()
{
	return partSelection;
}

bool DDB_TestCapturePartSelector()
{
	if (partScreenshotFile[0] == 0)
		return true;
	if (screenshotCallback == 0)
	{
		inputError = true;
		inputErrorMessage = "no screenshot callback for part selector capture";
		return false;
	}
	if (!screenshotCallback(partScreenshotFile))
	{
		inputError = true;
		inputErrorMessage = "part selector screenshot failed";
		return false;
	}
	return true;
}

bool DDB_TestHasError()
{
	return inputError;
}

const char* DDB_TestGetError()
{
	return inputErrorMessage;
}

void DDB_TestLoadInput(const char* fileName)
{
	inputError = false;
	inputErrorMessage = 0;
	interactiveInput = false;
	inputStopped = false;
	inputBarrier = false;
	waitKey = WaitKey_None;
	waitKeyValue = 0x0D;
	menuKeyPos = -1;
	menuResolvedKey = 0;
	menuRetries = 0;
	menuWaitTicks = 0;
	mutedWindows = 0;
	inkeyWaitTicks = 0;
	waitTextTicks = 0;
	chanceRuleCount = 0;
	if (inputBegin != 0) Free((void*)inputBegin);
	input = inputBegin = inputEnd = 0;
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0) return;
	uint64_t fileSize = File_GetSize(file);
	inputBegin = input = (const char*)AllocateBlock("Test input", (size_t)fileSize, false);
	if (inputBegin != 0)
		inputEnd = inputBegin + File_Read(file, (void*)input, fileSize);
	File_Close(file);
	if (inputBegin == 0) input = inputEnd = 0;
}

#endif
