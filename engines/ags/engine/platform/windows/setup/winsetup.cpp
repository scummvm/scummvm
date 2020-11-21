//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS

#include <windows.h>
#include <commctrl.h>
#include <crtdbg.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <memory>
#include <algorithm>
#include <set>
#include <vector>
#include "ac/gamestructdefines.h"
#undef RGB
#undef PALETTE
#define RGB void*
#define PALETTE void*
#include "gfx/gfxdriverfactory.h"
#include "gfx/gfxfilter.h"
#include "gfx/graphicsdriver.h"
#include "main/config.h"
#include "main/graphics_mode.h"
#include "platform/base/agsplatformdriver.h"
#include "resource/resource.h"
#include "util/file.h"
#include "util/string_utils.h"

#define AL_ID(a,b,c,d)     (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))

#define DIGI_DIRECTAMX(n)        AL_ID('A','X','A'+(n),' ')
// This DirectX hardware mixer is crap, it crashes the program
// when two sound effects are played at once
#define DIGI_DIRECTX(n)          AL_ID('D','X','A'+(n),' ')
#define DIGI_WAVOUTID(n)         AL_ID('W','O','A'+(n),' ')
#define DIGI_NONE  0
#define MIDI_AUTODETECT       -1 
#define MIDI_NONE             0 
#define MIDI_WIN32MAPPER         AL_ID('W','3','2','M')

extern "C"
{
    HWND win_get_window();
}

namespace AGS
{
namespace Engine
{

using namespace AGS::Common;

//=============================================================================
//
// WinConfig struct, keeps all configurable data.
//
//=============================================================================
struct WinConfig
{
    String Title;
    String VersionString;

    String DataDirectory;
    String UserSaveDir;
    GameResolutionType GameResType;
    Size   GameResolution;
    int    GameColourDepth;
    bool   LetterboxByDesign;

    String GfxDriverId;
    String GfxFilterId;
    Size   ScreenSize;
    GameFrameSetup FsGameFrame;
    GameFrameSetup WinGameFrame;
    int    RefreshRate;
    bool   Windowed;
    bool   VSync;
    bool   RenderAtScreenRes;
    bool   AntialiasSprites;

    int    DigiID;
    int    MidiID;
    bool   ThreadedAudio;
    bool   UseVoicePack;

    bool   MouseAutoLock;
    float  MouseSpeed;

    int    SpriteCacheSize;
    String DefaultLanguageName;
    String Language;

    WinConfig();
    void SetDefaults();
    void Load(const ConfigTree &cfg);
    void Save(ConfigTree &cfg);
};

WinConfig::WinConfig()
{
    SetDefaults();
}

void WinConfig::SetDefaults()
{
    DataDirectory = ".";
    GameResType = kGameResolution_Undefined;
    GameColourDepth = 0;
    LetterboxByDesign = false;

    GfxFilterId = "StdScale";
    GfxDriverId = "D3D9";
    ScreenSize = get_desktop_size();
    FsGameFrame.ScaleDef = kFrame_MaxProportional;
    WinGameFrame.ScaleDef = kFrame_MaxRound;
    RefreshRate = 0;
    Windowed = false;
    VSync = false;
    RenderAtScreenRes = false;
    AntialiasSprites = false;

    MouseAutoLock = false;
    MouseSpeed = 1.f;

    DigiID = -1; // autodetect
    MidiID = -1;
    ThreadedAudio = false;
    UseVoicePack = true;

    SpriteCacheSize = 1024 * 128;
    DefaultLanguageName = "Game Default";

    Title = "Game Setup";
}

void WinConfig::Load(const ConfigTree &cfg)
{
    DataDirectory = INIreadstring(cfg, "misc", "datadir", DataDirectory);
    UserSaveDir = INIreadstring(cfg, "misc", "user_data_dir");
    // Backward-compatible resolution type
    GameResType = (GameResolutionType)INIreadint(cfg, "misc", "defaultres", GameResType);
    if (GameResType < kGameResolution_Undefined || GameResType >= kNumGameResolutions)
        GameResType = kGameResolution_Undefined;
    GameResolution.Width = INIreadint(cfg, "misc", "game_width", GameResolution.Width);
    GameResolution.Height = INIreadint(cfg, "misc", "game_height", GameResolution.Height);
    GameColourDepth = INIreadint(cfg, "misc", "gamecolordepth", GameColourDepth);
    LetterboxByDesign = INIreadint(cfg, "misc", "letterbox", 0) != 0;

    GfxDriverId = INIreadstring(cfg, "graphics", "driver", GfxDriverId);
    GfxFilterId = INIreadstring(cfg, "graphics", "filter", GfxFilterId);
    ScreenSize.Width = INIreadint(cfg, "graphics", "screen_width", ScreenSize.Width);
    ScreenSize.Height = INIreadint(cfg, "graphics", "screen_height", ScreenSize.Height);

    parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_fs", make_scaling_option(FsGameFrame)), FsGameFrame);
    parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_win", make_scaling_option(WinGameFrame)), WinGameFrame);

    RefreshRate = INIreadint(cfg, "graphics", "refresh", RefreshRate);
    Windowed = INIreadint(cfg, "graphics", "windowed", Windowed ? 1 : 0) != 0;
    VSync = INIreadint(cfg, "graphics", "vsync", VSync ? 1 : 0) != 0;
    RenderAtScreenRes = INIreadint(cfg, "graphics", "render_at_screenres", RenderAtScreenRes ? 1 : 0) != 0;

    AntialiasSprites = INIreadint(cfg, "misc", "antialias", AntialiasSprites ? 1 : 0) != 0;

    DigiID = read_driverid(cfg, "sound", "digiid", DigiID);
    MidiID = read_driverid(cfg, "sound", "midiid", MidiID);
    ThreadedAudio = INIreadint(cfg, "sound", "threaded", ThreadedAudio ? 1 : 0) != 0;
    UseVoicePack = INIreadint(cfg, "sound", "usespeech", UseVoicePack ? 1 : 0) != 0;

    MouseAutoLock = INIreadint(cfg, "mouse", "auto_lock", MouseAutoLock ? 1 : 0) != 0;
    MouseSpeed = INIreadfloat(cfg, "mouse", "speed", 1.f);
    if (MouseSpeed <= 0.f)
        MouseSpeed = 1.f;

    SpriteCacheSize = INIreadint(cfg, "misc", "cachemax", SpriteCacheSize);
    Language = INIreadstring(cfg, "language", "translation", Language);
    DefaultLanguageName = INIreadstring(cfg, "language", "default_translation_name", DefaultLanguageName);

    Title = INIreadstring(cfg, "misc", "titletext", Title);
}

void WinConfig::Save(ConfigTree &cfg)
{
    INIwritestring(cfg, "misc", "user_data_dir", UserSaveDir);

    INIwritestring(cfg, "graphics", "driver", GfxDriverId);
    INIwritestring(cfg, "graphics", "filter", GfxFilterId);
    INIwritestring(cfg, "graphics", "screen_def", Windowed ? "scaling" : "explicit");
    INIwriteint(cfg, "graphics", "screen_width", ScreenSize.Width);
    INIwriteint(cfg, "graphics", "screen_height", ScreenSize.Height);
    INIwritestring(cfg, "graphics", "game_scale_fs", make_scaling_option(FsGameFrame));
    INIwritestring(cfg, "graphics", "game_scale_win", make_scaling_option(WinGameFrame));
    INIwriteint(cfg, "graphics", "refresh", RefreshRate);
    INIwriteint(cfg, "graphics", "windowed", Windowed ? 1 : 0);
    INIwriteint(cfg, "graphics", "vsync", VSync ? 1 : 0);
    INIwriteint(cfg, "graphics", "render_at_screenres", RenderAtScreenRes ? 1 : 0);

    INIwriteint(cfg, "misc", "antialias", AntialiasSprites ? 1 : 0);

    write_driverid(cfg, "sound", "digiid", DigiID);
    write_driverid(cfg, "sound", "midiid", MidiID);
    INIwriteint(cfg, "sound", "threaded", ThreadedAudio ? 1 : 0);
    INIwriteint(cfg, "sound", "usespeech", UseVoicePack ? 1 : 0);

    INIwriteint(cfg, "mouse", "auto_lock", MouseAutoLock ? 1 : 0);
    INIwritestring(cfg, "mouse", "speed", String::FromFormat("%0.1f", MouseSpeed));

    INIwriteint(cfg, "misc", "cachemax", SpriteCacheSize);
    INIwritestring(cfg, "language", "translation", Language);
}


//=============================================================================
//
// WinAPI interaction helpers
//
//=============================================================================

int AddString(HWND hwnd, LPCTSTR text, DWORD_PTR data = 0L)
{
    int index = SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)text);
    if (index >= 0)
        SendMessage(hwnd, CB_SETITEMDATA, index, data);
    return index;
}

