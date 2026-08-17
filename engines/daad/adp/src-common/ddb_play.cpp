
#include <ddb.h>
#include <ddb_scr.h>
#include <ddb_test.h>
#include <ddb_vid.h>
#include <ddb_pal.h>
#include <dmg.h>
#ifdef HAS_VIRTUALFILESYSTEM
#include <dim.h>
#endif
#include <os_char.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

#if defined(_DOS)
#include <conio.h>
#elif defined(_STDCLIB) && !defined(NO_PRINTF)
#include <stdio.h>
#endif

#ifdef _ATARIST
#include <mint/cookie.h>
#endif

#define MAX_FILES          64
#define NAME_BUFFER_SIZE 2048

static int         ddbCount = 0;
static int         scrCount = 0;
static int         ddbSelected = 0;
static char*       files[MAX_FILES];
#ifdef HAS_VIRTUALFILESYSTEM
static int         ddbFiles[MAX_FILES];
static int         ddbFileCount = -1;
#endif
static int         fileCount = 0;
static char*       nameBuffer = 0;
static char        ddbFileName[FILE_MAX_PATH];
static char        introScreenName[FILE_MAX_PATH];
static char        startupPattern[FILE_MAX_PATH];

#define MAX_MANIFEST_DISKS 4

struct StartupConfig
{
	int  parts;
	int  disks;
	int  boxX;
	int  boxY;
	int  ink;
	DDB_ScreenMode videoMode;
	char windowTitle[128];
	char windowIcon[FILE_MAX_PATH];
	char configFile[FILE_MAX_PATH];
	// Optional file manifests (FILES=, FILES2=... CFG keys): comma-separated
	// contents of each disk, letting the loader skip the slow directory
	// enumeration on floppy media. Index 0 is disk 1.
	char diskFiles[MAX_MANIFEST_DISKS][160];
	bool hasBoxX;
	bool hasBoxY;
	bool hasInk;
	bool hasVideoMode;
	bool hasWindowTitle;
	bool hasWindowIcon;
};

enum LoaderPromptMode
{
	LoaderPrompt_None,
	LoaderPrompt_Part,
	LoaderPrompt_Disk,
};

static StartupConfig    startupConfig;
static LoaderPromptMode loaderPromptMode = LoaderPrompt_None;
static int              diskPromptNumber = 0;
static bool             diskPromptConfirmed = false;

#if HAS_VIRTUALFILESYSTEM
static char             diskImages[MAX_FILES][FILE_MAX_PATH];
static int              diskImageCount = 0;

static void CollectDiskImages();
#endif

static int              currentDiskNumber = 0;
static DDB_ScreenMode   startupScreenModeOverride = ScreenMode_Default;
static DDB_StartupVideoModePolicy startupVideoModePolicy = DDB_StartupVideoModePolicy_Configurable;

#ifdef HAS_VIRTUALFILESYSTEM
static void IgnoreDDBWarning(const char* message)
{
	(void)message;
}
#endif

static void CloseEnum();
static void EnumFiles(const char* pattern = "*");
static int CountFiles(const char* extension);
static const char* GetFile(const char* extension, int index);
static int CountDDBFiles();
static const char* GetDDBFile(int index);
static bool GetDDBMetadata(const char* fileName, DDB_Machine* machine, DDB_Language* language, DDB_Version* version);
static void LoaderScreenUpdate(int elapsed);
static void WaitForKeyUpdate(int elapsed);

static void ResetStartupConfig();
static void LoadStartupConfig();
static void SaveStartupVideoMode(const char* ddbFile, DDB_ScreenMode mode);
static int GetConfiguredPartCount();
static void ShowLoaderPrompt(int parts, DDB_Language language);
static void ShowDiskPrompt(int diskNumber, DDB_Language language);
static const char* ResolveSelectedDDBFile(int partIndex);
static bool EnsureSelectedPartMediaSync(int partIndex, DDB_Language language);
static const char* GetDisplaySCRFileErrorString();

#ifdef _WEB
static bool EnsureSelectedPartMediaAsync(int partIndex, DDB_Language language);
static const char* GetSnapshot(int index);
static int snapshotCount = 0;
#endif

static void LogBackBufferAnalysisForDDB(const char* fileName, DDB_Machine machine, DDB* ddb)
{
	(void)fileName;
	(void)machine;
	(void)ddb;
}

#ifdef _AMIGA
enum AmigaKeyboardLayout
{
	AmigaKeyboardLayout_English,
	AmigaKeyboardLayout_Spanish,
};
extern bool VID_IsAGAAvailable();
extern void VID_SetDisplayColorModeHint(uint8_t colorMode);
extern void OpenKeyboard();
extern void SetKeyboardLayout(AmigaKeyboardLayout layout);
extern bool OpenAudio();
#endif

#ifdef _AMIGA
static AmigaKeyboardLayout GetAmigaKeyboardLayout(DDB_Language language)
{
	switch (language)
	{
		case DDB_SPANISH:
			return AmigaKeyboardLayout_Spanish;

		case DDB_ENGLISH:
		default:
			return AmigaKeyboardLayout_English;
	}
}
#endif

static bool FileExistsByName(const char* fileName)
{
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;
	File_Close(file);
	return true;
}

void DDB_SetStartupScreenModeOverride(DDB_ScreenMode mode)
{
	startupScreenModeOverride = mode;
}

void DDB_ClearStartupScreenModeOverride()
{
	startupScreenModeOverride = ScreenMode_Default;
}

static const char* GetDisplaySCRFileErrorString()
{
	if (DDB_GetError() == DDB_ERROR_INVALID_FILE)
		return "Unsupported or invalid loading screen file";
	return DDB_GetErrorString();
}

static const char* DescribeScreenMode(DDB_ScreenMode mode)
{
	switch (mode)
	{
		case ScreenMode_CGA:    return "CGA";
		case ScreenMode_EGA:    return "EGA";
		case ScreenMode_VGA16:  return "VGA";
		case ScreenMode_VGA:    return "SVGA";
		case ScreenMode_HiRes:  return "HiRes";
		case ScreenMode_SHiRes: return "SuperHiRes";
		default:                return "Default";
	}
}

static bool GetModeSpecificIntroScreen(DDB_ScreenMode mode, const char** introScreen)
{
	const char* extension = 0;
	switch (mode)
	{
		case ScreenMode_EGA:    extension = ".egs"; break;
		case ScreenMode_CGA:    extension = ".cgs"; break;
		case ScreenMode_VGA16:  extension = ".vgs"; break;
		// 256-colour VGA (320x200 Mode-X, DOS only) gets its own screen: the
		// SVGA .scx / .scr are 640x400 and would overflow the 320x200 buffer.
		// Never resolved on other platforms, so this costs them no probe.
		case ScreenMode_VGA:    extension = ".xgs"; break;
		case ScreenMode_SHiRes: extension = ".scx"; break;
		default: break;
	}

	if (extension == 0)
		return false;
	if (CountFiles(extension) <= 0)
		return false;
	if (introScreen != 0)
		*introScreen = GetFile(extension, 0);
	return true;
}

static DDB_ScreenMode ParseScreenModeName(const char* text)
{
	if (StrIComp(text, "CGA") == 0)
		return ScreenMode_CGA;
	if (StrIComp(text, "EGA") == 0)
		return ScreenMode_EGA;
	if (StrIComp(text, "VGA") == 0 || StrIComp(text, "VGA16") == 0)
		return ScreenMode_VGA16;
	if (StrIComp(text, "SVGA") == 0 || StrIComp(text, "SGA") == 0)
		return ScreenMode_VGA;
	if (StrIComp(text, "HIRES") == 0)
		return ScreenMode_HiRes;
	if (StrIComp(text, "SUPERHIRES") == 0 || StrIComp(text, "SHIRES") == 0)
		return ScreenMode_SHiRes;
	return ScreenMode_Default;
}

static uint32_t GetScreenModeFlag(DDB_ScreenMode mode)
{
	switch (mode)
	{
		case ScreenMode_CGA:    return DDB_DataFileMode_CGA;
		case ScreenMode_EGA:    return DDB_DataFileMode_EGA;
		case ScreenMode_VGA16:  return DDB_DataFileMode_VGA16;
		case ScreenMode_VGA:    return DDB_DataFileMode_VGA;
		case ScreenMode_HiRes:  return DDB_DataFileMode_HiRes;
		case ScreenMode_SHiRes: return DDB_DataFileMode_SHiRes;
		default:                return 0;
	}
}

static int CountScreenModes(uint32_t mask)
{
	int count = 0;
	while (mask != 0)
	{
		count += (int)(mask & 1);
		mask >>= 1;
	}
	return count;
}

static DDB_ScreenMode ChooseDefaultScreenModeForMask(uint32_t modeMask, DDB_ScreenMode fallback)
{
	static const DDB_ScreenMode order[] =
	{
		ScreenMode_CGA,
		ScreenMode_EGA,
		ScreenMode_VGA16,
		ScreenMode_VGA,
		ScreenMode_HiRes,
		ScreenMode_SHiRes,
	};

	if ((modeMask & GetScreenModeFlag(fallback)) != 0)
		return fallback;

	for (unsigned i = 0; i < sizeof(order) / sizeof(order[0]); i++)
	{
		if ((modeMask & GetScreenModeFlag(order[i])) != 0)
			return order[i];
	}

	return fallback;
}

static DDB_ScreenMode ChooseHighestScreenModeForMask(uint32_t modeMask, DDB_ScreenMode fallback)
{
	static const DDB_ScreenMode order[] =
	{
		ScreenMode_SHiRes,
		ScreenMode_HiRes,
		ScreenMode_VGA,
		ScreenMode_VGA16,
		ScreenMode_EGA,
		ScreenMode_CGA,
	};

	for (unsigned i = 0; i < sizeof(order) / sizeof(order[0]); i++)
	{
		if ((modeMask & GetScreenModeFlag(order[i])) != 0)
			return order[i];
	}

	return fallback;
}

static bool IsKnownScreenMode(DDB_ScreenMode mode)
{
	switch (mode)
	{
		case ScreenMode_Default:
		case ScreenMode_CGA:
		case ScreenMode_EGA:
		case ScreenMode_VGA16:
		case ScreenMode_VGA:
		case ScreenMode_HiRes:
		case ScreenMode_SHiRes:
			return true;
		default:
			return false;
	}
}

void DDB_SetStartupVideoModePolicy(DDB_StartupVideoModePolicy policy)
{
	startupVideoModePolicy = policy;
}

