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

#include "common/util.h"
#include "sherlock/objects.h"
#include "sherlock/people.h"
#include "sherlock/scene.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

#define START_FRAME 0

#define NUM_ADJUSTED_WALKS 21

// Distance to walk around WALK_AROUND boxes
#define CLEAR_DIST_X 5
#define CLEAR_DIST_Y 0

#define ADJUST_COORD(COORD) \
	if (COORD.x != -1) \
		COORD.x *= FIXED_INT_MULTIPLIER; \
	if (COORD.y != -1) \
		COORD.y *= FIXED_INT_MULTIPLIER

SherlockEngine *BaseObject::_vm;
bool BaseObject::_countCAnimFrames;

/*----------------------------------------------------------------*/

void BaseObject::setVm(SherlockEngine *vm) {
	_vm = vm;
	_countCAnimFrames = false;
}

BaseObject::BaseObject() {
	_type = INVALID;
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;
	_sequenceNumber = 0;
	_startSeq = 0;
	_walkCount = 0;
	_allow = 0;
	_frameNumber = 0;
	_lookFlag = 0;
	_requiredFlag[0] = _requiredFlag[1] = 0;
	_status = 0;
	_misc = 0;
	_maxFrames = 0;
	_flags = 0;
	_aType = OBJECT;
	_lookFrames = 0;
	_seqCounter = 0;
	_lookcAnim = 0;
	_seqStack = 0;
	_seqTo = 0;
	_descOffset = 0;
	_seqCounter2 = 0;
	_seqSize = 0;
	_quickDraw = 0;
	_scaleVal = 0;
	_gotoSeq = 0;
	_talkSeq = 0;
	_restoreSlot = 0;
}

bool BaseObject::hasAborts() const {
	int seqNum = _talkSeq;

	// See if the object is in its regular sequence
	bool startChecking = !seqNum || _type == CHARACTER;

	uint idx = 0;
	do
	{
		// Get the Frame value
		int v = _sequences[idx++];

		// See if we found an Allow Talk Interrupt Code
		if (startChecking && v == ALLOW_TALK_CODE)
			return true;

		// If we've started checking and we've encountered another Talk or Listen Sequence Code,
		// then we're done checking this sequence because this is where it would repeat
		if (startChecking && (v == TALK_SEQ_CODE || v == TALK_LISTEN_CODE))
			break;

		// See if we've found the beginning of a Talk Sequence
		if ((v == TALK_SEQ_CODE && seqNum < 128) || (v == TALK_LISTEN_CODE && seqNum >= 128)) {
			// If checking was already on and we came across one of these codes, then there couldn't
			// have been an Allow Talk Interrupt code in the sequence we were checking, so we're done.
			if (startChecking)
				break;

			seqNum--;
			// See if we're at the correct Talk Sequence Number
			if (!(seqNum & 127))
			{
				// Correct Sequence, Start Checking Now
				startChecking = true;
			}
		} else {
			// Move ahead any extra because of special control codes
			switch (v) {
			case 0:				idx++; break;
			case MOVE_CODE:
			case TELEPORT_CODE:	idx += 4; break;
			case CALL_TALK_CODE:idx += 8; break;
			case HIDE_CODE:		idx += 2; break;
			default: break;
			}
		}
	} while (idx < _seqSize);

	return false;
}

