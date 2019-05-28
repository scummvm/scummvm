/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// TODO: Remove header when the latest changes to the Windows SDK have been integrated into MingW
//       For reference, the interface definitions here are imported from the SDK headers and MingW-w64

#ifndef BACKEND_WIN32_DIALOGS_MINGW_H
#define BACKEND_WIN32_DIALOGS_MINGW_H

#if defined(WIN32)
#if defined(__GNUC__)
#ifdef __MINGW32__

#ifdef _WIN32_WINNT
	#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <initguid.h>
#include <shlwapi.h>
#include <shlguid.h>

// MinGW does not understand COM interfaces
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

// Dialog GUID definitions
DEFINE_GUID(CLSID_FileOpenDialog, 0xdc1c5a9c, 0xe88a, 0x4dde, 0xa5,0xa1, 0x60,0xf8,0x2a,0x20,0xae,0xf7);
DEFINE_GUID(IID_IFileOpenDialog, 0xd57c7288, 0xd4ad, 0x4768, 0xbe,0x02, 0x9d,0x96,0x95,0x32,0xd9,0x60);
DEFINE_GUID(IID_IShellItem, 0x43826d1e, 0xe718, 0x42ee, 0xbc,0x55, 0xa1,0xe2,0x61,0xc3,0x7b,0xfe);

typedef enum _SIGDN {
	SIGDN_NORMALDISPLAY	= 0,
	SIGDN_PARENTRELATIVEPARSING	= 0x80018001,
	SIGDN_DESKTOPABSOLUTEPARSING	= 0x80028000,
	SIGDN_PARENTRELATIVEEDITING	= 0x80031001,
	SIGDN_DESKTOPABSOLUTEEDITING	= 0x8004c000,
	SIGDN_FILESYSPATH	= 0x80058000,
	SIGDN_URL	= 0x80068000,
	SIGDN_PARENTRELATIVEFORADDRESSBAR	= 0x8007c001,
	SIGDN_PARENTRELATIVE	= 0x80080001,
	SIGDN_PARENTRELATIVEFORUI	= 0x80094001
} 	SIGDN;

enum _SICHINTF {
	SICHINT_DISPLAY	= 0,
	SICHINT_ALLFIELDS	= 0x80000000,
	SICHINT_CANONICAL	= 0x10000000,
	SICHINT_TEST_FILESYSPATH_IF_NOT_EQUAL	= 0x20000000
} ;
typedef DWORD SICHINTF;

// Shell item
#define INTERFACE IShellItem
DECLARE_INTERFACE_(IShellItem, IUnknown) {	
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;
	STDMETHOD (BindToHandler) (IBindCtx *pbc, REFGUID bhid, REFIID riid, void **ppv) PURE;
	STDMETHOD (GetParent) (IShellItem **ppsi) PURE;
	STDMETHOD (GetDisplayName) (SIGDN sigdnName, LPWSTR *ppszName) PURE;
	STDMETHOD (GetAttributes) (SFGAOF sfgaoMask, SFGAOF *psfgaoAttribs) PURE;
	STDMETHOD (Compare) (IShellItem *psi, SICHINTF hint, int *piOrder) PURE;
};
#undef INTERFACE

// Mingw-specific defines for dialog integration
typedef struct _COMDLG_FILTERSPEC {
	LPCWSTR pszName;
	LPCWSTR pszSpec;
} COMDLG_FILTERSPEC;

typedef enum FDAP {
	FDAP_BOTTOM	= 0,
	FDAP_TOP	= 1
} 	FDAP;