#if defined(_DOS) || (defined(_STDCLIB) && !defined(NO_PRINTF))
static bool PromptForVideoMode(const char* ddbFile, uint32_t modeMask, DDB_ScreenMode* selectedMode)
{
	struct ModeOption
	{
		char key;
		DDB_ScreenMode mode;
	};

	static const DDB_ScreenMode order[] =
	{
		ScreenMode_CGA,
		ScreenMode_EGA,
		ScreenMode_VGA16,
		ScreenMode_VGA,
		ScreenMode_HiRes,
		ScreenMode_SHiRes,
	};
	ModeOption options[6];
	int optionCount = 0;

	#if defined(_DOS)
	cputs("\r\nSelect video mode for ");
	cputs(ddbFile);
	cputs(":\r\n");
	#else
	printf("\nSelect video mode for %s:\n", ddbFile);
	#endif
	for (unsigned i = 0; i < sizeof(order) / sizeof(order[0]); i++)
	{
		if ((modeMask & GetScreenModeFlag(order[i])) == 0)
			continue;
		options[optionCount].key = (char)('1' + optionCount);
		options[optionCount].mode = order[i];
		#if defined(_DOS)
		cputs("  ");
		putch(options[optionCount].key);
		cputs(". ");
		cputs(DescribeScreenMode(order[i]));
		cputs("\r\n");
		#else
		printf("  %c. %s\n", options[optionCount].key, DescribeScreenMode(order[i]));
		#endif
		optionCount++;
	}

	for (;;)
	{
		#if defined(_DOS)
		cputs("Choice: ");
		int ch = getch();
		putch(ch);
		cputs("\r\n");
		#else
		printf("Choice: ");
		fflush(stdout);

		int ch = getchar();
		if (ch == EOF)
			return false;
		while (ch == '\r' || ch == '\n')
		{
			ch = getchar();
			if (ch == EOF)
				return false;
		}

		int discard = ch;
		while (discard != '\n' && discard != '\r' && discard != EOF)
			discard = getchar();
		#endif

		for (int i = 0; i < optionCount; i++)
		{
			if (ch == options[i].key)
			{
				*selectedMode = options[i].mode;
				return true;
			}
		}

		#if defined(_DOS)
		cputs("Invalid choice.\r\n");
		#else
		printf("Invalid choice.\n");
		#endif
	}
}
#endif

static DDB_ScreenMode GetPreferredIntroScreenMode(DDB_ScreenMode resolvedMode)
{
	if (startupScreenModeOverride != ScreenMode_Default)
		return startupScreenModeOverride;
	if (startupConfig.hasVideoMode && startupConfig.videoMode == resolvedMode)
		return resolvedMode;
	if (startupVideoModePolicy == DDB_StartupVideoModePolicy_OverrideOrHighest)
		return resolvedMode;
	return ScreenMode_Default;
}

static void SetStartupPattern(const char* pattern)
{
	StrCopy(startupPattern, sizeof(startupPattern), pattern);
}

static bool StartupIsDiskMounted()
{
#ifdef HAS_VIRTUALFILESYSTEM
	return File_IsDiskMounted();
#else
	return false;
#endif
}

static bool StartupMountDisk(const char* fileName)
{
#ifdef HAS_VIRTUALFILESYSTEM
	return File_MountDisk(fileName);
#else
	(void)fileName;
	return false;
#endif
}

static void StartupUnmountDisk()
{
#ifdef HAS_VIRTUALFILESYSTEM
	File_UnmountDisk();
#endif
}

static const char* GetLastPathSeparator(const char* path)
{
	const char* slash = StrRChr(path, '/');
	const char* backslash = StrRChr(path, '\\');
	if (slash == 0)
		return backslash;
	if (backslash == 0)
		return slash;
	return slash > backslash ? slash : backslash;
}

static void CopyConfigString(char* output, size_t outputSize, const char* value)
{
	while (IsSpace(*value))
		value++;

	const char* end = value + StrLen(value);
	while (end > value && IsSpace((uint8_t)end[-1]))
		end--;

	if (end > value + 1 && ((value[0] == '"' && end[-1] == '"') || (value[0] == '\'' && end[-1] == '\'')))
	{
		value++;
		end--;
	}

	size_t length = (size_t)(end - value);
	if (length >= outputSize)
		length = outputSize - 1;
	if (length > 0)
		MemCopy(output, value, length);
	output[length] = 0;
}

static void ResolveConfigAdjacentPath(const char* cfgFile, const char* value, char* output, size_t outputSize)
{
	if (outputSize == 0)
		return;

	CopyConfigString(output, outputSize, value);
	if (output[0] == 0)
		return;

	if (GetLastPathSeparator(output) != 0)
		return;

	const char* separator = GetLastPathSeparator(cfgFile);
	if (separator == 0)
		return;

	char resolved[FILE_MAX_PATH];
	size_t prefixLength = (size_t)(separator - cfgFile + 1);
	if (prefixLength >= sizeof(resolved))
		prefixLength = sizeof(resolved) - 1;
	MemCopy(resolved, cfgFile, prefixLength);
	resolved[prefixLength] = 0;
	StrCat(resolved, sizeof(resolved), output);
	StrCopy(output, outputSize, resolved);
}

static void RescanCurrentFiles()
{
	if (StartupIsDiskMounted())
		EnumFiles();
	else
		EnumFiles(startupPattern[0] == 0 ? "*" : startupPattern);
}

static bool ParseConfigInt(const char* text, int* value)
{
	int parsed = 0;
	bool hasDigits = false;
	while (IsSpace(*text))
		text++;
	if (*text == 0)
		return false;
	while (*text >= '0' && *text <= '9')
	{
		hasDigits = true;
		parsed = parsed * 10 + (*text - '0');
		text++;
	}
	while (IsSpace(*text))
		text++;
	if (!hasDigits || *text != 0)
		return false;
	*value = parsed;
	return true;
}

static void ResetStartupConfig()
{
	MemClear(&startupConfig, sizeof(startupConfig));
	startupConfig.parts = 0;
	startupConfig.disks = 0;
	startupConfig.ink = 0x0F;
	startupConfig.videoMode = ScreenMode_Default;
	VID_SetWindowTitle(0);
	VID_SetWindowIcon(0);
	loaderPromptMode = LoaderPrompt_None;
	diskPromptNumber = 0;
	diskPromptConfirmed = false;
	currentDiskNumber = 0;
	startupPattern[0] = 0;

	#if HAS_VIRTUALFILESYSTEM
	diskImageCount = 0;
	for (int i = 0; i < MAX_FILES; i++)
		diskImages[i][0] = 0;
	#endif
}

static void ApplyConfigEntry(const char* cfgFile, const char* key, const char* value)
{
	int parsed = 0;
	if (StrIComp(key, "TITLE") == 0 || StrIComp(key, "WINDOWTITLE") == 0)
	{
		CopyConfigString(startupConfig.windowTitle, sizeof(startupConfig.windowTitle), value);
		startupConfig.hasWindowTitle = startupConfig.windowTitle[0] != 0;
		return;
	}
	if (StrIComp(key, "ICON") == 0 || StrIComp(key, "WINDOWICON") == 0)
	{
		ResolveConfigAdjacentPath(cfgFile, value, startupConfig.windowIcon, sizeof(startupConfig.windowIcon));
		startupConfig.hasWindowIcon = startupConfig.windowIcon[0] != 0;
		return;
	}
	if (StrIComp(key, "VIDEOMODE") == 0 || StrIComp(key, "SCREENMODE") == 0)
	{
		char buffer[32];
		CopyConfigString(buffer, sizeof(buffer), value);
		startupConfig.videoMode = ParseScreenModeName(buffer);
		startupConfig.hasVideoMode = startupConfig.videoMode != ScreenMode_Default;
		return;
	}
	if (ToUpper((uint8_t)key[0]) == 'F' && ToUpper((uint8_t)key[1]) == 'I' &&
	    ToUpper((uint8_t)key[2]) == 'L' && ToUpper((uint8_t)key[3]) == 'E' &&
	    ToUpper((uint8_t)key[4]) == 'S')
	{
		int disk = 1;
		if (key[5] >= '2' && key[5] <= '0' + MAX_MANIFEST_DISKS && key[6] == 0)
			disk = key[5] - '0';
		else if (key[5] != 0)
			return;
		CopyConfigString(startupConfig.diskFiles[disk - 1],
			sizeof(startupConfig.diskFiles[0]), value);
		return;
	}

	if (!ParseConfigInt(value, &parsed))
		return;

	if (StrIComp(key, "PARTS") == 0 && parsed > 0)
		startupConfig.parts = parsed;
	else if (StrIComp(key, "DISKS") == 0 && parsed > 0)
		startupConfig.disks = parsed;
	else if (StrIComp(key, "SCRBOXX") == 0)
	{
		startupConfig.boxX = parsed;
		startupConfig.hasBoxX = true;
	}
	else if (StrIComp(key, "SCRBOXY") == 0)
	{
		startupConfig.boxY = parsed;
		startupConfig.hasBoxY = true;
	}
	else if (StrIComp(key, "SCRINK") == 0)
	{
		startupConfig.ink = parsed & 0xFF;
		startupConfig.hasInk = true;
	}
}

// Parses one startup CFG file. Returns false if the file cannot be opened.
static bool ParseStartupConfigFile(const char* cfgFile)
{
	File* file = File_Open(cfgFile, ReadOnly);
	if (file == 0)
		return false;
	StrCopy(startupConfig.configFile, sizeof(startupConfig.configFile), cfgFile);

	uint64_t size = File_GetSize(file);
	if (size == 0 || size > 8192)
	{
		File_Close(file);
		return true;
	}

	char* buffer = Allocate<char>("startup cfg", (uint32_t)size + 1);
	if (buffer == 0)
	{
		File_Close(file);
		return true;
	}

	if (File_Read(file, buffer, size) != size)
	{
		Free(buffer);
		File_Close(file);
		return true;
	}
	File_Close(file);
	buffer[size] = 0;

	char* line = buffer;
	while (*line != 0)
	{
		char* next = line;
		while (*next != 0 && *next != '\n' && *next != '\r')
			next++;
		char saved = *next;
		*next = 0;

		char* start = line;
		while (IsSpace(*start))
			start++;
		if (*start != 0 && *start != ';' && *start != '#' && *start != '[')
		{
			char* end = start + StrLen(start);
			while (end > start && IsSpace((uint8_t)end[-1]))
				*--end = 0;
			char* equals = (char*)StrRChr(start, '=');
			if (equals != 0)
			{
				char* value = equals + 1;
				*equals = 0;
				char* keyEnd = equals;
				while (keyEnd > start && IsSpace((uint8_t)keyEnd[-1]))
					*--keyEnd = 0;
				while (IsSpace(*value))
					value++;
				ApplyConfigEntry(cfgFile, start, value);
			}
		}

		*next = saved;
		line = next;
		while (*line == '\n' || *line == '\r')
			line++;
	}

	VID_SetWindowTitle(startupConfig.hasWindowTitle ? startupConfig.windowTitle : 0);
	VID_SetWindowIcon(startupConfig.hasWindowIcon ? startupConfig.windowIcon : 0);

	DebugPrintf("Loaded startup CFG %s (parts=%d disks=%d boxX=%d%s boxY=%d%s ink=%d%s title=%s icon=%s)\n",
		cfgFile,
		startupConfig.parts,
		startupConfig.disks,
		startupConfig.boxX,
		startupConfig.hasBoxX ? "" : " default",
		startupConfig.boxY,
		startupConfig.hasBoxY ? "" : " default",
		startupConfig.ink,
		startupConfig.hasInk ? "" : " default",
		startupConfig.hasWindowTitle ? startupConfig.windowTitle : "default",
		startupConfig.hasWindowIcon ? startupConfig.windowIcon : "default");

	Free(buffer);
	return true;
}

static void LoadStartupConfig()
{
	for (int i = 0; i < fileCount; i++)
	{
		const char* dot = StrRChr(files[i], '.');
		if (dot != 0 && StrIComp(dot, ".cfg") == 0)
		{
			ParseStartupConfigFile(files[i]);
			return;
		}
	}
}

