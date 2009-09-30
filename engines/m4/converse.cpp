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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/array.h"
#include "common/hashmap.h"

#include "m4/converse.h"
#include "m4/resource.h"
#include "m4/globals.h"
#include "m4/m4_views.h"
#include "m4/compression.h"

namespace M4 {

#define CONV_ENTRIES_X_OFFSET 20
#define CONV_ENTRIES_Y_OFFSET 4
#define CONV_ENTRIES_HEIGHT 20
#define CONV_MAX_SHOWN_ENTRIES 5

#define CONVERSATION_ENTRY_HIGHLIGHTED 22
#define	CONVERSATION_ENTRY_NORMAL 3

// Conversation chunks
// Header
#define  HEAD_CONV MKID_BE('CONV')		// conversation
#define CHUNK_DECL MKID_BE('DECL')		// declaration
#define CHUNK_NODE MKID_BE('NODE')		// node
#define CHUNK_LNOD MKID_BE('LNOD')		// linear node
#define CHUNK_ETRY MKID_BE('ETRY')		// entry
#define CHUNK_TEXT MKID_BE('TEXT')		// text
#define CHUNK_MESG MKID_BE('MESG')		// message
// Conversation chunks - entry related (unconditional)
#define CHUNK_RPLY MKID_BE('RPLY')		// reply
#define CHUNK_HIDE MKID_BE('HIDE')		// hide entry
#define CHUNK_UHID MKID_BE('UHID')		// unhide entry
#define CHUNK_DSTR MKID_BE('DSTR')		// destroy entry
// Conversation chunks - entry related (conditional)
#define CHUNK_CRPL MKID_BE('CRPL')		// reply
#define CHUNK_CHDE MKID_BE('CHDE')		// hide entry
#define CHUNK_CUHD MKID_BE('CUHD')		// unhide entry
#define CHUNK_CDST MKID_BE('DDTS')		// destroy entry
// Conversation chunks - branching and logic (unconditional)
#define CHUNK_ASGN MKID_BE('ASGN')		// assign
#define CHUNK_GOTO MKID_BE('GOTO')		// goto chunk
#define CHUNK_EXIT MKID_BE('EXIT')		// exit/return from goto
// Conversation chunks - branching and logic (conditional)
#define CHUNK_CASN MKID_BE('CASN')		// assign
#define CHUNK_CCGO MKID_BE('CCGO')		// goto chunk
#define CHUNK_CEGO MKID_BE('CEGO')		// exit/return from goto
// Others
#define CHUNK_FALL MKID_BE('FALL')		// fallthrough
#define CHUNK_WRPL MKID_BE('WRPL')		// weighted reply chunk
#define CHUNK_WPRL MKID_BE('WPRL')		// weighted preply chunk


ConversationView::ConversationView(M4Engine *vm): View(vm, Common::Rect(0,
		vm->_screen->height() - INTERFACE_HEIGHT, vm->_screen->width(), vm->_screen->height())) {

	_screenType = VIEWID_CONVERSATION;
	_screenFlags.layer = LAYER_INTERFACE;
	_screenFlags.visible = false;
	_screenFlags.get = SCREVENT_MOUSE;
	_conversationState = kNoConversation;
	_currentHandle = NULL;
}

ConversationView::~ConversationView() {
	_activeItems.clear();
}

void ConversationView::setNode(int32 nodeIndex) {
	_highlightedIndex = -1;
	_xEnd = CONV_ENTRIES_X_OFFSET;
	_vm->_font->setFont(FONT_CONVERSATION);

	// TODO: Conversation styles and colors
	_vm->_font->setColors(2, 1, 3);

	_currentNodeIndex = nodeIndex;

	_activeItems.clear();

	if (nodeIndex != -1) {
		ConvEntry *node = _vm->_converse->getNode(nodeIndex);

		for (uint i = 0; i < node->entries.size(); ++i) {
			if (!node->entries[i]->visible)
				continue;

			if ((int)_activeItems.size() > CONV_MAX_SHOWN_ENTRIES) {
				warning("TODO: scrolling. Max shown entries are %i, skipping entry %i",
						CONV_MAX_SHOWN_ENTRIES, i);
			}

			// Add node to active items list
			_activeItems.push_back(node->entries[i]);

			if (node->entries[i]->autoSelect || strlen(node->entries[i]->text) == 0) {
				//printf("Auto selecting entry %i of node %i\n", i, nodeIndex);
				selectEntry(i);
				return;
			}

			// Figure out the longest string to determine where option highlighting ends
			int tempX = _vm->_font->getWidth(node->entries[i]->text, 0) +
				CONV_ENTRIES_X_OFFSET + 10;
			_xEnd = MAX(_xEnd, tempX);
		}

		// Make sure that there aren't too many entries
		//assert((int)_activeItems.size() < (height() - CONV_ENTRIES_Y_OFFSET) / CONV_ENTRIES_HEIGHT);

		// Fallthrough
		if (node->fallthroughMinEntries >= 0 && node->fallthroughOffset >= 0) {
			//printf("Current node falls through node at offset %i when entries are less or equal than %i\n",
			//		node->fallthroughOffset, node->fallthroughMinEntries);
			if (_activeItems.size() <= (uint32)node->fallthroughMinEntries) {
				const EntryInfo *entryInfo = _vm->_converse->getEntryInfo(node->fallthroughOffset);
				//printf("Entries are less than or equal to %i, falling through to node at offset %i, index %i\n",
				//		node->fallthroughMinEntries, node->fallthroughOffset, entryInfo->nodeIndex);
				setNode(entryInfo->nodeIndex);
				return;
			}
		}

		_entriesShown = true;
		_conversationState = kConversationOptionsShown;
	}
}

void ConversationView::onRefresh(RectList *rects, M4Surface *destSurface) {
	//if (!this->isVisible())
	//	return;
	clear();

	if (_entriesShown) {
		// Write out the conversation options
		_vm->_font->setFont(FONT_CONVERSATION);
		for (int i = 0; i < (int)_activeItems.size(); ++i) {
			// TODO: scrolling
			if (i > CONV_MAX_SHOWN_ENTRIES - 1)
				break;

			_vm->_font->setColor((_highlightedIndex == i) ? CONVERSATION_ENTRY_HIGHLIGHTED :
				CONVERSATION_ENTRY_NORMAL);

			_vm->_font->writeString(this, _activeItems[i]->text, CONV_ENTRIES_X_OFFSET,
				CONV_ENTRIES_Y_OFFSET + CONV_ENTRIES_HEIGHT * i, 0, 0);
		}
	}
	View::onRefresh(rects, destSurface);
}

bool ConversationView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	//if (!this->isVisible())
	//	return false;
	if (!_entriesShown)
		return false;
	if (eventType == KEVENT_KEY)
		return false;

