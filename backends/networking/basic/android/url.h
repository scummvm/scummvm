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
#ifndef BACKENDS_NETWORKING_BASIC_ANDROID_URL_H
#define BACKENDS_NETWORKING_BASIC_ANDROID_URL_H

#include <jni.h>

#include "backends/networking/basic/url.h"

namespace Networking {

class AndroidURL : public URL {
public:
	static URL *parseURL(const Common::String &url);

	AndroidURL(JNIEnv *env, jobject url);

	~AndroidURL() override;

	Common::String getScheme() const override;
	Common::String getHost() const override;
	int getPort(bool returnDefault = false) const override;
private:
	jobject _url;
};

} // End of Namespace Networking

#endif