int InsertString(HWND hwnd, LPCTSTR text, int at_index, DWORD_PTR data = 0L)
{
    int index = SendMessage(hwnd, CB_INSERTSTRING, at_index, (LPARAM)text);
    if (index >= 0)
        SendMessage(hwnd, CB_SETITEMDATA, index, data);
    return index;
}

int GetItemCount(HWND hwnd)
{
    return SendMessage(hwnd, CB_GETCOUNT, 0, 0L);
}

bool GetCheck(HWND hwnd)
{
    return SendMessage(hwnd, BM_GETCHECK, 0, 0) != FALSE;
}

void SetCheck(HWND hwnd, bool check)
{
    SendMessage(hwnd, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0);
}

int GetCurSel(HWND hwnd)
{
    return SendMessage(hwnd, CB_GETCURSEL, 0, 0);
}

void SetCurSel(HWND hwnd, int cur_sel)
{
    SendMessage(hwnd, CB_SETCURSEL, cur_sel, 0);
}

typedef bool (*PfnCompareCBItemData)(DWORD_PTR data1, DWORD_PTR data2);

bool CmpICBItemDataAsStr(DWORD_PTR data1, DWORD_PTR data2)
{
    LPCTSTR text_ptr1 = (LPCTSTR)data1;
    LPCTSTR text_ptr2 = (LPCTSTR)data2;
    return text_ptr1 && text_ptr2 && StrCmpI(text_ptr1, text_ptr2) == 0 || !text_ptr1 && !text_ptr2;
}

int SetCurSelToItemData(HWND hwnd, DWORD_PTR data, PfnCompareCBItemData pfn_cmp = NULL, int def_sel = -1)
{
    int count = SendMessage(hwnd, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; ++i)
    {
        DWORD_PTR item_data = SendMessage(hwnd, CB_GETITEMDATA, i, 0);
        if (pfn_cmp && pfn_cmp(item_data, data) || !pfn_cmp && item_data == data)
        {
            LRESULT res = SendMessage(hwnd, CB_SETCURSEL, i, 0);
            if (res != CB_ERR)
                return res;
            break;
        }
    }
    return SendMessage(hwnd, CB_SETCURSEL, def_sel, 0);
}

int SetCurSelToItemDataStr(HWND hwnd, LPCTSTR text, int def_sel = -1)
{
    return SetCurSelToItemData(hwnd, (DWORD_PTR)text, CmpICBItemDataAsStr, def_sel);
}

DWORD_PTR GetCurItemData(HWND hwnd, DWORD_PTR def_value = 0)
{
    int index = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (index >= 0)
        return SendMessage(hwnd, CB_GETITEMDATA, index, 0);
    return def_value;
}

String GetText(HWND hwnd)
{
    TCHAR short_buf[MAX_PATH + 1];
    int len = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    if (len > 0)
    {
        TCHAR *buf = len >= sizeof(short_buf) ? new TCHAR[len + 1] : short_buf;
        SendMessage(hwnd, WM_GETTEXT, len + 1, (LPARAM)buf);
        String s = buf;
        if (buf != short_buf)
            delete [] buf;
        return s;
    }
    return "";
}

void SetText(HWND hwnd, LPCTSTR text)
{
    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)text);
}

void ResetContent(HWND hwnd)
{
    SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
}

void SetSliderRange(HWND hwnd, int min, int max)
{
    SendMessage(hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(min, max));
}

int GetSliderPos(HWND hwnd)
{
    return SendMessage(hwnd, TBM_GETPOS, 0, 0);
}