	int localY = y - _coords.top;
	int selectedIndex = _highlightedIndex;

	switch (eventType) {
	case MEVENT_MOVE:
		if ((x < CONV_ENTRIES_X_OFFSET) || (x >= _xEnd) || (localY < CONV_ENTRIES_Y_OFFSET))
			_highlightedIndex = -1;
		else {
			int index = (localY - CONV_ENTRIES_Y_OFFSET) / CONV_ENTRIES_HEIGHT;
			_highlightedIndex = (index >= (int)_activeItems.size()) ? -1 : index;
		}
		break;

	case MEVENT_LEFT_RELEASE:
		if (_highlightedIndex != -1) {
			selectEntry(selectedIndex);
		}
		break;

	default:
		break;
	}

	return true;
}

void ConversationView::selectEntry(int entryIndex) {
	char buffer[20];
	sprintf(buffer, "%s.raw", _activeItems[entryIndex]->voiceFile);

	_entriesShown = false;
	_conversationState = kEntryIsActive;
	_vm->_player->setCommandsAllowed(false);
	// Necessary, as entries can be selected programmatically
	_highlightedIndex = entryIndex;

	// Play the selected entry's voice
	if (strlen(_activeItems[entryIndex]->voiceFile) > 0) {
		_currentHandle = _vm->_sound->getHandle();
		_vm->_sound->playVoice(buffer, 255);
	} else {
		_currentHandle = NULL;
	}

	// Hide selected entry, unless it has a persistent flag set
	if (!(_activeItems[entryIndex]->flags & kEntryPersists)) {
		//printf("Hiding selected entry\n");
		_vm->_converse->getNode(_currentNodeIndex)->entries[entryIndex]->visible = false;
	} else {
		//printf("Selected entry is persistent, not hiding it\n");
	}
}

void ConversationView::updateState() {
	switch (_conversationState) {
		case kConversationOptionsShown:
			return;
		case kEntryIsActive:
		case kReplyIsActive:
			// FIXME: for now, we determine whether a conversation entry is
			// finished by waiting for its associated speech file to finish playing
			if (_currentHandle != NULL && _vm->_sound->isHandleActive(_currentHandle)) {
				return;
			} else {
				playNextReply();
			}	// end else
			break;
		case kNoConversation:
			return;
		default:
			error("Unknown converstation state");
			break;
	}
}

void ConversationView::playNextReply() {
	char buffer[20];

	assert(_highlightedIndex >= 0);

	// Start playing the first reply
	for (uint32 i = 0; i < _activeItems[_highlightedIndex]->entries.size(); i++) {
		ConvEntry *currentEntry = _activeItems[_highlightedIndex]->entries[i];

		if (currentEntry->isConditional) {
			if (!_vm->_converse->evaluateCondition(
				_vm->_converse->getValue(currentEntry->condition.offset),
				currentEntry->condition.op, currentEntry->condition.val))
				continue;	// don't play this reply
		}

		if (currentEntry->entryType != kWeightedReply) {
			sprintf(buffer, "%s.raw", currentEntry->voiceFile);
			if (strlen(currentEntry->voiceFile) > 0) {
				_currentHandle = _vm->_sound->getHandle();
				_vm->_sound->playVoice(buffer, 255);
				// Remove reply from the list of replies
				_activeItems[_highlightedIndex]->entries.remove_at(i);
				_conversationState = kReplyIsActive;
				return;
			} else {
				_currentHandle = NULL;
			}
		} else {
			int selectedWeight = _vm->_random->getRandomNumber(currentEntry->totalWeight - 1) + 1;
			//printf("Selected weight: %i\n", selectedWeight);
			int previousWeight = 1;
			int currentWeight = 0;

			for (uint32 j = 0; j < currentEntry->entries.size(); j++) {
				currentWeight += currentEntry->entries[j]->weight;
				if (selectedWeight >= previousWeight && selectedWeight <= currentWeight) {
					sprintf(buffer, "%s.raw", currentEntry->entries[j]->voiceFile);
					if (strlen(currentEntry->entries[j]->voiceFile) > 0) {
						_currentHandle = _vm->_sound->getHandle();
						_vm->_sound->playVoice(buffer, 255);
						// Remove reply from the list of replies
						_activeItems[_highlightedIndex]->entries.remove_at(i);
						_conversationState = kReplyIsActive;
						return;
					} else {
						_currentHandle = NULL;
					}
					break;
				}
				previousWeight += currentWeight;
			}	// end for j
		}	// end if
	}	// end for i

	// If we reached here, there are no more replies, so perform the active entry's actions

	//printf("Current selection does %i actions\n", _activeItems[entryIndex]->actions.size());
	for (uint32 i = 0; i < _activeItems[_highlightedIndex]->actions.size(); i++) {
		if (!_vm->_converse->performAction(_activeItems[_highlightedIndex]->actions[i]))
			break;
	}	// end for

	// Refresh the conversation node, in case it hasn't changed
	setNode(_currentNodeIndex);

	_entriesShown = true;
	_vm->_player->setCommandsAllowed(true);

	// Check if the conversation has been ended
	if (_currentNodeIndex == -1) {
		_conversationState = kNoConversation;
	}
}

