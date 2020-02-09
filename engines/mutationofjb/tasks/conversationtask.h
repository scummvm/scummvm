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
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mutationofjb/commands/talkcommand.h"
#include "mutationofjb/conversationlinelist.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/tasks/task.h"
#include "mutationofjb/widgets/conversationwidget.h"

namespace MutationOfJB {

class SayTask;
class ScriptExecutionContext;

class ConversationTask : public Task, public ConversationWidgetCallback {
public:
	ConversationTask(uint8 sceneId, const ConversationInfo &convInfo, TalkCommand::Mode mode) : _sceneId(sceneId), _convInfo(convInfo), _mode(mode), _currentGroupIndex(0), _currentItem(nullptr), _substate(IDLE), _haveChoices(false), _innerExecCtx(nullptr) {}
	~ConversationTask() override {}

	void start() override;
	void update() override;

	void onChoiceClicked(ConversationWidget *, int response, uint32 data) override;
private:
	void showChoicesOrPick();
	const ConversationInfo::ItemGroup &getCurrentGroup() const;
	void finish();
	void startExtra();
	void gotoNextGroup();
	void createSayTasks(const ConversationLineList::Line *line);
	uint8 getSpeechColor(const ConversationLineList::Speech &speech);

	uint8 _sceneId;
	const ConversationInfo &_convInfo;
	TalkCommand::Mode _mode;
	uint _currentGroupIndex;
	const ConversationInfo::Item *_currentItem;
	TaskPtr _sayTask;

	enum Substate {
		IDLE,
		SAYING_QUESTION,
		SAYING_RESPONSE,
		SAYING_NO_QUESTIONS,
		RUNNING_EXTRA
	};

	Substate _substate;
	bool _haveChoices;
	ScriptExecutionContext *_innerExecCtx;
};

}
