
package org.scummvm.scummvm;

import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.os.Handler;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

import static org.scummvm.scummvm.ScummVMEventsBase.JE_MULTI;

public class MultitouchHelper {
	private final ScummVM _scummvm;

	private boolean _candidateStartOfMultitouchSession;
	// a flag indicating whether we are in multitouch mode (more than one fingers down)
	private boolean _multitouchMode;

	// within a multitouch session (until a cancel event or no more multiple fingers down) the IDs for each finger-pointer is persisted and is consistent across events
	// we can use the ids to track and utilize the movement of a specific finger (while ignoring the rest)
	// Currently, we are using the last finger down, as the finger that moves the cursor
	private int     _firstPointerId;
	private int     _secondPointerId;
	private int     _thirdPointerId;

	private int     _cachedActionEventOnPointer2DownX;
	private int     _cachedActionEventOnPointer2DownY;
	// The "level" of multitouch that is detected.
	// We do not support downgrading a level, ie. if a three finger multitouch is detected,
	// then raising one finger will void the multitouch session,
	// rather than revert to two fingers multitouch.
	// Similarly we do not support upgrading a level, ie. if we are already handling a two finger multitouch,
	// then putting down another finger will void the session,
	// rather than upgrade it to three fingers multitouch.
	// TODO for this purpose we need to allow some limited time limit (delay _kLevelDecisionDelayMs) before deciding
	//      if the user did a two finger multitouch or intents to do a three finger multitouch
	// Valid values for _multitouchLevel: 0, 2, 3
	private int     _multitouchLevel;

	private final int _kLevelDecisionDelayMs = 400; // in milliseconds

	// messages for MultitouchHelperHandler
	final static int MSG_MT_DECIDE_MULTITOUCH_SESSION_TIMEDOUT = 1;
	final static int MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT = 2;

	final private MultitouchHelper.MultitouchHelperHandler _multiTouchLevelUpgradeHandler = new MultitouchHelper.MultitouchHelperHandler(this);

	// constructor
	public MultitouchHelper(ScummVM scummvm) {
		_scummvm = scummvm;

		_multitouchMode = false;
		_multitouchLevel =  0;
		_candidateStartOfMultitouchSession = false;

		resetPointers();
	}

	public void resetPointers() {
		_firstPointerId  = -1;
		_secondPointerId = -1;
		_thirdPointerId  = -1;
	}

	public boolean isMultitouchMode() {
		return _multitouchMode;
	}
	public int getMultitouchLevel() {
		return _multitouchLevel;
	}

	public void setMultitouchMode(boolean enabledFlg) {
		_multitouchMode = enabledFlg;
	}
	public void setMultitouchLevel(int mtlevel) {
		_multitouchLevel = mtlevel;
	}


	// TODO Maybe for consistency purposes, maybe sent all (important) UP events that were not sent, when ending a multitouch session?
	public boolean handleMotionEvent(final MotionEvent event) {

		// constants from APIv5:
		// (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT
		//final int pointer = (action & 0xff00) >> 8;

		int pointerIndex = -1;
		int actionEventX;
		int actionEventY;

		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			// start of a multitouch session! one finger down -- this is sent for the first pointer who touches the screen
			resetPointers();
			setMultitouchLevel(0);
			setMultitouchMode(false);
			_candidateStartOfMultitouchSession = true;
			_multiTouchLevelUpgradeHandler.clear();
//			_multiTouchLevelUpgradeHandler.sendMessageDelayed(_multiTouchLevelUpgradeHandler.obtainMessage(MSG_MT_DECIDE_MULTITOUCH_SESSION_TIMEDOUT), _kLevelDecisionDelayMs);
			pointerIndex = 0;
			_firstPointerId = event.getPointerId(pointerIndex);
			// TODO - do we want this as true?
			return false;
		} else if (event.getAction() == MotionEvent.ACTION_CANCEL) {
			resetPointers();
			setMultitouchLevel(0);
			setMultitouchMode(false);
			_multiTouchLevelUpgradeHandler.clear();
			return true;
		} else if (event.getAction() == MotionEvent.ACTION_OUTSIDE) {
			return false;
		}

