#ifndef __WIN_VERSION_H_
#define __WIN_VERSION_H_

struct WinVersion 
{
	enum {
		WIN_9X,
		WIN_NT,
		WIN_2K,
		WIN_XP,
		WIN_NEW
	} version;

	bool isWinNT;

	WinVersion();
};

WinVersion::WinVersion()
{
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	GetVersionEx(&vi);

	isWinNT = (vi.dwPlatformId != VER_PLATFORM_WIN32s && vi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);

	if(!isWinNT)
		version = WIN_9X;
	else if(vi.dwMajorVersion > 5)
		version = WIN_NEW;
	else if(vi.dwMajorVersion > 4)
		if(vi.dwMinorVersion >= 1)
			version = WIN_XP;
		else
			version = WIN_2K;
	else
		version = WIN_NT;
}

const WinVersion& getWinVersion()
{
	static WinVersion ver;
	return ver;
}

bool isWinNT()
{
	return getWinVersion().isWinNT;
}

#endif //__WIN_VERSION_H_