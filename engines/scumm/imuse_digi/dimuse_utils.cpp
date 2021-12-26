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

#include "scumm/imuse_digi/dimuse_engine.h"

namespace Scumm {

int IMuseDigital::addTrackToList(IMuseDigiTrack **listPtr, IMuseDigiTrack *listPtr_Item) {
	// [0] is ->prev, [1] is ->next
	if (!listPtr_Item || listPtr_Item->prev || listPtr_Item->next) {
		debug(5, "IMuseDigital::addTrackToList(): ERROR: arguments might be null");
		return -5;
	} else {
		// Set item's next element to the list
		listPtr_Item->next = *listPtr;

		if (*listPtr) {
			// If the list is empty, use this item as the list
			(*listPtr)->prev = listPtr_Item;
		}

		// Set the previous element of the item as nullptr,
		// effectively making the item the first element of the list
		listPtr_Item->prev = nullptr;

		// Update the list with the new data
		*listPtr = listPtr_Item;

	}
	return 0;
}

int IMuseDigital::removeTrackFromList(IMuseDigiTrack **listPtr, IMuseDigiTrack *listPtr_Item) {
	IMuseDigiTrack *currentTrack = *listPtr;
	IMuseDigiTrack *nextTrack;
	if (listPtr_Item && currentTrack) {
		do {
			if (currentTrack == listPtr_Item)
				break;
			currentTrack = currentTrack->next;
		} while (currentTrack);

		if (currentTrack) {
			nextTrack = listPtr_Item->next;

			if (nextTrack)
				nextTrack->prev = listPtr_Item->prev;

			if (listPtr_Item->prev) {
				listPtr_Item->prev->next = listPtr_Item->next;
			} else {
				*listPtr = listPtr_Item->next;
			}

			listPtr_Item->prev = nullptr;
			listPtr_Item->next = nullptr;
			return 0;
		} else {
			debug(5, "IMuseDigital::removeTrackFromList(): ERROR: item not on list");
			return -3;
		}
	} else {
		debug(5, "IMuseDigital::removeTrackFromList(): ERROR: arguments might be null");
		return -5;
	}
}

int IMuseDigital::addStreamZoneToList(IMuseDigiStreamZone **listPtr, IMuseDigiStreamZone *listPtr_Item) {
	if (!listPtr_Item || listPtr_Item->prev || listPtr_Item->next) {
		debug(5, "IMuseDigital::addStreamZoneToList(): ERROR: arguments might be null");
		return -5;
	} else {
		// Set item's next element to the list
		listPtr_Item->next = *listPtr;

		if (*listPtr) {
			// If the list is empty, use this item as the list
			(*listPtr)->prev = listPtr_Item;
		}

		// Set the previous element of the item as nullptr,
		// effectively making the item the first element of the list
		listPtr_Item->prev = nullptr;

		// Update the list with the new data
		*listPtr = listPtr_Item;

	}
	return 0;
}

int IMuseDigital::removeStreamZoneFromList(IMuseDigiStreamZone **listPtr, IMuseDigiStreamZone *listPtr_Item) {
	IMuseDigiStreamZone *currentStrZone = *listPtr;
	IMuseDigiStreamZone *nextStrZone;
	if (listPtr_Item && currentStrZone) {
		do {
			if (currentStrZone == listPtr_Item)
				break;
			currentStrZone = currentStrZone->next;
		} while (currentStrZone);

		if (currentStrZone) {
			nextStrZone = listPtr_Item->next;

			if (nextStrZone)
				nextStrZone->prev = listPtr_Item->prev;

			if (listPtr_Item->prev) {
				listPtr_Item->prev->next = listPtr_Item->next;
			} else {
				*listPtr = listPtr_Item->next;
			}

			listPtr_Item->prev = nullptr;
			listPtr_Item->next = nullptr;
			return 0;
		} else {
			debug(5, "IMuseDigital::removeStreamZoneFromList(): ERROR: item not on list");
			return -3;
		}
	} else {
		debug(5, "IMuseDigital::removeStreamZoneFromList(): ERROR: arguments might be null");
		return -5;
	}
}

int IMuseDigital::clampNumber(int value, int minValue, int maxValue) {
	if (value < minValue)
		return minValue;

	if (value > maxValue)
		return maxValue;

	return value;
}

int IMuseDigital::clampTuning(int value, int minValue, int maxValue) {
	if (minValue > value) {
		value += (12 * ((minValue - value) + 11) / 12);
	}

	if (maxValue < value) {
		value -= (12 * ((value - maxValue) + 11) / 12);
	}

	return value;
}

int IMuseDigital::checkHookId(int &trackHookId, int sampleHookId) {
	if (sampleHookId) {
		if (trackHookId == sampleHookId) {
			trackHookId = 0;
			return 0;
		} else {
			return -1;
		}
	} else if (trackHookId == 128) {
		trackHookId = 0;
		return -1;
	} else {
		return 0;
	}
}

} // End of namespace Scumm
