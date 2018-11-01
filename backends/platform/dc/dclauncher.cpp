#include "DCLauncherDialog.h"

#include "dc.h"

#include "common/translation.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "gui/gui-manager.h"
#include "gui/massadd.h"
#include "gui/browser.h"

#include "base/plugins.h"

#include <ronin/gddrive.h>

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
