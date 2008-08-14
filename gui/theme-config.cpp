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
 * $URL$
 * $Id$
 */

#include "gui/theme.h"
#include "gui/eval.h"

namespace GUI {

const char *Theme::_defaultConfigINI =
"# Define our classic greenish theme here\n"
"[320xY]\n"
"def_widgetSize=kNormalWidgetSize\n"
"def_buttonWidth=kButtonWidth\n"
"def_buttonHeight=kButtonHeight\n"
"def_sliderWidth=kSliderWidth\n"
"def_sliderHeight=kSliderHeight\n"
"def_kLineHeight=12\n"
"def_kFontHeight=10\n"
"def_insetX=10\n"
"def_insetY=10\n"
"def_insetW=(w - 2 * 10)\n"
"def_insetH=(h - 30)\n"
"def_gameOptionsLabelWidth=60\n"
"def_tabPopupsLabelW=100\n"
"def_aboutXOff=3\n"
"def_aboutYOff=2\n"
"def_aboutOuterBorder=10\n"
"def_scummmainHOffset=8\n"
"def_scummmainVSpace=5\n"
"def_scummmainVAddOff=2\n"
"def_scummmainButtonWidth=90\n"
"def_scummmainButtonHeight=16\n"
"def_scummhelpX=5\n"
"def_scummhelpW=(w - 2 * 5)\n"
"def_midiControlsSpacing=1\n"
"def_vcAudioTabIndent=0\n"
"def_vcAudioTabSpacing=2\n"
"use=XxY\n"
"\n"
"# Override audio tab\n"
"set_parent=gameoptions\n"
"vBorder=5\n"
"\n"
"# audio tab\n"
"opYoffset=vBorder\n"
"useWithPrefix=audioControls globaloptions_\n"
"useWithPrefix=subtitleControls globaloptions_\n"
"\n"
"# volume tab\n"
"opYoffset=vBorder\n"
"useWithPrefix=volumeControls globaloptions_\n"
"\n"
"# audio tab\n"
"opYoffset=vBorder\n"
"gameoptions_audioCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + 6)\n"
"useWithPrefix=audioControls gameoptions_\n"
"useWithPrefix=subtitleControls gameoptions_\n"
"\n"
"# volume tab\n"
"opYoffset=vBorder\n"
"gameoptions_volumeCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + 6)\n"
"useWithPrefix=volumeControls gameoptions_\n"
"\n"
"TabWidget.tabWidth=0\n"
"TabWidget.tabHeight=16\n"
"TabWidget.titleVPad=2\n"
"# Scumm Saveload dialog\n"
"scummsaveload=8 8 (w - 2 * 8) (h - 16)\n"
"set_parent=scummsaveload\n"
"scummsaveload_title=10 2 (parent.w - 2 * 10) kLineHeight\n"
"scummsaveload_list=10 18 prev.w (parent.h - 17 - buttonHeight - 8 - self.y)\n"
"scummsaveload_thumbnail=(parent.w - (kThumbnailWidth + 22)) 18\n"
"scummsaveload_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"scummsaveload_choose=(prev.x2 + 10) prev.y prev.w prev.h\n"
"scummsaveload_delete=prev.x2 prev.y prev.w prev.h\n"
"scummsaveload_extinfo.visible=false\n"
"\n"
"# MM NES resolution\n"
"[256x240]\n"
"useAsIs=320xY\n"
"\n"
"[XxY]\n"
"def_widgetSize=kBigWidgetSize\n"
"def_buttonWidth=kBigButtonWidth\n"
"def_buttonHeight=kBigButtonHeight\n"
"def_sliderWidth=kBigSliderWidth\n"
"def_sliderHeight=kBigSliderHeight\n"
"def_kLineHeight=16\n"
"def_kFontHeight=14\n"
"def_insetX=10\n"
"def_insetY=20\n"
"def_insetW=(w - 2 * 10)\n"
"def_insetH=(h - 2 * 40)\n"
"def_gameOptionsLabelWidth=90\n"
"def_tabPopupsLabelW=150\n"
"def_aboutXOff=8\n"
"def_aboutYOff=5\n"
"def_aboutOuterBorder=80\n"
"def_scummmainHOffset=12\n"
"def_scummmainVSpace=7\n"
"def_scummmainVAddOff=3\n"
"def_scummmainButtonWidth=160\n"
"def_scummmainButtonHeight=28\n"
"def_scummhelpW=370\n"
"def_scummhelpX=((w - scummhelpW) / 2)\n"
"def_midiControlsSpacing=2\n"
"def_vcAudioTabIndent=10\n"
"def_vcAudioTabSpacing=4\n"
"##### Widgets config\n"
"ListWidget.leftPadding=4\n"
"ListWidget.rightPadding=0\n"
"ListWidget.topPadding=2\n"
"ListWidget.bottomPadding=2\n"
"ListWidget.hlLeftPadding=2\n"
"ListWidget.hlRightPadding=1\n"
"PopUpWidget.leftPadding=4\n"
"PopUpWidget.rightPadding=0\n"
"TabWidget.tabWidth=70\n"
"TabWidget.tabHeight=21\n"
"TabWidget.titleVPad=2\n"
"\n"
"###### chooser\n"
"opHeight=(h * 7 / 10)\n"
"useWithPrefix=chooser defaultChooser_\n"
"\n"
"##### browser\n"
"brW=((w * 7) / 8)\n"
"brH=((h * 9) / 10)\n"
"browser=((w - brW) / 2) ((h - brH) / 2) brW brH\n"
"set_parent=browser\n"
"browser_headline=10 kLineHeight (parent.w - 2 * 10) kLineHeight\n"
"browser_headline.align=kTextAlignCenter\n"
"browser_path=10 prev.y2 prev.w prev.h\n"
"browser_list=10 prev.y2 prev.w (parent.h - 3 * kLineHeight - buttonHeight - 14)\n"
"browser_up=10 (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"browser_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"browser_choose=(prev.x2 + 10) prev.y prev.w prev.h\n"
"\n"
"##### launcher\n"
"hBorder=10\n"
"launcher_version=hBorder 8 (w - 2 * hBorder) kLineHeight\n"
"launcher_version.align=kTextAlignCenter\n"
"top=(h - 8 - buttonHeight)\n"
"numButtons=4\n"
"space=8\n"
"butWidth=((w - 2 * hBorder - space * (numButtons - 1)) / numButtons)\n"
"launcher_quit_button=hBorder top butWidth buttonHeight\n"
"launcher_about_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"launcher_options_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"launcher_start_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"top=(top - buttonHeight * 2)\n"
"numButtons=4\n"
"space=10\n"
"butWidth=((w - 2 * hBorder - space * (numButtons - 1)) / numButtons)\n"
"launcher_loadGame_button=hBorder top butWidth buttonHeight\n"
"launcher_addGame_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"launcher_editGame_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"launcher_removeGame_button=(prev.x2 + space) prev.y prev.w prev.h\n"
"launcher_list=hBorder (kLineHeight + 16) (w - 2 * hBorder) (top - kLineHeight - 20)\n"
"\n"
"### global options\n"
"globaloptions=insetX insetY insetW insetH\n"
"set_parent=globaloptions\n"
"vBorder=5\n"
"globaloptions_tabwidget=0 vBorder parent.w (parent.h - buttonHeight - 8 - 2 * vBorder)\n"
"\n"
"# graphics tab\n"
"opYoffset=vBorder\n"
"opXoffset=0\n"
"useWithPrefix=graphicsControls globaloptions_\n"
"\n"
"# audio tab\n"
"opYoffset=vBorder\n"
"useWithPrefix=audioControls globaloptions_\n"
"useWithPrefix=subtitleControls globaloptions_\n"
"\n"
"# volume tab\n"
"opYoffset=vBorder\n"
"useWithPrefix=volumeControls globaloptions_\n"
"\n"
"# MIDI tab\n"
"opYoffset=vBorder\n"
"useWithPrefix=midiControls globaloptions_\n"
"\n"
"# paths tab\n"
"yoffset=vBorder\n"
"glOff=((buttonHeight - kLineHeight) / 2 + 2)\n"
"globaloptions_savebutton=10 yoffset (buttonWidth + 5) buttonHeight\n"
"globaloptions_savepath=(prev.x2 + 20) (yoffset + glOff) (parent.w - (prev.w + 20) - 15) kLineHeight\n"
"yoffset=(yoffset + buttonHeight + 4)\n"
"globaloptions_extrabutton=10 yoffset (buttonWidth + 5) buttonHeight\n"
"globaloptions_extrapath=(prev.x2 + 20) (yoffset + glOff) (parent.w - (prev.w + 20) - 15) kLineHeight\n"
"yoffset=(yoffset + buttonHeight + 4)\n"
"globaloptions_themebutton=10 yoffset (buttonWidth + 5) buttonHeight\n"
"globaloptions_themepath=(prev.x2 + 20) (yoffset + glOff) (parent.w - (prev.w + 20) - 15) kLineHeight\n"
"yoffset=(yoffset + buttonHeight + 4)\n"
"globaloptions_pluginsbutton=10 yoffset (buttonWidth + 5) buttonHeight\n"
"globaloptions_pluginspath=(prev.x2 + 20) (yoffset + glOff) (parent.w - (prev.w + 20) - 15) kLineHeight\n"
"yoffset=(yoffset + buttonHeight + 4)\n"
"globaloptions_keysbutton=10 yoffset (buttonWidth + 5) buttonHeight\n"
"\n"
"# Misc options\n"
"yoffset=vBorder\n"
"glOff=((buttonHeight - kLineHeight) / 2 + 2)\n"
"globaloptions_themebutton2=10 yoffset buttonWidth buttonHeight\n"
"globaloptions_curtheme=(prev.x2 + 20) (yoffset + glOff) (parent.w - (prev.w + 20) - 10) kLineHeight\n"
"yoffset=(yoffset + buttonHeight + 4)\n"
"globaloptions_autosaveperiod=10 yoffset (parent.w - 10 - 25) (kLineHeight + 2)\n"
"\n"
"globaloptions_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"globaloptions_ok=(prev.x2 + 10) prev.y prev.w prev.h\n"
"\n"
"### game options\n"
"gameoptions=insetX insetY insetW insetH\n"
"set_parent=gameoptions\n"
"vBorder=5\n"
"gox=5\n"
"gow=(parent.w - 15)\n"
"\n"
"gameoptions_tabwidget=0 vBorder parent.w (parent.h - buttonHeight - 8 - 2 * vBorder)\n"
"\n"
"# game tab\n"
"opYoffset=vBorder\n"
"gameoptions_id=gox (opYoffset + 2) gameOptionsLabelWidth kLineHeight\n"
"gameoptions_id.align=kTextAlignRight\n"
"gameoptions_domain=prev.x2 (prev.y - 1) (parent.w - gameOptionsLabelWidth - 10 - gox) (prev.h + 2)\n"
"opYoffset=(opYoffset + prev.h + 5)\n"
"gameoptions_name=gox (opYoffset + 2) gameOptionsLabelWidth kLineHeight\n"
"gameoptions_name.align=kTextAlignRight\n"
"gameoptions_desc=prev.x2 (prev.y - 1) (parent.w - gameOptionsLabelWidth - 10 - gox) (prev.h + 2)\n"
"opYoffset=(opYoffset + prev.h + 7)\n"
"gameoptions_lang=gox (opYoffset - 1) gow (kLineHeight + 2)\n"
"opYoffset=(opYoffset + prev.h + 5)\n"
"gameoptions_platform=prev.x opYoffset prev.w prev.h\n"
"opYoffset=(opYoffset + prev.h + 5)\n"
"\n"
"# paths tab\n"
"opYoffset=vBorder\n"
"goOff=((buttonHeight - kLineHeight) / 2 + 2)\n"
"gameoptions_savepath=gox opYoffset (buttonWidth + 5) buttonHeight\n"
"gameoptions_savepathText=(prev.x2 + 20) (opYoffset + goOff) (parent.w - self.x - 10) kLineHeight\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"gameoptions_extrapath=gox opYoffset (buttonWidth + 5) buttonHeight\n"
"gameoptions_extrapathText=(prev.x2 + 20) (opYoffset + goOff) (parent.w - self.x - 10) kLineHeight\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"gameoptions_gamepath=gox opYoffset (buttonWidth + 5) buttonHeight\n"
"gameoptions_gamepathText=(prev.x2 + 20) (opYoffset + goOff) (parent.w - self.x - 10) kLineHeight\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"\n"
"# graphics tab\n"
"opYoffset=vBorder\n"
"opXoffset=gox\n"
"gameoptions_graphicsCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"useWithPrefix=graphicsControls gameoptions_\n"
"\n"
"# audio tab\n"
"opYoffset=vBorder\n"
"gameoptions_audioCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"useWithPrefix=audioControls gameoptions_\n"
"useWithPrefix=subtitleControls gameoptions_\n"
"\n"
"# volume tab\n"
"opYoffset=vBorder\n"
"gameoptions_volumeCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"useWithPrefix=volumeControls gameoptions_\n"
"\n"
"# midi tab\n"
"opYoffset=vBorder\n"
"gameoptions_midiCheckbox=gox opYoffset (parent.w - gox - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"useWithPrefix=midiControls gameoptions_\n"
"\n"
"gameoptions_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"gameoptions_ok=(prev.x2 + 10) prev.y prev.w prev.h\n"
"\n"
"### keys dialog\n"
"keysdialog=(w / 20) (h / 10) (w - w / 10) (h - h / 4)\n"
"set_parent=keysdialog\n"
"keysdialog_map=(parent.w - buttonWidth - 10) (10 + 2 * kLineHeight) buttonWidth buttonHeight\n"
"keysdialog_ok=prev.x (prev.y2 + 4) prev.w prev.h\n"
"keysdialog_cancel=prev.x (prev.y2 + 4) prev.w prev.h\n"
"keysdialog_action=10 10 (parent.w - 20) kLineHeight\n"
"keysdialog_action.align=kTextAlignCenter\n"
"keysdialog_list=prev.x (prev.y + 2 * kLineHeight) (parent.w - buttonWidth - 30) (parent.h - kLineHeight * 6)\n"
"keysdialog_mapping=prev.x (prev.y + prev.h + kLineHeight) (parent.w - buttonWidth - 20) kLineHeight\n"
"\n"
"### mass add dialog\n"
"massadddialog=10 20 300 174\n"
"set_parent=massadddialog\n"
"massadddialog_caption=10 (10 + 1 * kLineHeight) (parent.w - 2*10) kLineHeight\n"
"massadddialog_caption.align=kTextAlignCenter\n"
"massadddialog_dirprogress=10 (10 + 3 * kLineHeight) (parent.w - 2*10) kLineHeight\n"
"massadddialog_dirprogress.align=kTextAlignCenter\n"
"massadddialog_gameprogress=10 (10 + 4 * kLineHeight) (parent.w - 2*10) kLineHeight\n"
"massadddialog_gameprogress.align=kTextAlignCenter\n"
"massadddialog_ok=((parent.w - (buttonWidth * 2)) / 2) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"massadddialog_cancel=(prev.x2 + 10) prev.y prev.w prev.h\n"
"\n"
"\n"
"##### SCUMM dialogs\n"
"scummDummyDialog=0 80 0 16\n"
"\n"
"use=scummmain\n"
"## Engine config\n"
"# note that scummconfig size depends on overall height\n"
"# hence it is on the end of the list\n"
"opYoffset=8\n"
"useWithPrefix=volumeControls scummconfig_\n"
"useWithPrefix=subtitleControls scummconfig_\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"soWidth=(39 + 3 * buttonWidth)\n"
"scummconfig_keys=(soWidth - 3 * (buttonWidth + 4) + 6) opYoffset (buttonWidth - 10) buttonHeight\n"
"scummconfig_cancel=(prev.x2 + 4) prev.y (prev.w + 10) prev.h\n"
"scummconfig_ok=(prev.x2 + 4) prev.y prev.w prev.h\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"scummconfig=((w - soWidth) / 2) ((h - opYoffset) / 2) soWidth (opYoffset + 8)\n"
"\n"
"## Help\n"
"scummHelpNumLines=15\n"
"shH=(5 + (2 + scummHelpNumLines) * kFontHeight + buttonHeight + 7)\n"
"scummhelp=scummhelpX ((h - shH) / 2) scummhelpW shH\n"
"scummhelp_title=10 5 scummhelpW kFontHeight\n"
"scummhelp_key.x=10\n"
"scummhelp_key.yoffset=5\n"
"scummhelp_key.w=80\n"
"scummhelp_key.h=kFontHeight\n"
"scummhelp_dsc.x=90\n"
"scummhelp_dsc.yoffset=5\n"
"scummhelp_dsc.w=(scummhelpW - 10 - 90)\n"
"scummhelp_dsc.h=kFontHeight\n"
"scummhelp_prev=10 (5 + kFontHeight * (scummHelpNumLines + 2) + 2) buttonWidth buttonHeight\n"
"scummhelp_next=(prev.x2 + 8) prev.y prev.w prev.h\n"
"scummhelp_close=(scummhelpW - 8 - buttonWidth) prev.y prev.w prev.h\n"
"\n"
"# Saveload dialog\n"
"scummsaveload=8 8 (w - 2 * 8) (h - 16)\n"
"set_parent=scummsaveload\n"
"scummsaveload_title=10 2 (parent.w - 2 * 10 - 180) kLineHeight\n"
"scummsaveload_title.align=kTextAlignCenter\n"
"scummsaveload_list=10 18 prev.w (parent.h - 17 - buttonHeight - 8 - self.y)\n"
"scummsaveload_thumbnail=(parent.w - (kThumbnailWidth + 18)) 22\n"
"scummsaveload_thumbnail.hPad=4\n"
"scummsaveload_thumbnail.vPad=4\n"
"scummsaveload_thumbnail.fillR=0\n"
"scummsaveload_thumbnail.fillG=0\n"
"scummsaveload_thumbnail.fillB=0\n"
"scummsaveload_cancel=(parent.w - 2 * (buttonWidth + 10)) (parent.h - buttonHeight - 8) buttonWidth buttonHeight\n"
"scummsaveload_choose=(prev.x2 + 10) prev.y prev.w prev.h\n"
"scummsaveload_delete=prev.x (prev.y - 30) prev.w prev.h\n"
"scummsaveload_extinfo.visible=true\n"
"\n"
"############################################\n"
"[chooser]\n"
"chooserW=(w - 2 * 8)\n"
"chooser=((w - chooserW) / 2) ((h - opHeight) / 2) chooserW opHeight\n"
"chooser_headline=10 6 (chooserW - 2 * 10) (kLineHeight)\n"
"chooser_headline.align=kTextAlignCenter\n"
"chooser_list=10 (6 + kLineHeight + 2) prev.w (opHeight - self.y - buttonHeight - 12)\n"
"chooser_cancel=(chooserW - 2 * (buttonWidth + 10)) (opHeight - buttonHeight - 8) buttonWidth buttonHeight\n"
"chooser_ok=(prev.x2 + 10) prev.y prev.w prev.h\n"
"\n"
"[graphicsControls]\n"
"gcx=10\n"
"gcw=(parent.w - 2 * 10)\n"
"grModePopup=(gcx - 5) (opYoffset - 1) (gcw + 5) (kLineHeight + 2)\n"
"opYoffset=(opYoffset + kLineHeight + 4)\n"
"grRenderPopup=prev.x (opYoffset - 1) prev.w prev.h\n"
"opYoffset=(opYoffset + kLineHeight + 4)\n"
"grFullscreenCheckbox=gcx opYoffset (parent.w - gcx - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"grAspectCheckbox=prev.x opYoffset prev.w prev.h\n"
"opYoffset=(opYoffset + buttonHeight)\n"
"\n"
"[audioControls]\n"
"aux=10\n"
"auw=(parent.w - 2 * 10)\n"
"auMidiPopup=(aux - 5) (opYoffset - 1) (auw + 5) (kLineHeight + 2)\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"auSampleRatePopup=(aux - 5) (opYoffset - 1) (auw + 5) (kLineHeight + 2)\n"
"opYoffset=(opYoffset + buttonHeight + 4)\n"
"\n"
"[volumeControls]\n"
"vctextw=(95 + vcAudioTabIndent)\n"
"vcxoff=(opXoffset + vctextw + 15)\n"
"vcx=(opXoffset + 10)\n"
"vcMusicText=vcx (opYoffset + 2) vctextw kLineHeight\n"
"vcMusicText.align=kTextAlignRight\n"
"vcMusicSlider=vcxoff opYoffset sliderWidth sliderHeight\n"
"vcMusicLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n"
"opYoffset=(opYoffset + sliderHeight + vcAudioTabSpacing)\n"
"vcSfxText=vcx (opYoffset + 2) vctextw kLineHeight\n"
"vcSfxText.align=kTextAlignRight\n"
"vcSfxSlider=vcxoff opYoffset sliderWidth sliderHeight\n"
"vcSfxLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n"
"opYoffset=(opYoffset + sliderHeight + vcAudioTabSpacing)\n"
"vcSpeechText=vcx (opYoffset + 2) vctextw kLineHeight\n"
"vcSpeechText.align=kTextAlignRight\n"
"vcSpeechSlider=vcxoff opYoffset sliderWidth sliderHeight\n"
"vcSpeechLabel=(vcxoff + prev.w + 10) (opYoffset + 2) 24 kLineHeight\n"
"opYoffset=(opYoffset + sliderHeight + vcAudioTabSpacing)\n"
"\n"
"[midiControls]\n"
"mcx=10\n"
"mcFontButton=mcx opYoffset buttonWidth buttonHeight\n"
"mcFontPath=(prev.x2 + 20) (opYoffset + 3) (parent.w - (buttonWidth + 20) - 15 - kLineHeight - 10) kLineHeight\n"
"mcFontClearButton=(prev.x2 + 10) (opYoffset + 2) kLineHeight kLineHeight\n"
"opYoffset=(opYoffset + buttonHeight + 2 * midiControlsSpacing)\n"
"mcMixedCheckbox=mcx opYoffset (parent.w - mcx - 5) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n"
"mcMt32Checkbox=mcx opYoffset prev.w buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n"
"mcGSCheckbox=mcx opYoffset prev.w buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + midiControlsSpacing)\n"
"mcMidiGainText=mcx (opYoffset + 2) 95 kLineHeight\n"
"mcMidiGainText.align=kTextAlignRight\n"
"mcMidiGainSlider=(prev.x2 + 10) opYoffset sliderWidth sliderHeight\n"
"mcMidiGainLabel=(prev.x2 + 10) (opYoffset + 2) 40 kLineHeight\n"
"opYoffset=(opYoffset + sliderHeight + midiControlsSpacing)\n"
"\n"
"[subtitleControls]\n"
"sbx=(opXoffset + 10)\n"
"sbYoff=(buttonHeight / 8)\n"
"sbOff=((sliderHeight - kLineHeight) / 2 + 2)\n"
"sbtextw=(100 + vcAudioTabIndent)\n"
"opYoffset=(opYoffset + sbYoff)\n"
"subToggleDesc=sbx (opYoffset + sbYoff) sbtextw buttonHeight\n"
"subToggleDesc.align=kTextAlignRight\n"
"subToggleButton=prev.x2 (opYoffset - sbYoff) (buttonWidth + 54) buttonHeight\n"
"opYoffset=(opYoffset + buttonHeight + 6)\n"
"subSubtitleSpeedDesc=sbx (opYoffset + sbOff) sbtextw kLineHeight\n"
"subSubtitleSpeedDesc.align=kTextAlignRight\n"
"subSubtitleSpeedSlider=prev.x2 opYoffset sliderWidth sliderHeight\n"
"subSubtitleSpeedLabel=(prev.x2 + 10) (opYoffset + sbOff) 24 kLineHeight\n"
"opYoffset=(opYoffset + sliderHeight + 8)\n"
"\n"
"[scummmain]\n"
"## Main dialog\n"
"# note that scummmain size depends on overall height\n"
"smY=(scummmainVSpace + scummmainVAddOff)\n"
"scummmain_resume=scummmainHOffset smY scummmainButtonWidth scummmainButtonHeight\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"smY=(smY + scummmainVSpace)\n"
"scummmain_load=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"scummmain_save=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"smY=(smY + scummmainVSpace)\n"
"scummmain_options=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"scummmain_about=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"scummmain_help=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"smY=(smY + scummmainVSpace)\n"
"scummmain_quit=prev.x smY prev.w prev.h\n"
"smY=(smY + scummmainButtonHeight + scummmainVAddOff)\n"
"smW=(scummmainButtonWidth + 2 * scummmainHOffset)\n"
"smH=(smY + scummmainVSpace)\n"
"scummmain=((w - smW) / 2) ((h - smH) / 2) smW smH\n"
"\n"

"#### Global Main Menu Dialog"
"[globalmain]\n"
"# note that globalmain size depends on overall height\n"
"gmY=(scummmainVSpace + scummmainVAddOff)\n"
"globalmain_resume=scummmainHOffset gmY scummmainButtonWidth scummmainButtonHeight\n"
"gmY=(gmY + scummmainButtonHeight + scummmainVAddOff)\n"
"gmY=(gmY + scummmainVSpace)\n"
"globalmain_options=prev.x gmY prev.w prev.h\n"
"gmY=(gmY + scummmainButtonHeight + scummmainVAddOff)\n"
"globalmain_about=prev.x gmY prev.w prev.h\n"
"gmY=(gmY + scummmainButtonHeight + scummmainVAddOff)\n"
"gmY=(gmY + scummmainVSpace)\n"
"globalmain_rtl=prev.x gmY prev.w prev.h\n"
"gmY=(gmY + scummmainButtonHeight + scummmainVAddOff)\n"
"gmW=(scummmainButtonWidth + 2 * scummmainHOffset)\n"
"globalmain_quit=prev.x gmY prev.w prev.h\n"
"gmY=(gmY + scummmainButtonHeight + scummmainVAddOff)\n"
"gmW=(scummmainButtonWidth + 2 * scummmainHOffset)\n"
"gmH=(gmY + scummmainVSpace)\n"
"globalmain=((w - gmW) / 2) ((h - gmH) / 2) gmW gmH\n"
"\n"
"# PSP GUI\n"
"[480x272]\n"
"def_buttonWidth=100\n"
"def_buttonHeight=23\n"
"def_insetX=20\n"
"def_insetY=10\n"
"def_insetW=(w - 2 * insetX)\n"
"def_insetH=(h - 13 - insetY)\n"
"def_launcherVersionX=50\n"
"def_launcherVersionY=5\n"
"def_midiControlsSpacing=2\n"
"def_gameOptionsOverrideVPad=10\n"
"def_aboutXOff=3\n"
"def_aboutYOff=2\n"
"def_aboutOuterBorder=10\n"
"\n"
"use=XxY\n"
"";

using Common::String;

void Theme::processSingleLine(const String &section, const String &prefix, const String &name, const String &str) {
	int level = 0;
	int start = 0;
	uint i;
	int value;
	const char *selfpostfixes[] = {"self.x", "self.y", "self.w", "self.h"};
	const char *postfixesXYWH[] = {".x", ".y", ".w", ".h"};
	const char *postfixesRGB[] = {".r", ".g", ".b"};
	int npostfix = 0;
	const String prefixedname(prefix + name);

	// Make self.BLAH work, but not self.ANYTHING.BLAH
	if (!strchr(prefixedname.c_str(), '.')) {
		for (i = 0; i < ARRAYSIZE(postfixesXYWH); i++) {
			String to(prefixedname);

			to += postfixesXYWH[i];

			_evaluator->setAlias(selfpostfixes[i], to);
			_evaluator->setVar(to, EVAL_UNDEF_VAR);
		}
	}

	// Count the number of parameters, so that we know if they're meant to
	// be XY[WH] or RGB.

	int ntmppostfix = 0;

	for (i = 0; i < str.size(); i++) {
		if (isspace(str[i]) && level == 0) {
			ntmppostfix++;
		}

		if (str[i] == '(')
			level++;
		else if (str[i] == ')') {
			if (level == 0) {
				error("Extra ')' in section: [%s] expression: \"%s\" start is at: %d",
					  section.c_str(), name.c_str(), start);
			}
			level--;
		}
	}

	if (level > 0)
		error("Missing ')' in section: [%s] expression: \"%s\" start is at: %d",
			  section.c_str(), name.c_str(), start);

	const char **postfixes = (ntmppostfix == 2) ? postfixesRGB : postfixesXYWH;

	// Now do it for real, only this time we already know the parentheses
	// are balanced.

	for (i = 0; i < str.size(); i++) {
		if (isspace(str[i]) && level == 0) {
			value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name + postfixes[npostfix], start);
			_evaluator->setVar(prefixedname + postfixes[npostfix++], value);
			start = i + 1;
		}
		if (str[i] == '(')
			level++;
		else if (str[i] == ')')
			level--;
	}

