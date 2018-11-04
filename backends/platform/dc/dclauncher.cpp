#define FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp

#include "DCLauncherDialog.h"

#include "dc.h"

#include "common/translation.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "gui/gui-manager.h"
#include "gui/massadd.h"
#include "gui/browser.h"
#include "gui/launcher.h"

#include "base/plugins.h"

#include "icon.h"

#include <ronin/gddrive.h>

extern Icon icon;

DCLauncherDialog::DCLauncherDialog()
	: LauncherDialog(), cdWasOpen(false)
{
	PluginManager::instance().loadAllPlugins();
#if defined(DYNAMIC_MODULES)
	if (PluginManager::instance().getPlugins(PLUGIN_TYPE_ENGINE).empty()) {
		GUI::BrowserDialog browser(_("Select plugin directory"), true);
		if (browser.runModal() > 0) {
			Common::FSNode result = browser.getResult();
			((OSystem_Dreamcast *)g_system)->setPluginCustomDirectory(result.getPath());
			PluginManager::instance().loadAllPlugins();
		}
	}
#endif
	massAdd();
}

void DCLauncherDialog::massAdd()
{
	GUI::MassAddDialog massAddDlg(Common::FSNode(""), true);
	massAddDlg.runModal();
	// Update the ListWidget and force a redraw
	updateListing();
	g_gui.scheduleTopDialogRedraw();
}

void DCLauncherDialog::handleTickle()
{
	unsigned int param[4];
	gdGdcGetDrvStat(param);

	int cdState = param[0];
	if (cdState >= 6 && !cdWasOpen) {
		cdfs_reinit();
		updateListing();
		g_gui.scheduleTopDialogRedraw();
		cdWasOpen = true;
	}
	else if (cdState > 0 && cdState < 6 && cdWasOpen) {
		cdWasOpen = false;
		cdfs_reinit();
		massAdd();
	}
}

static bool isIcon(const Common::FSNode &entry)
{
	int l = entry.getDisplayName().size();
	if (l>4 && !strcasecmp(entry.getDisplayName().c_str()+l-4, ".ICO"))
		return true;
	else
		return false;
}

static bool loadIcon(Icon &icon, const char *filename) {
	return icon.load(filename);
}

static bool findIcon(Icon &icon, const Common::String &path) {
	Common::FSNode dir(path);
	Common::FSList filelist, dirlist;

	dir.getChildren(filelist, Common::FSNode::kListFilesOnly);
	for (Common::FSList::const_iterator entry = filelist.begin(); entry != filelist.end(); ++entry) {
		if (isIcon(*entry)) {
			if (loadIcon(icon, entry->getPath().c_str()))
				return true;
		}
	}

	dir.getChildren(dirlist, Common::FSNode::kListDirectoriesOnly);
	for (Common::FSList::const_iterator entry = dirlist.begin(); entry != dirlist.end(); ++entry) {
		if (entry->isDirectory()) {
			if (findIcon(icon, entry->getPath()))
				return true;
		}
	}

	return false;
}

void DCLauncherDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	char icofn[520];
	LauncherDialog::handleCommand(sender, cmd, data);
	// Set the icon if a game has been selected
	Common::String gameid = ConfMan.get("gameid");
	if (!gameid.empty()) {
		Common::String path = ConfMan.get("path");
		snprintf(icofn, sizeof(icofn), "%s%s.ICO", path.c_str(), gameid.c_str());
		if (loadIcon(icon, icofn))
			return;
		else if (findIcon(icon, path))
			return;
		else {
			// Use the default icon
			#include "deficon.h"
			icon.load(scummvm_icon, sizeof(scummvm_icon));
		}
	}
}
