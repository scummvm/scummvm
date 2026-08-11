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

#include "mtropolis/plugin/hoologic.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Hoologic {

BitmapVariableModifier::BitmapVariableModifier() {
}

BitmapVariableModifier::~BitmapVariableModifier() {
}

bool BitmapVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::BitmapVariableModifier &data) {
	return true;
}

bool BitmapVariableModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState BitmapVariableModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void BitmapVariableModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void BitmapVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> BitmapVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BitmapVariableModifier(*this));
}

const char *BitmapVariableModifier::getDefaultName() const {
	return "Bitmap Variable Modifier"; // ???
}

CaptureBitmapModifier::CaptureBitmapModifier() {
}

CaptureBitmapModifier::~CaptureBitmapModifier() {
}

bool CaptureBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::CaptureBitmapModifier &data) {
	return true;
}

bool CaptureBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState CaptureBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void CaptureBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void CaptureBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> CaptureBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CaptureBitmapModifier(*this));
}

const char *CaptureBitmapModifier::getDefaultName() const {
	return "Capture Bitmap Modifier"; // ???
}

ImportBitmapModifier::ImportBitmapModifier() {
}

ImportBitmapModifier::~ImportBitmapModifier() {
}

bool ImportBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ImportBitmapModifier &data) {
	return true;
}

bool ImportBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState ImportBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void ImportBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ImportBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ImportBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ImportBitmapModifier(*this));
}

const char *ImportBitmapModifier::getDefaultName() const {
	return "Import Bitmap Modifier"; // ???
}

DisplayBitmapModifier::DisplayBitmapModifier() {
}

DisplayBitmapModifier::~DisplayBitmapModifier() {
}

bool DisplayBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::DisplayBitmapModifier &data) {
	return true;
}

bool DisplayBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState DisplayBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void DisplayBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void DisplayBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> DisplayBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new DisplayBitmapModifier(*this));
}

const char *DisplayBitmapModifier::getDefaultName() const {
	return "Display Bitmap Modifier"; // ???
}

ScaleBitmapModifier::ScaleBitmapModifier() {
}

ScaleBitmapModifier::~ScaleBitmapModifier() {
}

bool ScaleBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ScaleBitmapModifier &data) {
	return true;
}

bool ScaleBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState ScaleBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void ScaleBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ScaleBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ScaleBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ScaleBitmapModifier(*this));
}

const char *ScaleBitmapModifier::getDefaultName() const {
	return "Scale Bitmap Modifier"; // ???
}

SaveBitmapModifier::SaveBitmapModifier() {
}

SaveBitmapModifier::~SaveBitmapModifier() {
}

bool SaveBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::SaveBitmapModifier &data) {
	return true;
}

bool SaveBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState SaveBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void SaveBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void SaveBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> SaveBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SaveBitmapModifier(*this));
}

const char *SaveBitmapModifier::getDefaultName() const {
	return "Save Bitmap Modifier"; // ???
}

PrintBitmapModifier::PrintBitmapModifier() {
}

PrintBitmapModifier::~PrintBitmapModifier() {
}

bool PrintBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PrintBitmapModifier &data) {
	return true;
}

bool PrintBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PrintBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PrintBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PrintBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> PrintBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PrintBitmapModifier(*this));
}

const char *PrintBitmapModifier::getDefaultName() const {
	return "Print Bitmap Modifier"; // ???
}

PainterModifier::PainterModifier() {
}

PainterModifier::~PainterModifier() {
}

bool PainterModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PainterModifier &data) {
	return true;
}

bool PainterModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PainterModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PainterModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PainterModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> PainterModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PainterModifier(*this));
}

const char *PainterModifier::getDefaultName() const {
	return "Painter Modifier"; // ???
}

KeyStateModifier::KeyStateModifier() {
}

KeyStateModifier::~KeyStateModifier() {
}

bool KeyStateModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::KeyStateModifier &data) {
	return true;
}

bool KeyStateModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState KeyStateModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void KeyStateModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void KeyStateModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> KeyStateModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new KeyStateModifier(*this));
}

const char *KeyStateModifier::getDefaultName() const {
	return "KeyState Modifier"; // ???
}

HoologicPlugIn::HoologicPlugIn()
	: _bitmapVariableModifierFactory(this)
	, _captureBitmapModifierFactory(this)
	, _importBitmapModifierFactory(this)
	, _displayBitmapModifierFactory(this)
	, _scaleBitmapModifierFactory(this)
	, _saveBitmapModifierFactory(this)
	, _printBitmapModifierFactory(this)
	, _painterModifierFactory(this)
	, _keyStateModifierFactory(this)
	{	
}

HoologicPlugIn::~HoologicPlugIn() {
}

void HoologicPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	//Bitmap plugin
	registrar->registerPlugInModifier("hlBitmapVariable", &_bitmapVariableModifierFactory);
	registrar->registerPlugInModifier("hlCaptureBitmap", &_captureBitmapModifierFactory);
	registrar->registerPlugInModifier("hlImportBitmap", &_importBitmapModifierFactory);
	registrar->registerPlugInModifier("hlDisplayBitmap", &_displayBitmapModifierFactory);
	registrar->registerPlugInModifier("hlScaleBitmap", &_scaleBitmapModifierFactory);
	registrar->registerPlugInModifier("hlSaveBitmap", &_saveBitmapModifierFactory);
	registrar->registerPlugInModifier("hlPrintBitmap", &_printBitmapModifierFactory);

	//Painter plugin
	registrar->registerPlugInModifier("hlPainter", &_painterModifierFactory);

	//KeyState plugin
	registrar->registerPlugInModifier("hlKeyState", &_keyStateModifierFactory);
}

} // End of namespace Hoologic

namespace PlugIns {

Common::SharedPtr<PlugIn> createHoologic() {
	return Common::SharedPtr<PlugIn>(new Hoologic::HoologicPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