void BaseObject::checkObject() {
	Scene &scene = *_vm->_scene;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	int checkFrame = _allow ? MAX_FRAME : FRAMES_END;
	bool codeFound;

	if (_seqTo) {
		byte *ptr = &_sequences[_frameNumber];
		if (*ptr == _seqTo) {
			// The sequence is completed. Reset to normal
			*ptr = _seqTo + (IS_ROSE_TATTOO ? 0 : SEQ_TO_CODE + 128);
			_seqTo = 0;
		} else {
			// Continue doing sequence
			if (*ptr > _seqTo)
				*ptr -= 1;
			else
				*ptr += 1;

			return;
		}
	}

	++_frameNumber;

	do {
		if (!_sequences) {
			warning("checkObject: _sequences is not set");
			break;
		}

		// Check for end of sequence
		codeFound = checkEndOfSequence();

		if (_sequences[_frameNumber] >= 128 && _frameNumber < checkFrame) {
			codeFound = true;
			int v = _sequences[_frameNumber];

			// Check for a Talk or Listen Sequence
			if (IS_ROSE_TATTOO && v == ALLOW_TALK_CODE) {
				if (_gotoSeq) {
					setObjTalkSequence(_gotoSeq);
					_gotoSeq = 0;
				} else {
					++_frameNumber;
				}
			} else if (IS_ROSE_TATTOO && (v == TALK_SEQ_CODE || v == TALK_LISTEN_CODE)) {
				if (_talkSeq)
					setObjTalkSequence(_talkSeq);
				else
					setObjSequence(0, false);
			} else  if (v >= GOTO_CODE) {
				// Goto code found
				v -= GOTO_CODE;
				_seqCounter2 = _seqCounter;
				_seqStack = _frameNumber + 1;
				setObjSequence(v, false);
			} else if (v >= SOUND_CODE && (v < (SOUND_CODE + 30))) {
				codeFound = true;
				++_frameNumber;
				v -= SOUND_CODE + (IS_SERRATED_SCALPEL ? 1 : 0);

				if (sound._soundOn && !_countCAnimFrames) {
					if (!scene._sounds[v]._name.empty() && sound._digitized)
						sound.playLoadedSound(v, WAIT_RETURN_IMMEDIATELY);
				}
			} else if (v >= FLIP_CODE && v < (FLIP_CODE + 3)) {
				// Flip code
				codeFound = true;
				++_frameNumber;
				v -= FLIP_CODE;

				// Alter the flipped status
				switch (v) {
				case 0:
					// Clear the flag
					_flags &= ~OBJ_FLIPPED;
					break;
				case 1:
					// Set the flag
					_flags |= OBJ_FLIPPED;
					break;
				case 2:
					// Toggle the flag
					_flags ^= OBJ_FLIPPED;
					break;
				default:
					break;
				}
			} else if (IS_ROSE_TATTOO && v == TELEPORT_CODE) {
				_position.x = READ_LE_UINT16(&_sequences[_frameNumber + 1]);
				_position.y = READ_LE_UINT16(&_sequences[_frameNumber + 3]);

				_frameNumber += 5;
			} else if (IS_ROSE_TATTOO && v == CALL_TALK_CODE) {
				Common::String filename;
				for (int idx = 0; idx < 8; ++idx) {
					if (_sequences[_frameNumber + 1 + idx] != 1)
						filename += (char)_sequences[_frameNumber + 1 + idx];
					else
						break;
				}

				_frameNumber += 8;
				talk.talkTo(filename);

			} else if (IS_ROSE_TATTOO && v == HIDE_CODE) {
				switch (_sequences[_frameNumber + 2]) {
				case 1:
					// Hide Object
					if (scene._bgShapes[_sequences[_frameNumber + 1] - 1]._type != HIDDEN)
						scene._bgShapes[_sequences[_frameNumber + 1] - 1].toggleHidden();
					break;

				case 2:
					// Activate Object
					if (scene._bgShapes[_sequences[_frameNumber + 1] - 1]._type == HIDDEN)
						scene._bgShapes[_sequences[_frameNumber + 1] - 1].toggleHidden();
					break;

				case 3:
					// Toggle Object
					scene._bgShapes[_sequences[_frameNumber + 1] - 1].toggleHidden();
					break;

				default:
					break;
				}
				_frameNumber += 3;

			} else {
				v -= 128;

				// 68-99 is a sequence code
				if (v > SEQ_TO_CODE) {
					if (IS_ROSE_TATTOO) {
						++_frameNumber;
						byte *p = &_sequences[_frameNumber];
						_seqTo = *p;
						*p = *(p - 2);

						if (*p > _seqTo)
							*p -= 1;
						else
							*p += 1;

						--_frameNumber;
					} else {
						byte *p = &_sequences[_frameNumber];
						v -= SEQ_TO_CODE;	// # from 1-32
						_seqTo = v;
						*p = *(p - 1);

						if (*p > 128)
							// If the high bit is set, convert to a real frame
							*p -= (byte)(SEQ_TO_CODE - 128);

						if (*p > _seqTo)
							*p -= 1;
						else
							*p += 1;

						// Will be incremented below to return back to original value
						--_frameNumber;
						v = 0;
					}
				} else if (IS_ROSE_TATTOO && v == 10) {
					// Set delta for objects
					_delta = Common::Point(READ_LE_UINT16(&_sequences[_frameNumber + 1]),
						READ_LE_UINT16(&_sequences[_frameNumber + 3]));
					_noShapeSize = Common::Point(0, 0);
					_frameNumber += 4;

				} else if (v == 10) {
					// Set delta for objects
					Common::Point pt(_sequences[_frameNumber + 1], _sequences[_frameNumber + 2]);
					if (pt.x > 128)
						pt.x = (pt.x - 128) *  -1;
					else
						pt.x--;

					if (pt.y > 128)
						pt.y = (pt.y - 128) * -1;
					else
						pt.y--;

					_delta = pt;
					_frameNumber += 2;

				} else if (v < USE_COUNT) {
					for (int idx = 0; idx < NAMES_COUNT; ++idx) {
						checkNameForCodes(_use[v]._names[idx]);
					}

					if (_use[v]._useFlag)
						_vm->setFlags(_use[v]._useFlag);
				}

				++_frameNumber;
			}
		}
	} while (codeFound);
}

bool BaseObject::checkEndOfSequence() {
	Screen &screen = *_vm->_screen;
	int checkFrame = _allow ? MAX_FRAME : FRAMES_END;
	bool result = false;

	if (_type == REMOVE || _type == INVALID)
		return false;

	if (_frameNumber < 0 || _frameNumber >= checkFrame || _sequences[_frameNumber] == 0) {
		result = true;

		if (_frameNumber < 0 || _frameNumber >= (checkFrame - 1)) {
			_frameNumber = START_FRAME;
		}  else {
			// Determine next sequence to use
			int seq = _sequences[_frameNumber + 1];

			// If the object has been turned off, we're going nowhere
			if (IS_ROSE_TATTOO && (_type == HIDE_SHAPE || _type == HIDDEN || _type == REMOVE))
				return false;

			if (seq == 99) {
				--_frameNumber;
				screen._backBuffer1.SHtransBlitFrom(*_imageFrame, _position);
				screen._backBuffer2.SHtransBlitFrom(*_imageFrame, _position);
				_type = INVALID;
			} else if (IS_ROSE_TATTOO && _talkSeq && seq == 0) {
				setObjTalkSequence(_talkSeq);
			} else {
				setObjSequence(seq, false);
			}
		}

		if (_allow && _frameNumber == 0) {
			// canimation just ended
			if (_type != NO_SHAPE && _type != REMOVE) {
				_type = REMOVE;

				if (!_countCAnimFrames) {
					// Save details before shape is removed
					_delta.x = _imageFrame->_frame.w;
					_delta.y = _imageFrame->_frame.h;
					_position += _imageFrame->_offset;

					// Free the images
					delete _images;
					_images = nullptr;
					_imageFrame = nullptr;
				}
			} else {
				_type = INVALID;
			}
		}
	}

	return result;
}

