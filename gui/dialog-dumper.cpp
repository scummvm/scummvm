/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gui/dialog-dumper.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/translation.h"
#include "common/util.h"

#include "gui/ThemeEngine.h"
#include "gui/dialog.h"
#include "gui/dialog-registry.h"
#include "gui/dialog-registrations.h"
#include "gui/editgamedialog.h"
#include "gui/gui-manager.h"
#include "gui/integrity-dialog.h"
#include "gui/textviewer.h"
#include "gui/widget.h"

#include "engines/metaengine.h"

#if defined(ENABLE_SCUMM) && !defined(DYNAMIC_MODULES)
#include "engines/scumm/dialogs.h"
#endif

#include "base/plugins.h"

namespace GUI {

static Common::String lowercased(const Common::String &str) {
	Common::String result(str);
	result.toLowercase();
	return result;
}

DialogDumper::DialogDumper(const DumpConfig &config)
	: _config(config) {
	// Register all built-in GUI dialogs
	DialogDumperRegistry::clear();
	registerAllDialogs();

	// The text viewer shows the log file in normal use, which changes on every run.
	// Show a fixed text instead so the snapshots can be compared.
	DialogDumperRegistry::registerDialog("TextViewerDialog", createTextViewerDialog, Common::String());
}

Dialog *DialogDumper::createTextViewerDialog(DialogDumper &dumper, const Common::String &engineId) {
	Common::Path sample(dumper._config.outputDir + "/.textviewer-sample.txt", '/');
	if (!Common::FSNode(sample).exists()) {
		Common::DumpFile out;
		if (out.open(sample, true)) {
			for (int i = 1; i <= 60; ++i)
				out.writeString(Common::String::format("Line %d: The quick brown fox jumps over the lazy dog.\n", i));
			out.close();
		}
	}
	return new TextViewerDialog(sample);
}

Dialog *DialogDumper::createEditGameDialog(DialogDumper &dumper, const Common::String &engineId) {
	return new EditGameDialog(dumper.createTempDomain(engineId));
}

Dialog *DialogDumper::createIntegrityDialog(DialogDumper &dumper, const Common::String &engineId) {
#ifdef USE_HTTP
	// The integrity check scans the game directory, use an empty one.
	Common::Path emptyDir(dumper._config.outputDir + "/.empty", '/');
	Common::FSNode emptyNode(emptyDir);
	if (!emptyNode.isDirectory())
		emptyNode.createDirectory();
	return new IntegrityDialog("https://example.invalid", dumper.createTempDomain(engineId, emptyDir));
#else
	return nullptr;
#endif
}

DialogDumper::~DialogDumper() {
	removeTempDomains();
	// Do not leave the registered factories around until static destruction.
	DialogDumperRegistry::clear();
}

Common::String DialogDumper::createTempDomain(const Common::String &engineId, const Common::Path &path) {
	// The game of the temporary target does not matter much, the first one
	// supported by the engine is enough to get its options.
	Common::String domain = "dump-" + engineId;
	Common::String gameId;

	const PluginList &plugins = PluginMan.getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);
	for (const Plugin *plugin : plugins) {
		const MetaEngineDetection &detection = plugin->get<MetaEngineDetection>();
		if (engineId != detection.getName())
			continue;
		PlainGameList games = detection.getSupportedGames();
		if (!games.empty())
			gameId = games[0].gameId;
		break;
	}

	if (!ConfMan.hasGameDomain(domain)) {
		ConfMan.addGameDomain(domain);
		_tempDomains.push_back(domain);
	}
	ConfMan.setPath("path", path, domain);
	ConfMan.set("engineid", engineId, domain);
	ConfMan.set("gameid", gameId, domain);
	ConfMan.set("description", gameId, domain);
	// Not saved to the configuration file
	ConfMan.set("id_came_from_command_line", "1", domain);
	return domain;
}