	value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name + postfixes[npostfix], start);

	if (value == EVAL_STRING_VAR)
		_evaluator->setStringVar(prefixedname, _evaluator->lastToken());

	// process VAR=VALUE construct
	if (npostfix == 0) {
		_evaluator->setVar(name, value);

		// Fix bug #1742561: "GUI: Missaligned text"
		// "blah.align=foo" should be prefixed too
		_evaluator->setVar(prefixedname, value);
	} else {
		_evaluator->setVar(prefixedname + postfixes[npostfix], value);
	}

	// If we have all 4 parameters, set .x2 and .y2
	if (npostfix == 3) {
		_evaluator->setVar(prefixedname + ".x2",
			_evaluator->getVar(prefixedname + ".x") + _evaluator->getVar(prefixedname + ".w"));
		_evaluator->setVar(prefixedname + ".y2",
			_evaluator->getVar(prefixedname + ".y") + _evaluator->getVar(prefixedname + ".h"));
	}

	if (npostfix != 0)
		setSpecialAlias("prev", prefixedname);
}


void Theme::processResSection(Common::ConfigFile &config, const String &name, bool skipDefs, const String &prefix) {
	debug(3, "Reading section: [%s]", name.c_str());

	const Common::ConfigFile::SectionKeyList &keys = config.getKeys(name);

	Common::ConfigFile::SectionKeyList::const_iterator iterk;
	for (iterk = keys.begin(); iterk != keys.end(); ++iterk) {
		if (iterk->key == "set_parent") {
			setSpecialAlias("parent", prefix + iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("set_")) {
			_evaluator->setAlias(iterk->key.c_str() + 4, prefix + iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("def_")) {
			if (!skipDefs)
				_evaluator->setVar(name, prefix + iterk->key, iterk->value);
			continue;
		}
		if (iterk->key == "use") {
			if (iterk->value == name)
				error("Theme section [%s]: cannot use itself", name.c_str());
			if (!config.hasSection(iterk->value))
				error("Undefined use of section [%s]", iterk->value.c_str());
			processResSection(config, iterk->value, true);
			continue;
		}
		if (iterk->key == "useAsIs") {
			if (iterk->value == name)
				error("Theme section [%s]: cannot use itself", name.c_str());
			if (!config.hasSection(iterk->value))
				error("Undefined use of section [%s]", iterk->value.c_str());
			processResSection(config, iterk->value);
			continue;
		}
		if (iterk->key == "useWithPrefix") {
			const char *temp = iterk->value.c_str();
			const char *pos = strrchr(temp, ' ');

			if (pos == NULL)
				error("2 arguments required for useWithPrefix keyword");

			String n(temp, strchr(temp, ' ') - temp);
			String pref(pos + 1);

			if (n == name)
				error("Theme section [%s]: cannot use itself", n.c_str());
			if (!config.hasSection(n))
				error("Undefined use of section [%s]", n.c_str());
			processResSection(config, n, true, pref);
			continue;
		}
		processSingleLine(name, prefix, iterk->key, iterk->value);
	}
}

void Theme::setSpecialAlias(const String &alias, const String &name) {
	const char *postfixes[] = {".x", ".y", ".w", ".h", ".x2", ".y2"};
	int i;

	for (i = 0; i < ARRAYSIZE(postfixes); i++) {
		String from(alias + postfixes[i]);
		String to(name + postfixes[i]);

		_evaluator->setAlias(from.c_str(), to);
	}
}

bool Theme::isThemeLoadingRequired() {
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (_loadedThemeX == x && _loadedThemeY == y)
		return false;

	_loadedThemeX = x;
	_loadedThemeY = y;

	return true;
}

bool Theme::sectionIsSkipped(Common::ConfigFile &config, const char *name, int w, int h) {
	if (!config.hasKey("skipFor", name))
		return false;

	String res;

	config.getKey("skipFor", name, res);

	int x, y;
	int phase = 0;
	const char *ptr = res.c_str();

	x = y = 0;

	while (phase != 3) {
		switch (phase) {
		case 0:
			if (*ptr >= '0' && *ptr <= '9') {
				x = x * 10 + *ptr - '0';
			} else if (*ptr == 'X') {
				phase = 1;
			} else if (*ptr == 'x') {
				phase = 1;
			} else {
				error("Syntax error. Wrong resolution in skipFor in section %s", name);
			}
			break;
		case 1:
			if (*ptr >= '0' && *ptr <= '9') {
				y = y * 10 + *ptr - '0';
			} else if (*ptr == 'Y' || !*ptr || *ptr == ',') {
				phase = 2;

				if ((x == w || x == 0) && (y == h || y == 0))
					return true;
				if (!*ptr)
					return false;
				if (*ptr == ',') {
					phase = x = y = 0;
				}
			} else {
				error("Syntax error. Wrong resolution in skipFor in section %s", name);
			}
			break;
		case 2:
			if (*ptr == ',') {
				phase = x = y = 0;
			} else if (!*ptr) {
				if ((x == w || x == 0) && (y == h || y == 0))
					return true;
				return false;
			} else {
				error ("Syntax error. Wrong resolution in skipFor in section %s", name);
			}
			break;
		default:
			break;
		}

		ptr++;
	}

	return false;
}

void Theme::loadTheme(Common::ConfigFile &config, bool reset) {
	char name[80];
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (reset)
		_evaluator->reset();

	strcpy(name, "XxY");
	if (config.hasSection(name) && !sectionIsSkipped(config, "XxY", x, y))
		processResSection(config, name);

	sprintf(name, "%dxY", x);
	if (config.hasSection(name) && !sectionIsSkipped(config, name, x, y))
		processResSection(config, name);

	sprintf(name, "Xx%d", y);
	if (config.hasSection(name) && !sectionIsSkipped(config, name, x, y))
		processResSection(config, name);

	sprintf(name, "%dx%d", x, y);
	if (config.hasSection(name) && !sectionIsSkipped(config, name, x, y))
		processResSection(config, name);

	debug(3, "Number of variables: %d", _evaluator->getNumVars());
}

void Theme::loadTheme(Common::ConfigFile &config, bool reset, bool doBackendSpecificPostProcessing) {
	loadTheme(config, reset);

	if (doBackendSpecificPostProcessing && !g_system->getExtraThemeConfig().empty()) {
		Common::ConfigFile myConfig;
		Common::String myConfigINI = g_system->getExtraThemeConfig();
		Common::MemoryReadStream s((const byte *)myConfigINI.c_str(), myConfigINI.size());
		myConfig.loadFromStream(s);
		loadTheme(myConfig, false);
	}
}

} // End of namespace GUI