//--------------------------------------------------------------------------

void Converse::startConversation(const char *convName, bool showConverseBox, ConverseStyle style) {
	if (_vm->isM4())
		loadConversation(convName);
	else
		loadConversationMads(convName);

	if (!_vm->isM4()) showConverseBox = false;		// TODO: remove

	_playerCommandsAllowed = _vm->_player->commandsAllowed;
	if (_vm->isM4())	// TODO: remove (interface not implemented yet in MADS games)
		_interfaceWasVisible = _vm->_interfaceView->isVisible();
	_vm->_player->setCommandsAllowed(false);
	_style = style;

	if (showConverseBox) {
		_vm->_conversationView->show();
		_vm->_mouse->lockCursor(CURSOR_ARROW);

		if (_interfaceWasVisible)
			_vm->_interfaceView->hide();

		_vm->_conversationView->setNode(0);
		_vm->_conversationView->show();
	}
}

void Converse::endConversation() {
	_vm->_conversationView->setNode(-1);
	_vm->_conversationView->hide();
	// TODO: do a more proper cleanup here
	_convNodes.clear();
	_variables.clear();
	_offsetMap.clear();
	_vm->_player->setCommandsAllowed(_playerCommandsAllowed);
	if (_interfaceWasVisible)
		_vm->_interfaceView->show();
}

