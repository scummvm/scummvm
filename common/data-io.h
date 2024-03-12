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

#ifndef COMMON_DATA_IO_H
#define COMMON_DATA_IO_H

#include "common/scummsys.h"

namespace Common {

enum class EndianStorageFormat {
	Big,
	Little,
};

// Traits for a storage format.  You can specialize this to make more storage formats
// that have their own behavior.
template<class TDataFormat>
struct DataFormatTraits {
};

template<>
struct DataFormatTraits<EndianStorageFormat> {
	static inline bool isLittleEndian(EndianStorageFormat storageFormat) {
		return storageFormat == EndianStorageFormat::Little;
	}
};

template<class TDataFormat, class T>
struct SimpleDataIO {
	static const uint kMaxSize = sizeof(T);

	static uint computeSize(TDataFormat dataFormat);

	static void encode(TDataFormat dataFormat, byte *data, const T &value);
	static void decode(TDataFormat dataFormat, const byte *data, T &value);
};

template<class TDataFormat, class T>
uint SimpleDataIO<TDataFormat, T>::computeSize(TDataFormat dataFormat) {
	return sizeof(T);
}

template<class TDataFormat, class T>
void SimpleDataIO<TDataFormat, T>::encode(TDataFormat dataFormat, byte *data, const T &value) {
	const byte *valueBytes = reinterpret_cast<const byte *>(&value);
	byte *dataBytes = reinterpret_cast<byte *>(data);

	const bool isTargetLE = DataFormatTraits<TDataFormat>::isLittleEndian(dataFormat);
#ifdef SCUMM_LITTLE_ENDIAN
	const bool isSystemLE = true;
#endif
#ifdef SCUMM_BIG_ENDIAN
	const bool isSystemLE = false;
#endif

	const bool requiresSwap = (isSystemLE != isTargetLE);

	byte temp[sizeof(T)];

	if (requiresSwap) {
		for (uint i = 0; i < sizeof(T); i++)
			temp[i] = valueBytes[sizeof(T) - 1 - i];
	} else {
		for (uint i = 0; i < sizeof(T); i++)
			temp[i] = valueBytes[i];
	}

	for (uint i = 0; i < sizeof(T); i++)
		dataBytes[i] = temp[i];
}

template<class TDataFormat, class T>
void SimpleDataIO<TDataFormat, T>::decode(TDataFormat dataFormat, const byte *data, T &value) {
	byte *valueBytes = reinterpret_cast<byte *>(&value);
	const byte *dataBytes = reinterpret_cast<const byte *>(data);

	const bool isTargetLE = DataFormatTraits<TDataFormat>::isLittleEndian(dataFormat);
#ifdef SCUMM_LITTLE_ENDIAN
	const bool isSystemLE = true;
#endif
#ifdef SCUMM_BIG_ENDIAN
	const bool isSystemLE = false;
#endif

	const bool requiresSwap = (isSystemLE != isTargetLE);

	byte temp[sizeof(T)];

	if (requiresSwap) {
		for (uint i = 0; i < sizeof(T); i++)
			temp[i] = dataBytes[sizeof(T) - 1 - i];
	} else {
		for (uint i = 0; i < sizeof(T); i++)
			temp[i] = dataBytes[i];
	}

	for (uint i = 0; i < sizeof(T); i++)
		valueBytes[i] = temp[i];
}

template<class TDataFormat, class T>
struct DataIO {
};

template<class TDataFormat>
struct DataIO<TDataFormat, uint8> : public SimpleDataIO<TDataFormat, uint8> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, uint16> : public SimpleDataIO<TDataFormat, uint16> {
};
template<class TDataFormat>
struct DataIO<TDataFormat, uint32> : public SimpleDataIO<TDataFormat, uint32> {
};
template<class TDataFormat>
struct DataIO<TDataFormat, uint64> : public SimpleDataIO<TDataFormat, uint64> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, int8> : public SimpleDataIO<TDataFormat, int8> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, int16> : public SimpleDataIO<TDataFormat, int16> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, int32> : public SimpleDataIO<TDataFormat, int32> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, int64> : public SimpleDataIO<TDataFormat, int64> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, char> : public SimpleDataIO<TDataFormat, char> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, float> : public SimpleDataIO<TDataFormat, float> {
};

template<class TDataFormat>
struct DataIO<TDataFormat, double> : public SimpleDataIO<TDataFormat, double> {
};

template<class TDataFormat, class T, class... TMore>
struct DataMultipleIO;

template<class TDataFormat, class T>
struct DataMultipleIO<TDataFormat, T> {
	static const uint kMaxSize = DataIO<TDataFormat, T>::kMaxSize;

