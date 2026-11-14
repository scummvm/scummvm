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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/diagnostics/milestone_overlay.h"

#include "common/debug.h"

#include "ripper/detection.h"

namespace Ripper {

MilestoneOverlay::MilestoneOverlay(Milestones &milestones,
		ScreenPresenter &presenter) :
		_milestones(milestones), _presenter(presenter), _enabled(false) {
	_milestones.setChangeObserver(this);
}

MilestoneOverlay::~MilestoneOverlay() {
	_milestones.setChangeObserver(nullptr);
}

void MilestoneOverlay::setEnabled(bool enabled) {
	if (_enabled == enabled)
		return;

	_enabled = enabled;
	debugC(1, kDebugMilestones,
		"Ripper: milestone change overlay enabled=%d command=OVERLAY_MILESTONES",
		enabled);
}

void MilestoneOverlay::onMilestoneChanged(uint flag, bool value) {
	if (!_enabled)
		return;

	const Common::String label = _milestones.label(flag);
	const Common::String message = Common::String::format(
		"Milestone %u: %s [%s]", flag,
		label.empty() ? "<unnamed>" : label.c_str(), value ? "set" : "unset");
	_presenter.showMessage(message);
	debugC(2, kDebugMilestones,
		"Ripper: queued milestone overlay flag=%u label='%s' value=%d pending=%u",
		flag, label.c_str(), value, _presenter.pendingMessageCount());
}

} // End of namespace Ripper