void Converse::loadConversation(const char *convName) {
	char name[40];
	char buffer[256];
	sprintf(name, "%s.chk", convName);
	Common::SeekableReadStream *convS = _vm->res()->get(name);
	uint32 header = convS->readUint32LE();
	uint32 size;
	uint32 chunk;
	uint32 data = 0;
	uint32 i = 0;
	ConvEntry* curEntry = NULL;
	ConvEntry* replyEntry = NULL;
	int32 currentWeightedEntry = -1;
	EntryAction* curAction = NULL;
	uint32 curNode = 0;
	uint32 chunkPos = 0;
	uint32 val;
	int32 autoSelectIndex = -1;
	int returnAddress = -1;

	bool debugFlag = false;		// set to true for debug messages

	// Conversation *.chk files contain a 'CONV' header in LE format
	if (header != HEAD_CONV) {
		warning("Unexpected conversation file external header");
		return;
	}
	size = convS->readUint32LE();	// is this used at all?
	if (debugFlag) printf("Conv chunk size (external header): %i\n", size);

	// Conversation name, which is the conversation file's name
	// without the extension
	convS->read(buffer, 8);
	if (debugFlag) printf("Conversation name: %s\n", buffer);

	while (true) {
		chunkPos = convS->pos();
		chunk = convS->readUint32LE();	// read chunk
		if (convS->eos()) break;

		if (debugFlag) printf("***** Pos: %i -> ", chunkPos);
		switch (chunk) {
			case CHUNK_DECL:	// Declare
				if (debugFlag) printf("DECL chunk\n");
				data = convS->readUint32LE();
				if (debugFlag) printf("Tag: %i\n", data);
				if (data > 0)
					warning("Tag > 0 in DECL chunk, value is: %i", data);		// TODO
				val = convS->readUint32LE();
				if (debugFlag) printf("Value: %i\n", val);
				data = convS->readUint32LE();
				if (debugFlag) printf("Flags: %i\n", data);
				if (data > 0)
					warning("Flags != 0 in DECL chunk, value is %i", data);		// TODO
				setValue(chunkPos, val);
				break;
			// ----------------------------------------------------------------------------
			case CHUNK_NODE:	// Node
			case CHUNK_LNOD:	// Linear node
				// Create new node
				curEntry = new ConvEntry();
				curEntry->offset = chunkPos;
				curEntry->entryType = (chunk == CHUNK_NODE) ? kNode : kLinearNode;
				curEntry->fallthroughMinEntries = -1;
				curEntry->fallthroughOffset = -1;
				if (chunk == CHUNK_NODE) {
					if (debugFlag) printf("NODE chunk\n");
				} else {
					if (debugFlag) printf("LNOD chunk\n");
				}
				curNode = convS->readUint32LE();
				if (debugFlag) printf("Node number: %i\n", curNode);
				data = convS->readUint32LE();
				if (debugFlag) printf("Tag: %i\n", data);
				if (chunk == CHUNK_LNOD) {
					autoSelectIndex = convS->readUint32LE();
					if (debugFlag) printf("Autoselect entry number: %i\n", autoSelectIndex);
				}
				size = convS->readUint32LE();
				if (debugFlag) printf("Number of entries: %i\n", size);
				for (i = 0; i < size; i++) {
					data = convS->readUint32LE();
					if (debugFlag) printf("Entry %i: %i\n", i + 1, data);
				}
				_convNodes.push_back(curEntry);
				setEntryInfo(curEntry->offset, curEntry->entryType, curNode, -1);
				break;
			case CHUNK_ETRY:	// Entry
				// Create new entry
				curEntry = new ConvEntry();
				curEntry->offset = chunkPos;
				curEntry->entryType = kEntry;
				strcpy(curEntry->voiceFile, "");
				strcpy(curEntry->text, "");
				if (debugFlag) printf("ETRY chunk\n");
				data = convS->readUint32LE();
				if (debugFlag) printf("Unknown (attributes perhaps?): %i\n", data);
				data = convS->readUint32LE();
				if (debugFlag) printf("Entry flags: ");
				if (debugFlag) if (data & kEntryInitial) printf ("Initial ");
				if (debugFlag) if (data & kEntryPersists) printf ("Persists ");
				if (debugFlag) printf("\n");
				curEntry->flags = data;
				curEntry->visible = (curEntry->flags & kEntryInitial) ? true : false;
				if (autoSelectIndex >= 0) {
					if (_convNodes[curNode]->entries.size() == (uint32)autoSelectIndex) {
						curEntry->autoSelect = true;
						autoSelectIndex = -1;
					} else {
						curEntry->autoSelect = false;
					}
				} else {
					curEntry->autoSelect = false;
				}
				_convNodes[curNode]->entries.push_back(curEntry);
				setEntryInfo(curEntry->offset, curEntry->entryType,
							 curNode, _convNodes[curNode]->entries.size() - 1);
				replyEntry = NULL;
				break;
			case CHUNK_WPRL:	// Weighted preply
				// WPRL chunks are random entries that the character would say, not an NPC
				// They don't seem to be used in Orion Burger
				warning("WPRL chunk - treating as WRPL chunk");
			case CHUNK_WRPL:	// Weighted reply
			case CHUNK_CRPL:	// Conditional reply
			case CHUNK_RPLY:	// Reply
				{
				ConvEntry* weightedEntry = NULL;
				// Create new reply
				replyEntry = new ConvEntry();
				replyEntry->offset = chunkPos;
				strcpy(replyEntry->voiceFile, "");

				// Conditional part
				if (chunk == CHUNK_CRPL) {
					replyEntry->isConditional = true;
					replyEntry->condition.offset = convS->readUint32LE();
					replyEntry->condition.op = convS->readUint32LE();
					replyEntry->condition.val = convS->readUint32LE();
				} else {
					replyEntry->isConditional = false;
				}

				if (chunk == CHUNK_WPRL || chunk == CHUNK_WRPL) {
					replyEntry->entryType = kWeightedReply;
					replyEntry->totalWeight = 0;
					if (debugFlag) printf("WRPL chunk\n");
					size = convS->readUint32LE();
					if (debugFlag) printf("Weighted reply %i - %i entries:\n", i, size);
					for (i = 0; i < size; i++) {
						weightedEntry = new ConvEntry();
						weightedEntry->offset = chunkPos;
						strcpy(weightedEntry->voiceFile, "");
						weightedEntry->entryType = kWeightedReply;
						data = convS->readUint32LE();
						if (debugFlag) printf("- Weight: %i ", data);
						weightedEntry->weight = data;
						replyEntry->totalWeight += weightedEntry->weight;
						data = convS->readUint32LE();
						if (debugFlag) printf("offset: %i\n", data);
						replyEntry->entries.push_back(weightedEntry);
					}
					currentWeightedEntry = 0;
				} else {
					replyEntry->entryType = kReply;
					if (debugFlag) printf("RPLY chunk\n");
					data = convS->readUint32LE();
					if (debugFlag) printf("Reply data offset: %i\n", data);
				}

				curEntry->entries.push_back(replyEntry);
				setEntryInfo(replyEntry->offset, replyEntry->entryType,
							 curNode, _convNodes[curNode]->entries.size() - 1);
				// Seek to chunk data (i.e. TEXT/MESG tag, which is usually right
				// after this chunk but it can be further on in conditional reply chunks
				assert((int)data >= convS->pos());
				// If the entry's data is not right after the entry, remember the position
				// to return to after the data is read
				if (chunk == CHUNK_CRPL && (int)data != convS->pos())
					returnAddress = convS->pos();
				convS->seek(data, SEEK_SET);
				}
				break;
			// ----------------------------------------------------------------------------
			case CHUNK_TEXT:	// Text (contains text and voice)
			case CHUNK_MESG:	// Message (contains voice only)
				{
				ConvEntry* parentEntry = NULL;
				if (chunk == CHUNK_TEXT) {
					if (debugFlag) printf("TEXT chunk\n");
				} else {
					if (debugFlag) printf("MESG chunk\n");
				}

				if (replyEntry == NULL) {
					parentEntry = curEntry;
				} else if (replyEntry != NULL && replyEntry->entryType != kWeightedReply) {
					parentEntry = replyEntry;
				} else if (replyEntry != NULL && replyEntry->entryType == kWeightedReply) {
					parentEntry = replyEntry->entries[currentWeightedEntry];
					currentWeightedEntry++;
				}

				size = convS->readUint32LE();
				if (debugFlag) printf("Entry data size: %i\n", size);
				convS->read(buffer, 8);
				size -= 8;		// subtract maximum length of voice file name
				// If the file name is 8 characters, it will not be 0-terminated, so use strncpy
				strncpy(parentEntry->voiceFile, buffer, 8);
				parentEntry->voiceFile[8] = '\0';
				if (debugFlag) printf("Voice file: %s\n", parentEntry->voiceFile);

				if (chunk == CHUNK_TEXT) {
					convS->read(buffer, size);
					if (debugFlag) printf("Text: %s\n", buffer);
					sprintf(parentEntry->text, "%s", buffer);
				} else {
					while (size > 0) {
						data = convS->readUint32LE();
						if (debugFlag) printf("Unknown: %i\n", data);	// TODO
						size -= 4;
					}
				}
				// Now that the data chunk has been read, if we skipped a reply node,
				// jump back to it
				if (returnAddress != -1) {
					convS->seek(returnAddress, SEEK_SET);
					returnAddress = -1;
				}
				}
				break;
			// ----------------------------------------------------------------------------
			// Entry action chunks
			case CHUNK_CASN:	// Conditional assign
			case CHUNK_ASGN:	// Assign
				curAction = new EntryAction();
				if (debugFlag) printf("ASGN chunk\n");
				curAction->actionType = kAssignValue;

				// Conditional part
				if (chunk == CHUNK_CASN) {
					curAction->isConditional = true;
					curAction->condition.offset = convS->readUint32LE();
					curAction->condition.op = convS->readUint32LE();
					curAction->condition.val = convS->readUint32LE();
				} else {
					curAction->isConditional = false;
				}

				curAction->targetOffset = convS->readUint32LE();
				assert(convS->readUint32LE() == kOpAssign);
				curAction->value = convS->readUint32LE();
				break;
			case CHUNK_CCGO:	// Conditional go to entry
			case CHUNK_CHDE:	// Conditional hide entry
			case CHUNK_CUHD:	// Conditional unhide entry
			case CHUNK_CDST:	// Conditional destroy entry
			case CHUNK_CEGO:	// Conditional exit conversation / go to

			case CHUNK_GOTO:	// Go to entry
			case CHUNK_HIDE:	// Hide entry
			case CHUNK_UHID:	// Unhide entry
			case CHUNK_DSTR:	// Destroy entry
			case CHUNK_EXIT:	// Exit conversation
				curAction = new EntryAction();

				// Conditional part
				if (chunk == CHUNK_CCGO || chunk == CHUNK_CHDE || chunk == CHUNK_CUHD ||
					chunk == CHUNK_CDST || chunk == CHUNK_CEGO) {
					curAction->isConditional = true;
					curAction->condition.offset = convS->readUint32LE();
					curAction->condition.op = convS->readUint32LE();
					curAction->condition.val = convS->readUint32LE();
				} else {
					curAction->isConditional = false;
				}

				if (chunk == CHUNK_GOTO || chunk == CHUNK_CCGO) {
					curAction->actionType = kGotoEntry;
					if (debugFlag) printf("GOTO chunk\n");
				} else if (chunk == CHUNK_HIDE || chunk == CHUNK_CHDE) {
					curAction->actionType = kHideEntry;
					if (debugFlag) printf("HIDE chunk\n");
				} else if (chunk == CHUNK_UHID || chunk == CHUNK_CUHD) {
					curAction->actionType = kUnhideEntry;
					if (debugFlag) printf("UHID chunk\n");
				} else if (chunk == CHUNK_DSTR || chunk == CHUNK_CDST) {
					curAction->actionType = kDestroyEntry;
					if (debugFlag) printf("DSTR chunk\n");
				} else if (chunk == CHUNK_EXIT || chunk == CHUNK_CEGO) {
					curAction->actionType = kExitConv;
					if (debugFlag) printf("EXIT chunk\n");
				}
				data = convS->readUint32LE();
				if (debugFlag) printf("Offset: %i\n", data);
				curAction->targetOffset = data;
				curEntry->actions.push_back(curAction);
				break;
			case CHUNK_FALL:	// Fallthrough
				if (debugFlag) printf("FALL chunk\n");
				size = convS->readUint32LE();
				if (debugFlag) printf("Minimum nodes: %i\n", size);
				_convNodes[curNode]->fallthroughMinEntries = size;
				data = convS->readUint32LE();
				if (debugFlag) printf("Offset: %i\n", data);
				_convNodes[curNode]->fallthroughOffset = data;
				break;
			// ----------------------------------------------------------------------------
			default:
				// Should never happen
				error("Unknown conversation chunk");
				break;
		}
	}

	_vm->res()->toss(name);
}