// Probes a couple of well-known CFG names directly, avoiding the directory
// enumeration entirely. A direct open is a cheap hash lookup even on floppy
// media, while enumerating a floppy directory costs a seek per file.
static bool TryDirectStartupConfig()
{
	static const char* candidates[] = { "adp.cfg", "part1.cfg", 0 };
	for (int n = 0; candidates[n] != 0; n++)
	{
		if (ParseStartupConfigFile(candidates[n]))
			return true;
	}
	return false;
}

// Fills the file table from a FILES=/FILESn= manifest instead of scanning
// the directory. The list is a claim about the disk contents: subsequent
// opens verify it against the actual medium (a wrong disk simply fails and
// the caller falls back to the insert-disk prompt).
static int activeManifestDisk = 0;

static bool HaveManifestForDisk(int disk)
{
	return disk >= 1 && disk <= MAX_MANIFEST_DISKS &&
		startupConfig.diskFiles[disk - 1][0] != 0;
}

static bool PopulateFilesFromManifest(int disk)
{
	if (!HaveManifestForDisk(disk))
		return false;

	CloseEnum();
	DDB_FlushDataFileProbeCache();
	fileCount = 0;
	#ifdef HAS_VIRTUALFILESYSTEM
	ddbFileCount = -1;
	#endif
	nameBuffer = Allocate<char>("EnumFiles", NAME_BUFFER_SIZE);
	if (nameBuffer == 0)
		return false;

	const char* src = startupConfig.diskFiles[disk - 1];
	char* ptr = nameBuffer;
	char* end = nameBuffer + NAME_BUFFER_SIZE;
	while (*src != 0 && fileCount < MAX_FILES && ptr < end - 1)
	{
		while (*src == ',' || IsSpace(*src))
			src++;
		if (*src == 0)
			break;
		files[fileCount] = ptr;
		while (*src != 0 && *src != ',' && ptr < end - 1)
			*ptr++ = *src++;
		while (ptr > files[fileCount] && IsSpace((uint8_t)ptr[-1]))
			ptr--;
		*ptr++ = 0;
		if (ptr - files[fileCount] > 1)
			fileCount++;
	}
	activeManifestDisk = disk;
	DebugPrintf("Using disk %d manifest: %d file(s), no enumeration\n", disk, fileCount);
	return true;
}

static void SaveStartupVideoMode(const char* ddbFile, DDB_ScreenMode mode)
{
#if defined(_STDCLIB) && !defined(NO_PRINTF)
	char cfgPath[FILE_MAX_PATH];
	if (startupConfig.configFile[0] != 0)
		StrCopy(cfgPath, sizeof(cfgPath), startupConfig.configFile);
	else
		StrCopy(cfgPath, sizeof(cfgPath), ChangeExtension(ddbFile, ".cfg"));

	char line[64];
	int length = snprintf(line, sizeof(line), "VIDEOMODE=%s\n", DescribeScreenMode(mode));
	if (length <= 0 || length >= (int)sizeof(line))
		return;

	File* existing = File_Open(cfgPath, ReadOnly);
	char* buffer = 0;
	uint64_t size = 0;
	if (existing != 0)
	{
		size = File_GetSize(existing);
		if (size > 8192)
			size = 8192;
		buffer = Allocate<char>("startup cfg save", (uint32_t)size + 1);
		if (buffer != 0)
		{
			uint64_t read = File_Read(existing, buffer, size);
			buffer[read] = 0;
			size = read;
		}
		File_Close(existing);
	}

	File* file = File_Create(cfgPath);
	if (file == 0)
	{
		if (buffer != 0)
			Free(buffer);
		return;
	}

	if (buffer != 0 && size > 0)
	{
		File_Write(file, buffer, size);
		if (buffer[size - 1] != '\n')
			File_Write(file, "\n", 1);
		Free(buffer);
	}
	File_Write(file, line, (uint64_t)length);
	File_Close(file);

	StrCopy(startupConfig.configFile, sizeof(startupConfig.configFile), cfgPath);
	startupConfig.videoMode = mode;
	startupConfig.hasVideoMode = true;
#else
	(void)ddbFile;
	(void)mode;
#endif
}

#if HAS_VIRTUALFILESYSTEM
static void CollectDiskImages()
{
	diskImageCount = 0;
	for (int i = 0; i < fileCount && diskImageCount < MAX_FILES; i++)
	{
		const char* dot = StrRChr(files[i], '.');
		if (dot == 0)
			continue;
		if (StrIComp(dot, ".adf") != 0 && StrIComp(dot, ".dsk") != 0 &&
			StrIComp(dot, ".img") != 0 && StrIComp(dot, ".st") != 0)
			continue;
		StrCopy(diskImages[diskImageCount], FILE_MAX_PATH, files[i]);
		diskImageCount++;
	}
	DebugPrintf("Collected %d disk image candidate(s)\n", diskImageCount);
}
#endif

static int GetConfiguredPartCount()
{
	if (startupConfig.parts > 0)
		return startupConfig.parts;
	#ifdef _WEB
	if (ddbCount == 0)
		return snapshotCount;
	#endif
	return ddbCount;
}

static int GetTargetDiskForPart(int partIndex)
{
	if (startupConfig.disks <= 0)
		return 0;
	int disk = partIndex + 1;
	return disk > 0 && disk <= startupConfig.disks ? disk : 0;
}

#if HAS_VIRTUALFILESYSTEM
static int GetDiskNumberForImage(const char* path)
{
	for (int i = 0; i < diskImageCount; i++)
	{
		if (StrIComp(diskImages[i], path) == 0)
			return i + 1;
	}
	return 0;
}

static bool MountDiskByNumber(int diskNumber)
{
	if (diskNumber <= 0 || diskNumber > diskImageCount)
		return false;
	if (StartupIsDiskMounted() && currentDiskNumber == diskNumber)
		return true;

	if (StartupIsDiskMounted())
		StartupUnmountDisk();

	if (!StartupMountDisk(diskImages[diskNumber - 1]))
	{
		currentDiskNumber = 0;
		RescanCurrentFiles();
		return false;
	}

	currentDiskNumber = diskNumber;
	EnumFiles();
	return true;
}
#endif

static bool HasLocalDataForDDB(const char* ddbFile, DDB_Machine machine, DDB_Version version)
{
	if (ddbFile == 0 || ddbFile[0] == 0)
		return false;
	if (!DDB_SupportsDataFile(version, machine))
		return true;
	return DDB_GetDataFileModes(ddbFile, machine) != 0;
}

static const char* ResolveSelectedDDBFile(int partIndex)
{
	if (partIndex < 0)
		return "";
	if (startupConfig.disks > 0 && StartupIsDiskMounted())
	{
		int targetDisk = GetTargetDiskForPart(partIndex);
		if (targetDisk == 0 || currentDiskNumber != targetDisk || CountDDBFiles() == 0)
			return "";
		return GetDDBFile(0);
	}
	int count = CountDDBFiles();
	#ifdef _WEB
	if (count == 0 && snapshotCount > 0)
		return partIndex < snapshotCount ? GetSnapshot(partIndex) : "";
	#endif
	if (startupConfig.disks > 0)
	{
		// Physical multi-disk game: each disk carries its own part, so file
		// positions say nothing about part numbers (after a swap the only
		// DDB present sits at index 0 whatever its part is). Match by name
		// instead: per-disk DDBs are expected to carry their part number in
		// the file name (part1.ddb, part2.ddb, ...). Positions are only
		// trusted when every configured part is present at once.
		char partDigit = (char)('1' + partIndex);
		for (int n = 0; n < count; n++)
		{
			const char* file = GetDDBFile(n);
			for (const char* ptr = file; *ptr != 0; ptr++)
			{
				if (*ptr == partDigit)
					return file;
			}
		}
		if (count >= GetConfiguredPartCount() && partIndex < count)
			return GetDDBFile(partIndex);
		return "";
	}
	if (partIndex >= count)
		return "";
	return GetDDBFile(partIndex);
}

static bool SelectedPartNeedsDisk(int partIndex, DDB_Machine machine, DDB_Version version)
{
	if (startupConfig.disks <= 0)
		return false;
	const char* ddbFile = ResolveSelectedDDBFile(partIndex);
	if (ddbFile[0] == 0)
		return true;
	return !HasLocalDataForDDB(ddbFile, machine, version);
}

static uint8_t diskPromptKey = 0;
static uint32_t diskPromptPollStart = 0;

static void DiskPromptKeyUpdate(int elapsed)
{
	(void)elapsed;
	if (VID_AnyKey())
	{
		uint8_t key = 0, ext = 0;
		VID_GetKey(&key, &ext, 0);
		diskPromptKey = key;
		VID_Quit();
		return;
	}
	// Poll tick: a disk change needs no confirmation, so periodically hand
	// control back to the prompt loop for a quiet look at the drive.
	uint32_t nowMs = 0;
	VID_GetMilliseconds(&nowMs);
	if (nowMs - diskPromptPollStart > 2000)
	{
		diskPromptKey = 0;
		VID_Quit();
	}
}

static bool EnsureSelectedPartMediaSync(int partIndex, DDB_Language language)
{
	DDB_Machine machine = DDB_MACHINE_AMIGA;
	DDB_Language unusedLanguage = language;
	DDB_Version version = DDB_VERSION_2;
	const char* ddbFile = ResolveSelectedDDBFile(partIndex);
	if (ddbFile[0] != 0)
		GetDDBMetadata(ddbFile, &machine, &unusedLanguage, &version);
	if (!SelectedPartNeedsDisk(partIndex, machine, version))
		return true;

	int targetDisk = GetTargetDiskForPart(partIndex);
	if (targetDisk == 0)
		return false;

	// No open file may stay bound to the outgoing volume across the swap;
	// the selected part reopens its graphics database when it loads.
	VID_CloseDataFile();

	// Classic insert-disk behavior: the prompt stays up while the drive is
	// polled, so just inserting the right disk continues the game; a swap
	// needs a moment to settle (motor, volume mount), which instant retries
	// would misread as a wrong disk. Any key forces an immediate check and
	// ESC aborts.
	bool diskPromptScreenSaved = VID_BackupScreen();
	ShowDiskPrompt(targetDisk, language);
	for (;;)
	{
		diskPromptKey = 0;
		VID_GetMilliseconds(&diskPromptPollStart);
		VID_MainLoop(0, DiskPromptKeyUpdate);
		if (exitGame || diskPromptKey == 27)
		{
			if (diskPromptScreenSaved)
				VID_RestoreBackupScreen();
			return false;
		}

		#if HAS_VIRTUALFILESYSTEM
		if (diskImageCount > 0 && targetDisk <= diskImageCount)
		{
			// Mounted disk images either work or never will; no retry.
			bool ok = MountDiskByNumber(targetDisk);
			if (ok)
			{
				ddbCount = CountDDBFiles();
				ddbFile = ResolveSelectedDDBFile(partIndex);
				if (ddbFile[0] != 0)
				{
					GetDDBMetadata(ddbFile, &machine, &unusedLanguage, &version);
					ok = HasLocalDataForDDB(ddbFile, machine, version);
				}
				else
					ok = false;
			}
			if (diskPromptScreenSaved)
				VID_RestoreBackupScreen();
			return ok;
		}
		#endif

		OS_RemountBootMedia();
		if (!PopulateFilesFromManifest(targetDisk))
			RescanCurrentFiles();
		ddbCount = CountDDBFiles();
		ddbFile = ResolveSelectedDDBFile(partIndex);
		DebugPrintf("Disk %d prompt: %d DDBs after rescan, part %d resolves to '%s'\n",
			targetDisk, ddbCount, partIndex + 1, ddbFile);
		if (ddbFile[0] != 0)
		{
			GetDDBMetadata(ddbFile, &machine, &unusedLanguage, &version);
			if (HasLocalDataForDDB(ddbFile, machine, version))
			{
				if (diskPromptScreenSaved)
					VID_RestoreBackupScreen();
				return true;
			}
			DebugPrintf("Disk %d prompt: '%s' present but its data file is missing\n",
				targetDisk, ddbFile);
		}
	}
}