	static uint computeSize(TDataFormat dataFormat);

	static void encode(TDataFormat dataFormat, byte *data, const T &value);
	static void decode(TDataFormat dataFormat, const byte *data, T &value);
};

template<class TDataFormat, class T>
uint DataMultipleIO<TDataFormat, T>::computeSize(TDataFormat dataFormat) {
	return DataIO<TDataFormat, T>::computeSize(dataFormat);
}

template<class TDataFormat, class T>
void DataMultipleIO<TDataFormat, T>::encode(TDataFormat dataFormat, byte *data, const T &value) {
	return DataIO<TDataFormat, T>::encode(dataFormat, data, value);
}

template<class TDataFormat, class T>
void DataMultipleIO<TDataFormat, T>::decode(TDataFormat dataFormat, const byte *data, T &value) {
	return DataIO<TDataFormat, T>::decode(dataFormat, data, value);
}

template<class TDataFormat, class T, uint TSize>
struct DataMultipleIO<TDataFormat, T[TSize]> {
	static const uint kMaxSize = DataIO<TDataFormat, T>::kMaxSize * TSize;

	static uint computeSize(TDataFormat dataFormat);

	static void encode(TDataFormat dataFormat, byte *data, const T (&value)[TSize]);
	static void decode(TDataFormat dataFormat, const byte *data, T (&value)[TSize]);
};

template<class TDataFormat, class T, uint TSize>
uint DataMultipleIO<TDataFormat, T[TSize]>::computeSize(TDataFormat dataFormat) {
	return DataMultipleIO<TDataFormat, T>::computeSize(dataFormat) * TSize;
}

template<class TDataFormat, class T, uint TSize>
void DataMultipleIO<TDataFormat, T[TSize]>::encode(TDataFormat dataFormat, byte *data, const T (&value)[TSize]) {
	const uint elementSize = DataIO<TDataFormat, T>::computeSize(dataFormat);
	for (uint i = 0; i < TSize; i++)
		DataMultipleIO<TDataFormat, T>::encode(dataFormat, data + elementSize * i, value[i]);
}

template<class TDataFormat, class T, uint TSize>
void DataMultipleIO<TDataFormat, T[TSize]>::decode(TDataFormat dataFormat, const byte *data, T (&value)[TSize]) {
	const uint elementSize = DataIO<TDataFormat, T>::computeSize(dataFormat);
	for (uint i = 0; i < TSize; i++)
		DataMultipleIO<TDataFormat, T>::decode(dataFormat, data + elementSize * i, value[i]);
}

template<class TDataFormat, class T, class... TMore>
struct DataMultipleIO {
	static const uint kMaxSize = DataIO<TDataFormat, T>::kMaxSize + DataMultipleIO<TDataFormat, TMore...>::kMaxSize;

	static uint computeSize(TDataFormat dataFormat);

	static void encode(TDataFormat dataFormat, byte *data, const T &firstValue, const TMore &...moreValues);
	static void decode(TDataFormat dataFormat, const byte *data, T &firstValue, TMore &...moreValues);
};

template<class TDataFormat, class T, class... TMore>
uint DataMultipleIO<TDataFormat, T, TMore...>::computeSize(TDataFormat dataFormat) {
	return DataMultipleIO<TDataFormat, T>::computeSize(dataFormat) + DataMultipleIO<TDataFormat, TMore...>::computeSize(dataFormat);
}

template<class TDataFormat, class T, class... TMore>
void DataMultipleIO<TDataFormat, T, TMore...>::encode(TDataFormat dataFormat, byte *data, const T &firstValue, const TMore &...moreValues) {
	DataMultipleIO<TDataFormat, T>::encode(dataFormat, data, firstValue);
	DataMultipleIO<TDataFormat, TMore...>::encode(dataFormat, data + DataMultipleIO<TDataFormat, T>::computeSize(dataFormat), moreValues...);
}

template<class TDataFormat, class T, class... TMore>
void DataMultipleIO<TDataFormat, T, TMore...>::decode(TDataFormat dataFormat, const byte *data, T &firstValue, TMore &...moreValues) {
	DataMultipleIO<TDataFormat, T>::decode(dataFormat, data, firstValue);
	DataMultipleIO<TDataFormat, TMore...>::decode(dataFormat, data + DataMultipleIO<TDataFormat, T>::computeSize(dataFormat), moreValues...);
}

} // End of namespace Common

#endif
