#ifndef __VIDEO_MEMORY_INFORMATION_H_INCLUDED__
#define __VIDEO_MEMORY_INFORMATION_H_INCLUDED__

#include <vector>
#include "XMath\xmath.h"

//Вызывать только после вызова CoInitializeEx(0, COINIT_MULTITHREADED);
int GetVideoMemory();
struct IDirect3D9;

bool CheckDeviceType(IDirect3D9* lpD3D, int xscr, int yscr, bool fullscreen, bool stencil, bool alpha, std::vector<DWORD>* multisamplemode = 0);
bool getSupportedResolutions(IDirect3D9* lpD3D, bool fullscreen, bool stencil, bool alpha, std::vector<Vect2i>& modes);

#endif