#ifdef _WEB
static bool EnsureSelectedPartMediaAsync(int partIndex, DDB_Language language)
{
	DDB_Machine machine = DDB_MACHINE_AMIGA;
	DDB_Language unusedLanguage = language;
	DDB_Version version = DDB_VERSION_2;
	const char* ddbFile = ResolveSelectedDDBFile(partIndex);
	if (ddbFile[0] != 0)
		GetDDBMetadata(ddbFile, &machine, &unusedLanguage, &version);
	if (!SelectedPartNeedsDisk(partIndex, machine, version))
		return true;

	int targetDisk = GetTargetDiskForPart(partIndex);
	if (targetDisk == 0)
		return false;
	if (!diskPromptConfirmed)
	{
		loaderPromptMode = LoaderPrompt_Disk;
		diskPromptNumber = targetDisk;
		ShowDiskPrompt(targetDisk, language);
		VID_SetTextInputMode(true);
		VID_MainLoopAsync(0, LoaderScreenUpdate);
		return false;
	}

	diskPromptConfirmed = false;
	loaderPromptMode = LoaderPrompt_None;
	if (diskImageCount > 0 && targetDisk <= diskImageCount)
	{
		if (!MountDiskByNumber(targetDisk))
			return false;
	}
	else
	{
		RescanCurrentFiles();
	}

	ddbCount = CountDDBFiles();
	ddbFile = ResolveSelectedDDBFile(partIndex);
	if (ddbFile[0] == 0)
		return false;
	GetDDBMetadata(ddbFile, &machine, &unusedLanguage, &version);
	return HasLocalDataForDDB(ddbFile, machine, version);
}
#endif

static uint16_t Read16BE(const uint8_t* ptr)
{
	return (uint16_t)(((uint16_t)ptr[0] << 8) | ptr[1]);
}

static bool ProbeDAT5HeaderExact(const char* datFileName, uint16_t* width, uint16_t* height, uint8_t* colorMode)
{
	File* dat = File_Open(datFileName, ReadOnly);
	if (dat == 0)
		return false;

	uint8_t header[16];
	bool ok = File_Read(dat, header, sizeof(header)) == sizeof(header);
	File_Close(dat);
	if (!ok)
		return false;

	if (!(header[0] == 'D' && header[1] == 'A' && header[2] == 'T' && header[3] == 0 &&
		header[4] == 0 && header[5] == 5))
		return false;

	if (width) *width = Read16BE(header + 0x06);
	if (height) *height = Read16BE(header + 0x08);
	if (colorMode) *colorMode = header[0x0E];
	return true;
}

static bool ProbeDAT5Header(const char* fileName, uint16_t* width, uint16_t* height, uint8_t* colorMode)
{
	if (ProbeDAT5HeaderExact(ChangeExtension(fileName, ".dat"), width, height, colorMode))
		return true;
	return ProbeDAT5HeaderExact(ChangeExtension(fileName, ".DAT"), width, height, colorMode);
}

#ifdef _AMIGA
static uint8_t GetAmigaDisplayColorModeHint(const char* fileName)
{
	uint8_t colorMode = 0;
	if (ProbeDAT5Header(fileName, 0, 0, &colorMode))
		return colorMode;
	return DMG_DAT5_COLORMODE_PLANAR4;
}
#endif

static bool ValidateResolvedVideoConfig(const char* fileName, DDB_Machine machine, DDB_ScreenMode screenMode, uint8_t planes)
{
#ifdef _DOS
	uint16_t width = 0;
	uint16_t height = 0;
	uint8_t colorMode = 0;
	if (ProbeDAT5Header(fileName, &width, &height, &colorMode) && !DMG_DAT5ModeIsDOSSupported(colorMode))
	{
		DebugPrintf("Rejecting unsupported DAT5 during DOS initial probe (mode=%u size=%ux%u planes=%u screen=%u)\n",
			(unsigned)colorMode,
			(unsigned)width,
			(unsigned)height,
			(unsigned)planes,
			(unsigned)screenMode);
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}
#endif

#ifdef _AMIGA
	if (planes >= 8 && !VID_IsAGAAvailable())
	{
		DebugPrintf("Rejecting DAT5 Planar8 during initial probe on non-AGA machine\n");
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}
#endif

#ifdef _ATARIST
	if (machine == DDB_MACHINE_ATARIST)
	{
		long cookieValue = 0;
		bool hasFalconVideo = Getcookie(C__VDO, &cookieValue) == C_FOUND && cookieValue >= 0x00030000L;
		uint16_t width = 0;
		uint16_t height = 0;
		uint8_t colorMode = 0;
		if (ProbeDAT5Header(fileName, &width, &height, &colorMode))
		{
			bool supported = false;
			if (width == 320 && height == 200)
			{
				if (colorMode == DMG_DAT5_COLORMODE_PLANAR4ST && planes == 4 && screenMode == ScreenMode_VGA16)
					supported = true;
				else if (colorMode == DMG_DAT5_COLORMODE_PLANAR8ST && hasFalconVideo && planes == 8 && screenMode == ScreenMode_VGA)
					supported = true;
			}

			if (!supported)
			{
				DebugPrintf("Rejecting unsupported DAT5 during initial ST probe (mode=%u size=%ux%u planes=%u screen=%u)\n",
					(unsigned)colorMode,
					(unsigned)width,
					(unsigned)height,
					(unsigned)planes,
					(unsigned)screenMode);
				DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
				return false;
			}
		}
	}
#endif

	(void)fileName;
	(void)machine;
	(void)screenMode;
	(void)planes;
	return true;
}

#if HAS_PCX
static uint16_t Read16LE(const uint8_t* ptr)
{
	return (uint16_t)(((uint16_t)ptr[1] << 8) | ptr[0]);
}

static void BuildFileNameWithExtension(const char* fileName, const char* extension, char* output, size_t outputSize)
{
	StrCopy(output, outputSize, fileName);
	char* dot = (char*)StrRChr(output, '.');
	if (dot == 0)
		dot = output + StrLen(output);
	StrCopy(dot, output + outputSize - dot, extension);
}

static bool HasPCXHeader(const char* fileName, DDB_ScreenMode* screenMode)
{
	uint8_t header[128];
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;

	bool ok = File_Read(file, header, sizeof(header)) == sizeof(header) &&
		header[0] == 0x0A && header[2] == 1 && header[3] == 8 && header[65] == 1;
	File_Close(file);
	if (!ok)
		return false;

	if (screenMode != 0)
	{
		int width = (int)(Read16LE(header + 8) - Read16LE(header + 4) + 1);
		int height = (int)(Read16LE(header + 10) - Read16LE(header + 6) + 1);
		if (width == 640 && height == 400)
			*screenMode = ScreenMode_SHiRes;
		else if (width == 640 && height == 200)
			*screenMode = ScreenMode_HiRes;
		else
			*screenMode = ScreenMode_VGA;
	}

	return true;
}

static const char* FindPCXIntroScreen(const char* fileName, DDB_Machine machine, DDB_Version version, DDB_ScreenMode* screenMode)
{
	static char introScreen[FILE_MAX_PATH];
	if (machine == DDB_MACHINE_IBMPC)
	{
		if (version < DDB_VERSION_2)
			return 0;

		BuildFileNameWithExtension(fileName, ".VGA", introScreen, sizeof(introScreen));
		if (!FileExistsByName(introScreen))
		{
			BuildFileNameWithExtension(fileName, ".vga", introScreen, sizeof(introScreen));
			if (FileExistsByName(introScreen))
				goto found;
		}
		else
			goto found;
	}

	BuildFileNameWithExtension(fileName, ".PCX", introScreen, sizeof(introScreen));
	if (!FileExistsByName(introScreen))
	{
		BuildFileNameWithExtension(fileName, ".pcx", introScreen, sizeof(introScreen));
		if (!FileExistsByName(introScreen))
			return 0;
	}

	found:
	if (HasPCXHeader(introScreen, screenMode))
		return introScreen;

	*screenMode = ScreenMode_VGA;
	return introScreen;
}
#endif

static bool HasPI1Header(const char* fileName)
{
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;

	uint64_t size = File_GetSize(file);
	File_Close(file);
	return size == 32034 || size == 32066;
}

static bool HasFalconRawHeader(const char* fileName)
{
	uint8_t header[4];
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;

	uint64_t size = File_GetSize(file);
	bool ok = size == (uint64_t)(4 + 256 * 3 + 64000) && File_Read(file, header, sizeof(header)) == sizeof(header);
	File_Close(file);
	return ok && header[0] == 'F' && header[1] == 'C' && header[2] == 'R' && header[3] == '1';
}

static bool SupportsFalconRawIntroScreens()
{
	return true;
}

static bool ShouldPreferAmigaSCR(const char* fileName, DDB_Machine machine, DDB_ScreenMode* screenMode)
{
	if (fileName == 0 || fileName[0] == 0)
		return false;
	if (machine != DDB_MACHINE_ATARIST)
		return false;
	if (CountFiles(".ch0") > 0)
		return false;
	if (HasPI1Header(fileName))
		return false;
	if (HasFalconRawHeader(fileName))
		return false;
	#if HAS_PCX
	if (HasPCXHeader(fileName, screenMode))
		return false;
	#endif
	return true;
}