void BaseObject::setObjSequence(int seq, bool wait) {
	Scene &scene = *_vm->_scene;
	int checkFrame = _allow ? MAX_FRAME : FRAMES_END;

	if (IS_ROSE_TATTOO && (seq == -1 || seq == 255))
		// This means goto beginning
		seq = 0;

	if (seq >= 128) {
		// Loop the sequence until the count exceeded
		seq -= 128;

		++_seqCounter;
		if (_seqCounter >= seq) {
			// Go to next sequence
			if (_seqStack) {
				_frameNumber = _seqStack;
				_seqStack = 0;
				_seqCounter = _seqCounter2;
				_seqCounter2 = 0;
				if (_frameNumber >= checkFrame)
					_frameNumber = START_FRAME;

				return;
			}

			_frameNumber += 2;
			if (_frameNumber >= checkFrame)
				_frameNumber = 0;

			// For Rose Tattoo, save the starting frame for new sequences
			if (IS_ROSE_TATTOO)
				_startSeq = _frameNumber;

			_seqCounter = 0;
			if (_sequences[_frameNumber] == 0)
				seq = _sequences[_frameNumber + 1];
			else
				return;
		} else {
			// Find beginning of sequence
			if (IS_ROSE_TATTOO) {
				// Use the saved start of the sequence to reset the frame
				_frameNumber = _startSeq;
			} else {
				// For Scalpel, scan backwards from the end of the sequence to find its start
				do {
					--_frameNumber;
				} while (_frameNumber > 0 && _sequences[_frameNumber] != 0);

				if (_frameNumber != 0)
					_frameNumber += 2;
			}

			return;
		}
	} else {
		// Reset sequence counter
		_seqCounter = 0;
	}

	int idx = 0;
	int seqCc = 0;

	while (seqCc < seq && idx < checkFrame) {
		if (IS_SERRATED_SCALPEL) {
			++idx;

			if (_sequences[idx] == 0) {
				++seqCc;
				idx += 2;
			}
		} else {
			byte s = _sequences[idx];

			if (s == 0) {
				++seqCc;
				++idx;
			} else if (s == MOVE_CODE || s == TELEPORT_CODE) {
				idx += 4;
			} else if (s == CALL_TALK_CODE) {
				idx += 8;
			} else if (s == HIDE_CODE) {
				idx += 2;
			}

			++idx;
		}
	}

	if (idx >= checkFrame)
		idx = 0;
	_frameNumber = idx;
	_startSeq = idx;

	if (wait) {
		seqCc = idx;
		while (_sequences[idx] != 0)
			++idx;

		idx = idx - seqCc + 2;
		for (; idx > 0; --idx)
			scene.doBgAnim();
	}
}

int BaseObject::checkNameForCodes(const Common::String &name, FixedTextActionId fixedTextActionId) {
	FixedText &fixedText = *_vm->_fixedText;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
	bool printed = false;

	scene.toggleObject(name);

	if (name.hasPrefix("*")) {
		// A code was found
		printed = true;
		char ch = (name == "*") ? 0 : toupper(name[1]);

		switch (ch) {
		case 'C':
			talk.talkTo(name.c_str() + 2);
			break;

		case 'T':
		case 'B':
		case 'F':
		case 'W':
			// Nothing: action was already done before canimation
			break;

		case 'G':
		case 'A': {
			// G: Have object go somewhere
			// A: Add onto existing co-ordinates
			Common::String sx(name.c_str() + 2, name.c_str() + 5);
			Common::String sy(name.c_str() + 5, name.c_str() + 8);

			if (ch == 'G')
				_position = Common::Point(atoi(sx.c_str()), atoi(sy.c_str()));
			else
				_position += Common::Point(atoi(sx.c_str()), atoi(sy.c_str()));
			break;
		}

		case 'V':
			// Do nothing for Verb codes. This is only a flag for Inventory syntax
			break;

		default:
			if (ch >= '0' && ch <= '9') {
				scene._goToScene = atoi(name.c_str() + 1);

				if (IS_SERRATED_SCALPEL && scene._goToScene < 97) {
					Scalpel::ScalpelMap &map = *(Scalpel::ScalpelMap *)_vm->_map;
					if (map[scene._goToScene].x) {
						map._overPos.x = (map[scene._goToScene].x - 6) * FIXED_INT_MULTIPLIER;
						map._overPos.y = (map[scene._goToScene].y + 9) * FIXED_INT_MULTIPLIER;
					}
				}

				const char *p;
				if ((p = strchr(name.c_str(), ',')) != nullptr) {
					++p;

					Common::String s(p, p + 3);
					people._savedPos.x = atoi(s.c_str());

					s = Common::String(p + 3, p + 6);
					people._savedPos.y = atoi(s.c_str());

					s = Common::String(p + 6, p + 9);
					people._savedPos._facing = atoi(s.c_str());
					if (people._savedPos._facing == 0)
						people._savedPos._facing = 10;
				} else if ((p = strchr(name.c_str(), '/')) != nullptr) {
					people._savedPos = PositionFacing(1, 0, 100 + atoi(p + 1));
				}
			} else {
				scene._goToScene = 100;
			}

			people[HOLMES]._position = Point32(0, 0);
			break;
		}
	} else if (name.hasPrefix("!")) {
		// Message attached to canimation
		int messageNum = atoi(name.c_str() + 1);
		ui._infoFlag = true;
		ui.clearInfo();
		Common::String errorMessage = fixedText.getActionMessage(fixedTextActionId, messageNum);
		screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, "%s", errorMessage.c_str());
		ui._menuCounter = 25;
	} else if (name.hasPrefix("@")) {
		// Message attached to canimation
		ui._infoFlag = true;
		ui.clearInfo();
		screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, "%s", name.c_str() + 1);
		printed = true;
		ui._menuCounter = 25;
	}

	return printed;
}

/*----------------------------------------------------------------*/

void Sprite::clear() {
	_name = "";
	_description = "";
	_examine.clear();
	_pickUp = "";
	_walkSequences.clear();
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;
	_walkCount = 0;
	_oldWalkSequence = 0;
	_allow = 0;
	_frameNumber = 0;
	_position.x = _position.y = 0;
	_delta.x = _delta.y = 0;
	_oldPosition.x = _oldPosition.y = 0;
	_oldSize.x = _oldSize.y = 0;
	_goto.x = _goto.y = 0;
	_type = INVALID;
	_pickUp.clear();
	_noShapeSize.x = _noShapeSize.y = 0;
	_status = 0;
	_misc = 0;
	_altImages = nullptr;
	_altSeq = 0;
	_centerWalk = 0;

	for (int i = 0; i < 8; i++)
		_stopFrames[i] = nullptr;
}