void SetSliderPos(HWND hwnd, int pos)
{
    SendMessage(hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
}

void MakeFullLongPath(const char *path, char *out_buf, int buf_len)
{
    GetFullPathName(path, buf_len, out_buf, NULL);
    GetLongPathName(out_buf, out_buf, buf_len);
}


//=============================================================================
//
// Browse-for-folder dialog
//
//=============================================================================

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        // Set initial selection
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData);
    }
    return 0;
}

bool BrowseForFolder(String &dir_buf)
{
    bool res = false;
    CoInitialize(NULL);

    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Select location for game saves and custom data files";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)dir_buf.GetCStr();
    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
    if (pidl)
    {
        char path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path) != FALSE)
        {
            dir_buf = path;
            res = true;
        }
        CoTaskMemFree(pidl);
    }

    CoUninitialize();
    return res;
}


//=============================================================================
//
// WinSetupDialog, handles the dialog UI.
//
//=============================================================================
class WinSetupDialog
{
public:
    enum GfxModeSpecial
    {
        kGfxMode_None    = -1,
        kGfxMode_Desktop = -2,
        kGfxMode_GameRes = -3,
    };

    static const int MouseSpeedMin = 1;
    static const int MouseSpeedMax = 100;

public:
    WinSetupDialog(const ConfigTree &cfg_in, ConfigTree &cfg_out, const String &data_dir, const String &version_str);
    ~WinSetupDialog();
    static SetupReturnValue ShowModal(const ConfigTree &cfg_in, ConfigTree &cfg_out,
                                      const String &data_dir, const String &version_str);

private:
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Event handlers
    INT_PTR OnInitDialog(HWND hwnd);
    INT_PTR OnCommand(WORD id);
    INT_PTR OnListSelection(WORD id);
    void OnCustomSaveDirBtn();
    void OnCustomSaveDirCheck();
    void OnGfxDriverUpdate();
    void OnGfxFilterUpdate();
    void OnGfxModeUpdate();
    void OnScalingUpdate(HWND hlist, GameFrameSetup &frame_setup, bool windowed);
    void OnWindowedUpdate();
    void ShowAdvancedOptions();

    // Helper structs
    typedef std::vector<DisplayMode> VDispModes;
    // NOTE: we have to implement IGfxModeList for now because we are using
    // few engine functions that take IGfxModeList as parameter
    struct GfxModes : public IGfxModeList
    {
        VDispModes Modes;

        virtual int  GetModeCount() const;
        virtual bool GetMode(int index, DisplayMode &mode) const;
    };

    typedef std::vector<GfxFilterInfo> VFilters;
    struct DriverDesc
    {
        String      Id;            // internal id
        String      UserName;      // human-friendly driver name
        GfxModes    GfxModeList;   // list of supported modes
        VFilters    FilterList;    // list of supported filters
        int         UseColorDepth; // recommended display depth
    };

    // Operations
    void AddScalingString(HWND hlist, int scaling_factor);
    void FillGfxFilterList();
    void FillGfxModeList();
    void FillLanguageList();
    void FillScalingList(HWND hlist, GameFrameSetup &frame_setup, bool windowed);
    void InitGfxModes();
    void InitDriverDescFromFactory(const String &id);
    void SaveSetup();
    void SelectNearestGfxMode(const Size screen_size);
    void SetGfxModeText();
    void UpdateMouseSpeedText();

    // Dialog singleton and properties
    static WinSetupDialog *_dlg;
    HWND _hwnd;
    WinConfig _winCfg;
    const ConfigTree &_cfgIn;
    ConfigTree &_cfgOut;
    // Window size
    Size _winSize;
    Size _baseSize;
    // Driver descriptions
    typedef std::shared_ptr<DriverDesc> PDriverDesc;
    typedef std::map<String, PDriverDesc> DriverDescMap;
    DriverDescMap _drvDescMap;
    PDriverDesc _drvDesc;
    GfxFilterInfo _gfxFilterInfo;
    // Resolution limits
    Size _desktopSize;
    Size _maxWindowSize;
    Size _minGameSize;
    int _maxGameScale = 0;
    int _minGameScale = 0;

    // Dialog controls
    HWND _hVersionText = NULL;
    HWND _hCustomSaveDir = NULL;
    HWND _hCustomSaveDirBtn = NULL;
    HWND _hCustomSaveDirCheck = NULL;
    HWND _hGfxDriverList = NULL;
    HWND _hGfxModeList = NULL;
    HWND _hGfxFilterList = NULL;
    HWND _hFsScalingList = NULL;
    HWND _hWinScalingList = NULL;
    HWND _hDigiDriverList = NULL;
    HWND _hMidiDriverList = NULL;
    HWND _hLanguageList = NULL;
    HWND _hSpriteCacheList = NULL;
    HWND _hWindowed = NULL;
    HWND _hVSync = NULL;
    HWND _hRenderAtScreenRes = NULL;
    HWND _hRefresh85Hz = NULL;
    HWND _hAntialiasSprites = NULL;
    HWND _hThreadedAudio = NULL;
    HWND _hUseVoicePack = NULL;
    HWND _hAdvanced = NULL;
    HWND _hGameResolutionText = NULL;
    HWND _hGfxModeText = NULL;
    HWND _hMouseLock = NULL;
    HWND _hMouseSpeed = NULL;
    HWND _hMouseSpeedText = NULL;
};

WinSetupDialog *WinSetupDialog::_dlg = NULL;

WinSetupDialog::WinSetupDialog(const ConfigTree &cfg_in, ConfigTree &cfg_out, const String &data_dir, const String &version_str)
    : _hwnd(NULL)
    , _cfgIn(cfg_in)
    , _cfgOut(cfg_out)
{
    _winCfg.DataDirectory = data_dir;
    _winCfg.VersionString = version_str;
}

WinSetupDialog::~WinSetupDialog()
{
}

SetupReturnValue WinSetupDialog::ShowModal(const ConfigTree &cfg_in, ConfigTree &cfg_out,
                                           const String &data_dir, const String &version_str)
{
    _dlg = new WinSetupDialog(cfg_in, cfg_out, data_dir, version_str);
    INT_PTR dlg_res = DialogBoxParam(GetModuleHandle(NULL), (LPCTSTR)IDD_SETUP, win_get_window(),
        (DLGPROC)WinSetupDialog::DialogProc, 0L);
    delete _dlg;
    _dlg = NULL;

    switch (dlg_res)
    {
    case IDOKRUN: return kSetup_RunGame;
    case IDOK: return kSetup_Done;
    default: return kSetup_Cancel;
    }
}