void Converse::loadConversationMads(const char *convName) {
	char name[40];
	char buffer[256];
	char *buf;
	Common::SeekableReadStream *convS;
	int curPos = 0;
	int unk = 0;
	uint32 stringIndex = 0;
	uint32 stringCount = 0;
	int flags = 0;
	int count = 0;
	uint32 i, j;
	ConvEntry* curEntry = NULL;
	MessageEntry *curMessage;
	Common::Array<char *> messageList;
	// TODO

	// CND file
	sprintf(name, "%s.cnd", convName);
	MadsPack convDataD(name, _vm);

	// ------------------------------------------------------------
	// Chunk 0
	convS = convDataD.getItemStream(0);
	printf("Chunk 0\n");
	printf("Conv stream size: %i\n", convS->size());

	while (!convS->eos()) { // FIXME (eos changed)
		printf("%i ", convS->readByte());
	}
	printf("\n");

	// ------------------------------------------------------------
	// Chunk 1
	convS = convDataD.getItemStream(1);
	printf("Chunk 1\n");
	printf("Conv stream size: %i\n", convS->size());

	while (!convS->eos()) { // FIXME (eos changed)
		printf("%i ", convS->readByte());
	}
	printf("\n");

	// ------------------------------------------------------------
	// Chunk 2
	convS = convDataD.getItemStream(2);
	printf("Chunk 2\n");
	printf("Conv stream size: %i\n", convS->size());

	while (!convS->eos()) { // FIXME (eos changed)
		printf("%i ", convS->readByte());
	}
	printf("\n");

	// ------------------------------------------------------------
	// CNV file
	sprintf(name, "%s.cnv", convName);
	MadsPack convData(name, _vm);
	// *.cnv files have 7 chunks
	// Here is the chunk output of conv001.cnv (from the dump_file command)
	/*
	Dumping conv001.cnv, size: 3431
	Dumping compressed chunk 1 of 7, size is 150
	Dumping compressed chunk 2 of 7, size is 130
	Dumping compressed chunk 3 of 7, size is 224
	Dumping compressed chunk 4 of 7, size is 92
	Dumping compressed chunk 5 of 7, size is 168
	Dumping compressed chunk 6 of 7, size is 4064
	Dumping compressed chunk 7 of 7, size is 2334
	*/

	// ------------------------------------------------------------
	// TODO: finish this
	// Chunk 0
	convS = convData.getItemStream(0);
	printf("Chunk 0\n");
	printf("Conv stream size: %i\n", convS->size());
	printf("%i ", convS->readUint16LE());
	printf("%i ", convS->readUint16LE());
	printf("%i ", convS->readUint16LE());
	printf("%i ", convS->readUint16LE());
	printf("%i ", convS->readUint16LE());
	printf("%i ", convS->readUint16LE());
	printf("\n");
	count = convS->readUint16LE();	// conversation face count (usually 2)
	printf("Conversation faces: %i\n", count);
	for (i = 0; i < 5; i++) {
		convS->read(buffer, 16);
		printf("Face %i: %s ", i + 1, buffer);
	}
	printf("\n");

	// 5 face slots
	// 1 = face slot has a face (with the filename specified above)
	// 0 = face slot doesn't contain face data
	for (i = 0; i < 5; i++) {
		printf("%i ", convS->readUint16LE());
	}
	printf("\n");

	convS->read(buffer, 14);		// speech file
	printf("Speech file: %s\n", buffer);

	while (!convS->eos()) { // FIXME: eos changed
		printf("%i ", convS->readByte());
	}
	printf("\n");

	delete convS;

	// ------------------------------------------------------------
	// Chunk 1: Conversation nodes
	convS = convData.getItemStream(1);
	printf("Chunk 1: conversation nodes\n");
	printf("Conv stream size: %i\n", convS->size());

	while (true) {
		uint16 id = convS->readUint16LE();
		if (convS->eos()) break;

		curEntry = new ConvEntry();
		curEntry->id = id;
		curEntry->entryCount = convS->readUint16LE();
		curEntry->flags = convS->readUint16LE();
		if (curEntry->entryCount == 1 && curEntry->flags != 65535) {
			warning("Entry count is 1 and flags is not 65535 (it's %i)", flags);
		} else if (curEntry->entryCount != 1 && flags != 0) {
			warning("Entry count > 1 and flags is not 0 (it's %i)", flags);
		}
		unk = convS->readUint16LE();
		assert (unk == 65535);
		unk = convS->readUint16LE();
		assert (unk == 65535);
		_convNodes.push_back(curEntry);
		printf("Node %i, ID %i, entries %i\n", _convNodes.size(), curEntry->id, curEntry->entryCount);
		// flags = 0: node has more than 1 entry
		// flags = 65535: node has 1 entry
	}
	printf("Conversation has %i nodes\n", _convNodes.size());
	printf("\n");

	delete convS;

	// ------------------------------------------------------------
	// Chunk 4 contains the conversation string offsets of chunk 5
	// (unused, as it's unneeded - we find the offsets ourselves)

	// ------------------------------------------------------------
	// Chunk 5 contains the conversation strings
	convS = convData.getItemStream(5);
	//printf("Chunk 5: conversation strings\n");
	//printf("Conv stream size: %i\n", convS->size());

	*buffer = 0;

	while (true) {
		//if (curPos == 0)
		//	printf("%i: Offset %i: ", _convStrings.size(), convS->pos());
		uint8 b = convS->readByte();
		if (convS->eos()) break;

		buffer[curPos++] = b;
		if (buffer[curPos - 1] == '~') {		// filter out special characters
			curPos--;
			continue;
		}
		if (buffer[curPos - 1] == '\0') {
			// end of string
			//printf("%s\n", buffer);
			buf = new char[strlen(buffer)];
			sprintf(buf, "%s", buffer);
			_convStrings.push_back(buf);
			curPos = 0;
			*buffer = 0;
		}
	}

	delete convS;

	// ------------------------------------------------------------
	// Chunk 2: entry data
	convS = convData.getItemStream(2);
	//printf("Chunk 2 - entry data\n");
	//printf("Conv stream size: %i\n", convS->size());

	for (i = 0; i < _convNodes.size(); i++) {
		for (j = 0; j < _convNodes[i]->entryCount; j++) {
			curEntry = new ConvEntry();
			stringIndex = convS->readUint16LE();
			if (stringIndex != 65535)
				sprintf(curEntry->text, "%s", _convStrings[stringIndex]);
			else
				*curEntry->text = 0;
			curEntry->id = convS->readUint16LE();
			curEntry->offset = convS->readUint16LE();
			curEntry->size = convS->readUint16LE();

			_convNodes[i]->entries.push_back(curEntry);
			//printf("Node %i, entry %i, id %i, offset %i, size %i, text: %s\n",
			//		i, j, curEntry->id, curEntry->offset, curEntry->size, curEntry->text);
		}
	}

	delete convS;

	// ------------------------------------------------------------
	// Chunk 3: message (MESG) chunks, created from the strings of chunk 5
	convS = convData.getItemStream(3);
	//printf("Chunk 3 - MESG chunk data\n");
	//printf("Conv stream size: %i\n", convS->size());

	while (true) {
		uint16 index = convS->readUint16LE();
		if (convS->eos()) break;

		curMessage = new MessageEntry();
		stringIndex = index;
		stringCount = convS->readUint16LE();
		*buffer = 0;
		//printf("Message: %i\n", _madsMessageList.size());
		for (i = stringIndex; i < stringIndex + stringCount; i++) {
			//printf("%i: %s\n", i, _convStrings[i]);
			curMessage->messageStrings.push_back(_convStrings[i]);
		}
		_madsMessageList.push_back(curMessage);
		//printf("----------\n");
	}
	//printf("\n");

	delete convS;

	// ------------------------------------------------------------
	// TODO: finish this
	// Chunk 6: conversation script
	convS = convData.getItemStream(6);
	printf("Chunk 6\n");
	printf("Conv stream size: %i\n", convS->size());
	/*while (!convS->eos()) { // FIXME (eos changed)
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("%i ", convS->readByte());
		printf("\n");
	}
	return;*/

	for (i = 0; i < _convNodes.size(); i++) {
		for (j = 0; j < _convNodes[i]->entryCount; j++) {
			printf("*** Node %i entry %i data size %i\n", i, j, _convNodes[i]->entries[j]->size);
			printf("Entry ID %i, text %s\n", _convNodes[i]->entries[j]->id, _convNodes[i]->entries[j]->text);
			Common::SubReadStream *entryStream = new Common::SubReadStream(convS, _convNodes[i]->entries[j]->size);
			readConvEntryActions(entryStream, _convNodes[i]->entries[j]);
			delete entryStream;
			printf("--------------------\n");
		}
	}

	delete convS;
}