void Sprite::setImageFrame() {
	int frameNum = MAX(_frameNumber, 0);
	int imageNumber = _walkSequences[_sequenceNumber][frameNum];

	if (IS_SERRATED_SCALPEL)
		imageNumber = imageNumber + _walkSequences[_sequenceNumber][0] - 2;
	else if (imageNumber > _maxFrames)
		imageNumber = 1;

	// Get the images to use
	ImageFile *images = _altSeq ? _altImages : _images;
	assert(images);

	if (IS_3DO) {
		// only do this to the image-array with 110 entries
		// map uses another image-array and this code
		if (images->size() == 110) {
			// 3DO has 110 animation frames inside walk.anim
			// PC has 55
			// this adjusts the framenumber accordingly
			// sort of HACK
			imageNumber *= 2;
		}
	} else if (IS_ROSE_TATTOO) {
		--imageNumber;
	}

	// Set the frame pointer
	_imageFrame = &(*images)[imageNumber];
}

void Sprite::checkSprite() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	Point32 pt;
	Common::Rect objBounds;
	Common::Point spritePt(_position.x / FIXED_INT_MULTIPLIER, _position.y / FIXED_INT_MULTIPLIER);

	if (_type != CHARACTER || (IS_SERRATED_SCALPEL && talk._talkCounter))
		return;

	pt = _walkCount ? _position + _delta : _position;
	pt.x /= FIXED_INT_MULTIPLIER;
	pt.y /= FIXED_INT_MULTIPLIER;

	if (IS_ROSE_TATTOO) {
		checkObject();

		// For Rose Tattoo, we only do the further processing for Sherlock
		if (this != &people[HOLMES])
			return;
	}

	for (uint idx = 0; idx < scene._bgShapes.size() && !talk._talkToAbort; ++idx) {
		Object &obj = scene._bgShapes[idx];
		if (obj._aType <= PERSON || obj._type == INVALID || obj._type == HIDDEN)
			continue;

		if (obj._type == NO_SHAPE) {
			objBounds = Common::Rect(obj._position.x, obj._position.y,
				obj._position.x + obj._noShapeSize.x + 1, obj._position.y + obj._noShapeSize.y + 1);
		} else {
			int xp = obj._position.x + obj._imageFrame->_offset.x;
			int yp = obj._position.y + obj._imageFrame->_offset.y;
			objBounds = Common::Rect(xp, yp,
				xp + obj._imageFrame->_frame.w + 1, yp + obj._imageFrame->_frame.h + 1);
		}

		if (objBounds.contains(pt)) {
			if (objBounds.contains(spritePt)) {
				// Current point is already inside the the bounds, so impact occurred
				// on a previous call. So simply do nothing until we're clear of the box
				switch (obj._aType) {
				case TALK_MOVE:
					if (_walkCount) {
						// Holmes is moving
						obj._type = HIDDEN;
						obj.setFlagsAndToggles();
						talk.talkTo(obj._use[0]._target);
					}
					break;

				case PAL_CHANGE:
				case PAL_CHANGE2:
					if (_walkCount) {
						int palStart = atoi(obj._use[0]._names[0].c_str()) * 3;
						int palLength = atoi(obj._use[0]._names[1].c_str()) * 3;
						int templ = atoi(obj._use[0]._names[2].c_str()) * 3;
						if (templ == 0)
							templ = 100;

						// Ensure only valid palette change data found
						if (palLength > 0) {
							// Figure out how far into the shape Holmes is so that we
							// can figure out what percentage of the original palette
							// to set the current palette to
							int palPercent = (pt.x - objBounds.left) * 100 / objBounds.width();
							palPercent = palPercent * templ / 100;
							if (obj._aType == PAL_CHANGE)
								// Invert percentage
								palPercent = 100 - palPercent;

							for (int i = palStart; i < (palStart + palLength); ++i)
								screen._sMap[i] = screen._cMap[i] * palPercent / 100;

							events.pollEvents();
							screen.setPalette(screen._sMap);
						}
					}
					break;

				case TALK:
				case TALK_EVERY:
					obj._type = HIDDEN;
					obj.setFlagsAndToggles();
					talk.talkTo(obj._use[0]._target);
					break;

				default:
					break;
				}
			} else {
				// New impact just occurred
				switch (obj._aType) {
				case BLANK_ZONE:
					// A blank zone masks out all other remaining zones underneath it.
					// If this zone is hit, exit the outer loop so we do not check anymore
					return;

				case SOLID:
				case TALK:
					// Stop walking
					if (obj._aType == TALK) {
						obj.setFlagsAndToggles();
						talk.talkTo(obj._use[0]._target);
					} else {
						gotoStand();
					}
					break;

				case TALK_EVERY:
					if (obj._aType == TALK_EVERY) {
						obj._type = HIDDEN;
						obj.setFlagsAndToggles();
						talk.talkTo(obj._use[0]._target);
					} else {
						gotoStand();
					}
					break;

				case FLAG_SET:
					obj.setFlagsAndToggles();
					obj._type = HIDDEN;
					break;

				case WALK_AROUND:
					if (objBounds.contains(people[HOLMES]._walkTo.front())) {
						// Reached zone
						gotoStand();
					} else {
						// Destination not within box, walk to best corner
						Common::Point walkPos;

						if (spritePt.x >= objBounds.left && spritePt.x < objBounds.right) {
							// Impact occurred due to vertical movement. Determine whether to
							// travel to the left or right side
							if (_delta.x > 0)
								// Go to right side
								walkPos.x = objBounds.right + CLEAR_DIST_X;
							else if (_delta.x < 0) {
								// Go to left side
								walkPos.x = objBounds.left - CLEAR_DIST_X;
							} else {
								// Going straight up or down. So choose best side
								if (spritePt.x >= (objBounds.left + objBounds.width() / 2))
									walkPos.x = objBounds.right + CLEAR_DIST_X;
								else
									walkPos.x = objBounds.left - CLEAR_DIST_X;
							}

							walkPos.y = (_delta.y >= 0) ? objBounds.top - CLEAR_DIST_Y :
								objBounds.bottom + CLEAR_DIST_Y;
						} else {
							// Impact occurred due to horizontal movement
							if (_delta.y > 0)
								// Go to bottom of box
								walkPos.y = objBounds.bottom + CLEAR_DIST_Y;
							else if (_delta.y < 0)
								// Go to top of box
								walkPos.y = objBounds.top - CLEAR_DIST_Y;
							else {
								// Going straight horizontal, so choose best side
								if (spritePt.y >= (objBounds.top + objBounds.height() / 2))
									walkPos.y = objBounds.bottom + CLEAR_DIST_Y;
								else
									walkPos.y = objBounds.top - CLEAR_DIST_Y;
							}

							walkPos.x = (_delta.x >= 0) ? objBounds.left - CLEAR_DIST_X :
								objBounds.right + CLEAR_DIST_X;
						}

						walkPos.x += people[HOLMES]._imageFrame->_frame.w / 2;
						people[HOLMES]._walkDest = walkPos;
						people[HOLMES]._walkTo.push(walkPos);
						people[HOLMES].setWalking();
					}
					break;

				case DELTA:
					_position.x += 200;
					break;

				default:
					break;
				}
			}
		}
	}
}

