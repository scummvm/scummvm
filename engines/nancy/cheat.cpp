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
 */

#include "engines/nancy/cheat.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/state/scene.h"

#include "common/winexe.h"
#include "common/stream.h"
#include "common/translation.h"

#include "gui/widgets/tab.h"
#include "gui/widgets/edittext.h"

namespace Nancy {

CheatDialog::CheatDialog(NancyEngine *engine) :
        GUI::Dialog(20, 20, 600, 440),
        _engine(engine) {
    _backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
    Common::WinResources *res = Common::WinResources::createFromEXE("game.exe");
    Common::Array<Common::WinResourceID> dialogIDs = res->getIDList(Common::kWinDialog);
    State::SceneInfo scene = _engine->scene->getSceneInfo();
    Time playerTime = _engine->scene->_timers.playerTime;
    Time timerTime = _engine->scene->_timers.timerTime;
    bool timerIsActive = _engine->scene->_timers.timerIsActive;
    if (!timerIsActive) {
        timerTime = 0;
    }
    char buf[4];

    GUI::TabWidget *_tabs = new GUI::TabWidget(this, 0, 0, 600, 370);
    new GUI::ButtonWidget(this, 420, 410, 60, 20, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
    new GUI::ButtonWidget(this, 520, 410, 60, 20, _("Ok"), Common::U32String(), GUI::kOKCmd);

    _tabs->addTab(_("General"), _("Cheat.General"));
    
    new GUI::StaticTextWidget(_tabs, 30, 20, 150, 20, _("Scene Data"), Graphics::kTextAlignLeft);
    _restartScene = new GUI::CheckboxWidget(_tabs, 35, 50, 150, 20, _("Restart the Scene"), _(""));
    _scene = new GUI::EditTextWidget(_tabs, 35, 75, 45, 20, _(itoa(scene.sceneID, buf, 10)), _(""), kInputSceneNr, kInputSceneNr);
    new GUI::StaticTextWidget(_tabs, 85, 75, 150, 20, _("Scene Number"), Graphics::kTextAlignLeft);
    _frame = new GUI::EditTextWidget(_tabs, 35, 100, 45, 20, _(itoa(scene.frameID, buf, 10)), _(""), kInputFrameNr, kInputFrameNr);
    new GUI::StaticTextWidget(_tabs, 85, 100, 150, 20, _("Frame Number"), Graphics::kTextAlignLeft);
    _offset = new GUI::EditTextWidget(_tabs, 35, 125, 45, 20, _(itoa(scene.verticalOffset, buf, 10)), _(""), kInputScroll, kInputScroll);
    new GUI::StaticTextWidget(_tabs, 85, 125, 150, 20, _("Background Top (Y)"), Graphics::kTextAlignLeft);

    new GUI::StaticTextWidget(_tabs, 30, 160, 150, 20, _("Hints Remaining"), Graphics::kTextAlignLeft);
    new GUI::StaticTextWidget(_tabs, 35, 185, 45, 20, _("Easy"), Graphics::kTextAlignLeft);
    _hintsRemainingEasy = new GUI::EditTextWidget(_tabs, 35, 205, 45, 20, _(itoa(_engine->scene->_hintsRemaining[0], buf, 10)), _(""), kInputHintsEasy, kInputHintsEasy);
    new GUI::StaticTextWidget(_tabs, 85, 185, 45, 20, _("Medium"), Graphics::kTextAlignLeft);
    _hintsRemainingMedium = new GUI::EditTextWidget(_tabs, 85, 205, 45, 20, _(itoa(_engine->scene->_hintsRemaining[1], buf, 10)), _(""), kInputHintsMedium, kInputHintsMedium);
    new GUI::StaticTextWidget(_tabs, 135, 185, 45, 20, _("Hard"), Graphics::kTextAlignLeft);
    _hintsRemainingHard = new GUI::EditTextWidget(_tabs, 135, 205, 45, 20, _(itoa(_engine->scene->_hintsRemaining[2], buf, 10)), _(""), kInputHintsHard, kInputHintsHard);
    
    new GUI::StaticTextWidget(_tabs, 250, 20, 150, 20, _("Player Data"), Graphics::kTextAlignLeft);
    new GUI::StaticTextWidget(_tabs, 255, 50, 150, 20, _("Player Time:"), Graphics::kTextAlignLeft);
    _playerTimeDays = new GUI::EditTextWidget(_tabs, 255, 75, 35, 20, _(itoa(playerTime.getDays(), buf, 10)), _(""), kInputPlayerTime, kInputPlayerTime);
    new GUI::StaticTextWidget(_tabs, 295, 75, 40, 20, _("Days"), Graphics::kTextAlignLeft);
    _playerTimeHours =new GUI::EditTextWidget(_tabs, 335, 75, 35, 20, _(itoa(playerTime.getHours(), buf, 10)), _(""), kInputPlayerTime, kInputPlayerTime);
    new GUI::StaticTextWidget(_tabs, 375, 75, 40, 20, _("Hours"), Graphics::kTextAlignLeft);
    _playerTimeMinutes =new GUI::EditTextWidget(_tabs, 415, 75, 35, 20, _(itoa(playerTime.getMinutes(), buf, 10)), _(""), kInputPlayerTime, kInputPlayerTime);
    new GUI::StaticTextWidget(_tabs, 455, 75, 50, 20, _("Minutes"), Graphics::kTextAlignLeft);
    _difficulty = new GUI::EditTextWidget(_tabs, 255, 105, 35, 20, _(itoa(_engine->scene->_difficulty, buf, 10)), _(""), kInputDifficulty, kInputDifficulty);
    new GUI::StaticTextWidget(_tabs, 295, 105, 150, 20, _("Player Difficulty Level"), Graphics::kTextAlignLeft);

    new GUI::StaticTextWidget(_tabs, 250, 140, 150, 20, _("Player Data"), Graphics::kTextAlignLeft);
    _timerOn = new GUI::CheckboxWidget(_tabs, 255, 170, 150, 20, _("Timer On"), _(""));
    _timerOn->setState(timerIsActive);
    _timerHours = new GUI::EditTextWidget(_tabs, 255, 195, 35, 20, _(itoa(timerTime.getTotalHours(), buf, 10)), _(""), kInputTimer, kInputTimer);
    new GUI::StaticTextWidget(_tabs, 295, 195, 40, 20, _("Hours"), Graphics::kTextAlignLeft);
    _timerMinutes = new GUI::EditTextWidget(_tabs, 335, 195, 35, 20, _(itoa(timerTime.getMinutes(), buf, 10)), _(""), kInputTimer, kInputTimer);
    new GUI::StaticTextWidget(_tabs, 375, 195, 50, 20, _("Minutes"), Graphics::kTextAlignLeft);
    _timerSeconds = new GUI::EditTextWidget(_tabs, 425, 195, 35, 20, _(itoa(timerTime.getSeconds(), buf, 10)), _(""), kInputTimer, kInputTimer);
    new GUI::StaticTextWidget(_tabs, 465, 195, 50, 20, _("Seconds"), Graphics::kTextAlignLeft);

    _tabs->addTab(_("Inventory"), _("Cheat.Inventory"));

    for (uint i = 0; i < dialogIDs.size(); ++i) {
        Common::SeekableReadStream *resStream = res->getResource(Common::kWinDialog, dialogIDs[i].getID());
        
        Common::String idString;
        resStream->skip(0x16);
        while (true) {
            char add = resStream->readByte();
            if (add != 0) {
                idString += add;
                resStream->skip(1);
            } else {
                resStream->skip(1);
                break;
            }
        }

        if (!idString.hasPrefix("Inventory")) {
            continue;
        }

        idString.trim();
        uint numItems = 0;
        
        while (resStream->pos() < resStream->size()) {
            if (resStream->readUint16LE() == 0xFFFF && resStream->readSint16LE() == 0x80) {
                // Found a resource, read its string id
                Common::String itemLabel;

                while (true) {
                    char add = resStream->readByte();
                    if (add != 0) {
                        itemLabel += add;
                        resStream->skip(1);
                    } else {
                        resStream->skip(1);
                        break;
                    }
                }
                GUI::CheckboxWidget *box = new GUI::CheckboxWidget(_tabs, 250 * (numItems / 10) + 20, (350 / 10) * (numItems % 10) + 15, 250, 250/10, _(itemLabel), Common::U32String());
                box->setState(_engine->scene->hasItem(numItems) == kTrue);
                _inventory.push_back(box);

                ++numItems;
            }
        }

        break;
    }

    _tabs->setActiveTab(0);
}

void CheatDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
    switch (cmd) {
    case GUI::kOKCmd: {
        if (_restartScene->getState()) {
            uint sceneID = atoi(Common::String(_scene->getEditString()).c_str());
            IFF iff(_engine, Common::String::format("S%u", sceneID));
            if (iff.load()) {
                _engine->scene->changeScene(
                    atoi(Common::String(_scene->getEditString()).c_str()),
                    atoi(Common::String(_frame->getEditString()).c_str()),
                    atoi(Common::String(_offset->getEditString()).c_str()),
                    true);
            } else {
                new GUI::StaticTextWidget(this, 20, 410, 150, 20, _("Invalid Scene ID!"), Graphics::kTextAlignLeft);
                return;
            }
        }

        if (_timerOn->getState()) {
            _engine->scene->_timers.timerIsActive = true;
            Time &timer = _engine->scene->_timers.timerTime;
            timer = 0;
            timer += 1000 * atoi(Common::String(_timerSeconds->getEditString()).c_str());
            timer += 60000 * atoi(Common::String(_timerMinutes->getEditString()).c_str());
            timer += 3600000 * atoi(Common::String(_timerHours->getEditString()).c_str());
        } else {
            _engine->scene->stopTimer();
        }

        Time &playerTime = _engine->scene->_timers.timerTime;
        playerTime = 0;
        playerTime += 60000 * atoi(Common::String(_playerTimeMinutes->getEditString()).c_str());
        playerTime += 3600000 * atoi(Common::String(_playerTimeHours->getEditString()).c_str());
        playerTime += 86400000 * atoi(Common::String(_playerTimeMinutes->getEditString()).c_str());

        _engine->scene->_difficulty = atoi(Common::String(_difficulty->getEditString()).c_str());
        _engine->scene->_hintsRemaining[0] = atoi(Common::String(_hintsRemainingEasy->getEditString()).c_str());
        _engine->scene->_hintsRemaining[1] = atoi(Common::String(_hintsRemainingMedium->getEditString()).c_str());
        _engine->scene->_hintsRemaining[2] = atoi(Common::String(_hintsRemainingHard->getEditString()).c_str());

        for (uint i = 0; i < _inventory.size(); ++i) {
            if (_engine->scene->hasItem(i) == kTrue && !_inventory[i]->getState()) {
                _engine->scene->removeItemFromInventory(i, false);
            } else if (_engine->scene->hasItem(i) == kFalse && _inventory[i]->getState()) {
                _engine->scene->addItemToInventory(i);
            }
        }
        cmd = GUI::kCloseCmd;

        break;
    }
    case kInputSceneNr:
        sanitizeInput(_scene);
        break;
    case kInputFrameNr: 
        sanitizeInput(_frame);
        break;
    case kInputScroll:
        sanitizeInput(_offset);
        break;
    case kInputDifficulty:
        sanitizeInput(_scene, 2);
        break;
    case kInputHintsEasy:
        sanitizeInput(_hintsRemainingEasy);
        break;
    case kInputHintsMedium:
        sanitizeInput(_hintsRemainingMedium);
        break;
    case kInputHintsHard:
        sanitizeInput(_hintsRemainingHard);
        break;
    case kInputPlayerTime:
        sanitizeInput(_playerTimeMinutes, 59);
        sanitizeInput(_playerTimeHours, 23);
        sanitizeInput(_playerTimeDays);
        break;
    case kInputTimer:
        sanitizeInput(_timerSeconds, 59);
        sanitizeInput(_timerMinutes, 59);
        sanitizeInput(_timerHours, 23);
        break;
    default:
        break;
    }
    
    Dialog::handleCommand(sender, cmd, data);
}

void CheatDialog::sanitizeInput(GUI::EditTextWidget *textWidget, int maxValue) {
    const Common::U32String &str = textWidget->getEditString();
    for (uint i = 0; i < str.size(); ++i) {
        if (!Common::isDigit(str[i])) {
            textWidget->setEditString(str.substr(0, i));
            break;
        }
    }

    if (maxValue > -1) {
        int number = atoi(Common::String(str).c_str());
        if (number > maxValue) {
            char *buf = new char[str.size() + 1];
            textWidget->setEditString(_(itoa(maxValue, buf, 10)));
            delete[] buf;
        }
    }

    textWidget->setCaretPos(str.size());
}

EventFlagDialog::EventFlagDialog(NancyEngine *engine) :
        GUI::Dialog(20, 20, 600, 440),
        _engine(engine) {
    _backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
    Common::WinResources *res = Common::WinResources::createFromEXE("game.exe");
    Common::Array<Common::WinResourceID> dialogIDs = res->getIDList(Common::kWinDialog);

    GUI::TabWidget *_tabs = new GUI::TabWidget(this, 0, 0, 600, 370);
    new GUI::ButtonWidget(this, 520, 410, 60, 20, _("Close"), Common::U32String(), GUI::kCloseCmd);

    for (uint i = 0; i < dialogIDs.size(); ++i) {
        Common::SeekableReadStream *resStream = res->getResource(Common::kWinDialog, dialogIDs[i].getID());
        
        Common::String idString;
        resStream->skip(0x16);
        while (true) {
            char add = resStream->readByte();
            if (add != 0) {
                idString += add;
                resStream->skip(1);
            } else {
                resStream->skip(1);
                break;
            }
        }

        if (!idString.hasPrefix("Event")) {
            continue;
        }
        
        idString.trim();
        _tabs->addTab(idString, Common::String("tab " + idString));
        uint numFlags = 0;

        while (resStream->pos() < resStream->size()) {
            if (resStream->readUint16LE() == 0xFFFF && resStream->readSint16LE() == 0x80) {
                // Found a resource, read its string id
                Common::String flagLabel;

                while (true) {
                    char add = resStream->readByte();
                    if (add != 0) {
                        flagLabel += add;
                        resStream->skip(1);
                    } else {
                        resStream->skip(1);
                        break;
                    }
                }

                // String begins with number so we assume it's an event flag radio button
                if (Common::isDigit(flagLabel.firstChar())) {
                    Common::String num;
                    uint j = 0;
                    while (true) {
                        if (Common::isDigit(flagLabel[j])) {
                            num += flagLabel[j];
                        } else {
                            break;
                        }
                        ++j;
                    }

                    uint32 command = atoi(num.c_str()) << 16 | 'ev';

                    GUI::CheckboxWidget *box = new GUI::CheckboxWidget(_tabs, 300 * (numFlags / 12) + 20, (350 / 12) * (numFlags % 12) + 15, 300, 350/12, _(flagLabel), Common::U32String(), command);
                    box->setState(_engine->scene->getEventFlag(command >> 16));

                    ++numFlags;
                }
            }
        }
    }

    _tabs->setActiveTab(0);
}

void EventFlagDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
    Dialog::handleCommand(sender, cmd, data);
    if (cmd & 'ev') {
        cmd >>= 16;
        _engine->scene->setEventFlag(cmd, data == 0 ? kFalse : kTrue);
    }
}

} // End of namespace Nancy