enum _FILEOPENDIALOGOPTIONS {
	FOS_OVERWRITEPROMPT	= 0x2,
	FOS_STRICTFILETYPES	= 0x4,
	FOS_NOCHANGEDIR	= 0x8,
	FOS_PICKFOLDERS	= 0x20,
	FOS_FORCEFILESYSTEM	= 0x40,
	FOS_ALLNONSTORAGEITEMS	= 0x80,
	FOS_NOVALIDATE	= 0x100,
	FOS_ALLOWMULTISELECT	= 0x200,
	FOS_PATHMUSTEXIST	= 0x800,
	FOS_FILEMUSTEXIST	= 0x1000,
	FOS_CREATEPROMPT	= 0x2000,
	FOS_SHAREAWARE	= 0x4000,
	FOS_NOREADONLYRETURN	= 0x8000,
	FOS_NOTESTFILECREATE	= 0x10000,
	FOS_HIDEMRUPLACES	= 0x20000,
	FOS_HIDEPINNEDPLACES	= 0x40000,
	FOS_NODEREFERENCELINKS	= 0x100000,
	FOS_DONTADDTORECENT	= 0x2000000,
	FOS_FORCESHOWHIDDEN	= 0x10000000,
	FOS_DEFAULTNOMINIMODE	= 0x20000000,
	FOS_FORCEPREVIEWPANEON	= 0x40000000,
	FOS_SUPPORTSTREAMABLEITEMS	= 0x80000000
};
typedef DWORD FILEOPENDIALOGOPTIONS;

// TODO: Need to implement these if they ever get used
typedef interface IFileDialogEvents IFileDialogEvents;
typedef interface IShellItemFilter IShellItemFilter;
typedef interface IShellItemArray IShellItemArray;

// Open dialog interface
#define INTERFACE IFileOpenDialog
DECLARE_INTERFACE_(IFileOpenDialog, IUnknown) {
	// IUnknown
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;
	// IModalWindow
	STDMETHOD (Show) (THIS_ HWND hwndOwner) PURE;
	// IFileDialog
	STDMETHOD (SetFileTypes) (THIS_ UINT cFileTypes, const COMDLG_FILTERSPEC *rgFilterSpec) PURE;
	STDMETHOD (SetFileTypeIndex) (THIS_ UINT iFileType) PURE;
	STDMETHOD (GetFileTypeIndex) (THIS_ UINT *piFileType) PURE;
	STDMETHOD (Advise) (THIS_ IFileDialogEvents *pfde, DWORD *pdwCookie) PURE;
	STDMETHOD (Unadvise) (THIS_ DWORD dwCookie) PURE;
	STDMETHOD (SetOptions) (THIS_ FILEOPENDIALOGOPTIONS fos) PURE;
	STDMETHOD (GetOptions) (THIS_ FILEOPENDIALOGOPTIONS *pfos) PURE;
	STDMETHOD (SetDefaultFolder) (THIS_ IShellItem *psi) PURE;
	STDMETHOD (SetFolder) (THIS_ IShellItem *psi) PURE;
	STDMETHOD (GetFolder) (THIS_ IShellItem **ppsi) PURE;
	STDMETHOD (GetCurrentSelection) (THIS_ IShellItem **ppsi) PURE;
	STDMETHOD (SetFileName) (THIS_ LPCWSTR pszName) PURE;
	STDMETHOD (GetFileName) (THIS_ LPWSTR *pszName) PURE;
	STDMETHOD (SetTitle) (THIS_ LPCWSTR pszTitle) PURE;
	STDMETHOD (SetOkButtonLabel) (THIS_ LPCWSTR pszText) PURE;
	STDMETHOD (SetFileNameLabel) (THIS_ LPCWSTR pszLabel) PURE;
	STDMETHOD (GetResult) (THIS_ IShellItem **ppsi) PURE;
	STDMETHOD (AddPlace) (THIS_ IShellItem *psi, FDAP fdap) PURE;
	STDMETHOD (SetDefaultExtension) (THIS_ LPCWSTR pszDefaultExtension) PURE;
	STDMETHOD (Close) (THIS_ HRESULT hr) PURE;
	STDMETHOD (SetClientGuid) (THIS_ REFGUID guid) PURE;
	STDMETHOD (ClearClientData) (THIS) PURE;
	STDMETHOD (SetFilter) (THIS_ IShellItemFilter *pFilter) PURE;
	// IFileOpenDialog
	STDMETHOD (GetResults) (THIS_ IShellItemArray **ppenum) PURE;
	STDMETHOD (GetSelectedItems) (THIS_ IShellItemArray **ppsai) PURE;
};
#undef INTERFACE

#endif // __MINGW32__
#endif // __GNUC__
#endif // WIN32

#endif // BACKEND_WIN32_DIALOGS_MINGW_H