static bool ResolveDDBVideoConfig(const char* fileName, DDB_Machine* machine, DDB_Language* language, DDB_Version* version, DDB_ScreenMode* screenMode, uint8_t* displayPlanes)
{
	DDB_Machine resolvedMachine = DDB_MACHINE_AMIGA;
	DDB_Language resolvedLanguage = DDB_SPANISH;
	DDB_Version resolvedVersion = DDB_VERSION_2;
	if (!GetDDBMetadata(fileName, &resolvedMachine, &resolvedLanguage, &resolvedVersion))
		return false;

#ifdef HAS_VIRTUALFILESYSTEM
	// The Amiga and Atari ST releases ship the same big-endian DDB, and the
	// original interpreter never checked its platform field — it just assumed the
	// screen format of the machine it was running on. Some releases (e.g. Templos)
	// therefore carry an Atari ST DDB inside an Amiga .adf. Trust the disk image
	// over the DDB field so the loading screen decodes in the right format; this
	// only flips between the Amiga/ST pair and leaves other platforms alone.
	switch (File_GetMountedDiskType())
	{
		case DIM_ADF:
			if (resolvedMachine == DDB_MACHINE_ATARIST)
				resolvedMachine = DDB_MACHINE_AMIGA;
			break;
		case DIM_FAT:
			if (resolvedMachine == DDB_MACHINE_AMIGA)
				resolvedMachine = DDB_MACHINE_ATARIST;
			break;
		default:
			break;
	}
#endif

#ifdef _DOS
	if (resolvedMachine != DDB_MACHINE_IBMPC &&
		resolvedMachine != DDB_MACHINE_AMIGA &&
		resolvedMachine != DDB_MACHINE_ATARIST)
	{
		DebugPrintf("ResolveDDBVideoConfig: rejecting unsupported %s DDB on DOS renderer\n",
			DDB_DescribeMachine(resolvedMachine));
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}
	if (resolvedMachine != DDB_MACHINE_IBMPC)
	{
		DebugPrintf("ResolveDDBVideoConfig: overriding machine %s -> IBM PC for DOS renderer\n",
			DDB_DescribeMachine(resolvedMachine));
		resolvedMachine = DDB_MACHINE_IBMPC;
	}
#endif

	if (machine)
		*machine = resolvedMachine;
	if (language)
		*language = resolvedLanguage;
	if (version)
		*version = resolvedVersion;

	DDB_ScreenMode resolvedScreenMode = DDB_GetDefaultScreenMode(resolvedMachine);
	uint8_t resolvedPlanes = 4;
	uint32_t supportedModes = DDB_GetDataFileModes(fileName, resolvedMachine);
	if (supportedModes != 0)
	{
		if (!IsKnownScreenMode(startupScreenModeOverride))
		{
			DebugPrintf("Ignoring invalid startup override mode value %u\n", (unsigned)startupScreenModeOverride);
			startupScreenModeOverride = ScreenMode_Default;
		}

		#ifdef _DOS
		uint32_t playableModes = supportedModes & VID_GetSupportedDataFileModes();
		if (playableModes == 0 && startupScreenModeOverride == ScreenMode_Default)
		{
			DebugPrintf("No DOS-playable data file mode found (data=0x%08lX)\n",
				(unsigned long)supportedModes);
			DDB_SetError(DDB_ERROR_VIDEO_HARDWARE_NOT_SUPPORTED);
			return false;
		}
		#endif

		DDB_ScreenMode selectedMode = startupVideoModePolicy == DDB_StartupVideoModePolicy_OverrideOrHighest ?
			ChooseHighestScreenModeForMask(supportedModes, resolvedScreenMode) :
			ChooseDefaultScreenModeForMask(supportedModes, resolvedScreenMode);
		if (startupScreenModeOverride != ScreenMode_Default)
		{
			if ((supportedModes & GetScreenModeFlag(startupScreenModeOverride)) == 0)
			{
				DebugPrintf("Ignoring startup override %s: unsupported by data file (mask=0x%08lX)\n",
					DescribeScreenMode(startupScreenModeOverride),
					(unsigned long)supportedModes);
				startupScreenModeOverride = ScreenMode_Default;
			}
			else
			{
				selectedMode = startupScreenModeOverride;
			}
		}
		#ifdef _DOS
		if (startupScreenModeOverride == ScreenMode_Default)
		{
			if (playableModes == 0)
			{
				DebugPrintf("No DOS-playable data file mode found after override validation (data=0x%08lX)\n",
					(unsigned long)supportedModes);
				DDB_SetError(DDB_ERROR_VIDEO_HARDWARE_NOT_SUPPORTED);
				return false;
			}
			if (startupVideoModePolicy == DDB_StartupVideoModePolicy_Configurable &&
				startupConfig.hasVideoMode && (playableModes & GetScreenModeFlag(startupConfig.videoMode)) != 0)
				selectedMode = startupConfig.videoMode;
			else
				selectedMode = ChooseHighestScreenModeForMask(playableModes, resolvedScreenMode);
		}
		#else
		else if (startupVideoModePolicy == DDB_StartupVideoModePolicy_Configurable &&
			startupConfig.hasVideoMode && (supportedModes & GetScreenModeFlag(startupConfig.videoMode)) != 0)
			selectedMode = startupConfig.videoMode;
		else if (startupVideoModePolicy == DDB_StartupVideoModePolicy_Configurable && CountScreenModes(supportedModes) > 1)
		{
			#if defined(_STDCLIB) && !defined(NO_PRINTF)
				if (PromptForVideoMode(fileName, supportedModes, &selectedMode))
					SaveStartupVideoMode(fileName, selectedMode);
			#endif
		}
		#endif

		if (!DDB_ResolveDataFile(fileName, resolvedMachine, selectedMode, 0, 0, &resolvedScreenMode, &resolvedPlanes))
		{
			DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
			return false;
		}
	}
	if (!ValidateResolvedVideoConfig(fileName, resolvedMachine, resolvedScreenMode, resolvedPlanes))
		return false;

	if (screenMode)
		*screenMode = resolvedScreenMode;
	if (displayPlanes)
		*displayPlanes = resolvedPlanes;

	return true;
}

static void EnumFiles(const char* pattern)
{
	FindFileResults r;

	CloseEnum();
	DDB_FlushDataFileProbeCache();
	fileCount = 0;
#ifdef HAS_VIRTUALFILESYSTEM
	ddbFileCount = -1;
#endif
	nameBuffer = Allocate<char>("EnumFiles", NAME_BUFFER_SIZE);
	if (nameBuffer == 0)
		return;
	
	if (File_FindFirst(pattern, &r))
	{
		char* ptr = nameBuffer;
		char* end = nameBuffer + NAME_BUFFER_SIZE;
		do
		{
			if (r.attributes & FileAttribute_Directory)
				continue;
			if (fileCount >= MAX_FILES)
				break;
			files[fileCount] = ptr;
			ptr += StrCopy(files[fileCount], end - ptr, r.fileName);
			fileCount++;
			if (fileCount == MAX_FILES || ptr >= end-1)
				break;
		} while (File_FindNext(&r));
	}

	// Sort files
	for (int n = 0; n < fileCount; n++)
	{
		for (int m = n + 1; m < fileCount; m++)
		{
			if (StrIComp(files[n], files[m]) > 0)
			{
				char* temp = files[n];
				files[n] = files[m];
				files[m] = temp;
			}
		}
	}
}

#ifdef HAS_VIRTUALFILESYSTEM
static void DetectDDBFiles()
{
	if (ddbFileCount >= 0)
		return;

	ddbFileCount = 0;
	int namedDDBCount = CountFiles(".ddb");
	#if HAS_PAWS
	namedDDBCount += CountFiles(".sdb");
	#endif
	DDB_SetWarningHandler(IgnoreDDBWarning);
	for (int n = 0; n < fileCount && ddbFileCount < MAX_FILES; n++)
	{
		if (namedDDBCount > 0)
		{
			const char* dot = StrRChr(files[n], '.');
			if (dot == 0 || (StrIComp(dot, ".ddb") != 0
				#if HAS_PAWS
				&& StrIComp(dot, ".sdb") != 0
				#endif
				))
				continue;
		}
		if (DDB_Check(files[n], 0, 0, 0))
		{
			ddbFiles[ddbFileCount++] = n;
		}
	}
	DDB_SetWarningHandler(0);
	DebugPrintf("Detected %d DDB candidates by content\n", ddbFileCount);
}
#endif

static void CloseEnum()
{
	if (nameBuffer)
	{
		Free(nameBuffer);
		nameBuffer = 0;
	}
	#ifdef HAS_VIRTUALFILESYSTEM
	ddbFileCount = -1;
	#endif
}

static int CountFiles(const char* extension)
{
	int count = 0;
	for (int n = 0; n < fileCount; n++)
	{
		const char* dot = StrRChr(files[n], '.');
		if (dot == 0)
			continue;
		if (StrIComp(dot, extension) == 0)
			count++;
	}
	return count;
}

static bool TryMountDiskImageWithDDBs()
{
#ifdef HAS_VIRTUALFILESYSTEM
	char* candidates = Allocate<char>("Mount candidates", MAX_FILES * FILE_MAX_PATH);
	if (candidates == 0)
		return false;
	int candidateCount = fileCount;
	if (candidateCount > MAX_FILES)
		candidateCount = MAX_FILES;

	for (int n = 0; n < candidateCount; n++)
		StrCopy(candidates + n * FILE_MAX_PATH, FILE_MAX_PATH, files[n]);

	for (int n = 0; n < candidateCount; n++)
	{
		if (!File_MountDisk(candidates + n * FILE_MAX_PATH))
			continue;

		currentDiskNumber = GetDiskNumberForImage(candidates + n * FILE_MAX_PATH);

		EnumFiles();
		if (CountDDBFiles() > 0)
		{
			Free(candidates);
			return true;
		}

		CloseEnum();
		File_UnmountDisk();
		currentDiskNumber = 0;
	}
	Free(candidates);
#endif
	return false;
}

static const char* GetFile(const char* extension, int index)
{
	int count = 0;
	for (int n = 0; n < fileCount; n++)
	{
		const char* dot = StrRChr(files[n], '.');
		if (dot == 0)
			continue;
		if (StrIComp(dot, extension) == 0)
		{
			if (count == index)
				return files[n];
			count++;
		}
	}
	return "";
}

static int CountDDBFiles()
{
	#ifdef HAS_VIRTUALFILESYSTEM
	DetectDDBFiles();
	return ddbFileCount;
	#else
	return CountFiles(".ddb")
		#if HAS_PAWS
		+ CountFiles(".sdb")
		#endif
		;
	#endif
}

static const char* GetDDBFile(int index)
{
	#ifdef HAS_VIRTUALFILESYSTEM
	DetectDDBFiles();
	if (index < 0 || index >= ddbFileCount)
		return "";
	return files[ddbFiles[index]];
	#else
	#if HAS_PAWS
	int ddbCount = CountFiles(".ddb");
	return index < ddbCount ? GetFile(".ddb", index) : GetFile(".sdb", index - ddbCount);
	#else
	return GetFile(".ddb", index);
	#endif
	#endif
}

static bool GetDDBMetadata(const char* fileName, DDB_Machine* machine, DDB_Language* language, DDB_Version* version)
{
	return DDB_Check(fileName, machine, language, version);
}

static void DrawPromptBox(const char* const* lines, int lineCount, uint8_t ink)
{
	int maxChars = 0;
	for (int y = 0; y < lineCount; y++)
	{
		int chars = (int)StrLen(lines[y]);
		if (chars > maxChars)
			maxChars = chars;
	}

	int promptWidth = maxChars * columnWidth;
	int promptHeight = lineCount * lineHeight;
	int promptX = startupConfig.hasBoxX ? startupConfig.boxX : (screenWidth - promptWidth) / 2;
	int promptY = startupConfig.hasBoxY ? startupConfig.boxY : (screenHeight - promptHeight) / 2;
	if (startupConfig.hasBoxY && lineHeight >= 16)
		promptY <<= 1;

	if (startupConfig.hasBoxX && (promptX < 0 || promptX + promptWidth > (int)screenWidth))
	{
		int scaledX = promptX >> 1;
		if (scaledX >= 0 && scaledX + promptWidth <= (int)screenWidth)
			promptX = scaledX;
	}
	if (startupConfig.hasBoxY && (promptY < 0 || promptY + promptHeight > (int)screenHeight))
	{
		int scaledY = promptY >> 1;
		if (scaledY >= 0 && scaledY + promptHeight <= (int)screenHeight)
			promptY = scaledY;
	}

	int minX = columnWidth;
	int minY = lineHeight;
	int maxX = (int)screenWidth - promptWidth - columnWidth;
	int maxY = (int)screenHeight - promptHeight - lineHeight;
	if (maxX < minX)
		maxX = minX;
	if (maxY < minY)
		maxY = minY;
	if (promptX < minX)
		promptX = minX;
	else if (promptX > maxX)
		promptX = maxX;
	if (promptY < minY)
		promptY = minY;
	else if (promptY > maxY)
		promptY = maxY;

	VID_Clear(promptX - columnWidth, promptY - lineHeight,
		promptWidth + columnWidth * 2, promptHeight + lineHeight * 2, 0);

	for (int y = 0; y < lineCount; y++)
	{
		for (int x = 0; lines[y][x] != 0; x++)
		{
			VID_DrawCharacter(promptX + x * columnWidth, promptY + y * lineHeight, lines[y][x], ink, 0);
		}
	}
}