void DialogDumper::removeTempDomains() {
	for (const Common::String &domain : _tempDomains)
		ConfMan.removeGameDomain(domain);
	_tempDomains.clear();
}

void DialogDumper::discoverEngineDialogs() {
	if (_engineDialogsDiscovered)
		return;
	_engineDialogsDiscovered = true;

	PluginMan.loadAllPluginsOfType(PLUGIN_TYPE_ENGINE_DETECTION);

	bool guiEditGameRegistered = false;
	const PluginList &plugins = PluginMan.getPlugins(PLUGIN_TYPE_ENGINE_DETECTION);
	for (const Plugin *plugin : plugins) {
		// Only the engine ID is captured, the plugin may be unloaded later.
		Common::String engineId = plugin->get<MetaEngineDetection>().getName();

		// The game options of the engine, which include the options
		// provided by buildEngineOptionsWidget().
		DialogDumperRegistry::registerEngineDialog(engineId, "EditGameDialog", createEditGameDialog);

		// Without the engines, the dialog is still dumped for a single engine.
		if (!guiEditGameRegistered) {
			guiEditGameRegistered = true;
			DialogDumperRegistry::registerDialog("EditGameDialog", createEditGameDialog, engineId);
#ifdef USE_HTTP
			DialogDumperRegistry::registerDialog("IntegrityDialog", createIntegrityDialog, engineId);
#endif
		}

		// Known engine-specific dialogs
#if defined(ENABLE_SCUMM) && !defined(DYNAMIC_MODULES)
		if (engineId == "scumm") {
			DialogDumperRegistry::registerEngineDialog("scumm", "LoomTownsDifficultyDialog",
				[](DialogDumper &, const Common::String &) -> Dialog * { return new Scumm::LoomTownsDifficultyDialog(); });
		}
#endif
		// Add more per-engine dialogs here as needed
	}
}

bool DialogDumper::matchesFilters(const RegisteredDialog &reg) const {
	if (!reg.engineId.empty()) {
		if (!_config.includeEngines)
			return false;
		if (!_config.engineFilter.empty()) {
			bool match = false;
			for (const Common::String &f : _config.engineFilter) {
				if (lowercased(f) == lowercased(reg.engineId)) {
					match = true;
					break;
				}
			}
			if (!match)
				return false;
		}
	}

	if (!_config.dialogFilter.empty()) {
		Common::String name = lowercased(reg.name);
		for (const Common::String &f : _config.dialogFilter) {
			if (name.contains(lowercased(f)))
				return true;
		}
		return false;
	}
	return true;
}