void Converse::readConvEntryActions(Common::SubReadStream *convS, ConvEntry *curEntry) {
	uint8 chunk;
	uint8 type;	// 255: normal, 11: conditional
	uint8 hasText1, hasText2;
	int target;
	int count = 0;
	int var, val;
	int messageIndex = 0;
	int unk = 0;

	while (true) {
		chunk = convS->readByte();
		if (convS->eos()) break;

		type = convS->readByte();

		switch (chunk) {
		case 1:
			printf("TODO: chunk type %i\n", chunk);
			break;
		case 2:
			printf("HIDE\n");
			convS->readByte();
			count = convS->readByte();
			printf("%i entries: ", count);
			for (int i = 0; i < count; i++)
				printf("%i %d", i, convS->readUint16LE());
			printf("\n");
			break;
		case 3:
			printf("UNHIDE\n");
			convS->readByte();
			count = convS->readByte();
			printf("%i entries: ", count);
			for (int i = 0; i < count; i++)
				printf("%i %d", i, convS->readUint16LE());
			printf("\n");
			break;
		case 4:		// MESSAGE
			printf("MESSAGE\n");

			if (type == 255) {
				//printf("unconditional\n");
			} else if (type == 11) {
				//printf("conditional\n");
			} else {
				printf("unknown type: %i\n", type);
			}

			// Conditional part
			if (type == 11) {
				unk = convS->readUint16LE();	//	1
				if (unk != 1)
					printf("Message: unk != 1 (it's %i)\n", unk);

					var = convS->readUint16LE();
					val = convS->readUint16LE();
					printf("Var %i == %i\n", var, val);
			}
			unk = convS->readUint16LE();	//	256
			if (unk != 256)
				printf("Message: unk != 256 (it's %i)\n", unk);

			// it seems that the first text entry is set when the message
			// chunk is supposed to be shown unconditionally, whereas the second text
			// entry is set when the message is supposed to be shown conditionally
			hasText1 = convS->readByte();
			hasText2 = convS->readByte();

			if (hasText1 == 1) {
				messageIndex = convS->readUint16LE();
				printf("Message 1 index: %i, text:\n", messageIndex);
				for (uint32 i = 0; i < _madsMessageList[messageIndex]->messageStrings.size(); i++) {
					printf("%s\n", _madsMessageList[messageIndex]->messageStrings[i]);
				}
			}

			if (hasText2 == 1) {
				messageIndex = convS->readUint16LE();
				if (hasText1 == 0) {
					printf("Message 2 index: %i, text:\n", messageIndex);
					for (uint32 i = 0; i < _madsMessageList[messageIndex]->messageStrings.size(); i++) {
						printf("%s\n", _madsMessageList[messageIndex]->messageStrings[i]);
					}
				}
			}

			break;
		case 5:		// AUTO
			printf("AUTO\n");
			for (int k = 0; k < 4; k++)
				convS->readByte();
			messageIndex = convS->readUint16LE();
			printf("Message index: %i, text:\n", messageIndex);
			for (uint32 i = 0; i < _madsMessageList[messageIndex]->messageStrings.size(); i++) {
				printf("%s\n", _madsMessageList[messageIndex]->messageStrings[i]);
			}

			convS->readUint16LE();
			break;
		case 6:
			printf("TODO: chunk type %i\n", chunk);
			break;
		case 7:		// GOTO
			unk = convS->readUint32LE();	// 0
			if (unk != 0 && unk != 1)
				printf("Goto: unk != 0 or 1 (it's %i)\n", unk);

			target = convS->readUint16LE();
			convS->readUint16LE();	// 255

			if (unk != 0)
				printf("Goto: unk != 0 (it's %i)\n", unk);

			if (target != 65535)
				printf("GOTO node %i\n", target);
			else
				printf("GOTO exit\n");
			break;
		case 8:
			printf("TODO: chunk type %i\n", chunk);
			break;
		case 9:		// ASSIGN
			//printf("ASSIGN\n");
			unk = convS->readUint32LE();	// 0

			if (unk != 0)
				printf("Assign: unk != 0 (it's %i)\n", unk);

			val = convS->readUint16LE();
			var = convS->readUint16LE();
			//printf("Var %i = %i\n", var, val);
			break;
		default:
			printf ("Unknown chunk type! (%i)\n", chunk);
			break;
		}
	}
	printf("\n");
}