static uint8_t GetAutoPromptInk()
{
	uint8_t best = 15;
	int bestScore = -1;
	for (int n = 1; n < 16; n++)
	{
		uint8_t r = 0, g = 0, b = 0;
		VID_GetPaletteColor((uint8_t)n, &r, &g, &b);
		int score = (int)r + (int)g + (int)b;
		if (score > bestScore)
		{
			bestScore = score;
			best = (uint8_t)n;
		}
	}

	return best;
}

static void BuildPromptLine(char* output, size_t outputSize, const char* templateText, int value)
{
	char digits[16];
	LongToChar(value, digits, 10);
	output[0] = 0;
	for (int i = 0; templateText[i] != 0; i++)
	{
		if (templateText[i] == '*')
			StrCat(output, outputSize, digits);
		else
		{
			char text[2] = { templateText[i], 0 };
			StrCat(output, outputSize, text);
		}
	}
}

static void ShowLoaderPrompt(int parts, DDB_Language language)
{
	char line0[64];
	char line1[64];
	const char* lines[2];
	if (language == DDB_SPANISH)
	{
		StrCopy(line0, sizeof(line0), "\x12Qu\x16 parte quieres");
		BuildPromptLine(line1, sizeof(line1), "   cargar (1-*)?  ", parts);
	}
	else
	{
		StrCopy(line0, sizeof(line0), " Which part do you ");
		BuildPromptLine(line1, sizeof(line1), "want to load (1-*)?", parts);
	}
	lines[0] = line0;
	lines[1] = line1;
	uint8_t ink = startupConfig.hasInk ? (uint8_t)startupConfig.ink : GetAutoPromptInk();
	DrawPromptBox(lines, 2, ink);
}

static void ShowDiskPrompt(int diskNumber, DDB_Language language)
{
	char line0[64];
	char line1[64];
	const char* lines[2];
	int lineCount = 1;
	if (language == DDB_SPANISH)
		BuildPromptLine(line0, sizeof(line0), " Inserta el disco * ", diskNumber);
	else
		BuildPromptLine(line0, sizeof(line0), "   Insert disk *   ", diskNumber);
	lines[0] = line0;
	#ifdef _WEB
	// The web player has no drive to poll: the prompt really is confirmed
	// with a keypress, so keep the instruction there.
	if (language == DDB_SPANISH)
		StrCopy(line1, sizeof(line1), " y pulsa una tecla  ");
	else
		StrCopy(line1, sizeof(line1), " and press any key ");
	lines[1] = line1;
	lineCount = 2;
	#else
	(void)line1;
	#endif
	uint8_t ink = startupConfig.hasInk ? (uint8_t)startupConfig.ink : GetAutoPromptInk();
	DrawPromptBox(lines, lineCount, ink);
}

static void LoaderScreenUpdate(int elapsed)
{
	(void)elapsed;
	if (VID_AnyKey())
	{
		uint8_t key = 0, ext = 0;
		VID_GetKey(&key, &ext, 0);
		if (key == 27)
			VID_Quit();
		else if (loaderPromptMode == LoaderPrompt_Disk)
		{
			diskPromptConfirmed = true;
			VID_Quit();
		}
		else if (key >= '1' && key <= '0' + GetConfiguredPartCount())
		{
			ddbSelected = key - '1';
			VID_Quit();
		}			
	}
}

static void WaitForKeyUpdate(int elapsed)
{
	(void)elapsed;
	if (VID_AnyKey())
	{
		uint8_t key = 0, ext = 0;
		VID_GetKey(&key, &ext, 0);
		VID_Quit();
	}
}

static void ScaleFadePalette(const uint32_t* sourcePalette, uint32_t* fadedPalette, uint16_t count, uint16_t scale)
{
	for (uint16_t n = 0; n < count; n++)
	{
		uint32_t v0 = ((sourcePalette[n] & 0xFF00FFUL) * scale) >> 8;
		uint32_t v1 = ((sourcePalette[n] & 0x00FF00UL) * scale) >> 8;
		fadedPalette[n] = (v0 & 0xFF00FFUL) | (v1 & 0x00FF00UL);
	}
}

#if _WEB

static int frame;
static const int fadeSteps = 8;
static const uint16_t fadeScale[fadeSteps] = { 256, 219, 183, 146, 110, 73, 37, 0 };
static uint32_t fadeSourcePalette[256];
static uint32_t fadeWorkingPalette[256];

static PlayerState state = Player_Starting;
static DDB*        ddb;

static const char* snapshotExtensions[] = {
	".z80",
	".sna",
	".tzx",
	".cdt",
	".sta",
	".vsf",
	".tap",
	".cas",
	".bin",
	".rom",
	".raw",
	0
};

static int CountSnapshots()
{
	int count = 0;
	for (int n = 0; snapshotExtensions[n]; n++)
		count += CountFiles(snapshotExtensions[n]);
	return count;
}

static const char* GetSnapshot(int index)
{
	int count = 0;
	for (int m = 0; m < fileCount; m++)
	{
		const char* dot = StrRChr(files[m], '.');
		if (dot == 0)
			continue;

		for (int n = 0; snapshotExtensions[n]; n++)
		{
			if (StrIComp(dot, snapshotExtensions[n]) == 0)
			{
				if (count == index)
					return files[m];
				count++;
				break;
			}
		}
	}
	return "";
}

static void FadeOutStep(int elapsed)
{
	(void)elapsed;
	uint16_t fadePaletteSize = VID_GetPaletteSize();
	uint16_t scale = fadeScale[frame];
	ScaleFadePalette(fadeSourcePalette, fadeWorkingPalette, fadePaletteSize, scale);
	VID_SetPaletteEntries(fadeWorkingPalette, fadePaletteSize, 0, false, true);
	frame++;
	
	if (frame >= fadeSteps)
	{
		VID_Quit();
	}
}