const Common::Rect Sprite::getOldBounds() const {
	return Common::Rect(_oldPosition.x, _oldPosition.y, _oldPosition.x + _oldSize.x, _oldPosition.y + _oldSize.y);
}

/*----------------------------------------------------------------*/

void WalkSequence::load(Common::SeekableReadStream &s) {
	char buffer[9];
	s.read(buffer, 9);
	_vgsName = Common::String(buffer);
	_horizFlip = s.readByte() != 0;

	_sequences.resize(s.readUint16LE());
	s.skip(4);		// Skip over pointer field of structure

	s.read(&_sequences[0], _sequences.size());
}

/*----------------------------------------------------------------*/

WalkSequences &WalkSequences::operator=(const WalkSequences &src) {
	resize(src.size());
	for (uint idx = 0; idx < size(); ++idx) {
		const WalkSequence &wSrc = src[idx];
		WalkSequence &wDest = (*this)[idx];
		wDest._horizFlip = wSrc._horizFlip;

		wDest._sequences.resize(wSrc._sequences.size());
		Common::copy(&wSrc._sequences[0], &wSrc._sequences[0] + wSrc._sequences.size(), &wDest._sequences[0]);
	}

	return *this;
}

/*----------------------------------------------------------------*/

ActionType::ActionType() {
	_cAnimNum = _cAnimSpeed = 0;
	_useFlag = 0;
}

void ActionType::load(Common::SeekableReadStream &s) {
	char buffer[12];

	_cAnimNum = s.readByte();
	_cAnimSpeed = s.readByte();
	if (_cAnimSpeed & 0x80)
		_cAnimSpeed = -(_cAnimSpeed & 0x7f);

	for (int idx = 0; idx < NAMES_COUNT; ++idx) {
		s.read(buffer, 12);
		_names[idx] = Common::String(buffer);
	}
}

/*----------------------------------------------------------------*/

UseType::UseType(): ActionType() {
}

void UseType::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
	char buffer[12];

	if (isRoseTattoo) {
		s.read(buffer, 12);
		_verb = Common::String(buffer);
	}

	ActionType::load(s);

	_useFlag = s.readSint16LE();

	if (!isRoseTattoo)
		s.skip(6);

	s.read(buffer, 12);
	_target = Common::String(buffer);
}

void UseType::load3DO(Common::SeekableReadStream &s) {
	char buffer[12];

	_cAnimNum = s.readByte();
	_cAnimSpeed = s.readByte();
	if (_cAnimSpeed & 0x80)
		_cAnimSpeed = -(_cAnimSpeed & 0x7f);

	for (int idx = 0; idx < NAMES_COUNT; ++idx) {
		s.read(buffer, 12);
		_names[idx] = Common::String(buffer);
	}

	_useFlag = s.readSint16BE();

	s.skip(6);

	s.read(buffer, 12);
	_target = Common::String(buffer);
}

void UseType::synchronize(Serializer &s) {
	s.syncString(_verb);
	s.syncAsSint16LE(_cAnimNum);
	s.syncAsSint16LE(_cAnimSpeed);
	s.syncAsSint16LE(_useFlag);

	for (int idx = 0; idx < 4; ++idx)
		s.syncString(_names[idx]);
	s.syncString(_target);
}

/*----------------------------------------------------------------*/

Object::Object(): BaseObject() {
	_sequenceOffset = 0;
	_pickup = 0;
	_defaultCommand = 0;
	_pickupFlag = 0;
}