void DialogDumper::dumpAll() {
	setSnapshotDirectory(_config.outputDir);

	// Default languages if none specified
	Common::Array<Common::String> languages = _config.languages;
	if (languages.empty()) {
#ifdef USE_TRANSLATION
		for (const Common::TLanguage &l : TransMan.getSupportedLanguageNames()) {
			Common::String id = TransMan.getLangById(l.id);
			if (!id.empty())
				languages.push_back(id);
		}
#endif
		// The built-in language is not part of the list of translations
		languages.insert_at(0, "en");
	}

	// Default resolutions if none specified
	Common::Array<int> widths = _config.resolutionsW;
	Common::Array<int> heights = _config.resolutionsH;
	if (widths.empty() || heights.size() != widths.size()) {
		const int defaults[] = { 320, 200, 320, 240, 640, 400, 640, 480, 800, 600 };
		widths.clear();
		heights.clear();
		for (uint i = 0; i < ARRAYSIZE(defaults); i += 2) {
			widths.push_back(defaults[i]);
			heights.push_back(defaults[i + 1]);
		}
	}

	// Ensure output directory exists
	Common::FSNode dumpDir(Common::Path(_config.outputDir, '/'));
	if (!dumpDir.isDirectory())
		dumpDir.createDirectory();

	// The file browsers open the last directory the user browsed, which shows local
	// paths and files in the snapshots. Point them to a fixed sample directory.
	// BrowserDialog stores the directory it lists, so restore the user's value at the end.
	const char *appDomain = Common::ConfigManager::kApplicationDomain;
	const bool hadLastPath = ConfMan.hasKey("browser_lastpath", appDomain);
	const Common::Path userLastPath = hadLastPath ? ConfMan.getPath("browser_lastpath", appDomain) : Common::Path();
	Common::Path browserDir(_config.outputDir + "/.browser-sample", '/');
	static const char *const sampleFolders[] = { "", "Game 1", "Game 2", "Game 3" };
	for (const char *folder : sampleFolders) {
		Common::FSNode node(browserDir.join(folder));
		if (!node.isDirectory())
			node.createDirectory();
	}
	ConfMan.setPath("browser_lastpath", browserDir, Common::ConfigManager::kTransientDomain);

	discoverEngineDialogs();

	const Common::Array<RegisteredDialog> &allDialogs = DialogDumperRegistry::getAllDialogs();

	for (uint resIdx = 0; resIdx < widths.size(); ++resIdx) {
		int w = widths[resIdx];
		int h = heights[resIdx];

		// Change resolution
		ConfMan.setInt("last_window_width", w, Common::ConfigManager::kApplicationDomain);
		ConfMan.setInt("last_window_height", h, Common::ConfigManager::kApplicationDomain);
		g_system->beginGFXTransaction();
		g_system->initSize(w, h);
		g_system->endGFXTransaction();
		g_gui.checkScreenChange();

		for (const Common::String &lang : languages) {
#ifdef USE_TRANSLATION
			TransMan.setLanguage(lang);
			// The fonts of the theme depend on the language
			ThemeEngine *theme = g_gui.theme();
			if (!g_gui.loadNewTheme(theme->getThemeId(), (ThemeEngine::GraphicsMode)theme->getGraphicsMode(), true))
				warning("Could not reload the theme for language '%s'", lang.c_str());
			else if (!lang.equalsIgnoreCase("en") && TransMan.currentIsBuiltinLanguage())
				warning("The theme does not support language '%s', the snapshots will be in English", lang.c_str());
#endif
			layoutParentDialogs(allDialogs);

			for (const RegisteredDialog &reg : allDialogs) {
				if (matchesFilters(reg))
					dumpDialog(reg, lang, w, h);
			}
		}
	}

	removeTempDomains();
	ConfMan.removeKey("browser_lastpath", Common::ConfigManager::kTransientDomain);
	if (hadLastPath)
		ConfMan.setPath("browser_lastpath", userLastPath, appDomain);
	else
		ConfMan.removeKey("browser_lastpath", appDomain);

	if (!_config.listOnly)
		generateIndexHtml();
}

void DialogDumper::layoutParentDialogs(const Common::Array<RegisteredDialog> &allDialogs) {
	// Some dialogs overlay a part of another dialog and take their size from its
	// layout, e.g. the download dialogs use Dialog.GlobalOptions and the browser uses
	// Dialog.Launcher.GameList. In normal use the parent is already open; here the
	// theme was just reloaded, so lay out the parents before any dialog is dumped.
	static const char *const parents[] = { "LauncherSimple", "GlobalOptionsDialog" };

	for (const char *parent : parents) {
		for (const RegisteredDialog &reg : allDialogs) {
			if (!reg.engineId.empty() || reg.name != parent || !reg.guiFactory)
				continue;
			Dialog *dialog = reg.guiFactory();
			if (dialog) {
				dialog->open();
				dialog->reflowLayout();
				dialog->close();
				delete dialog;
			}
		}
	}
}