INT_PTR WinSetupDialog::OnInitDialog(HWND hwnd)
{
    _hwnd                   = hwnd;
    _hVersionText           = GetDlgItem(_hwnd, IDC_VERSION);
    _hCustomSaveDir         = GetDlgItem(_hwnd, IDC_CUSTOMSAVEDIR);
    _hCustomSaveDirBtn      = GetDlgItem(_hwnd, IDC_CUSTOMSAVEDIRBTN);
    _hCustomSaveDirCheck    = GetDlgItem(_hwnd, IDC_CUSTOMSAVEDIRCHECK);
    _hGfxDriverList         = GetDlgItem(_hwnd, IDC_GFXDRIVER);
    _hGfxModeList           = GetDlgItem(_hwnd, IDC_GFXMODE);
    _hGfxFilterList         = GetDlgItem(_hwnd, IDC_GFXFILTER);
    _hFsScalingList         = GetDlgItem(_hwnd, IDC_FSSCALING);
    _hWinScalingList        = GetDlgItem(_hwnd, IDC_WINDOWSCALING);
    _hDigiDriverList        = GetDlgItem(_hwnd, IDC_DIGISOUND);
    _hMidiDriverList        = GetDlgItem(_hwnd, IDC_MIDIMUSIC);
    _hLanguageList          = GetDlgItem(_hwnd, IDC_LANGUAGE);
    _hSpriteCacheList       = GetDlgItem(_hwnd, IDC_SPRITECACHE);
    _hWindowed              = GetDlgItem(_hwnd, IDC_WINDOWED);
    _hVSync                 = GetDlgItem(_hwnd, IDC_VSYNC);
    _hRenderAtScreenRes     = GetDlgItem(_hwnd, IDC_RENDERATSCREENRES);
    _hRefresh85Hz           = GetDlgItem(_hwnd, IDC_REFRESH_85HZ);
    _hAntialiasSprites      = GetDlgItem(_hwnd, IDC_ANTIALIAS);
    _hThreadedAudio         = GetDlgItem(_hwnd, IDC_THREADEDAUDIO);
    _hUseVoicePack          = GetDlgItem(_hwnd, IDC_VOICEPACK);
    _hAdvanced              = GetDlgItem(_hwnd, IDC_ADVANCED);
    _hGameResolutionText    = GetDlgItem(_hwnd, IDC_RESOLUTION);
    _hGfxModeText           = GetDlgItem(_hwnd, IDC_GFXMODETEXT);
    _hMouseLock             = GetDlgItem(_hwnd, IDC_MOUSE_AUTOLOCK);
    _hMouseSpeed            = GetDlgItem(_hwnd, IDC_MOUSESPEED);
    _hMouseSpeedText        = GetDlgItem(_hwnd, IDC_MOUSESPEED_TEXT);

    _desktopSize = get_desktop_size();
    _maxWindowSize = _desktopSize;
    AGSPlatformDriver::GetDriver()->ValidateWindowSize(_maxWindowSize.Width, _maxWindowSize.Height, false);
    _minGameSize = Size(320, 200);
    _maxGameScale = 1;
    _minGameScale = 1;

    _winCfg.Load(_cfgIn);

    // Custom save dir controls
    String custom_save_dir = _winCfg.UserSaveDir;
    bool has_save_dir = !custom_save_dir.IsEmpty();
    if (!has_save_dir)
        custom_save_dir = _winCfg.DataDirectory;
    SetCheck(_hCustomSaveDirCheck, has_save_dir);
    char full_save_dir[MAX_PATH] = {0};
    MakeFullLongPath(custom_save_dir, full_save_dir, MAX_PATH);
    SetText(_hCustomSaveDir, full_save_dir);
    EnableWindow(_hCustomSaveDir, has_save_dir ? TRUE : FALSE);
    EnableWindow(_hCustomSaveDirBtn, has_save_dir ? TRUE : FALSE);

    // Resolution controls
    if (_winCfg.GameResolution.IsNull() &&
          (_winCfg.GameResType == kGameResolution_Undefined || _winCfg.GameResType == kGameResolution_Custom) ||
          _winCfg.GameColourDepth == 0)
        MessageBox(_hwnd, "Essential information about the game is missing in the configuration file. Setup program may be unable to deduce graphic modes properly.", "Initialization error", MB_OK | MB_ICONWARNING);

    if (_winCfg.GameResolution.IsNull())
        _winCfg.GameResolution = ResolutionTypeToSize(_winCfg.GameResType, _winCfg.LetterboxByDesign);

    SetText(_hwnd, _winCfg.Title);
    SetText(win_get_window(), _winCfg.Title);
    SetText(_hGameResolutionText, String::FromFormat("Native game resolution: %d x %d x %d",
        _winCfg.GameResolution.Width, _winCfg.GameResolution.Height, _winCfg.GameColourDepth));

    SetText(_hVersionText, _winCfg.VersionString);

    InitGfxModes();

    for (DriverDescMap::const_iterator it = _drvDescMap.begin(); it != _drvDescMap.end(); ++it)
        AddString(_hGfxDriverList, it->second->UserName, (DWORD_PTR)it->second->Id.GetCStr());
    SetCurSelToItemDataStr(_hGfxDriverList, _winCfg.GfxDriverId.GetCStr(), 0);
    OnGfxDriverUpdate();

    SetCheck(_hWindowed, _winCfg.Windowed);
    OnWindowedUpdate();

    FillScalingList(_hFsScalingList, _winCfg.FsGameFrame, false);
    FillScalingList(_hWinScalingList, _winCfg.WinGameFrame, true);

    SetCheck(_hVSync, _winCfg.VSync);

    SetCheck(_hRenderAtScreenRes, _winCfg.RenderAtScreenRes);

    AddString(_hDigiDriverList, "No Digital Sound", DIGI_NONE);
    AddString(_hDigiDriverList, "Default device (auto)", MIDI_AUTODETECT);
    AddString(_hDigiDriverList, "Default DirectSound Device", DIGI_DIRECTAMX(0));
    AddString(_hDigiDriverList, "Default WaveOut Device", DIGI_WAVOUTID(0));
    AddString(_hDigiDriverList, "DirectSound (Hardware mixer)", DIGI_DIRECTX(0));
    SetCurSelToItemData(_hDigiDriverList, _winCfg.DigiID);

    AddString(_hMidiDriverList, "No MIDI music", MIDI_NONE);
    AddString(_hMidiDriverList, "Default device (auto)", MIDI_AUTODETECT);
    AddString(_hMidiDriverList, "Win32 MIDI Mapper", MIDI_WIN32MAPPER);
    SetCurSelToItemData(_hMidiDriverList, _winCfg.MidiID);

    FillLanguageList();

    SetCheck(_hMouseLock, _winCfg.MouseAutoLock);

    SetSliderRange(_hMouseSpeed, MouseSpeedMin, MouseSpeedMax);
    int slider_pos = (int)(_winCfg.MouseSpeed * 10.f + .5f);
    SetSliderPos(_hMouseSpeed, slider_pos);
    UpdateMouseSpeedText();

    AddString(_hSpriteCacheList, "16 MB", 16);
    AddString(_hSpriteCacheList, "32 MB", 32);
    AddString(_hSpriteCacheList, "64 MB", 64);
    AddString(_hSpriteCacheList, "128 MB (default)", 128);
    AddString(_hSpriteCacheList, "256 MB", 256);
    AddString(_hSpriteCacheList, "384 MB", 384);
    AddString(_hSpriteCacheList, "512 MB", 512);
    SetCurSelToItemData(_hSpriteCacheList, _winCfg.SpriteCacheSize / 1024, NULL, 3);

    SetCheck(_hRefresh85Hz, _winCfg.RefreshRate == 85);
    SetCheck(_hAntialiasSprites, _winCfg.AntialiasSprites);
    SetCheck(_hThreadedAudio, _winCfg.ThreadedAudio);
    SetCheck(_hUseVoicePack, _winCfg.UseVoicePack);
    if (!File::TestReadFile("speech.vox"))
        EnableWindow(_hUseVoicePack, FALSE);

    if (INIreadint(_cfgIn, "disabled", "threaded_audio", 0) != 0)
        EnableWindow(_hThreadedAudio, FALSE);
    if (INIreadint(_cfgIn, "disabled", "speechvox", 0) != 0)
        EnableWindow(_hUseVoicePack, FALSE);
    if (INIreadint(_cfgIn, "disabled", "filters", 0) != 0)
        EnableWindow(_hGfxFilterList, FALSE);
    if (INIreadint(_cfgIn, "disabled", "render_at_screenres", 0) != 0)
        EnableWindow(_hRenderAtScreenRes, FALSE);

    RECT win_rect, gfx_rect, adv_rect, border;
    GetWindowRect(_hwnd, &win_rect);
    GetWindowRect(GetDlgItem(_hwnd, IDC_GFXOPTIONS), &gfx_rect);
    _winSize.Width = win_rect.right - win_rect.left;
    _winSize.Height = win_rect.bottom - win_rect.top;
    GetWindowRect(_hAdvanced, &adv_rect);
    border.left = border.top = border.right = border.bottom = 9;
    MapDialogRect(_hwnd, &border);
    _baseSize.Width = (adv_rect.right + (gfx_rect.left - win_rect.left)) - win_rect.left;
    _baseSize.Height = adv_rect.bottom - win_rect.top + border.bottom;

    MoveWindow(_hwnd, max(0, win_rect.left + (_winSize.Width - _baseSize.Width) / 2),
                      max(0, win_rect.top + (_winSize.Height - _baseSize.Height) / 2),
                      _baseSize.Width, _baseSize.Height, TRUE);
    SetFocus(GetDlgItem(_hwnd, IDOK));
    return FALSE; // notify WinAPI that we set focus ourselves
}

