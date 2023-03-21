#include "stdafx.h"
#include "url.h"

#ifdef __WIN32__
#include <windows.h>
#include <ShellAPI.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

void OpenURL(const std::string &url_str)
{
#ifdef __WIN32__
	ShellExecuteA(NULL, "open", url_str.c_str(), NULL, NULL, SW_SHOW);
#endif

#ifdef __APPLE__
	CFURLRef url = CFURLCreateWithBytes(
		NULL,                        // allocator
		(UInt8*)url_str.c_str(),     // URLBytes
		url_str.length(),            // length
		kCFStringEncodingASCII,      // encoding
		NULL                         // baseURL
		);
	LSOpenCFURLRef(url, 0);
	CFRelease(url);
#endif

#ifdef __GNUC__
	std::string command = "xdg-open " + url_str;
	system(command.c_str());
#endif
}