void DialogDumper::dumpDialog(const RegisteredDialog &reg, const Common::String &lang, int w, int h) {
	Dialog *dialog = nullptr;

	if (reg.guiFactory)
		dialog = reg.guiFactory();
	else if (reg.dumperFactory)
		dialog = reg.dumperFactory(*this, reg.factoryEngineId);

	if (!dialog)
		return;

	dialog->open();
	dialog->reflowLayout();

	// Build base filename: domain__dialogName
	Common::String domain = reg.engineId.empty() ? "gui" : reg.engineId;
	Common::String baseName = Common::String::format("%s__%s", domain.c_str(), reg.name.c_str());

	dumpDialogStates(*dialog, reg, baseName, lang, w, h);

	dialog->close();
	delete dialog;
}

namespace {

/** Prints the snapshot names of the states of a dialog, for --dump-list-only. */
class ListStatesCallback : public DialogDumpStateCallback {
public:
	ListStatesCallback(const Common::String &baseName, const Common::String &resLang)
		: _baseName(baseName), _resLang(resLang) {}

	void dumpState(Dialog &dialog, const Common::String &stateKey) override {
		if (!stateKey.empty())
			debugN("%s__state-%s%s\n", _baseName.c_str(), stateKey.c_str(), _resLang.c_str());
	}

private:
	const Common::String &_baseName;
	const Common::String &_resLang;
};

/** Captures every state of a dialog but the default one, already captured. */
class SaveStatesCallback : public DialogDumpStateCallback {
public:
	SaveStatesCallback(Graphics::Surface &surf, const Common::String &baseName, const Common::String &resLang)
		: _surf(surf), _baseName(baseName), _resLang(resLang) {}

	void dumpState(Dialog &dialog, const Common::String &stateKey) override {
		if (stateKey.empty())
			return;
		g_gui.redrawFull();
		g_system->grabOverlay(_surf);
		saveWidgetSnapshot(_surf, _baseName + "__state-" + stateKey + _resLang);
	}

private:
	Graphics::Surface &_surf;
	const Common::String &_baseName;
	const Common::String &_resLang;
};

} // End of anonymous namespace

void DialogDumper::dumpDialogStates(Dialog &dialog, const RegisteredDialog &reg,
		const Common::String &baseName, const Common::String &lang, int w, int h) {
	Common::String resLang = Common::String::format("__%dx%d__%s.png", w, h, lang.c_str());

	// If list-only mode, just print what would be dumped
	if (_config.listOnly) {
		debugN("%s%s\n", baseName.c_str(), resLang.c_str());
		ListStatesCallback listStates(baseName, resLang);
		dialog.iterateDumpStates(listStates);
		return;
	}

	Graphics::Surface surf;
	surf.create(g_system->getOverlayWidth(), g_system->getOverlayHeight(), g_system->getOverlayFormat());

	// The dialog as it opens
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveWidgetSnapshot(surf, baseName + resLang);

	// Every tab and the scrolled contents of the containers
	dialog.dumpWidgetTree(baseName, lang, w, h);

	// Then the variations of the dialog
	SaveStatesCallback saveStates(surf, baseName, resLang);
	dialog.iterateDumpStates(saveStates);

	surf.free();
}

