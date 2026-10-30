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

#ifndef BACKENDS_MIDI_WINDOWS_H
#define BACKENDS_MIDI_WINDOWS_H

#include "audio/musicplugin.h"
#include "common/func.h"

#include <windows.h>

/**
 * Enumerate WinMM devices and create Windows MIDI drivers.
 */
class WindowsMusicPlugin : public MusicPluginObject {
public:
	WindowsMusicPlugin();

	const char *getName() const override;
	const char *getId() const override;

	/**
	 * Enumerate available MIDI devices with WinMM.
	 */
	MusicDevices getDevices() const override;

	/**
	 * Create a driver for the selected device, or the default device when no match exists.
	 */
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle dev = 0) const override;

	/**
	 * Checks whether a device can actually be used.
	 * Always return trues on Windows MIDI backend.
	 */
	bool checkDevice(MidiDriver::DeviceHandle hdl, int checkFlags, bool quiet) const override;

	/**
	 * Start background WinMM warm-up thread.
	 *
	 * Must be called on backend init after SDL audio init.
	 * Windows MIDI Services init can block concurrent MME audo init,
	 * one of the possible audio backend on Windows.
	 * @see https://github.com/microsoft/MIDI/issues/1076
	 */
	static void startMidiWarmUpThread();

	/**
	 * Clear the warm-up thread result and resources.
	 *
	 * Should only be called on platform teardown.
	 */
	static void cleanupMidiWarmUpThread();

	/**
	 * Wait for background MIDI warm-up through a modal dialog when the GUI is active.
	 * A modal dialog will handle UI events, keeping the process responsive.
	 *
	 * @param isComplete A functor immediately returns whether the warm-up has been finished.
	 * @return True if completion was observed.
	 *         False if the wait was skipped because the GUI was unavailable or inactive,
	 *         or if the dialog closed before completion (e.g. on quit or return to launcher).
	 *
	 * @see setMidiWarmUpWaitCallback()
	 */
	typedef bool (*MidiWarmUpWaitCallback)(const Common::Functor0<bool> &isComplete);

	/**
	 * Register the function that displays progress dialog while MIDI warm-up is running.
	 * The main purpose of that dialog is to prevent UI thread hang by first WinMM API call.
	 *
	 * WinMM APIs may be called while the warm-up thread is still running.
	 * In that case, the plugin invokes this callback on the UI thread to spawn the dialog.
	 * The dialog polls @p isComplete while processing UI events.
	 *
	 * The Windows backend registers GUI::MidiDeviceScanDialog::waitForCompletion() as a callback.
	 *
	 * @param callback Function that spawns a progress dialog.
	 *
	 * @see MidiWarmUpWaitCallback
	 */
	static void setMidiWarmUpWaitCallback(MidiWarmUpWaitCallback callback);

private:
	/**
	 * Expose the plugin's private warm-up status query as Common::Functor0<bool>.
	 *
	 * The progress handler must poll this predicate on the UI thread while processing events.
	 * This lets the MIDI plugin use a GUI wait handler without depending on GUI classes.
	 */
	class MidiWarmUpCompletion : public Common::Functor0<bool> {
	public:
		/**
		 * Report whether the predicate has a valid query target.
		 *
		 * @return Always true, since the query is static and requires no plugin instance.
		 */
		bool isValid() const override { return true; }

		/**
		 * Query warm-up status on the UI thread without waiting.
		 *
		 * @return True if the worker has exited or no warm-up handle is retained,
		 *         otherwise false.
		 */
		bool operator()() const override { return isMidiWarmUpFinished(); }
	};

	/** Query completion on the UI thread without blocking; no worker means no pending warm-up. */
	static bool isMidiWarmUpFinished();

	/**
	 * Warm up WinMM in the worker thread.
	 *
	 * @remark
	 * Thread termination also signals completion.
	 */
	static unsigned __stdcall warmUpMidiThreadMain(void *arglist);

	/**
	 * Invoke the UI wait handler on the UI thread if MIDI warm-up is pending.
	 *
	 * Show a progress dialog if the warm-up is not finished yet, and GUI is available.
	 * Returns immediately if warm-up has been finished, or GUI is not available.
	 *
	 * @return True if no UI wait handler was invoked, or if it observed completion.
	 *         False if the handler skipped waiting because the GUI was unavailable or
	 *         inactive, or if the wait was interrupted (e.g. on quit or return to launcher).
	 */
	bool waitForMidiWarmUp() const;

	/** UI-thread-owned worker handle, retained until backend cleanup. */
	static HANDLE _midiWarmUpThreadHandle;

	/** UI-thread-only handler, installed by the backend and cleared before backend destruction. */
	static MidiWarmUpWaitCallback _midiWarmUpWaitCallback;

	/** Thread that created this plugin and is allowed to invoke the progress handler. */
	const DWORD _uiThreadId;
};

#endif
