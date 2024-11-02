
package org.scummvm.scummvm;

import static org.scummvm.scummvm.ScummVMEvents.JE_MOUSE_WHEEL_DOWN;
import static org.scummvm.scummvm.ScummVMEvents.JE_MOUSE_WHEEL_UP;
import static org.scummvm.scummvm.ScummVMEvents.JE_MULTI;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

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

	//
	private int     _cachedActionEventOnPointer1DownX;
	private int     _cachedActionEventOnPointer1DownY;

	// The "level" of multitouch that is detected.
	// We do not support downgrading a level, ie. if a three finger multitouch is detected,
	// then raising one finger will void the multitouch session,
	// rather than revert to two fingers multitouch.
	// Similarly we do not support upgrading a level, ie. if we are already handling a two finger multitouch,
	// then putting down another finger will void the session,
	// rather than upgrade it to three fingers multitouch.
	// INFO for this purpose we need to allow some limited time limit (delay _kLevelDecisionDelayMs) before deciding
	//      if the user did a two finger multitouch or intents to do a three finger multitouch
	// Valid values for _multitouchLevel: MULTITOUCH_UNDECIDED, MULTITOUCH_TWO_FINGERS, MULTITOUCH_THREE_FINGERS
	private final int MULTITOUCH_UNDECIDED = 0;
	private final int MULTITOUCH_TWO_FINGERS = 2;
	private final int MULTITOUCH_THREE_FINGERS = 3;
	private int     _multitouchLevel;

	private final int _kLevelDecisionDelayMs = 400; // in milliseconds
	private final int _kTouchMouseWheelDecisionDelayMs = 260; // in milliseconds - NOTE: Keep it significantly lower than _kLevelDecisionDelayMs

	// messages for MultitouchHelperHandler
	final static int MSG_MT_DECIDE_MULTITOUCH_SESSION_TIMEDOUT = 1;
	final static int MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT = 2;
	final static int MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT = 3;

	final private MultitouchHelper.MultitouchHelperHandler _multiTouchLevelUpgradeHandler = new MultitouchHelper.MultitouchHelperHandler(this);

	// Scroll handling variables (calling it "Mouse Wheel" as SCROLL event on Android touch interface refers to moving around a finger on the touch surfaces)
	private final int TOUCH_MOUSE_WHEEL_UNDECIDED = 0;
	private final int TOUCH_MOUSE_WHEEL_ACTIVE = 1;
	private final int TOUCH_MOUSE_WHEEL_NOT_HAPPENING = 3;

	// Both fingers need to have moved up or down to enter the scroll (mouse wheel) mode
	// The difference from the original positions (for both fingers respectively) is averaged and compared to the threshold below
	// The decision to enter scroll mode happens as long as there are two fingers down but we're still undecided,
	// (ie. undecided about whether this is a two finger event OR a third finger will follow OR it is a scroll event)
	private final int MOVE_THRESHOLD_FOR_TOUCH_MOUSE_WHEEL_DECISION = 20;

	private final int MOVE_THRESHOLD_FOR_SEND_TOUCH_MOUSE_WHEEL_EVENT = 30;

	// 0: Undecided for scrolling (mouse wheel)
	// 1: "scrolling"  mode active
	// 2: no "scrolling" (mouse wheel) (decided)
	// Scrolling (mouse wheel) mode is mutually exclusive with the rest of the multi-touch modes,
	// we can either send mouse wheel events or mouse click events in a multitouch session.
	private int _touchMouseWheelDecisionLevel = 0;

	// constructor
	public MultitouchHelper(ScummVM scummvm) {
		_scummvm = scummvm;

		_multitouchMode = false;
		_multitouchLevel =  MULTITOUCH_UNDECIDED;
		_candidateStartOfMultitouchSession = false;

		_touchMouseWheelDecisionLevel = TOUCH_MOUSE_WHEEL_UNDECIDED;
		resetPointers();
	}

	public void resetPointers() {
		_firstPointerId  = -1;
		_secondPointerId = -1;
		_thirdPointerId  = -1;
		_cachedActionEventOnPointer1DownX = -1;
		_cachedActionEventOnPointer1DownY = -1;
		_cachedActionEventOnPointer2DownX = -1;
		_cachedActionEventOnPointer2DownY = -1;
	}

	public boolean isMultitouchMode() {
		return _multitouchMode;
	}
	public int getMultitouchLevel() {
		return _multitouchLevel;
	}

	public int getTouchMouseWheelDecisionLevel() {
		return _touchMouseWheelDecisionLevel;
	}
	public boolean isTouchMouseWheel() { return  getTouchMouseWheelDecisionLevel() == TOUCH_MOUSE_WHEEL_ACTIVE; }

	public void setMultitouchMode(boolean enabledFlg) {
		_multitouchMode = enabledFlg;
	}
	public void setMultitouchLevel(int mtlevel) {
		_multitouchLevel = mtlevel;
	}

	public void setTouchMouseWheelDecisionLevel(int scrlevel) {
		_touchMouseWheelDecisionLevel = scrlevel;
	}

	// TODO Maybe for consistency purposes, maybe sent all (important) UP events that were not sent, when ending a multitouch session?
	public boolean handleMotionEvent(final MotionEvent event) {

		// constants from APIv5:
		// (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT
		//final int pointer = (action & 0xff00) >> 8;

		final int maskedAction = event.getActionMasked();

		int pointerIndex = -1;
		int actionEventX;
		int actionEventY;

		if (maskedAction == MotionEvent.ACTION_DOWN) {
			// start of a multitouch session! one finger down -- this is sent for the first pointer who touches the screen
			resetPointers();
			setMultitouchLevel(MULTITOUCH_UNDECIDED);
			setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_UNDECIDED);
			setMultitouchMode(false);
			_candidateStartOfMultitouchSession = true;
			_multiTouchLevelUpgradeHandler.clear();

			pointerIndex = 0;
			_firstPointerId = event.getPointerId(pointerIndex);
			_cachedActionEventOnPointer1DownX = (int) event.getX(pointerIndex);;
			_cachedActionEventOnPointer1DownY = (int) event.getY(pointerIndex);;
			return false;

		} else if (maskedAction == MotionEvent.ACTION_CANCEL) {
			resetPointers();
			setMultitouchLevel(MULTITOUCH_UNDECIDED);
			setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_UNDECIDED);
			setMultitouchMode(false);
			_multiTouchLevelUpgradeHandler.clear();
			return true;

		} else if (maskedAction == MotionEvent.ACTION_OUTSIDE) {
			return false;

		}

		if (event.getPointerCount() > 1 && event.getPointerCount() < 4) {
			// a multi-touch event
			if (_candidateStartOfMultitouchSession && event.getPointerCount() > 1) {
				_candidateStartOfMultitouchSession = false; // reset this flag
				setMultitouchMode(true);
			}

			if (isMultitouchMode()) {
				if (maskedAction == MotionEvent.ACTION_POINTER_DOWN) {
					pointerIndex = event.getActionIndex();
					if (event.getPointerCount() == 2) {
						_secondPointerId = event.getPointerId(pointerIndex);

						if (getMultitouchLevel() == MULTITOUCH_UNDECIDED) {
							_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT);
							_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT);
							if (pointerIndex != -1) {
								_cachedActionEventOnPointer2DownX = (int) event.getX(pointerIndex);
								_cachedActionEventOnPointer2DownY = (int) event.getY(pointerIndex);
							} else {
								_cachedActionEventOnPointer2DownX = -1;
								_cachedActionEventOnPointer2DownY = -1;
							}
							// Allow for some time before deciding a two finger touch event, since the user might be going for a three finger touch event
							_multiTouchLevelUpgradeHandler.sendMessageDelayed(_multiTouchLevelUpgradeHandler.obtainMessage(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT), _kLevelDecisionDelayMs);
							// Also allow for (less) time to check if this is a two-finger "mouse-wheel" event
							_multiTouchLevelUpgradeHandler.sendMessageDelayed(_multiTouchLevelUpgradeHandler.obtainMessage(MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT), _kTouchMouseWheelDecisionDelayMs);
							// Return as event "handled" here
							// while we wait for the decision to be made for the level of multitouch (two or three)
							//
							return true;
						}
						// Don't return here
						// We want to handle the case whereby we were in multitouch level 2, and we got a new pointer down event with 2 pointers count
						// This is the case where the user keeps one finger down and taps the second finger
						// This behavior should count as multiple right clicks (one for each new "tap" (ACTION_POINTER_DOWN event))
						// for user friendliness / control intuitiveness

					} else  if (event.getPointerCount() == 3) {
						_thirdPointerId = event.getPointerId(pointerIndex);
						if (getMultitouchLevel() == MULTITOUCH_UNDECIDED) {
							setMultitouchLevel(MULTITOUCH_THREE_FINGERS);
							_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT);
							setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_NOT_HAPPENING);
						}
					}
				} else {
					if (event.getPointerCount() == 2) {
						// we prioritize the second pointer/ finger
						pointerIndex = event.findPointerIndex(_secondPointerId);
						if (pointerIndex != -1) {
							actionEventX = (int)event.getX(pointerIndex);
							actionEventY = (int)event.getY(pointerIndex);
						} else {
							actionEventX = -1;
							actionEventY = -1;
						}

						if (getMultitouchLevel() == MULTITOUCH_UNDECIDED) {
							// Fast trigger an ACTION_POINTER_DOWN if:
							//  - we were not yet decided on which level to use
							//    AND either:
							//       - a finger got up (from 3 to 2, shouldn't happen) or
							//       - our main finger (second finger down) moved from cached position
							if (maskedAction == MotionEvent.ACTION_POINTER_UP
							    || (maskedAction == MotionEvent.ACTION_MOVE
							        && (actionEventX != _cachedActionEventOnPointer2DownX
							            || actionEventY != _cachedActionEventOnPointer2DownY))) {

								setMultitouchLevel(MULTITOUCH_TWO_FINGERS);
								_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT);

								// Checking if we can decide on a touch mouse wheel mode session
								int firstPointerIndex = event.findPointerIndex(_firstPointerId);
								int actionEventFirstPointerCoordY = -1;
								if (firstPointerIndex != -1) {
									actionEventFirstPointerCoordY = (int) event.getY(firstPointerIndex);
								}

								if (maskedAction == MotionEvent.ACTION_MOVE) {
									// Decide Scroll (touch mouse wheel) if:
									//  - we were not yet decided on which level to use
									//  - and two fingers are down (but not because we went from 3 to 2)
									//  - and it's a move event
									//  - and the movement distance of both fingers on y axis is around >= MOVE_THRESHOLD_FOR_TOUCH_MOUSE_WHEEL_DECISION
									//         (plus some other qualifying checks to determine significant and similar movement on both fingers)
									// NOTE the movementOfFinger2onY and movementOfFinger1onY gets higher (on subsequent events)
									//     if the user keeps moving their fingers (in the same direction),
									//     since it's in reference to the starting points for the fingers
									int movementOfFinger2onY = actionEventY - _cachedActionEventOnPointer2DownY;
									int movementOfFinger1onY = actionEventFirstPointerCoordY - _cachedActionEventOnPointer1DownY;
									int absMovementOfFinger2onY = Math.abs(movementOfFinger2onY);
									int absMovementOfFinger1onY = Math.abs(movementOfFinger1onY);
									int absDiffOfMovementOfFingersOnY = Math.abs(movementOfFinger2onY - movementOfFinger1onY);

									if (getTouchMouseWheelDecisionLevel() == TOUCH_MOUSE_WHEEL_UNDECIDED
									    && (movementOfFinger2onY > 0 && movementOfFinger1onY > 0) || (movementOfFinger2onY < 0 && movementOfFinger1onY < 0)
									    && absDiffOfMovementOfFingersOnY < MOVE_THRESHOLD_FOR_TOUCH_MOUSE_WHEEL_DECISION ) {

										if ((absMovementOfFinger2onY + absMovementOfFinger1onY) / 2 >=  MOVE_THRESHOLD_FOR_TOUCH_MOUSE_WHEEL_DECISION) {
											setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_ACTIVE);
											_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT);
										} else {
											// ignore this move event but don't forward it (return true as "event handled")
											// there's still potential to be a scroll (touch mouse wheel) event, with accumulated movement
											//
											// Also downgrade the multitouch level to undecided to re-enter this code segment next time
											// (the "countdown" for three-finger touch decision is not resumed)
											setMultitouchLevel(MULTITOUCH_UNDECIDED);
											return true;
										}
									} else {
										setMultitouchLevel(MULTITOUCH_UNDECIDED);
										return true;
									}
								} else {
									setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_NOT_HAPPENING);
									_multiTouchLevelUpgradeHandler.removeMessages(MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT);
								}
								// End of: Checking if we can decide on a touch mouse wheel mode session

								if (getTouchMouseWheelDecisionLevel() != TOUCH_MOUSE_WHEEL_ACTIVE) {
									// send the missing pointer down event first, before sending the actual current move event below
									_scummvm.pushEvent(JE_MULTI,
									                   event.getPointerCount(),
									                   MotionEvent.ACTION_POINTER_DOWN,
									                   actionEventX,
									                   actionEventY,
									                   0, 0);
								}
							}
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
					if ((isTouchMouseWheel()) ) {
						if (maskedAction == MotionEvent.ACTION_MOVE && pointerIndex == event.findPointerIndex(_secondPointerId)) {
							// The co-ordinates sent with the event are the original touch co-ordinates of the first finger
							// The mouse cursor should not move around while touch-mouse-wheel scrolling
							// Also for simplification we only use the move events for the second finger
							// and skip non-significant movements.
							int movementOfFinger2onY = actionEventY - _cachedActionEventOnPointer2DownY;
							if (Math.abs(movementOfFinger2onY) > MOVE_THRESHOLD_FOR_SEND_TOUCH_MOUSE_WHEEL_EVENT) {
								_scummvm.pushEvent((movementOfFinger2onY > 0 ) ? JE_MOUSE_WHEEL_UP : JE_MOUSE_WHEEL_DOWN,
								                   _cachedActionEventOnPointer1DownX,
								                   _cachedActionEventOnPointer1DownY,
								                   1, // This will indicate to the event handling code in native that it comes from touch interface
								                   0,
								                   0, 0);
								_cachedActionEventOnPointer2DownY = actionEventY;
							}
						} // otherwise don't push an event in this case and return true
					} else {
						// arg1 will be the number of fingers down in the MULTI event we send to events.cpp
						// arg2 is the event action
						// arg3 and arg4 are the X,Y coordinates for the "active pointer" index, which is the last finger down
						// (the second in two-fingers touch, or the third in a three-fingers touch mode)
						_scummvm.pushEvent(JE_MULTI,
						                   event.getPointerCount(),
						                   event.getAction(),
						                   actionEventX,
						                   actionEventY,
						                   0, 0);
					}
				}
			}
			return true;

		} else if (event.getPointerCount() >= 4) {
			// ignore if more than 3 fingers down. Mark as multitouch "handled" (return true)
			return true;

		} else if (event.getPointerCount() == 1 && isMultitouchMode() ) {
			// We were already in a Multitouch session, but we're left with one finger down now
			// Keep ignoring events for single pointer until we exit multitouch mode "session"
			// this is to catch the case of being left with only one finger still touching the surface
			// after lifting the rest of the fingers that were touching the surface
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
			super(Looper.getMainLooper());
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
		if ((msg.what == MSG_MT_UPGRADE_TO_LEVEL_3_TIMEDOUT && getMultitouchLevel() == MULTITOUCH_UNDECIDED)
		    || (msg.what == MSG_MT_CHECK_FOR_TOUCH_MOUSE_WHEEL_TIMEDOUT
                        && (getMultitouchLevel() == MULTITOUCH_UNDECIDED
                            || (getMultitouchLevel() == 2 && getTouchMouseWheelDecisionLevel() == TOUCH_MOUSE_WHEEL_UNDECIDED)))) {
			// Either:
			// - window of allowing upgrade to level 3 (three fingers) timed out
			// - or window of allowing time for checking for scroll (touch mouse wheel) timed out
			// decide level 2 (two fingers).
			setMultitouchLevel(MULTITOUCH_TWO_FINGERS);
			setTouchMouseWheelDecisionLevel(TOUCH_MOUSE_WHEEL_NOT_HAPPENING);

			// send the delayed pointer down event
			_scummvm.pushEvent(JE_MULTI,
			                   2,
			                   MotionEvent.ACTION_POINTER_DOWN,
			                   _cachedActionEventOnPointer2DownX,
			                   _cachedActionEventOnPointer2DownY,
			                   0, 0);
		}
	}

	public void clearEventHandler() {
		_multiTouchLevelUpgradeHandler.clear();
	}
}