void Object::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
	char buffer[41];
	s.read(buffer, 12);
	_name = Common::String(buffer);
	s.read(buffer, 41);
	_description = Common::String(buffer);

	_examine.clear();
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;

	s.skip(4);
	_sequenceOffset = s.readUint16LE();
	s.seek(10, SEEK_CUR);

	_walkCount = s.readByte();
	_allow = s.readByte();
	_frameNumber = s.readSint16LE();
	_sequenceNumber = s.readSint16LE();
	_position.x = s.readSint16LE();
	_position.y = s.readSint16LE();
	_delta.x = s.readSint16LE();
	_delta.y = s.readSint16LE();
	_type = (SpriteType)s.readUint16LE();
	_oldPosition.x = s.readSint16LE();
	_oldPosition.y = s.readSint16LE();
	_oldSize.x = s.readUint16LE();
	_oldSize.y = s.readUint16LE();

	_goto.x = s.readSint16LE();
	_goto.y = s.readSint16LE();
	if (!isRoseTattoo) {
		_goto.x = _goto.x * FIXED_INT_MULTIPLIER / 100;
		_goto.y = _goto.y * FIXED_INT_MULTIPLIER / 100;
	}

	_pickup = isRoseTattoo ? 0 : s.readByte();
	_defaultCommand = isRoseTattoo ? 0 : s.readByte();
	_lookFlag = s.readSint16LE();
	_pickupFlag = isRoseTattoo ? 0 : s.readSint16LE();
	_requiredFlag[0] = s.readSint16LE();
	_noShapeSize.x = s.readUint16LE();
	_noShapeSize.y = s.readUint16LE();
	_status = s.readUint16LE();
	_misc = s.readByte();
	_maxFrames = s.readUint16LE();
	_flags = s.readByte();

	if (!isRoseTattoo)
		_aOpen.load(s);

	_aType = (AType)s.readByte();
	_lookFrames = s.readByte();
	_seqCounter = s.readByte();
	if (isRoseTattoo) {
		_lookPosition.x = s.readUint16LE() * FIXED_INT_MULTIPLIER;
		_lookPosition.y = s.readSint16LE() * FIXED_INT_MULTIPLIER;
	} else {
		_lookPosition.x = s.readUint16LE() * FIXED_INT_MULTIPLIER / 100;
		_lookPosition.y = s.readByte() * FIXED_INT_MULTIPLIER;
	}
	_lookPosition._facing = s.readByte();
	_lookcAnim = s.readByte();

	if (!isRoseTattoo)
		_aClose.load(s);

	_seqStack = s.readByte();
	_seqTo = s.readByte();
	_descOffset = s.readUint16LE();
	_seqCounter2 = s.readByte();
	_seqSize = s.readUint16LE();

	if (isRoseTattoo) {
		for (int idx = 0; idx < 6; ++idx)
			_use[idx].load(s, true);

		// WORKAROUND: Fix German version using hatpin/pin in pillow in Pratt's loft
		if (_use[1]._target == "Nadel" && _use[1]._verb == "Untersuche"
				&& _use[2]._target == "Nadel" && _use[2]._verb == "Untersuche")
			_use[1]._target = "Alte Nadel";

		_quickDraw = s.readByte();
		_scaleVal = s.readUint16LE();
		_requiredFlag[1] = s.readSint16LE();
		_gotoSeq = s.readByte();
		_talkSeq = s.readByte();
		_restoreSlot = s.readByte();
	} else {
		s.skip(1);
		_aMove.load(s);
		s.skip(8);

		for (int idx = 0; idx < 4; ++idx)
			_use[idx].load(s, false);
	}
	//warning("object %s, useAnim %d", _name.c_str(), _use[0]._cAnimNum);
}

void Object::load3DO(Common::SeekableReadStream &s) {
	int32 streamStartPos = s.pos();
	char buffer[41];

	_examine.clear();
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;

	// on 3DO all of this data is reordered!!!
	// it seems that possibly the 3DO compiler reordered the global struct
	// 3DO size for 1 object is 588 bytes
	s.skip(4);
	_sequenceOffset = s.readUint16LE(); // weird that this seems to be LE
	s.seek(10, SEEK_CUR);

	// Offset 16
	_frameNumber = s.readSint16BE();
	_sequenceNumber = s.readSint16BE();
	_position.x = s.readSint16BE();
	_position.y = s.readSint16BE();
	_delta.x = s.readSint16BE();
	_delta.y = s.readSint16BE();
	_type = (SpriteType)s.readUint16BE();
	_oldPosition.x = s.readSint16BE();
	_oldPosition.y = s.readSint16BE();
	_oldSize.x = s.readUint16BE();
	_oldSize.y = s.readUint16BE();

	_goto.x = s.readSint16BE();
	_goto.y = s.readSint16BE();
	_goto.x = _goto.x * FIXED_INT_MULTIPLIER / 100;
	_goto.y = _goto.y * FIXED_INT_MULTIPLIER / 100;

	// Offset 42
	warning("pos %d", s.pos());

	// Unverified
	_lookFlag = s.readSint16BE();
	_pickupFlag = s.readSint16BE();
	_requiredFlag[0] = s.readSint16BE();
	_noShapeSize.x = s.readUint16BE();
	_noShapeSize.y = s.readUint16BE();
	_status = s.readUint16BE();
	// Unverified END

	_maxFrames = s.readUint16BE();
	// offset 56
	_lookPosition.x = s.readUint16BE() * FIXED_INT_MULTIPLIER / 100;
	// offset 58
	_descOffset = s.readUint16BE();
	_seqSize = s.readUint16BE();

	s.skip(2); // boundary filler

	// 288 bytes
	for (int idx = 0; idx < 4; ++idx) {
		_use[idx].load3DO(s);
		s.skip(2); // Filler
	}

	// 158 bytes
	_aOpen.load(s); // 2 + 12*4 bytes = 50 bytes
	s.skip(2); // Boundary filler
	_aClose.load(s);
	s.skip(2); // Filler
	_aMove.load(s);
	s.skip(2); // Filler

	// offset 508
	// 3DO: name is at the end
	s.read(buffer, 12);
	_name = Common::String(buffer);
	s.read(buffer, 41);
	_description = Common::String(buffer);

	// Unverified
	_walkCount = s.readByte();
	_allow = s.readByte();
	_pickup = s.readByte();
	_defaultCommand = s.readByte();
	// Unverified END

	// Probably those here?!?!
	_misc = s.readByte();
	_flags = s.readByte();

	// Unverified
	_aType = (AType)s.readByte();
	_lookFrames = s.readByte();
	_seqCounter = s.readByte();
	// Unverified END

	_lookPosition.y = s.readByte() * FIXED_INT_MULTIPLIER;
	_lookPosition._facing = s.readByte();

	// Unverified
	_lookcAnim = s.readByte();
	_seqStack = s.readByte();
	_seqTo = s.readByte();
	_seqCounter2 = s.readByte();
	// Unverified END

	s.skip(12); // Unknown

	//warning("object %s, offset %d", _name.c_str(), streamPos);
	//warning("object %s, lookPosX %d, lookPosY %d", _name.c_str(), _lookPosition.x, _lookPosition.y);
	//warning("object %s, defCmd %d", _name.c_str(), _defaultCommand);
	int32 dataSize = s.pos() - streamStartPos;
	assert(dataSize == 588);
}