INT_PTR WinSetupDialog::OnCommand(WORD id)
{
    switch (id)
    {
    case IDC_ADVANCED:  ShowAdvancedOptions(); break;
    case IDC_WINDOWED:  OnWindowedUpdate(); break;
    case IDC_CUSTOMSAVEDIRBTN: OnCustomSaveDirBtn(); break;
    case IDC_CUSTOMSAVEDIRCHECK: OnCustomSaveDirCheck(); break;
    case IDOK:
    case IDOKRUN:
        SaveSetup();
        // fall-through intended
    case IDCANCEL:
        EndDialog(_hwnd, id);
        return TRUE;
    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR WinSetupDialog::OnListSelection(WORD id)
{
    switch (id)
    {
    case IDC_GFXDRIVER: OnGfxDriverUpdate(); break;
    case IDC_GFXFILTER: OnGfxFilterUpdate(); break;
    case IDC_GFXMODE:   OnGfxModeUpdate(); break;
    case IDC_FSSCALING: OnScalingUpdate(_hFsScalingList, _winCfg.FsGameFrame, false); break;
    case IDC_WINDOWSCALING: OnScalingUpdate(_hWinScalingList, _winCfg.WinGameFrame, true); break;
    default:
        return FALSE;
    }
    return TRUE;
}

void WinSetupDialog::OnCustomSaveDirBtn()
{
    String save_dir = GetText(_hCustomSaveDir);
    if (BrowseForFolder(save_dir))
    {
        SetText(_hCustomSaveDir, save_dir);
    }
}

void WinSetupDialog::OnCustomSaveDirCheck()
{
    bool custom_save_dir = GetCheck(_hCustomSaveDirCheck);
    EnableWindow(_hCustomSaveDir, custom_save_dir ? TRUE : FALSE);
    EnableWindow(_hCustomSaveDirBtn, custom_save_dir ? TRUE : FALSE);
}

void WinSetupDialog::OnGfxDriverUpdate()
{
    _winCfg.GfxDriverId = (LPCTSTR)GetCurItemData(_hGfxDriverList);

    DriverDescMap::const_iterator it = _drvDescMap.find(_winCfg.GfxDriverId);
    if (it != _drvDescMap.end())
        _drvDesc = it->second;
    else
        _drvDesc.reset();

    FillGfxModeList();
    FillGfxFilterList();
}

void WinSetupDialog::OnGfxFilterUpdate()
{
    _winCfg.GfxFilterId = (LPCTSTR)GetCurItemData(_hGfxFilterList);

    _gfxFilterInfo = GfxFilterInfo();
    for (size_t i = 0; i < _drvDesc->FilterList.size(); ++i)
    {
        if (_drvDesc->FilterList[i].Id.CompareNoCase(_winCfg.GfxFilterId) == 0)
        {
            _gfxFilterInfo = _drvDesc->FilterList[i];
            break;
        }
    }
}

void WinSetupDialog::OnGfxModeUpdate()
{
    DWORD_PTR sel = GetCurItemData(_hGfxModeList);
    if (sel == kGfxMode_Desktop)
        _winCfg.ScreenSize = _desktopSize;
    else if (sel == kGfxMode_GameRes)
        _winCfg.ScreenSize = _winCfg.GameResolution;
    else
    {
        const DisplayMode &mode = _drvDesc->GfxModeList.Modes[sel];
        _winCfg.ScreenSize = Size(mode.Width, mode.Height);
    }
}

void WinSetupDialog::OnScalingUpdate(HWND hlist, GameFrameSetup &frame_setup, bool windowed)
{
    int scale = GetCurItemData(hlist);
    if (scale >= 0 && scale < kNumFrameScaleDef)
    {
        frame_setup.ScaleDef = (FrameScaleDefinition)scale;
        frame_setup.ScaleFactor = 0;
    }
    else
    {
        frame_setup.ScaleDef = kFrame_IntScale;
        frame_setup.ScaleFactor = scale >= 0 ? scale - kNumFrameScaleDef : scale;
    }

    if (windowed)
        SetGfxModeText();
}

void WinSetupDialog::OnWindowedUpdate()
{
    _winCfg.Windowed = GetCheck(_hWindowed);

    if (_winCfg.Windowed)
    {
        ShowWindow(_hGfxModeList, SW_HIDE);
        ShowWindow(_hGfxModeText, SW_SHOW);
        SetGfxModeText();
    }
    else
    {
        ShowWindow(_hGfxModeList, SW_SHOW);
        ShowWindow(_hGfxModeText, SW_HIDE);
    }

    SelectNearestGfxMode(_winCfg.ScreenSize);
}

void WinSetupDialog::ShowAdvancedOptions()
{
    // Reveal the advanced bit of the window
    ShowWindow(_hAdvanced, SW_HIDE);

    RECT win_rect;
    GetWindowRect(_hwnd, &win_rect);
    MoveWindow(_hwnd, max(0, win_rect.left + (_baseSize.Width - _winSize.Width) / 2),
                      max(0, win_rect.top + (_baseSize.Height - _winSize.Height) / 2),
                      _winSize.Width, _winSize.Height, TRUE);

    int offset = _winSize.Height - _baseSize.Height;
    RECT rc;
    int ctrl_ids[] = { IDC_VERSION, IDOK, IDOKRUN, IDCANCEL, 0 };
    for (int i = 0; ctrl_ids[i]; ++i)
    {
        HWND hctrl = GetDlgItem(_hwnd, ctrl_ids[i]);
        GetWindowRect(hctrl, &rc);
        ScreenToClient(_hwnd, (POINT*)&rc);
        ScreenToClient(_hwnd, (POINT*)&rc.right);
        MoveWindow(hctrl, rc.left, rc.top + offset, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
}

INT_PTR CALLBACK WinSetupDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        _ASSERT(_dlg != NULL && _dlg->_hwnd == NULL);
        return _dlg->OnInitDialog(hwndDlg);
    case WM_COMMAND:
        _ASSERT(_dlg != NULL && _dlg->_hwnd != NULL);
        if (HIWORD(wParam) == CBN_SELCHANGE)
            return _dlg->OnListSelection(LOWORD(wParam));
        return _dlg->OnCommand(LOWORD(wParam));
    case WM_HSCROLL:
        _ASSERT(_dlg != NULL && _dlg->_hwnd != NULL);
        _dlg->UpdateMouseSpeedText();
        return TRUE;
    default:
        return FALSE;
    }
}

int WinSetupDialog::GfxModes::GetModeCount() const
{
    return Modes.size();
}

bool WinSetupDialog::GfxModes::GetMode(int index, DisplayMode &mode) const
{
    if (index >= 0 && (size_t)index < Modes.size())
    {
        mode = Modes[index];
        return true;
    }
    return false;
}

void WinSetupDialog::AddScalingString(HWND hlist, int scaling_factor)
{
    String s;
    if (scaling_factor >= 0)
        s = String::FromFormat("x%d", scaling_factor);
    else
        s = String::FromFormat("1/%d", -scaling_factor);
    AddString(hlist, s, (DWORD_PTR)(scaling_factor >= 0 ? scaling_factor + kNumFrameScaleDef : scaling_factor));
}

void WinSetupDialog::FillGfxFilterList()
{
    ResetContent(_hGfxFilterList);

    if (!_drvDesc)
    {
        _gfxFilterInfo = GfxFilterInfo();
        return;
    }

    for (size_t i = 0; i < _drvDesc->FilterList.size(); ++i)
    {
        const GfxFilterInfo &info = _drvDesc->FilterList[i];
        if (INIreadint(_cfgIn, "disabled", info.Id, 0) == 0)
            AddString(_hGfxFilterList, info.Name, (DWORD_PTR)info.Id.GetCStr());
    }

    SetCurSelToItemDataStr(_hGfxFilterList, _winCfg.GfxFilterId, 0);
    OnGfxFilterUpdate();
}

void WinSetupDialog::FillGfxModeList()
{
    ResetContent(_hGfxModeList);

    if (!_drvDesc)
    {
        OnGfxModeUpdate();
        return;
    }

    const VDispModes &modes = _drvDesc->GfxModeList.Modes;
    bool has_desktop_mode = false;
    bool has_native_mode = false;
    String buf;
    for (VDispModes::const_iterator mode = modes.begin(); mode != modes.end(); ++mode)
    {
        if (mode->Width == _desktopSize.Width && mode->Height == _desktopSize.Height)
        {
            has_desktop_mode = true;
            continue;
        }
        else if (mode->Width == _winCfg.GameResolution.Width && mode->Height == _winCfg.GameResolution.Height)
        {
            has_native_mode = true;
            continue;
        }
        buf.Format("%d x %d", mode->Width, mode->Height);
        AddString(_hGfxModeList, buf, (DWORD_PTR)(mode - modes.begin()));
    }

    int spec_mode_idx = 0;
    if (has_desktop_mode)
        InsertString(_hGfxModeList, String::FromFormat("Desktop resolution (%d x %d)",
            _desktopSize.Width, _desktopSize.Height), spec_mode_idx++, (DWORD_PTR)kGfxMode_Desktop);
    if (has_native_mode)
        InsertString(_hGfxModeList, String::FromFormat("Native game resolution (%d x %d)",
            _winCfg.GameResolution.Width, _winCfg.GameResolution.Height), spec_mode_idx++, (DWORD_PTR)kGfxMode_GameRes);

    SelectNearestGfxMode(_winCfg.ScreenSize);
}

void WinSetupDialog::FillLanguageList()
{
    ResetContent(_hLanguageList);
    AddString(_hLanguageList, _winCfg.DefaultLanguageName.GetCStr());
    SetCurSel(_hLanguageList, 0);

    String path_mask = String::FromFormat("%s\\*.tra", _winCfg.DataDirectory.GetCStr());
    WIN32_FIND_DATAA file_data;
    HANDLE find_handle = FindFirstFile(path_mask, &file_data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        bool found_sel = false;
        do
        {
            LPTSTR ext = PathFindExtension(file_data.cFileName);
            if (ext && StrCmpI(ext, ".tra") == 0)
            {
                file_data.cFileName[0] = toupper(file_data.cFileName[0]);
                *ext = 0;
                int index = AddString(_hLanguageList, file_data.cFileName);
                if (!found_sel && _winCfg.Language.CompareNoCase(file_data.cFileName) == 0)
                {
                    SetCurSel(_hLanguageList, index);
                    found_sel = true;
                }
            }
        }
        while (FindNextFileA(find_handle, &file_data) != FALSE);
        FindClose(find_handle);
    }
}

void WinSetupDialog::FillScalingList(HWND hlist, GameFrameSetup &frame_setup, bool windowed)
{
    ResetContent(hlist);

    const int min_scale = min(_winCfg.GameResolution.Width / _minGameSize.Width, _winCfg.GameResolution.Height / _minGameSize.Height);
    const Size max_size = windowed ? _maxWindowSize : _winCfg.ScreenSize;
    const int max_scale = _winCfg.GameResolution.IsNull() ? 1 :
        min(max_size.Width / _winCfg.GameResolution.Width, max_size.Height / _winCfg.GameResolution.Height);
    _maxGameScale = max(1, max_scale);
    _minGameScale = -max(1, min_scale);

    if (windowed)
        AddString(hlist, "None (original game size)", 1 + kNumFrameScaleDef);

    AddString(hlist, "Max round multiplier", kFrame_MaxRound);
    AddString(hlist, "Fill whole screen", kFrame_MaxStretch);
    AddString(hlist, "Stretch, preserving aspect ratio", kFrame_MaxProportional);

    if (windowed && !_winCfg.GameResolution.IsNull())
    {
        // Add integer multipliers
        for (int scale = 2; scale <= _maxGameScale; ++scale)
            AddScalingString(hlist, scale);
    }

    SetCurSelToItemData(hlist,
        frame_setup.ScaleDef == kFrame_IntScale ? frame_setup.ScaleFactor + kNumFrameScaleDef : frame_setup.ScaleDef, NULL, 0);

    EnableWindow(hlist, SendMessage(hlist, CB_GETCOUNT, 0, 0) > 1 ? TRUE : FALSE);
    OnScalingUpdate(hlist, frame_setup, windowed);
}

void WinSetupDialog::InitGfxModes()
{
    InitDriverDescFromFactory("D3D9");
    InitDriverDescFromFactory("OGL");
    InitDriverDescFromFactory("Software");

    if (_drvDescMap.size() == 0)
        MessageBox(_hwnd, "Unable to detect any supported graphic drivers!", "Initialization error", MB_OK | MB_ICONERROR);
}

// "Less" predicate that compares two display modes only by their screen metrics
bool SizeLess(const DisplayMode &first, const DisplayMode &second)
{
    return Size(first.Width, first.Height) < Size(second.Width, second.Height);
}

void WinSetupDialog::InitDriverDescFromFactory(const String &id)
{
    IGfxDriverFactory *gfx_factory = GetGfxDriverFactory(id);
    if (!gfx_factory)
        return;
    IGraphicsDriver *gfx_driver = gfx_factory->GetDriver();
    if (!gfx_driver)
    {
        gfx_factory->Shutdown();
        return;
    }

    PDriverDesc drv_desc(new DriverDesc());
    drv_desc->Id = gfx_driver->GetDriverID();
    drv_desc->UserName = gfx_driver->GetDriverName();
    drv_desc->UseColorDepth =
        gfx_driver->GetDisplayDepthForNativeDepth(_winCfg.GameColourDepth ? _winCfg.GameColourDepth : 32);

    IGfxModeList *gfxm_list = gfx_driver->GetSupportedModeList(drv_desc->UseColorDepth);
    VDispModes &modes = drv_desc->GfxModeList.Modes;
    if (gfxm_list)
    {
        std::set<Size> unique_sizes; // trying to hide modes which only have different refresh rates
        for (int i = 0; i < gfxm_list->GetModeCount(); ++i)
        {
            DisplayMode mode;
            gfxm_list->GetMode(i, mode);
            if (mode.ColorDepth != drv_desc->UseColorDepth || unique_sizes.count(Size(mode.Width, mode.Height)) != 0)
                continue;
            unique_sizes.insert(Size(mode.Width, mode.Height));
            modes.push_back(mode);
        }
        std::sort(modes.begin(), modes.end(), SizeLess);
        delete gfxm_list;
    }
    if (modes.size() == 0)
    {
        // Add two default modes in hope that engine will be able to handle them (or fallbacks to something else)
        modes.push_back(DisplayMode(GraphicResolution(_desktopSize.Width, _desktopSize.Height, drv_desc->UseColorDepth)));
        modes.push_back(DisplayMode(GraphicResolution(_winCfg.GameResolution.Width, _winCfg.GameResolution.Height, drv_desc->UseColorDepth)));
    }

    drv_desc->FilterList.resize(gfx_factory->GetFilterCount());
    for (size_t i = 0; i < drv_desc->FilterList.size(); ++i)
    {
        drv_desc->FilterList[i] = *gfx_factory->GetFilterInfo(i);
    }

    gfx_factory->Shutdown();
    _drvDescMap[drv_desc->Id] = drv_desc;
}

void WinSetupDialog::SaveSetup()
{
    const bool custom_save_dir = GetCheck(_hCustomSaveDirCheck);
    if (custom_save_dir)
    {
        // Compare user path with the game data directory. If user chose
        // path pointing inside game's directory, then store relative
        // path instead; thus the path will keep pointing at game's
        // directory if user moves game elsewhere.
        String save_dir;
        save_dir = GetText(_hCustomSaveDir);
        char full_data_dir[MAX_PATH] = {0};
        char full_save_dir[MAX_PATH] = {0};
        MakeFullLongPath(_winCfg.DataDirectory, full_data_dir, MAX_PATH);
        MakeFullLongPath(save_dir, full_save_dir, MAX_PATH);
        char rel_save_dir[MAX_PATH] = {0};
        if (PathRelativePathTo(rel_save_dir, full_data_dir, FILE_ATTRIBUTE_DIRECTORY, full_save_dir, FILE_ATTRIBUTE_DIRECTORY) &&
            strstr(rel_save_dir, "..") == NULL)
        {
            _winCfg.UserSaveDir = rel_save_dir;
        }
        else
        {
            _winCfg.UserSaveDir = save_dir;
        }
    }
    else
    {
        _winCfg.UserSaveDir = "";
    }

    _winCfg.DigiID = GetCurItemData(_hDigiDriverList);
    _winCfg.MidiID = GetCurItemData(_hMidiDriverList);

    if (GetCurSel(_hLanguageList) == 0)
        _winCfg.Language.Empty();
    else
        _winCfg.Language = GetText(_hLanguageList);
    _winCfg.SpriteCacheSize = GetCurItemData(_hSpriteCacheList) * 1024;
    _winCfg.ThreadedAudio = GetCheck(_hThreadedAudio);
    _winCfg.UseVoicePack = GetCheck(_hUseVoicePack);
    _winCfg.VSync = GetCheck(_hVSync);
    _winCfg.RenderAtScreenRes = GetCheck(_hRenderAtScreenRes);
    _winCfg.AntialiasSprites = GetCheck(_hAntialiasSprites);
    _winCfg.RefreshRate = GetCheck(_hRefresh85Hz) ? 85 : 0;
    _winCfg.GfxFilterId = (LPCTSTR)GetCurItemData(_hGfxFilterList);

    _winCfg.MouseAutoLock = GetCheck(_hMouseLock);
    int slider_pos = GetSliderPos(_hMouseSpeed);
    _winCfg.MouseSpeed = (float)slider_pos / 10.f;

    _winCfg.Save(_cfgOut);
}

void WinSetupDialog::SelectNearestGfxMode(const Size screen_size)
{
    if (!_drvDesc)
    {
        OnGfxModeUpdate();
        return;
    }

    // First check two special modes
    if (screen_size == _desktopSize)
    {
        SetCurSelToItemData(_hGfxModeList, kGfxMode_Desktop);
    }
    else if (screen_size == _winCfg.GameResolution)
    {
        SetCurSelToItemData(_hGfxModeList, kGfxMode_GameRes);
    }
    else
    {
        // Look up for the nearest supported mode
        int index = -1;
        DisplayMode dm;
        if (find_nearest_supported_mode(_drvDesc->GfxModeList, screen_size, _drvDesc->UseColorDepth,
                                        NULL, NULL, dm, &index))
        {
            SetCurSelToItemData(_hGfxModeList, index, NULL, kGfxMode_Desktop);
        }
        else
            SetCurSelToItemData(_hGfxModeList, kGfxMode_Desktop);
    }
    OnGfxModeUpdate();
}

void WinSetupDialog::SetGfxModeText()
{
    Size sz;
    const GameFrameSetup &frame_setup = _winCfg.WinGameFrame;
    if (frame_setup.ScaleDef == kFrame_MaxStretch)
    {
        sz = _maxWindowSize;
    }
    else if (frame_setup.ScaleDef == kFrame_MaxProportional)
    {
        sz = ProportionalStretch(_maxWindowSize, _winCfg.GameResolution);
    }
    else
    {
        int scale = 0;
        if (frame_setup.ScaleDef == kFrame_MaxRound)
            scale = _maxGameScale;
        else
            scale = frame_setup.ScaleFactor;

        if (scale >= 0)
        {
            sz.Width  = _winCfg.GameResolution.Width * scale;
            sz.Height = _winCfg.GameResolution.Height * scale;
        }
        else
        {
            sz.Width  = _winCfg.GameResolution.Width / (-scale);
            sz.Height = _winCfg.GameResolution.Height / (-scale);
        }
    }
    String text = String::FromFormat("%d x %d", sz.Width, sz.Height);
    SetText(_hGfxModeText, text);
}

void WinSetupDialog::UpdateMouseSpeedText()
{
    int slider_pos = GetSliderPos(_hMouseSpeed);
    float mouse_speed = (float)slider_pos / 10.f;
    String text = mouse_speed == 1.f ? "Mouse speed: x 1.0 (Default)" : String::FromFormat("Mouse speed: x %0.1f", mouse_speed);
    SetText(_hMouseSpeedText, text);
}

//=============================================================================
//
// Windows setup entry point.
//
//=============================================================================
void SetWinIcon()
{
    SetClassLong(win_get_window(),GCL_HICON,
        (LONG) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON))); 
}

SetupReturnValue WinSetup(const ConfigTree &cfg_in, ConfigTree &cfg_out,
                          const String &game_data_dir, const String &version_str)
{
    return WinSetupDialog::ShowModal(cfg_in, cfg_out, game_data_dir, version_str);
}

} // namespace Engine
} // namespace AGS

#endif // AGS_PLATFORM_OS_WINDOWS