		if (event.getPointerCount() > 1 && event.getPointerCount() < 4) {
			// a multi-touch event
			if (_candidateStartOfMultitouchSession && event.getPointerCount() > 1) {
				setMultitouchMode(true);
			}

			if (isMultitouchMode()) {
				if ((event.getAction() & MotionEvent.ACTION_POINTER_DOWN) == MotionEvent.ACTION_POINTER_DOWN) {
					pointerIndex = event.getActionIndex();
					if (event.getPointerCount() == 2) {
						_secondPointerId = event.getPointerId(pointerIndex);
						if (getMultitouchLevel() == 0) {
							_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT);
							if (pointerIndex != -1) {
								_cachedActionEventOnPointer2DownX = (int) event.getX(pointerIndex);
								_cachedActionEventOnPointer2DownY = (int) event.getY(pointerIndex);
							} else {
								_cachedActionEventOnPointer2DownX = -1;
								_cachedActionEventOnPointer2DownY = -1;
							}
							_multiTouchLevelUpgradeHandler.sendMessageDelayed(_multiTouchLevelUpgradeHandler.obtainMessage(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT), _kLevelDecisionDelayMs);
						}
						return true;
					} else  if (event.getPointerCount() == 3) {
						_thirdPointerId = event.getPointerId(pointerIndex);
						if (getMultitouchLevel() == 0) {
							setMultitouchLevel(3);
						}
					}
				} else {
					if (event.getPointerCount() == 2) {
						// we prioritize the second pointer/ finger
						pointerIndex = event.findPointerIndex(_secondPointerId);

						if (getMultitouchLevel() == 0
						    && ((event.getAction() & MotionEvent.ACTION_POINTER_UP) == MotionEvent.ACTION_POINTER_UP
						        || (event.getAction() & MotionEvent.ACTION_MOVE) == MotionEvent.ACTION_MOVE)) {
							setMultitouchLevel(2);
							_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT);

							if (pointerIndex != -1) {
								actionEventX = (int)event.getX(pointerIndex);
								actionEventY = (int)event.getY(pointerIndex);
							} else {
								actionEventX = -1;
								actionEventY = -1;
							}

							// send the missing pointer down event first
							_scummvm.pushEvent(JE_MULTI,
								event.getPointerCount(),
								MotionEvent.ACTION_POINTER_DOWN,
								actionEventX,
								actionEventY,
								0, 0);
						}

					} else  if (event.getPointerCount() == 3) {
						// we prioritize the third pointer/ finger
						pointerIndex = event.findPointerIndex(_thirdPointerId);
					}
				}

//				if (pointerIndex == -1) {
//					Log.d(ScummVM.LOG_TAG,"Warning: pointerIndex == -1 and getPointerCount = " + event.getPointerCount());
//				}

				if (pointerIndex != -1) {
					actionEventX = (int)event.getX(pointerIndex);
					actionEventY = (int)event.getY(pointerIndex);
				} else {
					actionEventX = -1;
					actionEventY = -1;
				}

				// we are only concerned for events with fingers down equal to the decided level of multitouch session
				if (getMultitouchLevel() == event.getPointerCount()) {
					// arg1 will be the number of fingers down in the MULTI event we send to events.cpp
					_scummvm.pushEvent(JE_MULTI,
						event.getPointerCount(),
						event.getAction(),
						actionEventX,
						actionEventY,
						0, 0);
				}
			}
			return true;

		} else if (event.getPointerCount() >= 4) {
			// ignore if more than 3 fingers down. Mark as multitouch "handled" (return true)
			return true;

		} else if (event.getPointerCount() == 1 && isMultitouchMode() ) {
			// keep ignoring events until we exit multitouch mode "session"
			// this is to catch the case of progressively lifting fingers and being left with only one finger still touching the surface
			return true;
		} else {
			// one finger, no active multitouch mode "session". Mark as unhandled.
			return false;
		}
	}


	// Custom handler code (to avoid mem leaks, see warning "This Handler Class Should Be Static Or Leaks Might Occur”) based on:
	// https://stackoverflow.com/a/27826094
	public static class MultitouchHelperHandler extends Handler {

		private final WeakReference<MultitouchHelper> mListenerReference;

		public MultitouchHelperHandler(MultitouchHelper listener) {
			mListenerReference = new WeakReference<>(listener);
		}

		@Override
		public synchronized void handleMessage(@NonNull Message msg) {
			MultitouchHelper listener = mListenerReference.get();
			if(listener != null) {
				listener.handle_MTHH_Message(msg);
			}
		}

		public void clear() {
			this.removeCallbacksAndMessages(null);
		}
	}

	private void handle_MTHH_Message(final Message msg) {
		if (msg.what == MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT) {
			if (getMultitouchLevel() == 0) {
				// window of allowing upgrade to level 3 timed out, decide level two.
				setMultitouchLevel(2);

				// send the delayed pointer down event
				_scummvm.pushEvent(JE_MULTI,
					2,
					MotionEvent.ACTION_POINTER_DOWN,
					_cachedActionEventOnPointer2DownX,
					_cachedActionEventOnPointer2DownY,
					0, 0);
			}
		}
//		else if (msg.what == MSG_MT_DECIDE_MULTITOUCH_SESSION_TIMEDOUT) {
//			if (_candidateStartOfMultitouchSession  && !isMultitouchMode()) {
//				// window of considering touch as start of multitouch event timed out. Clear the candidate flag.
//				_candidateStartOfMultitouchSession = false;
//			}
//		}
	}

	public void clearEventHandler() {
		_multiTouchLevelUpgradeHandler.clear();
	}
}