void Object::toggleHidden() {
	if (_type != HIDDEN && _type != HIDE_SHAPE && _type != INVALID) {
		if (_seqTo != 0)
			_sequences[_frameNumber] = _seqTo + SEQ_TO_CODE + 128;
		_seqTo = 0;

		if (_images == nullptr || _images->size() == 0)
			// No shape to erase, so flag as hidden
			_type = HIDDEN;
		else
			// Otherwise, flag it to be hidden after it gets erased
			_type = HIDE_SHAPE;
	} else if (_type != INVALID) {
		if (_seqTo != 0)
			_sequences[_frameNumber] = _seqTo + SEQ_TO_CODE + 128;
		_seqTo = 0;

		_seqCounter = _seqCounter2 = 0;
		_seqStack = 0;
		_frameNumber = -1;

		if (_images == nullptr || _images->size() == 0) {
			_type = NO_SHAPE;
		} else {
			_type = ACTIVE_BG_SHAPE;
			int idx = _sequences[0];
			if (idx >= _maxFrames)
				// Turn on: set up first frame
				idx = 0;

			_imageFrame = &(*_images)[idx];
		}
	}
}

void Object::setObjTalkSequence(int seq) {
	Talk &talk = *_vm->_talk;

	// See if we're supposed to restore the object's sequence from the talk sequence stack
	if (seq == -1) {
		if (_seqTo != 0)
			_sequences[_frameNumber] = _seqTo;

		talk.pullSequence(_restoreSlot);
		return;
	}

	assert(_type != CHARACTER);

	talk.pushSequenceEntry(this);
	int talkSeqNum = seq;

	// Find where the talk sequence data begins in the object
	int idx = 0;
	for (;;) {
		// Get the Frame value
		byte f = _sequences[idx++];

		// See if we've found the beginning of a Talk Sequence
		if ((f == TALK_SEQ_CODE && seq < 128) || (f == TALK_LISTEN_CODE && seq > 128)) {
			--seq;

			// See if we're at the correct Talk Sequence Number
			if (!(seq & 127))
			{
				// Correct Sequence, Start Talking Here
				if (_seqTo != 0)
					_sequences[_frameNumber] = _seqTo;
				_frameNumber = idx;
				_seqTo = 0;
				_seqStack = 0;
				_seqCounter = 0;
				_seqCounter2 = 0;
				_talkSeq = talkSeqNum;
				break;
			}
		} else {
			// Move ahead any extra because of special control codes
			switch (f) {
			case 0: idx++; break;
			case MOVE_CODE:
			case TELEPORT_CODE: idx += 4; break;
			case CALL_TALK_CODE: idx += 8; break;
			case HIDE_CODE: idx += 2; break;
			default: break;
			}
		}

		// See if we're out of sequence data
		if (idx >= (int)_seqSize)
			break;
	}
}

void Object::setFlagsAndToggles() {
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	for (int useIdx = 0; useIdx < USE_COUNT; ++useIdx) {
		if (_use[useIdx]._useFlag) {
			if (!_vm->readFlags(_use[useIdx]._useFlag))
				_vm->setFlags(_use[useIdx]._useFlag);
		}

		if (_use[useIdx]._cAnimSpeed) {
			if (_use[useIdx]._cAnimNum == 0)
				// 0 is really a 10
				scene.startCAnim(9, _use[useIdx]._cAnimSpeed);
			else
				scene.startCAnim(_use[useIdx]._cAnimNum - 1, _use[useIdx]._cAnimSpeed);
		}

		if (!talk._talkToAbort) {
			for (int idx = 0; idx < NAMES_COUNT; ++idx)
				scene.toggleObject(_use[useIdx]._names[idx]);
		}
	}
}

void Object::adjustObject() {
	if (_type == REMOVE)
		return;

	if (IS_ROSE_TATTOO && (_delta.x || _delta.y)) {
		// The shape position is in pixels, and the delta is in fixed integer amounts
		int t;
		_noShapeSize.x += _delta.x;
		t = _noShapeSize.x / (FIXED_INT_MULTIPLIER / 10);
		_noShapeSize.x -= t * (FIXED_INT_MULTIPLIER / 10);
		_position.x += t;

		_noShapeSize.y += _delta.y;
		t = _noShapeSize.y / (FIXED_INT_MULTIPLIER / 10);
		_noShapeSize.y -= t * (FIXED_INT_MULTIPLIER / 10);
		_position.y += t;
	} else if (IS_SERRATED_SCALPEL) {
		// The delta is in whole pixels, so simply adjust the position with it
		_position += _delta;
	}

	if (_position.y > LOWER_LIMIT)
		_position.y = LOWER_LIMIT;

	if (_type != NO_SHAPE) {
		int frame = _frameNumber;
		if (frame == -1)
			frame = 0;

		int imgNum = _sequences[frame];
		if (imgNum > _maxFrames || imgNum == 0)
			imgNum = 1;

		_imageFrame = &(*_images)[imgNum - 1];
	}
}

