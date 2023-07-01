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

#if defined(__ANDROID__)

#include "backends/dlc/android/playstore.h"

namespace DLC {
namespace PlayStore {

PlayStore::PlayStore() {
	_playStoreObject = JNI::getPlayStoreDLCManager();
}

void PlayStore::init() {

}

void PlayStore::requestInfo() {
	JNIEnv *env = JNI::getEnv();
	jclass playStoreClass = env->GetObjectClass(_playStoreObject);

	jmethodID getPackLocationMethod = env->GetMethodID(playStoreClass, "getPackLocation", "(Ljava/lang/String;)Lcom/google/android/play/core/assetpacks/AssetPackLocation;");

	jstring packName = env->NewStringUTF("beneath_a_steel_sky");
	jobject packLocationObject = env->CallObjectMethod(_playStoreObject, getPackLocationMethod, packName);
	
	env->DeleteLocalRef(packName);
	env->DeleteLocalRef(packLocationObject);

	// TODO: return a string/struct for the location and add a function parameter i.e. packName
}

} // End of namespace PlayStore
} // End of namespace DLC

#endif