static void FadeOut()
{
	if (!VID_IsFadeEnabled())
		return;

	uint16_t fadePaletteSize = VID_GetPaletteSize();
	if (fadePaletteSize > 256)
		fadePaletteSize = 256;
	for (uint16_t i = 0; i < fadePaletteSize; i++)
	{
		uint8_t r, g, b;
		VID_GetPaletteColor(i, &r, &g, &b);
		fadeSourcePalette[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
	}
	frame = 0;
	FadeOutStep(0);

	VID_MainLoopAsync(0, FadeOutStep);
}

static const char* PlayerStateToString(PlayerState state)
{
	switch (state)
	{
		case Player_Starting: return "Starting";
		case Player_SelectingPart: return "SelectingPart";
		case Player_ShowingScreen: return "ShowingScreen";
		case Player_FadingOut: return "FadingOut";
		case Player_InGame: return "InGame";
		case Player_Finished: return "Finished";
		case Player_Error: return "Error";
	}
	return "Unknown";
}

void DDB_RestartAsyncPlayer()
{
	if (state == Player_Finished || state == Player_Error)
		state = Player_Starting;
}

PlayerState DDB_RunPlayerAsync(const char* location)
{
	#if HAS_TESTMODE
	if (!DDB_TestIsActive())
	#endif
	{
		uint32_t entropy = 0;
		SCR_GetMilliseconds(&entropy);
		RandSeedFromClock(entropy);
	}

	static DDB_ScreenMode screenMode = ScreenMode_VGA16;
	static uint8_t displayPlanes = 4;
	static DDB_Machine initializedMachine = DDB_MACHINE_AMIGA;
	static DDB_Version initializedVersion = DDB_VERSION_2;

	if (state == Player_Finished || state == Player_Error)
		return state;

	if (state == Player_FadingOut)
	{
		VID_ClearBuffer(true);
		VID_ClearBuffer(false);
		VID_SetPaletteRange(DefaultPalette, 16, 0, VID_GetPaletteSize() <= 16, true);

		DebugPrintf("Starting interpreter\n");
		DDB_Run(interpreter);
		return state = Player_InGame;
	}

	if (state == Player_InGame)
	{
		DDB_CloseInterpreter(interpreter);
		DDB_Close(ddb);
		VID_Finish();
		return state = Player_Finished;
	}

	if (state == Player_Starting)
	{
		if (startupPattern[0] == 0)
			ResetStartupConfig();
		DDB_Machine machine = DDB_MACHINE_AMIGA;
		DDB_Language language = DDB_SPANISH;
		DDB_Version version = DDB_VERSION_2;
		screenMode = DDB_GetDefaultScreenMode(machine);

		char path[FILE_MAX_PATH];
		StrCopy(path, FILE_MAX_PATH, location);
		StrCat(path, FILE_MAX_PATH, "*");
		SetStartupPattern(path);
		
		EnumFiles(path);
		LoadStartupConfig();
		CollectDiskImages();
		snapshotCount = CountSnapshots();
		ddbCount = CountDDBFiles();
		scrCount = CountFiles(".scr");
		const char* scrExtension = ".scr";
		const char* introScreen = 0;
		if (scrCount == 0)
		{
			scrCount = SupportsFalconRawIntroScreens() ? CountFiles(".fcr") : 0;
			scrExtension = ".fcr";
			if (scrCount > 0 && !HasFalconRawHeader(GetFile(".fcr", 0)))
				scrCount = 0;
		}
		if (scrCount == 0)
		{
			const char* preferredIntro = 0;
			if (startupScreenModeOverride != ScreenMode_Default &&
				GetModeSpecificIntroScreen(startupScreenModeOverride, &preferredIntro))
			{
				scrCount = 1;
				scrExtension =
					startupScreenModeOverride == ScreenMode_CGA ? ".cgs" :
					startupScreenModeOverride == ScreenMode_VGA16 ? ".vgs" : ".egs";
			}
			else if ((scrCount = CountFiles(".egs")) > 0)
			{
				scrExtension = ".egs";
			}
			else if ((scrCount = CountFiles(".cgs")) > 0)
			{
				scrExtension = ".cgs";
			}
			else if ((scrCount = CountFiles(".vgs")) > 0)
			{
				scrExtension = ".vgs";
			}
		}
		DebugPrintf("Found %d DDBs, %d snapshots and %d screens\n", ddbCount, snapshotCount, scrCount);
		if (ddbCount == 0 && snapshotCount == 0)
		{
			if (!TryMountDiskImageWithDDBs())
			{
				CloseEnum();
				DDB_SetError(DDB_ERROR_NO_DDBS_FOUND);
				return state = Player_Error;
			}

			snapshotCount = CountSnapshots();
			ddbCount = CountDDBFiles();
			scrCount = CountFiles(".scr");
			scrExtension = ".scr";
			if (scrCount == 0)
			{
				scrCount = SupportsFalconRawIntroScreens() ? CountFiles(".fcr") : 0;
				scrExtension = ".fcr";
				if (scrCount > 0 && !HasFalconRawHeader(GetFile(".fcr", 0)))
					scrCount = 0;
			}
		}

		{
			StrCopy(ddbFileName, FILE_MAX_PATH, ResolveSelectedDDBFile(0));
			if (!ResolveDDBVideoConfig(ddbFileName, &machine, &language, &version, &screenMode, &displayPlanes))
			{
				DebugPrintf("Rejected invalid DDB %s before initialization\n", ddbFileName);
				if (DDB_GetError() == DDB_ERROR_NONE)
					DDB_SetError(DDB_ERROR_INVALID_FILE);
				return state = Player_Error;
			}
			if (scrCount > 0 && StrIComp(scrExtension, ".scr") == 0)
			{
				const char* screenFile = GetFile(".scr", 0);
				if (ShouldPreferAmigaSCR(screenFile, machine, &screenMode))
					machine = DDB_MACHINE_AMIGA;
			}
			else if (scrCount > 0 && StrIComp(scrExtension, ".fcr") == 0)
				introScreen = GetFile(".fcr", 0);
			else
				introScreen = FindPCXIntroScreen(ddbFileName, machine, version, &screenMode);
			VID_SetDisplayPlanesHint(displayPlanes);
			DebugPrintf("Checked %s\n", ddbFileName);
			if (!VID_Initialize(machine, version, screenMode))
			{
				VID_ShowError(DDB_GetErrorString());
				CloseEnum();
				return state = Player_Error;
			}
				initializedMachine = machine;
				initializedVersion = version;
		    if (DDB_SupportsDataFile(version, machine) && !VID_LoadDataFile(ddbFileName))
			{
				DebugPrintf("VID_LoadDataFile(%s) failed: %s\n", ddbFileName, DDB_GetErrorString());
				VID_ShowError(DDB_GetErrorString());
				CloseEnum();
				VID_Finish();
				return state = Player_Error;
			}
		}
		
		if (scrCount > 0 || introScreen != 0)
		{
			const char* screenFile = introScreen != 0 ? introScreen : GetFile(scrExtension, 0);
			if (StrIComp(scrExtension, ".scr") == 0 && ShouldPreferAmigaSCR(screenFile, machine, &screenMode))
				machine = DDB_MACHINE_AMIGA;
			if (!VID_DisplaySCRFile(screenFile, machine, true))
			{
				VID_ShowError(DDB_GetErrorString());
				return state = Player_Error;
			}
			if (ddbCount == 1)
			{
				VID_MainLoopAsync(0, WaitForKeyUpdate);
				return state = Player_ShowingScreen;
			}
		}
		if (ddbCount > 1)
		if (GetConfiguredPartCount() > 1)
		{
			DebugPrintf("Showing part selector\n");
			ddbSelected = -1;
			loaderPromptMode = LoaderPrompt_Part;
			ShowLoaderPrompt(GetConfiguredPartCount(), language);
			VID_SetTextInputMode(true);
			VID_MainLoopAsync(0, LoaderScreenUpdate);
			return state = Player_SelectingPart;
		}
	}

	VID_SetTextInputMode(false);

	// Start the game

	if (ddbSelected == -1)
	{
		CloseEnum();
		VID_Finish();
		return state = Player_Finished;
	}

	DebugPrintf("Selected part %ld\n", (long)(ddbSelected + 1));
	DDB_Machine selectedMachine = DDB_MACHINE_AMIGA;
	DDB_Language selectedLanguage = DDB_SPANISH;
	DDB_Version selectedVersion = DDB_VERSION_2;
	DDB_ScreenMode selectedScreenMode = ScreenMode_VGA16;
	uint8_t selectedDisplayPlanes = 4;
	if (!EnsureSelectedPartMediaAsync(ddbSelected, selectedLanguage))
	{
		if (loaderPromptMode == LoaderPrompt_Disk)
			return state = Player_SelectingPart;
		DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
		return state = Player_Error;
	}
	StrCopy(ddbFileName, FILE_MAX_PATH, ResolveSelectedDDBFile(ddbSelected));

	if (!ResolveDDBVideoConfig(ddbFileName, &selectedMachine, &selectedLanguage, &selectedVersion, &selectedScreenMode, &selectedDisplayPlanes))
	{
		CloseEnum();
		if (DDB_GetError() == DDB_ERROR_NONE)
			DDB_SetError(DDB_ERROR_INVALID_FILE);
		return state = Player_Error;
	}
	if (selectedMachine != initializedMachine || selectedVersion != initializedVersion || selectedScreenMode != screenMode || selectedDisplayPlanes != displayPlanes)
	{
		VID_Finish();
		VID_SetDisplayPlanesHint(selectedDisplayPlanes);
		if (!VID_Initialize(selectedMachine, selectedVersion, selectedScreenMode))
		{
			CloseEnum();
			return state = Player_Error;
		}
		initializedMachine = selectedMachine;
		initializedVersion = selectedVersion;
		screenMode = selectedScreenMode;
		displayPlanes = selectedDisplayPlanes;
	}
	CloseEnum();
	
	DebugPrintf("Loading %s\n", ddbFileName);

	ddb = DDB_Load(ddbFileName);
	if (!ddb)
	{
		DebugPrintf("Error loading %s", ddbFileName);
		DebugPrintf(": %s\n", DDB_GetErrorString());
		return state = Player_Error;
	}
	DDB_CreateInterpreter(ddb, screenMode);
	if (interpreter == 0)
	{
		DebugPrintf("Error creating interpreter: %s\n", DDB_GetErrorString());
		DDB_Close(ddb);
		VID_Finish();
		return state = Player_Error;
	}
	if (!ddb->drawString && DDB_SupportsDataFile(ddb->version, ddb->target) && !VID_LoadDataFile(ddbFileName))
	{
		DebugPrintf("VID_LoadDataFile(%s) failed: %s\n", ddbFileName, DDB_GetErrorString());
		VID_ShowError(DDB_GetErrorString());
		DDB_CloseInterpreter(interpreter);
		DDB_Close(ddb);
		VID_Finish();
		return state = Player_Error;
	}
	#if HAS_PCX
	if (VID_HasExternalPictures())
		screenMode = ScreenMode_VGA;
	#endif
	FadeOut();
	return state = Player_FadingOut;
}

#else

static const int fadeSteps = 8;
static const uint16_t fadeScale[fadeSteps] = { 256, 219, 183, 146, 110, 73, 37, 0 };
static uint32_t fadeSourcePalette[256];
static uint32_t fadeWorkingPalette[256];

static void FadeOut()
{
	if (!VID_IsFadeEnabled())
		return;

	uint16_t fadePaletteSize = VID_GetPaletteSize();
	if (fadePaletteSize > 256)
		fadePaletteSize = 256;
	for (uint16_t i = 0; i < fadePaletteSize; i++)
	{
		uint8_t r, g, b;
		VID_GetPaletteColor(i, &r, &g, &b);
		fadeSourcePalette[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
	}
	for (int frame = 0; frame < fadeSteps; frame++)
	{
		uint16_t scale = fadeScale[frame];
		ScaleFadePalette(fadeSourcePalette, fadeWorkingPalette, fadePaletteSize, scale);
		VID_SetPaletteEntries(fadeWorkingPalette, fadePaletteSize, 0, false, true);
	}
}

// Checks for intro screen files and, if found, updates machine and screenMode accordingly
#if HAS_PCX
static void CheckIntroScreenFiles(const char* ddbFileName, const char** introScreen, DDB_Machine* machine, DDB_Version version, DDB_ScreenMode* screenMode)
#else
static void CheckIntroScreenFiles(const char** introScreen, DDB_Machine* machine, DDB_ScreenMode* screenMode)
#endif
{
	*introScreen = 0;
	scrCount = 0;

	DDB_ScreenMode preferredMode = GetPreferredIntroScreenMode(*screenMode);
	if (preferredMode != ScreenMode_Default)
	{
		if (GetModeSpecificIntroScreen(preferredMode, introScreen))
			return;
	}

	if ((preferredMode == ScreenMode_SHiRes || (preferredMode == ScreenMode_Default && *screenMode == ScreenMode_SHiRes)) &&
		(scrCount = CountFiles(".scx")) > 0)
	{
		*introScreen = GetFile(".scx", 0);
		return;
	}

	scrCount = CountFiles(".scr");
	if (scrCount > 0)
	{
		const char* screenFile = GetFile(".scr", 0);
		if (HasFalconRawHeader(screenFile))
			*introScreen = screenFile;
		if (ShouldPreferAmigaSCR(screenFile, *machine, screenMode))
			*machine = DDB_MACHINE_AMIGA;
		if (*introScreen == 0)
			*introScreen = screenFile;
	}
	else if ((scrCount = SupportsFalconRawIntroScreens() ? CountFiles(".fcr") : 0) > 0)
	{
		*introScreen = GetFile(".fcr", 0);
	}
	else if ((scrCount = CountFiles(".egs")) > 0)
	{
		if (preferredMode != ScreenMode_Default)
			return;
		*introScreen = GetFile(".egs", 0);
	}
	else if ((scrCount = CountFiles(".cgs")) > 0)
	{
		if (preferredMode != ScreenMode_Default)
			return;
		*introScreen = GetFile(".cgs", 0);
	}
	else if ((scrCount = CountFiles(".vgs")) > 0)
	{
		if (preferredMode != ScreenMode_Default)
			return;
		*introScreen = GetFile(".vgs", 0);
	}
	else
	{
		#if HAS_PCX
		const char* pcxIntro = FindPCXIntroScreen(ddbFileName, *machine, version, screenMode);
		if (pcxIntro != 0)
			*introScreen = pcxIntro;
		#endif
	}
}

bool DDB_RunPlayer()
{
	#if HAS_TESTMODE
	if (!DDB_TestIsActive())
	#endif
	{
		uint32_t entropy = 0;
		SCR_GetMilliseconds(&entropy);
		RandSeedFromClock(entropy);
	}

	ResetStartupConfig();
	DDB_Machine machine = DDB_MACHINE_AMIGA;
	DDB_Language language = DDB_SPANISH;
	DDB_ScreenMode screenMode = DDB_GetDefaultScreenMode(machine);
	DDB_Version version = DDB_VERSION_2;
	const char* introScreen = 0;
	bool introVisible = false;
	introScreenName[0] = 0;
	SetStartupPattern("*");

	activeManifestDisk = 0;
	if (TryDirectStartupConfig() && PopulateFilesFromManifest(1))
	{
		// CFG found by direct name probe and it carries a file manifest:
		// the directory enumeration is skipped entirely.
	}
	else
	{
		EnumFiles();
		if (startupConfig.configFile[0] == 0)
			LoadStartupConfig();
	}

	#if HAS_VIRTUALFILESYSTEM
	CollectDiskImages();
	#endif
	
	ddbCount = CountDDBFiles();
	if (ddbCount == 0)
	{
		if (!TryMountDiskImageWithDDBs())
		{
			CloseEnum();
			DDB_SetError(DDB_ERROR_NO_DDBS_FOUND);
			return false;
		}

		ddbCount = CountDDBFiles();
		if (ddbCount == 0)
		{
			CloseEnum();
			DDB_SetError(DDB_ERROR_NO_DDBS_FOUND);
			return false;
		}
	}

	StrCopy(ddbFileName, FILE_MAX_PATH, GetDDBFile(0));
	DebugPrintf("DDB_RunPlayer: selected DDB %s\n", ddbFileName);
	uint8_t displayPlanes = 4;

	if (!ResolveDDBVideoConfig(ddbFileName, &machine, &language, &version, &screenMode, &displayPlanes))
	{
		CloseEnum();
		if (DDB_GetError() == DDB_ERROR_NONE)
			DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}
	#ifdef _AMIGA
	VID_SetDisplayColorModeHint(GetAmigaDisplayColorModeHint(ddbFileName));
	#endif
	VID_SetDisplayPlanesHint(displayPlanes);

	#if HAS_PCX
	CheckIntroScreenFiles(ddbFileName, &introScreen, &machine, version, &screenMode);
	#else
	CheckIntroScreenFiles(&introScreen, &machine, &screenMode);
	#endif
	if (introScreen != 0 && introScreen[0] != 0)
	{
		StrCopy(introScreenName, sizeof(introScreenName), introScreen);
		introScreen = introScreenName;
	}

	#ifdef _AMIGA
	SetKeyboardLayout(GetAmigaKeyboardLayout(language));
	OpenKeyboard();
	OpenAudio();
	#endif

	if (!VID_Initialize(machine, version, screenMode))
		return false;
	if (introScreen != 0)
	{
		if (!VID_DisplaySCRFile(introScreen, machine, true))
			VID_ShowError(GetDisplaySCRFileErrorString());
		else
			introVisible = true;
	}

	bool promptScreenSaved = false;
	if (GetConfiguredPartCount() > 1)
	{
		DebugPrintf("Showing part selector\n");
		ddbSelected = -1;
		loaderPromptMode = LoaderPrompt_Part;
		promptScreenSaved = VID_BackupScreen();
		ShowLoaderPrompt(GetConfiguredPartCount(), language);
		#if HAS_TESTMODE
		if (DDB_TestIsActive() && DDB_TestGetPartSelection() > 0)
		{
			if (!DDB_TestCapturePartSelector())
			{
				CloseEnum();
				VID_Finish();
				return false;
			}
			int part = DDB_TestGetPartSelection();
			if (part < 1 || part > GetConfiguredPartCount())
				part = 1;
			ddbSelected = part - 1;
			DebugPrintf("Test mode auto-selected part %d\n", part);
		}
		else
		#endif
		{
			VID_MainLoop(0, LoaderScreenUpdate);
			if (exitGame)
				return true;
		}
		if (ddbSelected == -1)
		{
			CloseEnum();
			VID_Finish();
			return true;
		}
		DebugPrintf("Selected part %ld\n", (long)(ddbSelected + 1));
		if (promptScreenSaved)
			VID_RestoreBackupScreen();
		if (!EnsureSelectedPartMediaSync(ddbSelected, language))
		{
			CloseEnum();
			DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
			return false;
		}
		StrCopy(ddbFileName, FILE_MAX_PATH, ResolveSelectedDDBFile(ddbSelected));

		DDB_Machine selectedMachine = DDB_MACHINE_AMIGA;
		DDB_Language selectedLanguage = DDB_SPANISH;
		DDB_Version selectedVersion = DDB_VERSION_2;
		DDB_ScreenMode selectedScreenMode = ScreenMode_VGA16;
		uint8_t selectedDisplayPlanes = 4;
		if (!ResolveDDBVideoConfig(ddbFileName, &selectedMachine, &selectedLanguage, &selectedVersion, &selectedScreenMode, &selectedDisplayPlanes))
		{
			CloseEnum();
			if (DDB_GetError() == DDB_ERROR_NONE)
				DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}

		#ifdef _AMIGA
		VID_SetDisplayColorModeHint(GetAmigaDisplayColorModeHint(ddbFileName));
		#endif

		#if HAS_PCX
		CheckIntroScreenFiles(ddbFileName, &introScreen, &selectedMachine, selectedVersion, &selectedScreenMode);
		#else
		CheckIntroScreenFiles(&introScreen, &selectedMachine, &selectedScreenMode);
		#endif
		if (introScreen != 0 && introScreen[0] != 0)
		{
			StrCopy(introScreenName, sizeof(introScreenName), introScreen);
			introScreen = introScreenName;
		}

		if (selectedMachine != machine || selectedVersion != version || selectedScreenMode != screenMode || selectedDisplayPlanes != displayPlanes)
		{
			VID_Finish();
			VID_SetDisplayPlanesHint(selectedDisplayPlanes);
			if (!VID_Initialize(selectedMachine, selectedVersion, selectedScreenMode))
				return false;
			machine = selectedMachine;
			version = selectedVersion;
			screenMode = selectedScreenMode;
			displayPlanes = selectedDisplayPlanes;
			if (introScreen != 0)
				introVisible = VID_DisplaySCRFile(introScreen, machine, false);
		}
		else if (!promptScreenSaved)
		{
			// Backends without screen backup: remove the prompt boxes by
			// redisplaying the intro screen, or clearing when there is none.
			if (introScreen != 0)
				introVisible = VID_DisplaySCRFile(introScreen, machine, false);
			else
			{
				VID_Clear(0, 0, screenWidth, screenHeight, 0, Clear_All);
				introVisible = false;
			}
		}
		language = selectedLanguage;
		#ifdef _AMIGA
		SetKeyboardLayout(GetAmigaKeyboardLayout(language));
		#endif
	}

	CloseEnum();

	// State preserved across an EXIT n (AUTOLOAD) part hand-off: the user
	// variables (flags + object locations) survive while everything else resets.
	uint8_t* autoloadState = 0;
	uint16_t autoloadStateSize = 0;
	bool     autoloading = false;

	while (true)
	{
		DebugPrintf("Loading %s\n", ddbFileName);

		VID_ShowProgressBar(0);
		uint32_t tLoadStart = 0;
		VID_GetMilliseconds(&tLoadStart);
		DDB* ddb = DDB_Load(ddbFileName);
		if (!ddb)
		{
			DebugPrintf("Error loading %s", ddbFileName);
			DebugPrintf(": %s\n", DDB_GetErrorString());
			if (autoloadState) Free(autoloadState);
			return false;
		}
		#ifdef _AMIGA
		SetKeyboardLayout(GetAmigaKeyboardLayout(ddb->language));
		#endif
		if (DDB_RequiresBackBuffer(ddb))
			VID_EnableBackBuffer();

		uint32_t tAfterDDBLoad = 0;
		VID_GetMilliseconds(&tAfterDDBLoad);
		DebugPrintf("DDB_Load completed in %lu ms\n", (unsigned long)(tAfterDDBLoad - tLoadStart));
		VID_ShowProgressBar(64);

		DDB_CreateInterpreter(ddb, screenMode);
		if (interpreter == 0)
		{
			DebugPrintf("Error creating interpreter: %s\n", DDB_GetErrorString());
			DDB_Close(ddb);
			VID_Finish();
			if (autoloadState) Free(autoloadState);
			return false;
		}
		#if HAS_TESTMODE
		DDB_SetSkipTimedPauses(interpreter, VID_IsFastMode());
		#endif
		// The AUTOLOAD (EXIT n) part hand-off relies on this player reloading the
		// requested part; enable it only where that path is available. The Amiga
		// and Atari builds keep the full-reset fallback (complex memory layout).
		#if !defined(_AMIGA) && !defined(_ATARIST)
		DDB_SetAutoloadEnabled(interpreter, true);
		#endif

		// Restore the preserved user variables over the freshly reset state so the
		// autoloaded part continues from the previous part's game state.
		if (autoloadState != 0)
		{
			uint16_t n = autoloadStateSize;
			if (n > interpreter->saveStateSize) n = interpreter->saveStateSize;
			MemCopy(interpreter->buffer, autoloadState, n);
			Free(autoloadState);
			autoloadState = 0;
		}

		if (!ddb->drawString && DDB_SupportsDataFile(ddb->version, ddb->target) && !VID_LoadDataFile(ddbFileName))
		{
			DebugPrintf("VID_LoadDataFile(%s) failed: %s\n", ddbFileName, DDB_GetErrorString());
			VID_ShowError(DDB_GetErrorString());
			DDB_CloseInterpreter(interpreter);
			DDB_Close(ddb);
			VID_Finish();
			return false;
		}

		#if HAS_PCX
		if (VID_HasExternalPictures())
			screenMode = ScreenMode_VGA;
		#endif
		VID_ShowProgressBar(255);

		// The loading/intro screen only belongs to the initial boot; an
		// autoloaded part swaps in silently, as the original interpreter did.
		if (!autoloading)
		{
			if ((scrCount > 0 || introScreen != 0) && GetConfiguredPartCount() == 1)
			{
				if (!introVisible)
					introVisible = VID_DisplaySCRFile(introScreen, machine, false);
				DDB_Interpreter* i = interpreter;
				VID_MainLoop(0, WaitForKeyUpdate);
				interpreter = i;
				if (exitGame)
				{
					DDB_CloseInterpreter(interpreter);
					DDB_Close(ddb);
					VID_Finish();
					return true;
				}
			}
			if (scrCount > 0)
				FadeOut();
		}

		VID_ClearBuffer(true);
		VID_ClearBuffer(false);
		VID_SetPaletteRange(DefaultPalette, 16, 0, VID_GetPaletteSize() <= 16, true);

		DebugPrintf("Starting interpreter\n");
		DDB_Run(interpreter);

		if (interpreter->state == DDB_AUTOLOAD)
		{
			int part = interpreter->autoloadPart;
			DebugPrintf("AUTOLOAD: switching to part %d\n", part);

			// Preserve the user variables (flags + object locations) for the new part.
			autoloadStateSize = interpreter->saveStateSize;
			autoloadState = Allocate<uint8_t>("DDB Autoload state", autoloadStateSize);
			if (autoloadState != 0)
				MemCopy(autoloadState, interpreter->buffer, autoloadStateSize);

			DDB_CloseInterpreter(interpreter);
			DDB_Close(ddb);

			// Give the new part an independent random stream, identical to booting
			// it fresh, so a joined multi-part playthrough stays reproducible.
			RandReset();

			// DDB_AUTOLOAD broke the main loop through SCR_Quit, which also asked
			// the game to exit; clear that so the next part's loop keeps running.
			exitGame = false;

			// Resolve the requested part's DDB (all parts share the same platform
			// configuration, so no video re-initialisation is needed).
			if (!PopulateFilesFromManifest(activeManifestDisk))
				EnumFiles();
			#if HAS_VIRTUALFILESYSTEM
			CollectDiskImages();
			#endif
			ddbCount = CountDDBFiles();
			int partIndex = part - 1;
			if (!EnsureSelectedPartMediaSync(partIndex, language))
			{
				CloseEnum();
				if (autoloadState) Free(autoloadState);
				DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
				VID_Finish();
				return false;
			}
			StrCopy(ddbFileName, FILE_MAX_PATH, ResolveSelectedDDBFile(partIndex));
			CloseEnum();

			autoloading = true;
			continue;
		}

		DDB_CloseInterpreter(interpreter);
		DDB_Close(ddb);
		VID_Finish();

		if (autoloadState) Free(autoloadState);
		return true;
	}
}

#endif