int Object::pickUpObject(FixedTextActionId fixedTextActionId) {
	FixedText &fixedText = *_vm->_fixedText;
	Inventory &inv = *_vm->_inventory;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
	int pickup = _pickup & 0x7f;
	bool printed = false;
	int numObjects = 0;

	if (pickup == 99) {
		for (int idx = 0; idx < NAMES_COUNT && !talk._talkToAbort; ++idx) {
			if (checkNameForCodes(_use[0]._names[idx], kFixedTextAction_Invalid)) {
				if (!talk._talkToAbort)
					printed = true;
			}
		}

		return 0;
	}

	if (!pickup || (pickup > 50 && pickup <= 80)) {
		int message = _pickup;
		if (message > 50)
			message -= 50;

		ui._infoFlag = true;
		ui.clearInfo();
		Common::String errorMessage = fixedText.getActionMessage(fixedTextActionId, message);
		screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, "%s", errorMessage.c_str());
		ui._menuCounter = 30;
	} else {
		// Pick it up
		bool takeFlag = true;
		if ((_pickup & 0x80) == 0) {
			// Play an animation
			if (pickup > 80) {
				takeFlag = false;		// Don't pick it up
				scene.startCAnim(pickup - 81, 1);
				if (_pickupFlag)
					_vm->setFlags(_pickupFlag);
			} else {
				scene.startCAnim(pickup - 1, 1);
				if (!talk._talkToAbort) {
					// Erase the shape
					_type = _type == NO_SHAPE ? INVALID : REMOVE;
				}
			}

			if (talk._talkToAbort)
				return 0;
		} else {
			// Play generic pickup sequence
			// Original moved cursor position here
			people[HOLMES].goAllTheWay();
			ui._menuCounter = 25;
			ui._temp1 = 1;
		}

		for (int idx = 0; idx < NAMES_COUNT && !talk._talkToAbort; ++idx) {
			if (checkNameForCodes(_use[0]._names[idx], kFixedTextAction_Invalid)) {
				if (!talk._talkToAbort)
					printed = true;
			}
		}
		if (talk._talkToAbort)
			return 0;

		// Add the item to the player's inventory
		if (takeFlag)
			numObjects = inv.putItemInInventory(*this);

		if (!printed) {
			ui._infoFlag = true;
			ui.clearInfo();

			Common::String itemName = _description;

			// It's an item, make it lowercase
			switch (_vm->getLanguage()) {
			case Common::DE_DEU:
				// don't do this for German version
				break;
			default:
				// do it for English + Spanish version
				itemName.setChar(tolower(itemName[0]), 0);
				break;
			}

			screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, fixedText.getObjectPickedUpText(), itemName.c_str());
			ui._menuCounter = 25;
		}
	}

	return numObjects;
}

const Common::Rect Object::getNewBounds() const {
	Point32 pt = _position;
	if (_imageFrame)
		pt += _imageFrame->_offset;

	return Common::Rect(pt.x, pt.y, pt.x + frameWidth(), pt.y + frameHeight());
}

const Common::Rect Object::getNoShapeBounds() const {
	return Common::Rect(_position.x, _position.y,
		_position.x + _noShapeSize.x, _position.y + _noShapeSize.y);
}

const Common::Rect Object::getOldBounds() const {
	return Common::Rect(_oldPosition.x, _oldPosition.y,
		_oldPosition.x + _oldSize.x, _oldPosition.y + _oldSize.y);
}

/*----------------------------------------------------------------*/

void CAnim::load(Common::SeekableReadStream &s, bool isRoseTattoo, uint32 dataOffset) {
	char buffer[12];
	s.read(buffer, 12);
	_name = Common::String(buffer);

	if (isRoseTattoo) {
		Common::fill(&_sequences[0], &_sequences[30], 0);
		_dataSize = s.readUint32LE();
	} else {
		s.read(_sequences, 30);
	}

	_position.x = s.readSint16LE();
	_position.y = s.readSint16LE();

	if (isRoseTattoo) {
		_flags = s.readByte();
		_scaleVal = s.readSint16LE();
	} else {
		_dataSize = s.readUint32LE();
		_type = (SpriteType)s.readUint16LE();
		_flags = s.readByte();
	}

	_goto[0].x = s.readSint16LE();
	_goto[0].y = s.readSint16LE();
	_goto[0]._facing = s.readSint16LE();
	ADJUST_COORD(_goto[0]);

	if (isRoseTattoo) {
		// Get Goto position and facing for second NPC
		_goto[1].x = s.readSint16LE();
		_goto[1].y = s.readSint16LE();
		_goto[1]._facing = s.readSint16LE();
		ADJUST_COORD(_goto[1]);
	} else if (_goto[0].x != -1) {
		// For Serrated Scalpel, adjust the loaded co-ordinates
		_goto[0].x = _goto[0].x / 100;
		_goto[0].y = _goto[0].y / 100;
	}

	_teleport[0].x = s.readSint16LE();
	_teleport[0].y = s.readSint16LE();
	_teleport[0]._facing = s.readSint16LE();
	ADJUST_COORD(_teleport[0]);

	if (isRoseTattoo) {
		// Get Teleport position and facing for second NPC
		_teleport[1].x = s.readSint16LE();
		_teleport[1].y = s.readSint16LE();
		_teleport[1]._facing = s.readSint16LE();
		ADJUST_COORD(_teleport[1]);
	} else if (_teleport[0].x != -1) {
		// For Serrated Scalpel, adjust the loaded co-ordinates
		_teleport[0].x = _teleport[0].x / 100;
		_teleport[0].y = _teleport[0].y / 100;
	}

	// Save offset of data, which is actually inside another table inside the room data file
	// This table is at offset 44 for Serrated Scalpel
	// TODO: find it for the other game
	_dataOffset = dataOffset;
}

void CAnim::load3DO(Common::SeekableReadStream &s, uint32 dataOffset) {
	// this got reordered on 3DO
	// maybe it was the 3DO compiler

	_dataSize = s.readUint32BE();
	// Save offset of data, which is inside another table inside the room data file
	_dataOffset = dataOffset;

	_position.x = s.readSint16BE();
	_position.y = s.readSint16BE();

	_type = (SpriteType)s.readUint16BE();

	_goto[0].x = s.readSint16BE();
	_goto[0].y = s.readSint16BE();
	_goto[0]._facing = s.readSint16BE();

	_teleport[0].x = s.readSint16BE();
	_teleport[0].y = s.readSint16BE();
	_teleport[0]._facing = s.readSint16BE();

	char buffer[12];
	s.read(buffer, 12);
	_name = Common::String(buffer);

	s.read(_sequences, 30);
	_flags = s.readByte();

	s.skip(3); // Filler

	_goto[0].x = _goto[0].x * FIXED_INT_MULTIPLIER / 100;
	_goto[0].y = _goto[0].y * FIXED_INT_MULTIPLIER / 100;
	_teleport[0].x = _teleport[0].x * FIXED_INT_MULTIPLIER / 100;
	_teleport[0].y = _teleport[0].y * FIXED_INT_MULTIPLIER / 100;
}

/*----------------------------------------------------------------*/

SceneImage::SceneImage() {
	_images = nullptr;
	_maxFrames = 0;
	_filesize = 0;
}

} // End of namespace Sherlock