// Split "domain__Dialog[__variant]__WxH__lang.png" and write a browsable page
void DialogDumper::generateIndexHtml() {
	Common::FSNode dumpDir(Common::Path(_config.outputDir, '/'));
	Common::FSList files;
	dumpDir.getChildren(files, Common::FSNode::kListFilesOnly);

	Common::String json = "[";
	bool first = true;
	for (const Common::FSNode &file : files) {
		Common::String name = file.getName();
		if (!name.hasSuffix(".png"))
			continue;

		Common::StringArray parts;
		Common::String part;
		const uint end = name.size() - 4;
		for (uint i = 0; i < end; ++i) {
			if (name[i] == '_' && i + 1 < end && name[i + 1] == '_') {
				parts.push_back(part);
				part.clear();
				++i;
			} else {
				part += name[i];
			}
		}
		parts.push_back(part);
		if (parts.size() < 4)
			continue;

		Common::String variant;
		for (uint i = 2; i + 2 < parts.size(); ++i)
			variant += (variant.empty() ? "" : " / ") + parts[i];

		json += Common::String::format("%s{\"f\":\"%s\",\"d\":\"%s\",\"n\":\"%s\",\"v\":\"%s\",\"r\":\"%s\",\"l\":\"%s\"}",
			first ? "" : ",", name.c_str(), parts[0].c_str(), parts[1].c_str(), variant.c_str(),
			parts[parts.size() - 2].c_str(), parts[parts.size() - 1].c_str());
		first = false;
	}
	json += "]";

	Common::DumpFile outFile;
	if (!outFile.open(Common::Path(_config.outputDir + "/index.html", '/')))
		return;

	Common::String html =
		"<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\"><title>ScummVM dialog snapshots</title>\n"
		"<style>body{font-family:sans-serif;margin:1em}select,input{margin:0 .5em .5em 0}"
		".grid{display:flex;flex-wrap:wrap;gap:8px}.card{border:1px solid #ccc;padding:4px;max-width:340px;font-size:12px}"
		".card img{max-width:100%;image-rendering:pixelated}.card.sel{outline:3px solid #36f}</style></head>\n"
		"<body><h1>ScummVM dialog snapshots</h1>\n"
		"<div><select id=n></select><select id=d></select><select id=r></select><select id=l></select>"
		"<input id=q placeholder=\"filter variant\"><button id=cmp>Show selected side by side</button> "
		"<span id=cnt></span> <button id=more>Show more</button><br><small>Shift+click a snapshot to select it for comparison.</small></div>\n"
		"<div id=g class=grid></div>\n<script>\nconst D=";
	html += json;
	html +=
		";\nconst sel=new Set();let limit=300;\n"
		"function opts(id,k,all){const e=document.getElementById(id);"
		"e.innerHTML='<option value=\"\">'+all+'</option>'+[...new Set(D.map(x=>x[k]))].sort()"
		".map(v=>'<option>'+v+'</option>').join('');e.onchange=()=>{limit=300;render()}}\n"
		"opts('n','n','All dialogs');opts('d','d','All domains');opts('r','r','All resolutions');opts('l','l','All languages');\n"
		"document.getElementById('q').oninput=render;\n"
		"function render(){const n=document.getElementById('n').value,d=document.getElementById('d').value,"
		"r=document.getElementById('r').value,l=document.getElementById('l').value,"
		"q=document.getElementById('q').value.toLowerCase();"
		"const L=D.filter(x=>(!n||x.n==n)&&(!d||x.d==d)&&(!r||x.r==r)&&(!l||x.l==l)&&x.v.toLowerCase().includes(q));"
		"document.getElementById('cnt').textContent=Math.min(limit,L.length)+' of '+L.length+' images';"
		"document.getElementById('g').innerHTML=L.slice(0,limit).map(x=>'<div class=\"card'+(sel.has(x.f)?' sel':'')+'\" data-f=\"'+x.f+'\">"
		"<a href=\"'+x.f+'\"><img loading=lazy src=\"'+x.f+'\"></a><div><b>'+x.d+' '+x.n+'</b><br>'+x.v+'<br>'+x.r+' '+x.l+'</div></div>').join('');"
		"document.querySelectorAll('.card').forEach(c=>c.onclick=e=>{if(!e.shiftKey)return;e.preventDefault();"
		"const f=c.dataset.f;sel.has(f)?sel.delete(f):sel.add(f);c.classList.toggle('sel')})}\n"
		"document.getElementById('more').onclick=()=>{limit+=300;render()};\n"
		"document.getElementById('cmp').onclick=()=>{document.getElementById('g').innerHTML="
		"[...sel].map(f=>'<div class=card><img src=\"'+f+'\"><div>'+f+'</div></div>').join('')};\n"
		"render();\n</script></body></html>\n";

	outFile.write(html.c_str(), html.size());
	outFile.finalize();
	outFile.close();
}

} // End of namespace GUI
