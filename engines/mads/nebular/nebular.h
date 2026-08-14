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

#ifndef MADS_NEBULAR_H
#define MADS_NEBULAR_H

#include "mads/mads.h"

namespace MADS {
namespace RexNebular {

class MacNebular;

struct MADSSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;

	void readSavegameHeader(Common::SeekableReadStream *src);
	void writeSavegameHeader(Common::WriteStream *dest);
};

class RexNebularEngine : public MADSEngine {
private:
	friend class MacNebular;
	MacNebular *_macNebular = nullptr;

	void showRecipe();

protected:
	void applyGameSettings() override;
	Common::Point screenToGame(const Common::Point &point) const override;
	Common::Point gameToScreen(const Common::Point &point) const override;
	void presentScreen(int shakeOffset) override;
	bool handleMacEvent(Common::Event &event) override;
	void serviceMacintoshUI() override;
	void serviceMacintoshSound() override;

public:
	RexNebularEngine(OSystem *syst, const MADSGameDescription *gameDesc);
	~RexNebularEngine() override;

	Common::Error run() override;
	void syncRoom(Common::Serializer &s) override;
	void selectMacintoshDifficulty();
	int selectMacintoshResumeSlot();
	bool usesOriginalMacintoshMenus() const;
	int getMacintoshDisplaySize() const;
	bool getMacintoshHideMenuBar() const;
	bool getMacintoshPreferencesAtStartup() const;
	Common::String getMacintoshApplicationVersion() const;
	bool getMacintoshStoryLocked() const;
	bool verifyMacintoshStoryPassword(const Common::String &password) const;
	void setMacintoshDisplaySize(int displaySize, bool persist);
	void setMacintoshHideMenuBar(bool hide, bool persist);
	void setMacintoshPreferencesAtStartup(bool show, bool persist);
	void setMacintoshStoryLocked(bool locked,
		const Common::String &password);
	void setMacintoshOuterMenuActive(bool active);
	void notifyMacintoshOuterMenuFrameReady();
	void setMacintoshFullFrameActive(bool active);
	bool isMacintoshFullFrameActive() const;
	int runMacintoshCopyProtectionDialog(const Common::String &title,
		const Common::String &subtitle, const Common::String &prompt,
		char *target, int maxLength);

	int main_copy_verify() override;
	void global_init_code() override;
	void section_music(int section_num) override;
	void global_section_constructor() override;
	void global_daemon_code() override;
	void global_verb_filter() override;
	void global_pre_parser_code() override;
	void global_parser_code() override;
	void global_error_code() override;
	void global_room_init() override {}
	void global_sound_driver() override;
	bool hasInterfaceAnimations() const override;
	bool drawPopup() override;
	int editMacintoshPopup(char *target, int maxLength) override;
	void onPopupDestroyed() override;
	int getMacintoshTextWidth(FontPtr font, const char *text,
		int spacing) const override;
	bool drawMacintoshText(FontPtr font, Buffer *target, const char *text,
		int x, int y, int color, int spacing) const override;
	bool getInterfaceSentenceColor(byte &foreground) const override;
	bool hasMacintoshInterface() const override;
	bool setMacintoshPalette(const RGBcolor *palette, int firstColor,
		int numColors) override;
	bool getMacintoshPalette(RGBcolor *palette, int firstColor,
		int numColors) const override;
};

} // namespace RexNebular
} // namespace MADS

#endif