void Converse::setEntryInfo(int32 offset, EntryType type, int32 nodeIndex, int32 entryIndex) {
	char hashOffset[10];
	sprintf(hashOffset, "%i", offset);
	EntryInfo info;
	info.targetType = type;
	info.nodeIndex = nodeIndex;
	info.entryIndex = entryIndex;
	_offsetMap[hashOffset] = info;
	//printf("Set entry info: offset %i, type %i, node %i, entry %i\n", offset, type, nodeIndex, entryIndex);
}

const EntryInfo* Converse::getEntryInfo(int32 offset) {
	char hashOffset[10];
	sprintf(hashOffset, "%i", offset);
	OffsetHashMap::const_iterator entry = _offsetMap.find(hashOffset);
	if (entry != _offsetMap.end())
		return &(entry->_value);
	else
		error("Undeclared entry offset: %i", offset);
}

void Converse::setValue(int32 offset, int32 value) {
	char hashOffset[10];
	sprintf(hashOffset, "%i", offset);
	_variables[hashOffset] = value;
}

int32 Converse::getValue(int32 offset) {
	char hashOffset[10];
	sprintf(hashOffset, "%i", offset);
	ConvVarHashMap::const_iterator entry = _variables.find(hashOffset);
	if (entry != _variables.end())
		return entry->_value;
	else
		error("Undeclared variable offset: %i", offset);
}

