/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/textconsole.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/resource.h"
#include "qdengine/core/runtime/qd_dialogs.h"
#include "qdengine/core/runtime/qd_dialog_control.h"
#include "qdengine/core/system/graphics/ddraw_gr_dispatcher.h"
/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

namespace qdrt {
extern DDraw_grDispatcher *dd_grD;
};

/* --------------------------- PROTOTYPE SECTION ---------------------------- */

namespace qdlg {
BOOL APIENTRY settings_dlgproc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
void settings_dialog_init(HWND hdlg);
void settings_dialog_finit();

qdlgOption *find_option(int ctl_id);

void update_color_option();

}; // namespace qdlg

/* --------------------------- DEFINITION SECTION --------------------------- */

namespace qdlg {

typedef std::list<qdlgOption *> options_container_t;
options_container_t options_;

void set_icon(void *hwnd) {
#ifndef _DEBUG

warning("STUB: qdlg::set_icon()");
#if 0
	HICON ic = (HICON)LoadImage(NULL, "qd_game.ico", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	if (!ic)
		ic = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

	if (ic) {
		SendMessage((HWND)hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)ic);
		SendMessage((HWND)hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)ic);
	}
#endif

#endif
}

void settings_dialog() {

	warning("STUB: settings_dialog()");
#if 0
	InitCommonControls();
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), NULL, settings_dlgproc);
#endif
}



BOOL APIENTRY settings_dlgproc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	int ctl_id;

	warning("STUB: settings_dlgproc()");
#if 0
	switch (msg) {
	case WM_INITDIALOG:
		settings_dialog_init(hdlg);
		return 1;
	case WM_COMMAND:
		ctl_id = LOWORD(wParam);
		switch (ctl_id) {
		case IDOK:
			settings_dialog_finit();
			EndDialog(hdlg, 1);
			return 1;
		case IDCANCEL:
			EndDialog(hdlg, 0);
			return 1;
		case IDC_FULLSCREEN:
			update_color_option();
			return 1;
		}
		return 0;
	}
#endif
	return 0;
}

void settings_dialog_init(HWND hdlg) {

	warning("STUB: set_icon() method. Argument requires different data-type");
#if 0
	set_icon(hdlg);
#endif

	qdlgOption *p = new qdlgOptionDroplist;
	p -> set_control(hdlg, IDC_COLOR_DEPTH);
	p -> set_ini_key("qd_game.ini", "graphics", "color_depth");

	qdlgOptionDroplist *dp = static_cast<qdlgOptionDroplist *>(p);
	dp -> add_item("High Color (16bit)", 0);
	dp -> add_item("High Color (16bit)", 1);
	dp -> add_item("True Color (24bit)", 2);
	dp -> add_item("True Color (32bit)", 3);
	dp -> init_control();

	options_.push_back(p);

	p = new qdlgOptionCheckbox;
	p -> set_control(hdlg, IDC_FULLSCREEN);
	p -> set_ini_key("qd_game.ini", "graphics", "fullscreen");
	options_.push_back(p);

	p = new qdlgOptionCheckbox;
	p -> set_control(hdlg, IDC_SOUND);
	p -> set_ini_key("qd_game.ini", "sound", "enable_sound");
	options_.push_back(p);

	p = new qdlgOptionCheckbox;
	p -> set_control(hdlg, IDC_MUSIC);
	p -> set_ini_key("qd_game.ini", "sound", "enable_music");
	options_.push_back(p);

	qdlgOptionSlider *sp = new qdlgOptionSlider;
	sp -> set_control(hdlg, IDC_SOUND_VOLUME);
	sp -> set_value_range(0, 255, true);
	sp -> set_ini_key("qd_game.ini", "sound", "sound_volume");
	options_.push_back(sp);

	sp = new qdlgOptionSlider;
	sp -> set_control(hdlg, IDC_MUSIC_VOLUME);
	sp -> set_value_range(0, 255, true);
	sp -> set_ini_key("qd_game.ini", "sound", "music_volume");
	options_.push_back(sp);

#if 0
	SendDlgItemMessage(hdlg, IDC_SOUND_VOLUME, TBM_SETTICFREQ, (WPARAM)32, (LPARAM)0);
	SendDlgItemMessage(hdlg, IDC_MUSIC_VOLUME, TBM_SETTICFREQ, (WPARAM)32, (LPARAM)0);
#endif

	for (options_container_t::const_iterator it = options_.begin(); it != options_.end(); ++it)
		(*it) -> load_value();

	update_color_option();
}

void settings_dialog_finit() {
	for (options_container_t::const_iterator it = options_.begin(); it != options_.end(); ++it)
		(*it) -> save_value();
}

void update_color_option() {
	qdlgOption *sp = find_option(IDC_FULLSCREEN);
	if (!sp) return;

	qdlgOptionDroplist *dp = dynamic_cast<qdlgOptionDroplist *>(find_option(IDC_COLOR_DEPTH));
	if (!dp) return;

	for (int i = 0; i < 4; i++) {
		if (qdrt::dd_grD -> is_mode_supported(grPixelFormat(i)))
			dp -> enable_item(i);
		else
			dp -> disable_item(i);
	}

	dp -> init_control();
	if (!sp -> value()) {
		dp -> enable_control(false);

		int sx, sy;
		grPixelFormat pixel_format;
		if (qdrt::dd_grD->get_current_mode(sx, sy, pixel_format))
			dp -> set_value(pixel_format);
	} else
		dp -> enable_control(true);
}

qdlgOption *find_option(int ctl_id) {
	for (options_container_t::iterator it = options_.begin(); it != options_.end(); ++it) {
		if ((*it) -> control_ID() == ctl_id)
			return *it;
	}

	return NULL;
}

}; // namespace qdlg
