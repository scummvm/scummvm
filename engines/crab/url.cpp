#include "url.h"
#include "stdafx.h"

#ifdef __WIN32__
#include <ShellAPI.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CFBundle.h>
#endif

void OpenURL(const std::string &url_str) {
#ifdef __WIN32__
	ShellExecuteA(NULL, "open", url_str.c_str(), NULL, NULL, SW_SHOW);
#endif

#ifdef __APPLE__
	CFURLRef url = CFURLCreateWithBytes(
		NULL,                     // allocator
		(UInt8 *)url_str.c_str(), // URLBytes
		url_str.length(),         // length
		kCFStringEncodingASCII,   // encoding
		NULL                      // baseURL
	);
	LSOpenCFURLRef(url, 0);
	CFRelease(url);
#endif

#ifdef __GNUC__
	std::string command = "xdg-open " + url_str;
	system(command.c_str());
#endif
}