bool Converse::evaluateCondition(int32 leftVal, int32 op, int32 rightVal) {
	switch (op) {
	case kOpPercent:
		return (leftVal % rightVal == 0);
	case kOpGreaterOrEqual:
		return leftVal >= rightVal;
	case kOpLessOrEqual:
		return leftVal <= rightVal;
	case kOpGreaterThan:
		return leftVal > rightVal;
	case kOpLessThan:
		return leftVal < rightVal;
	case kOpNotEqual:
	case kOpCondNotEqual:
		return leftVal != rightVal;
	case kOpAssign:
		return leftVal == rightVal;
	case kOpAnd:
		return leftVal && rightVal;
	case kOpOr:
		return leftVal || rightVal;
	default:
		error("Unknown conditional operator: %i", op);
	}
}

bool Converse::performAction(EntryAction *action) {
	if (action->isConditional) {
		if (!evaluateCondition(getValue(action->condition.offset),
										action->condition.op, action->condition.val))
							return true;	// don't perform this action
	}

	if (action->actionType == kAssignValue) {
		//printf("Assigning variable at offset %i to value %i\n",
		//		action->targetOffset, action->value);
		setValue(action->targetOffset, action->value);
		return true;		// nothing else to do in an assignment action
	}

	const EntryInfo *entryInfo = getEntryInfo(action->targetOffset);
	ConvEntry *targetEntry;

	if (entryInfo->nodeIndex >= 0 && entryInfo->entryIndex >= 0)
		targetEntry = getNode(entryInfo->nodeIndex)->entries[entryInfo->entryIndex];
	else if (entryInfo->nodeIndex >= 0)
		targetEntry = getNode(entryInfo->nodeIndex);
	else
		error("Target node id is negative");

	switch (action->actionType) {
	case kGotoEntry:
		//printf("Goto entry at offset %i. Associated node is %i, entry %i\n",
		//	action->targetOffset, entryInfo->nodeIndex, entryInfo->entryIndex);
		_vm->_conversationView->setNode(entryInfo->nodeIndex);
		if (entryInfo->entryIndex >= 0)
			_vm->_conversationView->selectEntry(entryInfo->entryIndex);
		return false;
	case kHideEntry:
		//printf("Hide entry at offset %i. Associated node is %i, entry %i\n",
		//	targetEntry->offset, entryInfo->nodeIndex, entryInfo->entryIndex);
		targetEntry->visible = false;
		return true;
	case kUnhideEntry:
		//printf("Show entry at offset %i. Associated node is %i, entry %i\n",
		//	targetEntry->offset, entryInfo->nodeIndex, entryInfo->entryIndex);
		targetEntry->visible = true;
		return true;
	case kDestroyEntry:
		//printf("Destroy entry at offset %i. Associated node is %i, entry %i\n",
		//	targetEntry->offset, entryInfo->nodeIndex, entryInfo->entryIndex);
		if (entryInfo->entryIndex >= 0)
			getNode(entryInfo->nodeIndex)->entries.remove_at(entryInfo->entryIndex);
		else
			warning("Target entry is a node, not destroying it");
		targetEntry->visible = true;
		return true;
	case kExitConv:
		//printf("Exit conversation\n");
		endConversation();
		return false;
	default:
		warning("Unknown entry action");
		return false;
	}	// end switch
}

} // End of namespace M4
