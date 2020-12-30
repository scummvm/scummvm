package org.scummvm.scummvm;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.media.AudioManager;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.PointerIcon;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.documentfile.provider.DocumentFile;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.TreeSet;

import static android.content.res.Configuration.KEYBOARD_QWERTY;

public class ScummVMActivity extends Activity implements OnKeyboardVisibilityListener {

	/* Establish whether the hover events are available */
	private static boolean _hoverAvailable;

	private ClipboardManager _clipboardManager;

	private Version _currentScummVMVersion;
	private File _configScummvmFile;
	private File _actualScummVMDataDir;
	private File _possibleExternalScummVMDir;
	private File _usingScummVMSavesDir;
	boolean _externalPathAvailableForReadAccess;
//	private File _usingLogFile;

	// SAF related
	private LinkedHashMap<String, ParcelFileDescriptor> hackyNameToOpenFileDescriptorList;
	public final static int REQUEST_SAF = 50000;

	/**
	 * Ids to identify an external storage read (and write) request.
	 * They are app-defined int constants. The callback method gets the result of the request.
	 * Ie. returned in the Activity's onRequestPermissionsResult()
	 */

	private static final int MY_PERMISSIONS_REQUEST_READ_EXT_STORAGE = 100;
	private static final int MY_PERMISSIONS_REQUEST_WRITE_EXT_STORAGE = 101;
	private static final int MY_PERMISSION_ALL = 110;

	private static final String[] MY_PERMISSIONS_STR_LIST = {
		Manifest.permission.READ_EXTERNAL_STORAGE,
		Manifest.permission.WRITE_EXTERNAL_STORAGE,
	};

	static {
		try {
			MouseHelper.checkHoverAvailable(); // this throws exception if we're on too old version
			_hoverAvailable = true;
		} catch (Throwable t) {
			_hoverAvailable = false;
		}
	}

	//
	// --------------------------------------------------------------------------------------------------------------------------------------------
	// Code for emulated in-app keyboard largely copied
	// from https://github.com/pelya/commandergenius/tree/sdl_android/project
	//
	FrameLayout _videoLayout = null;

	private EditableSurfaceView _main_surface = null;
	private ImageView _toggleKeyboardBtnIcon = null;

	public View _screenKeyboard = null;
	static boolean keyboardWithoutTextInputShown = false;

//	boolean _isPaused = false;
	private InputMethodManager _inputManager = null;

	private final int[][] TextInputKeyboardList =
	{
		{ 0, R.xml.qwerty },
		{ 0, R.xml.qwerty_shift },
		{ 0, R.xml.qwerty_alt },
		{ 0, R.xml.qwerty_alt_shift }
	};

	@Override
	public void onConfigurationChanged(@NonNull Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		if (isHWKeyboardConnected()) {
			hideScreenKeyboard();
		}
	}

	private boolean isHWKeyboardConnected() {
		return getResources().getConfiguration().keyboard == KEYBOARD_QWERTY;
	}

	public boolean isKeyboardOverlayShown() {
		return keyboardWithoutTextInputShown;
	}

	public void showScreenKeyboardWithoutTextInputField(final int keyboard) {
		if (_main_surface != null) {
			_inputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
			if (!keyboardWithoutTextInputShown) {
				keyboardWithoutTextInputShown = true;
				runOnUiThread(new Runnable() {
					public void run() {
						//Log.d(ScummVM.LOG_TAG, "showScreenKeyboardWithoutTextInputField - captureMouse(false)");
						_main_surface.captureMouse(false);
						//_main_surface.showSystemMouseCursor(true);
						if (keyboard == 0) {
							// TODO do we need SHOW_FORCED HERE?
							//_inputManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
							//_inputManager.showSoftInput(_main_surface, InputMethodManager.SHOW_FORCED);

							_inputManager.toggleSoftInputFromWindow(_main_surface.getWindowToken(), InputMethodManager.SHOW_IMPLICIT, InputMethodManager.HIDE_IMPLICIT_ONLY);
							_inputManager.showSoftInput(_main_surface, InputMethodManager.SHOW_IMPLICIT);
							getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
						} else {
							if (_screenKeyboard != null) {
								return;
							}
							class BuiltInKeyboardView extends CustomKeyboardView {

								public boolean shift = false;
								public boolean alt = false;
								public final TreeSet<Integer> stickyKeys = new TreeSet<>();
								public long mEventPressTime = -1;
								public int mKeyRepeatedCount = 0;

								public BuiltInKeyboardView(Context context, android.util.AttributeSet attrs) {
									super(context, attrs);
								}

								public boolean onKeyDown(int key, final KeyEvent event) {
									//Log.d(ScummVM.LOG_TAG, "BuiltInKeyboardView- 001 - onKeyDown()" );
									return false;
								}

								public boolean onKeyUp(int key, final KeyEvent event) {
									//Log.d(ScummVM.LOG_TAG, "BuiltInKeyboardView - 001 - onKeyUp()" );
									return false;
								}

								public int getCompiledMetaState() {
									int retCompiledMetaState = 0;
									// search in the list of currently "ON" sticky keys to set the meta flags
									for (int stickyActiveKeyCode : stickyKeys) {
										switch (stickyActiveKeyCode) {
											case KeyEvent.KEYCODE_SHIFT_LEFT:
												retCompiledMetaState|= KeyEvent.META_SHIFT_LEFT_ON;
												break;
											case KeyEvent.KEYCODE_SHIFT_RIGHT:
												retCompiledMetaState|= KeyEvent.META_SHIFT_RIGHT_ON;
												break;
											case KeyEvent.KEYCODE_CTRL_LEFT:
												retCompiledMetaState|= KeyEvent.META_CTRL_LEFT_ON;
												break;
											case KeyEvent.KEYCODE_CTRL_RIGHT:
												retCompiledMetaState|= KeyEvent.META_CTRL_RIGHT_ON;
												break;
											case KeyEvent.KEYCODE_ALT_LEFT:
												retCompiledMetaState|= KeyEvent.META_ALT_LEFT_ON;
												break;
											case KeyEvent.KEYCODE_ALT_RIGHT:
												retCompiledMetaState|= KeyEvent.META_ALT_RIGHT_ON;
												break;
											case KeyEvent.KEYCODE_META_LEFT:
												retCompiledMetaState|= KeyEvent.META_META_LEFT_ON;
												break;
											case KeyEvent.KEYCODE_META_RIGHT:
												retCompiledMetaState|= KeyEvent.META_META_RIGHT_ON;
												break;
											case KeyEvent.KEYCODE_CAPS_LOCK:
												retCompiledMetaState|= KeyEvent.META_CAPS_LOCK_ON;
												break;
											case KeyEvent.KEYCODE_NUM_LOCK:
												retCompiledMetaState|= KeyEvent.META_NUM_LOCK_ON;
												break;
											case KeyEvent.KEYCODE_SCROLL_LOCK:
												retCompiledMetaState|= KeyEvent.META_SCROLL_LOCK_ON;
												break;
											case KeyEvent.KEYCODE_SYM:
												// TODO Do we have or need a SYM key?
												retCompiledMetaState|= KeyEvent.META_SYM_ON;
												break;
											default: break;
										}
									}

									if (shift && !alt) {
										retCompiledMetaState |= KeyEvent.META_SHIFT_LEFT_ON;
									}
									return retCompiledMetaState;
								}

								public void recheckStickyKeys() {
									// setting sticky keys to their proper on or off state
									// h
									boolean atLeastOneStickyKeyWasChanged = false;
									for (CustomKeyboard.CustomKey k: getKeyboard().getKeys()) {
										if (stickyKeys.contains(k.codes[0]) && !k.on) {
											k.on = true;
											atLeastOneStickyKeyWasChanged = true;
										} else if (!stickyKeys.contains(k.codes[0]) && k.sticky && k.on) {
											k.on = false;
											atLeastOneStickyKeyWasChanged = true;
										}
									}
									if (atLeastOneStickyKeyWasChanged) {
										invalidateAllKeys();
									}
								}

								public void ChangeKeyboard() {
									// Called when bringing up the keyboard
									// or pressing one of the special keyboard keys that change the layout (eg "123...")
									//
									int idx = (shift ? 1 : 0) + (alt ? 2 : 0);
									setKeyboard(new CustomKeyboard(ScummVMActivity.this, TextInputKeyboardList[idx][keyboard]));
									setPreviewEnabled(false);
									setProximityCorrectionEnabled(false);

									// setKeyboard() already invalidates all keys,
									// here we check for our memory of sticky keys state (and any that changed to on and were added to stickyKeys Set)
									recheckStickyKeys();
									//ScummVMActivity.this._scummvm.displayMessageOnOSD ("NEW KEYBOARD LAYOUT: QWERTY"
									//	+ (alt ? " ALT " : "") + (shift? " SHIFT" : ""));
								}

								public void resetEventAndTimestamps() {
									// clear event timestamps and repetition counts
									mEventPressTime = -1;
									mKeyRepeatedCount = -1;
								}
							}

							final BuiltInKeyboardView builtinKeyboard = new BuiltInKeyboardView(ScummVMActivity.this, null);
							builtinKeyboard.setAlpha(0.7f);
							builtinKeyboard.ChangeKeyboard();
							builtinKeyboard.setOnKeyboardActionListener(new CustomKeyboardView.OnKeyboardActionListener() {

								public void onPress(int key) {
									builtinKeyboard.resetEventAndTimestamps();

//									Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - onPress key: " + key );
									if (key == KeyEvent.KEYCODE_BACK) {
										return;
									}

									if (key <= 0) {
										return;
									}

									for (CustomKeyboard.CustomKey k: builtinKeyboard.getKeyboard().getKeys()) {
										if (k.sticky && key == k.codes[0]) {
											return;
										}
									}

									//
									// downTime (long) - The time (in SystemClock.uptimeMillis()) at which this key code originally went down.
									// ** Since this is a down event, this will be the same as getEventTime(). **
									// Note that when chording keys, this value is the down time of the most recently pressed key, which may not be the same physical key of this event.
									// eventTime (long) -  The time (in SystemClock.uptimeMillis()) at which this event happened.
									// TODO update repeat and event time? with
									builtinKeyboard.mEventPressTime =  SystemClock.uptimeMillis();

								}

								public void onRelease(int key) {
//									Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - onRelease key: " + key );
									if (key == KeyEvent.KEYCODE_BACK) {
										builtinKeyboard.setOnKeyboardActionListener(null);
										builtinKeyboard.resetEventAndTimestamps();
										showScreenKeyboardWithoutTextInputField(0); // Hide keyboard
										return;
									}

									// CustomKeyboard.KEYCODE_SHIFT is a special button (negative value)
									// which basically changes the keyboard to a another,"SHIFTED", layout (other keys)
									// In this layout, if it's NOT also an "ALT" layout, keys are assumed to get the LEFT SHIFT modifier by default
									if (key == CustomKeyboard.KEYCODE_SHIFT) {
										builtinKeyboard.shift = !builtinKeyboard.shift;
										builtinKeyboard.ChangeKeyboard();
										builtinKeyboard.resetEventAndTimestamps();
										return;
									}

									if (key == CustomKeyboard.KEYCODE_ALT) {
										builtinKeyboard.alt = !builtinKeyboard.alt;
										if (!builtinKeyboard.alt) {
											builtinKeyboard.shift = false;
										}
										builtinKeyboard.ChangeKeyboard();
										builtinKeyboard.resetEventAndTimestamps();
										return;
									}

									if (key <= 0) {
										builtinKeyboard.resetEventAndTimestamps();
										return;
									}

									//
									// TODO - Probably remove keys like caps lock, scroll lock, num lock, print etc...
									//
									for (CustomKeyboard.CustomKey k: builtinKeyboard.getKeyboard().getKeys()) {
										if (k.sticky && key == k.codes[0]) {
											if (builtinKeyboard.stickyKeys.contains(key)) {
												// if it was "remembered" (in stickyKeys set) as ON
												// (and it off by removing them from stickyKeys set)
												builtinKeyboard.stickyKeys.remove(key);
											} else {
												// if it was "remembered" (in stickyKeys set) as OFF
												// (turn it on by adding them to stickyKeys set)
												builtinKeyboard.stickyKeys.add(key);
											}
											builtinKeyboard.recheckStickyKeys();
											builtinKeyboard.resetEventAndTimestamps();
											return;
										}
									}

									int compiledMetaState = builtinKeyboard.getCompiledMetaState();

									//boolean shifted = false;
									if (key > 100000) {
										key -= 100000;
										//shifted = true;
										compiledMetaState |= KeyEvent.META_SHIFT_LEFT_ON;
									}

 									// The repeat argument (int) is:
									// A repeat count for down events (> 0 if this is after the  initial down)
									// or event count for multiple events.
									KeyEvent compiledKeyEvent = new KeyEvent(builtinKeyboard.mEventPressTime,
										SystemClock.uptimeMillis(),
										KeyEvent.ACTION_UP,
										key,
										builtinKeyboard.mKeyRepeatedCount,
										compiledMetaState);

									_main_surface.dispatchKeyEvent(compiledKeyEvent);
									builtinKeyboard.resetEventAndTimestamps();

									// Excluding the CAPS LOCK NUM LOCK AND SCROLL LOCK keys,
									// clear the state of all other sticky keys that are used in a key combo
									// when we reach this part of the code
									if (builtinKeyboard.stickyKeys.size() > 0) {
										HashSet<Integer> stickiesToReleaseSet = new HashSet<>();
										for (int tmpKeyCode : builtinKeyboard.stickyKeys) {
											if (tmpKeyCode != KeyEvent.KEYCODE_CAPS_LOCK
												&& tmpKeyCode != KeyEvent.KEYCODE_NUM_LOCK
												&& tmpKeyCode != KeyEvent.KEYCODE_SCROLL_LOCK) {
												stickiesToReleaseSet.add(tmpKeyCode);
											}
										}
										if (stickiesToReleaseSet.size() > 0) {
											builtinKeyboard.stickyKeys.removeAll(stickiesToReleaseSet);
											builtinKeyboard.recheckStickyKeys();
										}
									}
								}

								public void onText(CharSequence p1) {}

								// TODO - "Swipe" behavior does not seem to work currently. Should we support it?
								public void swipeLeft() {
									//Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - swipeLeft");
								}

								public void swipeRight() {
									//Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - swipeRight" );
								}

								public void swipeDown() {
									//Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - swipeDown" );
								}

								public void swipeUp() {
									//Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - swipeUp ");
								}
								public void onKey(int key, int[] keysAround) {
//									Log.d(ScummVM.LOG_TAG, "SHOW KEYBOARD - 001 - onKey key: " + key );
									if (builtinKeyboard.mEventPressTime == -1) {
										return;
									}

									if (builtinKeyboard.mKeyRepeatedCount < Integer.MAX_VALUE) {
										++builtinKeyboard.mKeyRepeatedCount;
									}
									int compiledMetaState = builtinKeyboard.getCompiledMetaState();

									// keys with keyCode greater than 100000, should be submitted with a LEFT_SHIFT_ modifier (and decreased by 100000 to get their proper code)
									if (key > 100000) {
										key -= 100000;
										compiledMetaState |= KeyEvent.META_SHIFT_LEFT_ON;
									}
									// update the eventTime after the above check for first time "hit"
									KeyEvent compiledKeyEvent = new KeyEvent(builtinKeyboard.mEventPressTime,
										SystemClock.uptimeMillis(),
										KeyEvent.ACTION_DOWN,
										key,
										builtinKeyboard.mKeyRepeatedCount,
										compiledMetaState);

									_main_surface.dispatchKeyEvent(compiledKeyEvent);
								}
							});

							_screenKeyboard = builtinKeyboard;
							// TODO better to have specific dimensions in dp and not adjusted to parent
							//		it may resolve the issue of resizing the keyboard wrongly (smaller) when returning to the suspended Activity in low resolution
							FrameLayout.LayoutParams sKeyboardLayout = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);

							_videoLayout.addView(_screenKeyboard, sKeyboardLayout);
							_videoLayout.bringChildToFront(_screenKeyboard);
						}
					}
				});
			} else {
				keyboardWithoutTextInputShown = false;
				runOnUiThread(new Runnable() {
					public void run() {
						if (_screenKeyboard != null ) {
							_videoLayout.removeView(_screenKeyboard);
							_screenKeyboard = null;
						}
						getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
						// TODO do we need this instead?
						// _inputManager.hideSoftInputFromWindow(_main_surface.getWindowToken(), 0);
						_inputManager.hideSoftInputFromWindow(_main_surface.getWindowToken(), InputMethodManager.HIDE_IMPLICIT_ONLY);

						DimSystemStatusBar.get().dim(_videoLayout);
						//DimSystemStatusBar.get().dim(_main_surface);
						//Log.d(ScummVM.LOG_TAG, "showScreenKeyboardWithoutTextInputField - captureMouse(true)");
						_main_surface.captureMouse(true);
						//_main_surface.showSystemMouseCursor(false);
					}
				});
			}
			// TODO Do we need to inform native ScummVM code of keyboard shown state?
//			_main_surface.nativeScreenKeyboardShown( keyboardWithoutTextInputShown ? 1 : 0 );
		}
	}

	public void showScreenKeyboard() {
		final boolean bGlobalsCompatibilityHacksTextInputEmulatesHwKeyboard = true;
		final int dGlobalsTextInputKeyboard = 1;

		if (isHWKeyboardConnected()) {
			return;
		}

		if (_main_surface != null) {

			if (bGlobalsCompatibilityHacksTextInputEmulatesHwKeyboard) {
				showScreenKeyboardWithoutTextInputField(dGlobalsTextInputKeyboard);
				//Log.d(ScummVM.LOG_TAG, "showScreenKeyboard - captureMouse(false)");
				_main_surface.captureMouse(false);
				//_main_surface.showSystemMouseCursor(true);
				return;
			}
			//Log.d(ScummVM.LOG_TAG, "showScreenKeyboard: YOU SHOULD NOT SEE ME!!!");

//			// TODO redundant ?
//			if (_screenKeyboard != null) {
//				return;
//			}
//
		}
	}

	public void hideScreenKeyboard() {

		final int dGlobalsTextInputKeyboard = 1;
		if (_main_surface != null) {
			if (keyboardWithoutTextInputShown) {
				showScreenKeyboardWithoutTextInputField(dGlobalsTextInputKeyboard);
				//Log.d(ScummVM.LOG_TAG, "hideScreenKeyboard - captureMouse(true)");
				_main_surface.captureMouse(true);
				//_main_surface.showSystemMouseCursor(false);
			}
		}
	}

	public void toggleScreenKeyboard() {
		if (isScreenKeyboardShown()) {
			hideScreenKeyboard();
		} else {
			showScreenKeyboard();
		}
	}

	public boolean isScreenKeyboardShown() {
		return _screenKeyboard != null;
	}

	public View getScreenKeyboard() {
		return _screenKeyboard;
	}

//	public View getMainSurfaceView() {
//		return _main_surface;
//	}

	//
	// END OF new screenKeyboardCode
	// ---------------------------------------------------------------------------------------------------------------------------
	//

	public final View.OnClickListener keyboardBtnOnClickListener = new View.OnClickListener() {
		@Override
		public void onClick(View v) {
			runOnUiThread(new Runnable() {
				public void run() {
					toggleScreenKeyboard();
				}
			});
		}
	};


	private class MyScummVM extends ScummVM {

		public MyScummVM(SurfaceHolder holder, final MyScummVMDestroyedCallback destroyedCallback) {
			super(ScummVMActivity.this.getAssets(), holder, destroyedCallback);
		}

		@Override
		protected void getDPI(float[] values) {
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			values[0] = metrics.xdpi;
			values[1] = metrics.ydpi;
		}

		@Override
		protected void displayMessageOnOSD(final String msg) {
			if (msg != null) {
				runOnUiThread(new Runnable() {
					public void run() {
						Toast.makeText(ScummVMActivity.this, msg, Toast.LENGTH_SHORT).show();
					}
				});
			}
		}

		@Override
		protected void openUrl(String url) {
			startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
		}

		@Override
		protected boolean hasTextInClipboard() {
			final android.content.ClipData clip = _clipboardManager.getPrimaryClip();

			return (clip != null
			        && clip.getItemCount() > 0
			        && clip.getItemAt(0).getText() != null);
		}

		@Override
		protected String getTextFromClipboard() {
			// based on: https://stackoverflow.com/q/37196571
			final android.content.ClipData clip = _clipboardManager.getPrimaryClip();

			if (clip == null
			    || clip.getItemCount() == 0
			    || clip.getItemCount() > 0 && clip.getItemAt(0).getText() == null) {
				return null;
			}

			return  clip.getItemAt(0).getText().toString();
		}

		@Override
		protected boolean setTextInClipboard(String textStr) {
			final android.content.ClipData clip = android.content.ClipData.newPlainText("ScummVM clip", textStr);
			_clipboardManager.setPrimaryClip(clip);
			return true;
		}

		@Override
		protected boolean isConnectionLimited() {
			// The WIFI Service must be looked up on the Application Context or memory will leak on devices < Android N (According to Android Studio warning)
			WifiManager wifiMgr = (WifiManager)getApplicationContext().getSystemService(Context.WIFI_SERVICE);
			if (wifiMgr != null && wifiMgr.isWifiEnabled()) {
				WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
				return (wifiInfo == null || wifiInfo.getNetworkId() == -1); //WiFi is on, but it's not connected to any network
			}
			return true;
		}

		@Override
		protected void setWindowCaption(final String caption) {
			runOnUiThread(new Runnable() {
				public void run() {
					setTitle(caption);
				}
			});
		}

		@Override
		protected void showVirtualKeyboard(final boolean enable) {
			runOnUiThread(new Runnable() {
				public void run() {
					//showKeyboard(enable);
					if (enable) {
						showScreenKeyboard();
					} else {
						hideScreenKeyboard();
					}
				}
			});
		}

		@Override
		protected void showKeyboardControl(final boolean enable) {
			runOnUiThread(new Runnable() {
				public void run() {
					showToggleKeyboardBtnIcon(enable);
				}
			});
		}

		@Override
		protected String[] getSysArchives() {
			Log.d(ScummVM.LOG_TAG, "Adding to Search Archive: " + _actualScummVMDataDir.getPath());
			if (_externalPathAvailableForReadAccess) {
				Log.d(ScummVM.LOG_TAG, "Adding to Search Archive: " + _possibleExternalScummVMDir.getPath());
				return new String[]{_actualScummVMDataDir.getPath(), _possibleExternalScummVMDir.getPath()};
			} else return new String[]{_actualScummVMDataDir.getPath()};
		}

		@Override
		protected String[] getAllStorageLocations() {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
			    && (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
			        || checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED)
			) {
				requestPermissions(MY_PERMISSIONS_STR_LIST, MY_PERMISSION_ALL);
			} else {
				return ExternalStorage.getAllStorageLocations(getApplicationContext()).toArray(new String[0]);
			}
			return new String[0]; // an array of zero length
		}

		@Override
		protected String[] getAllStorageLocationsNoPermissionRequest() {
			if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M
				|| checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
			) {
				return ExternalStorage.getAllStorageLocations(getApplicationContext()).toArray(new String[0]);
			}
			// TODO we might be able to return something even if READ_EXTERNAL_STORAGE was not granted
			//      but for now, just return nothing
			return new String[0]; // an array of zero length
		}

		// In this method we first try the old method for creating directories (mkdirs())
		// That should work with app spaces but will probably have issues with external physical "secondary" storage locations
		// (eg user SD Card) on some devices, anyway.
		@Override
		protected boolean createDirectoryWithSAF(String dirPath) {
			final boolean[] retRes = {false};

			Log.d(ScummVM.LOG_TAG, "Attempt to create folder on path: " + dirPath);
			File folderToCreate = new File (dirPath);
//			if (folderToCreate.canWrite()) {
//				Log.d(ScummVM.LOG_TAG, "This file node has write permission!" + dirPath);
//			}
//
//			if (folderToCreate.canRead()) {
//				Log.d(ScummVM.LOG_TAG, "This file node has read permission!" + dirPath);
//
//			}
//
//			if (folderToCreate.getParentFile() != null) {
//				if( folderToCreate.getParentFile().canWrite()) {
//					Log.d(ScummVM.LOG_TAG, "The parent of this node permits write operation!" + dirPath);
//				}
//
//				if (folderToCreate.getParentFile().canRead()) {
//					Log.d(ScummVM.LOG_TAG, "The parent of this node permits read operation!" + dirPath);
//
//				}
//			}

			if (folderToCreate.mkdirs()) {
				Log.d(ScummVM.LOG_TAG, "Folder created with the simple mkdirs() command!");
			} else {
				Log.d(ScummVM.LOG_TAG, "Folder creation with mkdirs() failed!");
				if (getStorageAccessFrameworkTreeUri() == null) {
					requestStorageAccessFramework(dirPath);
					Log.d(ScummVM.LOG_TAG, "Requested Storage Access via Storage Access Framework!");
				} else {
					Log.d(ScummVM.LOG_TAG, "Already requested Storage Access (Storage Access Framework) in the past (share prefs saved)!");
				}

				if (canWriteFile(folderToCreate, true)) {
					// TODO we should only need the callback if we want to do something with the file descriptor
					//  (the writeFile will close it afterwards if keepFileDescriptorOpen is false)
					Log.d(ScummVM.LOG_TAG, "(post SAF request) Writing is possible for this directory node");
					writeFile(folderToCreate, true, false, new MyWriteFileCallback() {
						@Override
						public void handle(Boolean created, String hackyFilename) {
							//Log.d(ScummVM.LOG_TAG, "Via callback: file operation success: " + created);
							retRes[0] = created;
						}
					});
				} else {
					Log.d(ScummVM.LOG_TAG, "(post SAF request) Error - writing is still not possible for this directory node");

				}
			}

//			// debug purpose
//			if (folderToCreate.canWrite()) {
//				// This is expected to return false here (since we don't check via SAF here)
//				Log.d(ScummVM.LOG_TAG, "(post SAF access) We can write in folder:" + dirPath);
//			}
//			if (folderToCreate.canRead()) {
//				// This will probably return true (at least for Android 28 and below)
//				Log.d(ScummVM.LOG_TAG, "(post SAF access) We can read from folder:" + dirPath);
//
//			}

			return retRes[0];
		}

		@Override
		protected String createFileWithSAF(String filePath) {
			final String[] retResStr = {""};
			File fileToCreate = new File (filePath);

			Log.d(ScummVM.LOG_TAG, "Attempting file creation for: " + filePath);

			// normal (no SAF) file create attempt
			boolean needToGoThroughSAF = false;
			try {
				if (fileToCreate.exists() || !fileToCreate.createNewFile()) {
					Log.d(ScummVM.LOG_TAG, "The file already exists!");
					// already existed
				} else {
					Log.d(ScummVM.LOG_TAG, "An empty file was created!");

				}
			} catch(Exception e) {
				//e.printStackTrace();
				needToGoThroughSAF = true;
			}

			if (needToGoThroughSAF) {
				Log.d(ScummVM.LOG_TAG, "File creation with createNewFile() failed!");
				if (getStorageAccessFrameworkTreeUri() == null) {
					requestStorageAccessFramework(filePath);
					Log.d(ScummVM.LOG_TAG, "Requested Storage Access via Storage Access Framework!");
				}

				if (canWriteFile(fileToCreate, false)) {
					// TODO we should only need the callback if we want to do something with the file descriptor
					//      (the writeFile will close it afterwards if keepFileDescriptorOpen is false)
					//      we need the fileDescriptor open for the native to continue the write operation
					Log.d(ScummVM.LOG_TAG, "(post SAF request check) File writing should be possible");
					writeFile(fileToCreate, false, true, new MyWriteFileCallback() {
						@Override
						public void handle(Boolean created, String hackyFilename) {
							//Log.d(ScummVM.LOG_TAG, "Via callback: file operation success: " + created + " :: " + hackyFilename);
							if (created) {
								retResStr[0] = hackyFilename;
							} else {
								retResStr[0] = "";
							}
						}
					});
				} else {
					Log.e(ScummVM.LOG_TAG, "(post SAF request) Error - writing is still not possible for this directory node");
				}
			}
			return retResStr[0];
		}

		@Override
		protected void closeFileWithSAF(String hackyFileName) {
			if (hackyNameToOpenFileDescriptorList.containsKey(hackyFileName)) {
				ParcelFileDescriptor openFileDescriptor = hackyNameToOpenFileDescriptorList.get(hackyFileName);

				Log.d(ScummVM.LOG_TAG, "Closing file descriptor for " + hackyFileName);
				if (openFileDescriptor != null) {
					try {
						openFileDescriptor.close();
					} catch (IOException e) {
						Log.e(ScummVM.LOG_TAG, e.getMessage());
						e.printStackTrace();
					}
				}
				hackyNameToOpenFileDescriptorList.remove(hackyFileName);
			}
		}

		// TODO do we also need SAF enabled methods for deletion (file/folder) and reading (for files), listing of files (for folders)?
	}

	private MyScummVM _scummvm;
	private ScummVMEventsBase _events;
	private MouseHelper _mouseHelper;
	private Thread _scummvm_thread;

	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		hackyNameToOpenFileDescriptorList = new LinkedHashMap<>();

		hideSystemUI();

		_videoLayout = new FrameLayout(this);
		SetLayerType.get().setLayerType(_videoLayout);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(_videoLayout);
		_videoLayout.setFocusable(true);
		_videoLayout.setFocusableInTouchMode(true);
		_videoLayout.requestFocus();

		_main_surface = new EditableSurfaceView(this);
		SetLayerType.get().setLayerType(_main_surface);

		_videoLayout.addView(_main_surface, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));

		_toggleKeyboardBtnIcon = new ImageView(this);
		_toggleKeyboardBtnIcon.setImageResource(R.drawable.ic_action_keyboard);
		FrameLayout.LayoutParams keybrdBtnlayout = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.TOP | Gravity.END);
		keybrdBtnlayout.setMarginEnd(15);
		keybrdBtnlayout.topMargin = 15;
		keybrdBtnlayout.rightMargin = 15;
		_videoLayout.addView(_toggleKeyboardBtnIcon, keybrdBtnlayout);
		_videoLayout.bringChildToFront(_toggleKeyboardBtnIcon);

		_main_surface.setFocusable(true);
		_main_surface.setFocusableInTouchMode(true);
		_main_surface.requestFocus();

		//Log.d(ScummVM.LOG_TAG, "onCreate - captureMouse(true)");
		//_main_surface.captureMouse(true, true);
		//_main_surface.showSystemMouseCursor(false);

		// TODO is this redundant since we call hideSystemUI() ?
		DimSystemStatusBar.get().dim(_videoLayout);

		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		// TODO needed?
		takeKeyEvents(true);

		_clipboardManager = (android.content.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);

		// REMOVED: Since getFilesDir() is guaranteed to exist, getFilesDir().mkdirs() might be related to crashes in Android version 9+ (Pie or above, API 28+)!

		// REMOVED: Setting savePath to Environment.getExternalStorageDirectory() + "/ScummVM/Saves/"
		//                            so that it will be in persistent external storage and not deleted on uninstall
		//                            This has the issue for external storage being unavailable on some devices
		//                            Is this persistence really important considering that Android does not really support it anymore
		//                               Exceptions (storage locations that are persisted or methods to access files across apps) are:
		//                                 - shareable media files (images, audio, video)
		//                                 - files stored with Storage Access Framework (SAF) which requires user interaction with FilePicker)
		//          Original fallback was getDir()
		//                            so app's internal space (which would be deleted on uninstall) was set as WORLD_READABLE which is no longer supported in newer versions of Android API
		//                            In newer APIs we can set that path as Context.MODE_PRIVATE which is the default - but this makes the files inaccessible to other apps

		_scummvm = new MyScummVM(_main_surface.getHolder(), new MyScummVMDestroyedCallback() {
		                                                        @Override
		                                                        public void handle(int exitResult) {
		                                                        	Log.d(ScummVM.LOG_TAG, "Via callback: ScummVM native terminated with code: " + exitResult);
		                                                        	// call onDestroy()
		                                                        	finish();
		                                                        }
		                                                    });

		// Currently in release builds version string does not contain the revision info
		// but in debug builds (daily builds) this should be there (see base/internal_version_h)
		_currentScummVMVersion = new Version(_scummvm.getInstallingScummVMVersionInfo());
		Log.d(ScummVM.LOG_TAG, "Current ScummVM version launching is: " + _currentScummVMVersion.getDescription() + " (" + _currentScummVMVersion.get() + ")");
		//
		// seekAndInitScummvmConfiguration() returns false if something went wrong
		// when initializing configuration (or when seeking and trying to use an existing ini file) for ScummVM
		if (!seekAndInitScummvmConfiguration()) {
			Log.e(ScummVM.LOG_TAG, "Error while trying to find and/or initialize ScummVM configuration file!");
			// in fact in all the cases where we return false, we also called finish()
		} else {
			// We should have a valid path to a configuration file here

			// Start ScummVM
//			Log.d(ScummVM.LOG_TAG, "CONFIG: " +  _configScummvmFile.getPath());
//			Log.d(ScummVM.LOG_TAG, "PATH: " +  _actualScummVMDataDir.getPath());
//			Log.d(ScummVM.LOG_TAG, "LOG: " +  _usingLogFile.getPath());
//			Log.d(ScummVM.LOG_TAG, "SAVEPATH: " +  _usingScummVMSavesDir.getPath());

			// TODO log file setting via "--logfile=" + _usingLogFile.getPath() causes crash
			//      probably because this option is specific to SDL_BACKEND (see: base/commandLine.cpp)
			_scummvm.setArgs(new String[]{
				"ScummVM",
				"--config=" + _configScummvmFile.getPath(),
				"--path=" + _actualScummVMDataDir.getPath(),
				"--savepath=" + _usingScummVMSavesDir.getPath()
			});

			Log.d(ScummVM.LOG_TAG, "Hover available: " + _hoverAvailable);
			_mouseHelper = null;
			if (_hoverAvailable) {
				_mouseHelper = new MouseHelper(_scummvm);
//				_mouseHelper.attach(_main_surface);
			}

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
				_events = new ScummVMEventsModern(this, _scummvm, _mouseHelper);
			} else {
				_events = new ScummVMEventsBase(this, _scummvm, _mouseHelper);
			}

			// On screen button listener
			//findViewById(R.id.show_keyboard).setOnClickListener(keyboardBtnOnClickListener);
			_toggleKeyboardBtnIcon.setOnClickListener(keyboardBtnOnClickListener);

			// Keyboard visibility listener - mainly to hide system UI if keyboard is shown and we return from Suspend to the Activity
			setKeyboardVisibilityListener(this);

			_main_surface.setOnKeyListener(_events);
			_main_surface.setOnTouchListener(_events);
			if (_mouseHelper != null) {
				_main_surface.setOnHoverListener(_mouseHelper);
			}

			_scummvm_thread = new Thread(_scummvm, "ScummVM");
			_scummvm_thread.start();
		}
	}

	@Override
	public void onStart() {
//		Log.d(ScummVM.LOG_TAG, "onStart");

		super.onStart();
	}

	@Override
	public void onResume() {
//		Log.d(ScummVM.LOG_TAG, "onResume");

//		_isPaused = false;

		super.onResume();

		if (_scummvm != null)
			_scummvm.setPause(false);
		//_main_surface.showSystemMouseCursor(false);
		//Log.d(ScummVM.LOG_TAG, "onResume - captureMouse(true)");
		_main_surface.captureMouse(true);
	}

	@Override
	public void onPause() {
//		Log.d(ScummVM.LOG_TAG, "onPause");

//		_isPaused = true;

		super.onPause();

		if (_scummvm != null)
			_scummvm.setPause(true);
		//_main_surface.showSystemMouseCursor(true);
		//Log.d(ScummVM.LOG_TAG, "onPause - captureMouse(false)");
		_main_surface.captureMouse(false);

	}

	@Override
	public void onStop() {
//		Log.d(ScummVM.LOG_TAG, "onStop");

		super.onStop();
	}

	@Override
	public void onDestroy() {
//		Log.d(ScummVM.LOG_TAG, "onDestroy");

		super.onDestroy();

		// close any open file descriptors due to the SAF code
		for (String hackyFileName : hackyNameToOpenFileDescriptorList.keySet()) {
			Log.d(ScummVM.LOG_TAG, "Destroy: Closing file descriptor for " + hackyFileName);

			ParcelFileDescriptor openFileDescriptor = hackyNameToOpenFileDescriptorList.get(hackyFileName);

			if (openFileDescriptor != null) {
				try {
					openFileDescriptor.close();
				} catch (IOException e) {
					Log.e(ScummVM.LOG_TAG, e.getMessage());
					e.printStackTrace();
				}
			}
		}
		hackyNameToOpenFileDescriptorList.clear();

		if (_events != null) {
			_events.clearEventHandler();
			_events.sendQuitEvent();

			try {
				// 1s timeout
				_scummvm_thread.join(1000);
			} catch (InterruptedException e) {
				Log.i(ScummVM.LOG_TAG, "Error while joining ScummVM thread", e);
			}

			_scummvm = null;
		}

		if (isScreenKeyboardShown()) {
			hideScreenKeyboard();
		}
		showToggleKeyboardBtnIcon(false);
	}


	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		if (requestCode == MY_PERMISSION_ALL) {
			int numOfReqPermsGranted = 0;
			// If request is cancelled, the result arrays are empty.
			if (grantResults.length > 0) {
				for (int iterGrantResult: grantResults) {
					if (iterGrantResult == PackageManager.PERMISSION_GRANTED) {
						Log.i(ScummVM.LOG_TAG, permissions[0] + " permission was granted at Runtime");
						++numOfReqPermsGranted;
					} else {
						Log.i(ScummVM.LOG_TAG, permissions[0] + " permission was denied at Runtime");
					}
				}
			}

			if (numOfReqPermsGranted != grantResults.length) {
				// permission denied! We won't be able to make use of functionality depending on this permission.
				Toast.makeText(this, "Until permission is granted, some storage locations may be inaccessible for r/w!", Toast.LENGTH_SHORT)
					.show();
			}
		} else if (requestCode == MY_PERMISSIONS_REQUEST_READ_EXT_STORAGE) {
			// If request is cancelled, the result arrays are empty.
			if (grantResults.length > 0
				&& grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				// permission was granted
				Log.i(ScummVM.LOG_TAG, "Read External Storage permission was granted at Runtime");
			} else {
				// permission denied! We won't be able to make use of functionality depending on this permission.
				Toast.makeText(this, "Until permission is granted, some storage locations may be inaccessible!", Toast.LENGTH_SHORT)
					.show();
			}
		} else if (requestCode == MY_PERMISSIONS_REQUEST_WRITE_EXT_STORAGE) {
			// If request is cancelled, the result arrays are empty.
			if (grantResults.length > 0
				&& grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				// permission was granted
				Log.i(ScummVM.LOG_TAG, "Write External Storage permission was granted at Runtime");
			} else {
				// permission denied! We won't be able to make use of functionality depending on this permission.
				Toast.makeText(this, "Until permission is granted, it might be impossible to write to some locations!", Toast.LENGTH_SHORT)
					.show();
			}
		}
	}


	@Override
	public boolean onTrackballEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTrackballEvent(e);

		return false;
	}

	@Override
	public boolean onGenericMotionEvent(final MotionEvent e) {
		if (_events != null)
			return _events.onGenericMotionEvent(e);

		return false;
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			hideSystemUI();
		}
//			showSystemMouseCursor(false);
//		} else {
//			showSystemMouseCursor(true);
//		}
	}

	// TODO setSystemUiVisibility is introduced in API 11 and deprecated in API 30 - When we move to API 30 we will have to replace this code
	//	https://developer.android.com/training/system-ui/immersive.html#java
	//
	//  The code sample in the url below contains code to switch between immersive and default mode
	//	https://github.com/android/user-interface-samples/tree/master/AdvancedImmersiveMode
	//  We could do something similar by making it a Global UI option.
	@TargetApi(Build.VERSION_CODES.KITKAT)
	private void hideSystemUI() {
		// Enables regular immersive mode.
		// For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
		// Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
		View decorView = getWindow().getDecorView();
		decorView.setSystemUiVisibility(
			View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
				// Set the content to appear under the system bars so that the
				// content doesn't resize when the system bars hide and show.
				| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
				// Hide the nav bar and status bar
				| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_FULLSCREEN);
	}

//	// Shows the system bars by removing all the flags
//	// except for the ones that make the content appear under the system bars.
//	@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
//	private void showSystemUI() {
//		View decorView = getWindow().getDecorView();
//		decorView.setSystemUiVisibility(
//		    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
//		    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
//		    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
//	}

//	// Show or hide the Android keyboard.
//	// Called by the override of showVirtualKeyboard()
//	@TargetApi(Build.VERSION_CODES.CUPCAKE)
//	private void showKeyboard(boolean show) {
//		//SurfaceView main_surface = findViewById(R.id.main_surface);
//		if (_main_surface != null) {
//
//			InputMethodManager imm = (InputMethodManager)
//				getSystemService(INPUT_METHOD_SERVICE);
//
//			if (show) {
//				imm.showSoftInput(_main_surface, InputMethodManager.SHOW_IMPLICIT);
//			} else {
//				imm.hideSoftInputFromWindow(_main_surface.getWindowToken(), InputMethodManager.HIDE_IMPLICIT_ONLY);
//			}
//		}
//	}
//
//	// Toggle showing or hiding the virtual keyboard.
//	// Called by keyboardBtnOnClickListener()
//	@TargetApi(Build.VERSION_CODES.CUPCAKE)
//	private void toggleKeyboard() {
//		//SurfaceView main_surface = findViewById(R.id.main_surface);
//		if (_main_surface != null ) {
//			InputMethodManager imm = (InputMethodManager)
//				getSystemService(INPUT_METHOD_SERVICE);
//
//			imm.toggleSoftInputFromWindow(_main_surface.getWindowToken(),
//				InputMethodManager.SHOW_IMPLICIT,
//				InputMethodManager.HIDE_IMPLICIT_ONLY);
//		}
//	}

	// Show or hide the semi-transparent keyboard btn (which is used to explicitly bring up the android keyboard).
	// Called by the override of showKeyboardControl()
	private void showToggleKeyboardBtnIcon(boolean show) {
		//ImageView keyboardBtn = findViewById(R.id.show_keyboard);
		if (_toggleKeyboardBtnIcon != null ) {
			if (show) {
				_toggleKeyboardBtnIcon.setVisibility(View.VISIBLE);
			} else {
				_toggleKeyboardBtnIcon.setVisibility(View.GONE);
			}
		}
	}

	// Listener to check for keyboard visibility changes
	// https://stackoverflow.com/a/36259261
	private void setKeyboardVisibilityListener(final OnKeyboardVisibilityListener onKeyboardVisibilityListener) {
		final View parentView = ((ViewGroup) findViewById(android.R.id.content)).getChildAt(0);
		if (parentView != null) {
			parentView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {

				private boolean alreadyOpen;
				private final int defaultKeyboardHeightDP = 100;
				private final int EstimatedKeyboardDP = defaultKeyboardHeightDP + (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP ? 48 : 0);
				private final Rect rect = new Rect();

				@TargetApi(Build.VERSION_CODES.CUPCAKE)
				@Override
				public void onGlobalLayout() {
					int estimatedKeyboardHeight = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, EstimatedKeyboardDP, parentView.getResources().getDisplayMetrics());
					parentView.getWindowVisibleDisplayFrame(rect);
					int heightDiff = parentView.getRootView().getHeight() - (rect.bottom - rect.top);
					boolean isShown = heightDiff >= estimatedKeyboardHeight;

					if (isShown == alreadyOpen) {
						Log.i(ScummVM.LOG_TAG, "Keyboard state:: ignoring global layout change...");
						return;
					}
					alreadyOpen = isShown;
					onKeyboardVisibilityListener.onVisibilityChanged(isShown);
				}
			});
		}
	}

	@Override
	public void onVisibilityChanged(boolean visible) {
//		Toast.makeText(HomeActivity.this, visible ? "Keyboard is active" : "Keyboard is Inactive", Toast.LENGTH_SHORT).show();
		hideSystemUI();
	}

	// Auxiliary function to overwrite a file (used for overwriting the scummvm.ini file with an existing other one)
	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	private static void copyFileUsingStream(File source, File dest) throws IOException {
		try (InputStream is = new FileInputStream(source); OutputStream os = new FileOutputStream(dest)) {
			copyStreamToStream(is, os);
		}
	}

	private static void copyStreamToStream(InputStream is, OutputStream os) throws IOException {
		byte[] buffer = new byte[1024];
		int length;
		while ((length = is.read(buffer)) > 0) {
			os.write(buffer, 0, length);
		}
	}

	/**
	 * Auxiliary function to read our ini configuration file
	 * Code is from https://stackoverflow.com/a/41084504
	 * returns The sections of the ini file as a Map of the header Strings to a Properties object (the key=value list of each section)
	 */
	@TargetApi(Build.VERSION_CODES.GINGERBREAD)
	private static Map<String, Properties> parseINI(Reader reader) throws IOException {
		final HashMap<String, Properties> result = new HashMap<>();
		new Properties() {

			private Properties section;

			@Override
			public Object put(Object key, Object value) {
				String header = (key + " " + value).trim();
				if (header.startsWith("[") && header.endsWith("]"))
					return result.put(header.substring(1, header.length() - 1),
						section = new Properties());
				else
					return section.put(key, value);
			}

		}.load(reader);
		return result;
	}

	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	private static String getVersionInfoFromScummvmConfiguration(String fullIniFilePath) {
		try (BufferedReader bufferedReader = new BufferedReader(new FileReader(fullIniFilePath))) {
			Map<String, Properties> parsedIniMap = parseINI(bufferedReader);
			if (!parsedIniMap.isEmpty()
			    && parsedIniMap.containsKey("scummvm")
			    && parsedIniMap.get("scummvm") != null) {
				return parsedIniMap.get("scummvm").getProperty("versioninfo", "");
			}
		} catch (IOException ignored) {
		} catch (NullPointerException ignored) {
		}
		return "";
	}

	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	private static String getSavepathInfoFromScummvmConfiguration(String fullIniFilePath) {
		try (BufferedReader bufferedReader = new BufferedReader(new FileReader(fullIniFilePath))) {
			Map<String, Properties> parsedIniMap = parseINI(bufferedReader);
			if (!parsedIniMap.isEmpty()
			    && parsedIniMap.containsKey("scummvm")
			    && parsedIniMap.get("scummvm") != null) {
				return parsedIniMap.get("scummvm").getProperty("savepath", "");
			}
		} catch (IOException ignored) {
		} catch (NullPointerException ignored) {
		}
		return "";
	}

	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	private boolean seekAndInitScummvmConfiguration() {

		// https://developer.android.com/reference/android/content/Context#getExternalFilesDir(java.lang.String)
		// The returned path may change over time if the calling app is moved to an adopted storage device, so only relative paths should be persisted.
		// Returns the absolute path to the directory on the primary shared/external storage device where the application can place persistent files it owns.
		// These files are internal to the applications, and not typically visible to the user as media.
		//
		// This is like getFilesDir() in that these files will be deleted when the application is uninstalled, however there are some important differences:
		//
		//    - Shared storage may not always be available, since removable media can be ejected by the user. Media state can be checked using Environment#getExternalStorageState(File).
		//    - There is no security enforced with these files. For example, any application holding Manifest.permission.WRITE_EXTERNAL_STORAGE can write to these files.
		//
		// If a shared storage device is emulated (as determined by Environment#isExternalStorageEmulated(File)), it's contents are backed by a private user data partition,
		// !! which means there is little benefit to storing data here instead of the private directories returned by getFilesDir(), etc.!!
		// TODO Maybe also make use Environment#isExternalStorageEmulated(File)) since this is not deprecated, to have more info available on storage
		// TODO (other methods *are* deprecated -- such as getExternalStoragePublicDirectory)
		// Starting in Build.VERSION_CODES.KITKAT, no permissions are required to read or write to the returned path; it's always accessible to the calling app.
		// This only applies to paths generated for package name of the calling application. To access paths belonging to other packages,
		// Manifest.permission.WRITE_EXTERNAL_STORAGE and/or Manifest.permission.READ_EXTERNAL_STORAGE are required.
		//
		// On devices with multiple users (as described by UserManager), each user has their own isolated shared storage. Applications only have access to the shared storage for the user they're running as.
		//
		// WARNING: The returned path may change over time if different shared storage media is inserted, so only relative paths should be persisted.
		//
		// If you supply a non-null type to this function, the returned file will be a path to a sub-directory of the given type.
		//
		// ----
		// Also note (via https://stackoverflow.com/a/41262228)
		// The getExternalFilesDir(String type) will call getExternalFilesDirs(String type) (notice the 's' at the final of the second method name).
		// The getExternalFilesDirs(String type) will find all dirs of the type, and calls ensureDirsExistOrFilter() at the end to ensure the directories exist.
		//
		// If the dir can't be reached, it will print a warning!
		//
		//   Log.w(ScummVM.LOG_TAG, "Failed to ensure directory: " + dir);
		//   dir = null;
		//
		// So, if your device has two sdcard paths, it will produce two dirs. If one is not available, the warning will come up.
		// ----

		_possibleExternalScummVMDir = getExternalFilesDir(null);
		_externalPathAvailableForReadAccess = false;
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState(_possibleExternalScummVMDir))
				|| Environment.MEDIA_UNKNOWN.equals(Environment.getExternalStorageState(_possibleExternalScummVMDir))
				|| Environment.MEDIA_MOUNTED_READ_ONLY.equals(Environment.getExternalStorageState(_possibleExternalScummVMDir))
			) {
				_externalPathAvailableForReadAccess = true;
			}
		} else {
			// before Lollipop just set it to true
			// TODO maybe check if this would cause issues
			_externalPathAvailableForReadAccess = true;
		}

		// Unlike getExternalFilesDir, this is guaranteed to ALWAYS be available
		//
		// NOTE: It is better to just always use the internal app path anyway for ScummVM, as "_actualScummVMDataDir" that is,
		//      to avoid issues with unavailable shared / external storage and to be (mostly) compatible with what the older versions did
		// WARNING: The returned path may change over time if the calling app is moved to an adopted storage device, so only relative paths should be persisted.
		_actualScummVMDataDir = getFilesDir();
		// Checking for null only makes sense if we were using external storage
//		if (_actualScummVMDataDir == null || !_actualScummVMDataDir.canRead()) {
//			new AlertDialog.Builder(this)
//				.setTitle(R.string.no_external_files_dir_access_title)
//				.setIcon(android.R.drawable.ic_dialog_alert)
//				.setMessage(R.string.no_external_files_dir_access)
//				.setNegativeButton(R.string.quit,
//					new DialogInterface.OnClickListener() {
//						public void onClick(DialogInterface dialog, int which) {
//							finish();
//						}
//					})
//				.show();
//			return false;
//		}

		Log.d(ScummVM.LOG_TAG, "Base ScummVM data folder is: " + _actualScummVMDataDir.getPath());
		String smallNodeDesc;
		File[] extfiles = _actualScummVMDataDir.listFiles();
		if (extfiles != null) {
			Log.d(ScummVM.LOG_TAG, "Size: "+ extfiles.length);
			for (File extfile : extfiles) {
				smallNodeDesc = "(F)";
				if (extfile.isDirectory()) {
					smallNodeDesc = "(D)";
				}
				Log.d(ScummVM.LOG_TAG, "Name: " + smallNodeDesc + " " + extfile.getName());
			}
		}

//		File internalScummVMLogsDir = new File(_actualScummVMDataDir, ".cache/scummvm/logs");
//		if (!internalScummVMLogsDir.exists() && internalScummVMLogsDir.mkdirs()) {
//			Log.d(ScummVM.LOG_TAG, "Created ScummVM Logs path: " + internalScummVMLogsDir.getPath());
//		} else if (internalScummVMLogsDir.isDirectory()) {
//			Log.d(ScummVM.LOG_TAG, "ScummVM Logs path already exists: " + internalScummVMLogsDir.getPath());
//		} else {
//			Log.e(ScummVM.LOG_TAG, "Could not create folder for ScummVM Logs path: " + internalScummVMLogsDir.getPath());
//			new AlertDialog.Builder(this)
//				.setTitle(R.string.no_log_file_title)
//				.setIcon(android.R.drawable.ic_dialog_alert)
//				.setMessage(R.string.no_log_file)
//				.setNegativeButton(R.string.quit,
//					new DialogInterface.OnClickListener() {
//						public void onClick(DialogInterface dialog, int which) {
//							finish();
//						}
//					})
//				.show();
//			return false;
//		}
//
//		_usingLogFile = new File(internalScummVMLogsDir, "scummvm.log");
//		try {
//			if (_usingLogFile.exists() || !_usingLogFile.createNewFile()) {
//				Log.d(ScummVM.LOG_TAG, "ScummVM Log file already exists!");
//				Log.d(ScummVM.LOG_TAG, "Existing ScummVM Log: " + _usingLogFile.getPath());
//			} else {
//				Log.d(ScummVM.LOG_TAG, "An empty ScummVM log file was created!");
//				Log.d(ScummVM.LOG_TAG, "New ScummVM Log: " + _usingLogFile.getPath());
//			}
//		} catch (Exception e) {
//			e.printStackTrace();
//			new AlertDialog.Builder(this)
//				.setTitle(R.string.no_log_file_title)
//				.setIcon(android.R.drawable.ic_dialog_alert)
//				.setMessage(R.string.no_log_file)
//				.setNegativeButton(R.string.quit,
//					new DialogInterface.OnClickListener() {
//						public void onClick(DialogInterface dialog, int which) {
//							finish();
//						}
//					})
//				.show();
//			return false;
//		}

		File internalScummVMConfigDir = new File(_actualScummVMDataDir, ".config/scummvm");
		if (!internalScummVMConfigDir.exists() && internalScummVMConfigDir.mkdirs()) {
			Log.d(ScummVM.LOG_TAG, "Created ScummVM Config path: " + internalScummVMConfigDir.getPath());
		} else if (internalScummVMConfigDir.isDirectory()) {
			Log.d(ScummVM.LOG_TAG, "ScummVM Config path already exists: " + internalScummVMConfigDir.getPath());
		} else {
			Log.e(ScummVM.LOG_TAG, "Could not create folder for ScummVM Config path: " + internalScummVMConfigDir.getPath());
			new AlertDialog.Builder(this)
				.setTitle(R.string.no_config_file_title)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(R.string.no_config_file)
				.setNegativeButton(R.string.quit,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							finish();
						}
					})
				.show();
			return false;
		}

		LinkedHashMap<String, File> candidateOldLocationsOfScummVMConfigMap = new LinkedHashMap<>();
		// Note: The "missing" case below for: (scummvm.ini)) (SDL port - A) is checked above; it is the same path we store the config file for 2.3+
		// SDL port was officially on the Play Store for versions 1.9+ up until and including 2.0)
		// Using LinkedHashMap because the order of searching is important
		// We want to re-use the more recent ScummVM old version too
		// TODO try getDir too without a path? just "." ??
		candidateOldLocationsOfScummVMConfigMap.put("(scummvm.ini) (SDL port - B)", new File(_actualScummVMDataDir, "../.config/scummvm/scummvm.ini"));
		if (_externalPathAvailableForReadAccess) {
			candidateOldLocationsOfScummVMConfigMap.put("(scummvm.ini) (SDL port - C)", new File(_possibleExternalScummVMDir, ".config/scummvm/scummvm.ini"));
			candidateOldLocationsOfScummVMConfigMap.put("(scummvm.ini) (SDL port - D)", new File(_possibleExternalScummVMDir, "../.config/scummvm/scummvm.ini"));
		}
		candidateOldLocationsOfScummVMConfigMap.put("(scummvm.ini) (SDL port - E)", new File(Environment.getExternalStorageDirectory(), ".config/scummvm/scummvm.ini"));
		candidateOldLocationsOfScummVMConfigMap.put("(scummvmrc) (version 1.8.1- or PlayStore 2.1.0) - Internal", new File(_actualScummVMDataDir, "scummvmrc"));
		if (_externalPathAvailableForReadAccess) {
			candidateOldLocationsOfScummVMConfigMap.put("(scummvmrc) (version 1.8.1- or PlayStore 2.1.0) - Ext Emu", new File(_possibleExternalScummVMDir, "scummvmrc"));
		}
		candidateOldLocationsOfScummVMConfigMap.put("(scummvmrc) (version 1.8.1- or PlayStore 2.1.0) - Ext SD", new File(Environment.getExternalStorageDirectory(), "scummvmrc"));
		candidateOldLocationsOfScummVMConfigMap.put("(.scummvmrc) (POSIX conformance) - Internal", new File(_actualScummVMDataDir, ".scummvmrc"));
		if (_externalPathAvailableForReadAccess) {
			candidateOldLocationsOfScummVMConfigMap.put("(.scummvmrc) (POSIX conformance) - Ext Emu", new File(_possibleExternalScummVMDir, ".scummvmrc"));
		}
		candidateOldLocationsOfScummVMConfigMap.put("(.scummvmrc) (POSIX conformance) - Ext SD)", new File(Environment.getExternalStorageDirectory(), ".scummvmrc"));

		String[] listOfAuxExtStoragePaths = _scummvm.getAllStorageLocationsNoPermissionRequest();
		// Add AUX external storage locations
		int incKeyId = 0;
		for (int incIndx = 0; incIndx + 1 < listOfAuxExtStoragePaths.length; incIndx += 2) {
			// exclude identical matches for internal and emulated external app dir, since we take them into account below explicitly
			if (listOfAuxExtStoragePaths[incIndx + 1].compareToIgnoreCase(_actualScummVMDataDir.getPath()) != 0
				&& listOfAuxExtStoragePaths[incIndx + 1].compareToIgnoreCase(_possibleExternalScummVMDir.getPath()) != 0
			) {
				//
				// Possible for Config file locations on top of paths returned by getAllStorageLocationsNoPermissionRequest
				//
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/.config/scummvm/scummvm.ini"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/../.config/scummvm/scummvm.ini"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/scummvm.ini"));

				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/scummvmrc"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/../scummvmrc"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/scummvmrc"));

				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/.scummvmrc"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" + getPackageName() + "/files/../.scummvmrc"));
				candidateOldLocationsOfScummVMConfigMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
					new File(listOfAuxExtStoragePaths[incIndx + 1] + "/.scummvmrc"));
			}
		}

		boolean scummVMConfigHandled = false;
		Version maxOldVersionFound = new Version("0"); // dummy initializer
		Version existingVersionFoundInScummVMDataDir = new Version("0"); // dummy initializer

		final Version version2_2_1_forPatch = new Version("2.2.1"); // patch for 2.2.1 Beta1 purposes
		boolean existingConfigInScummVMDataDirReplacedOnce = false; // patch for 2.2.1 Beta1 purposes

		// NOTE: our config file scummvm.ini is created directly inside the ScummVM internal app path
		//       this is probably due to a mistake (?), since we do create a config path for it above
		//       ( in File internalScummVMConfigDir , the sub-path ".config/scummvm")
		//       However, this is harmless, so we can keep it this way.
		//       Or we could change it in a future version.
		//       Keep in mind that changing the scummvm.ini config file location would require at the very least:
		//       - Moving the old scummvm.ini (if upgrading) to the new location and deleting it from the old one
		//       - Updating the ScummVM documentation about the new location
		_configScummvmFile = new File(_actualScummVMDataDir, "scummvm.ini");

		try {
			if (_configScummvmFile.exists() || !_configScummvmFile.createNewFile()) {
				Log.d(ScummVM.LOG_TAG, "ScummVM Config file already exists!");
				Log.d(ScummVM.LOG_TAG, "Existing ScummVM INI: " + _configScummvmFile.getPath());
				String existingVersionInfo = getVersionInfoFromScummvmConfiguration(_configScummvmFile.getPath());
				if (!TextUtils.isEmpty(existingVersionInfo) && !TextUtils.isEmpty(existingVersionInfo.trim()) ) {
					Log.d(ScummVM.LOG_TAG, "Existing ScummVM Version: " + existingVersionInfo.trim());
					Version tmpOldVersionFound = new Version(existingVersionInfo.trim());
					if (tmpOldVersionFound.compareTo(maxOldVersionFound) > 0) {
						maxOldVersionFound = tmpOldVersionFound;
						existingVersionFoundInScummVMDataDir = tmpOldVersionFound;
						//scummVMConfigHandled = false; // invalidate the handled flag
					}
				} else {
					Log.d(ScummVM.LOG_TAG, "Could not find info on existing ScummVM version. Unsupported or corrupt file?");
				}
				scummVMConfigHandled = true;
			} else {
				Log.d(ScummVM.LOG_TAG, "An empty ScummVM config file was created!");
				Log.d(ScummVM.LOG_TAG, "New ScummVM INI: " + _configScummvmFile.getPath());
			}

			//
			// NOTE: Android app's version number (in build.gradle) avoids the need to check against "upgrading" to a lower version,
			//       since Android will not allow that and will force the user to uninstall first any current higher version.

			// Do an exhaustive search to discover all old configs in order to:
			// - find a useable / recent existing one that we might want to upgrade from
			// - remove them as old remnants and avoid re-checking / re-using them in a subsequent installation
			for (String oldConfigFileDescription : candidateOldLocationsOfScummVMConfigMap.keySet()) {
				File oldCandidateScummVMConfig = candidateOldLocationsOfScummVMConfigMap.get(oldConfigFileDescription);
				Log.d(ScummVM.LOG_TAG, "Looking for old config " + oldConfigFileDescription + " ScummVM file...");
				if (oldCandidateScummVMConfig != null) {
					Log.d(ScummVM.LOG_TAG, "at Path: " + oldCandidateScummVMConfig.getPath() + "...");
					if (oldCandidateScummVMConfig.exists() && oldCandidateScummVMConfig.isFile()) {
						Log.d(ScummVM.LOG_TAG, "Old config " + oldConfigFileDescription + " ScummVM file was found!");
						String existingVersionInfo = getVersionInfoFromScummvmConfiguration(oldCandidateScummVMConfig.getPath());
						if (!TextUtils.isEmpty(existingVersionInfo) && !TextUtils.isEmpty(existingVersionInfo.trim())) {
							Log.d(ScummVM.LOG_TAG, "Old config's ScummVM version: " + existingVersionInfo.trim());
							Version tmpOldVersionFound = new Version(existingVersionInfo.trim());
							//
							// Replace the current config.ini with another recovered,
							//         if the recovered one is of higher version.
							//
							// patch for 2.2.1 Beta1: (additional check)
							//       if current version max is 2.2.1 and existingVersionFoundInScummVMDataDir is 2.2.1 (meaning we have a config.ini created for 2.2.1)
							//          and file location key starts with "A-" (aux external storage locations)
							//          and old version found is lower than 2.2.1
							//       Then: replace our current config ini and remove the recovered ini from the aux external storage
							if ((tmpOldVersionFound.compareTo(maxOldVersionFound) > 0)
								|| (!existingConfigInScummVMDataDirReplacedOnce
								&& existingVersionFoundInScummVMDataDir.compareTo(version2_2_1_forPatch) == 0
								&& tmpOldVersionFound.compareTo(version2_2_1_forPatch) < 0
								&& oldConfigFileDescription.startsWith("A-"))
							) {
								maxOldVersionFound = tmpOldVersionFound;
								scummVMConfigHandled = false; // invalidate the handled flag, since we found a new great(er) version so we should re-use that one
							}
						} else {
							Log.d(ScummVM.LOG_TAG, "Could not find info on the old config's ScummVM version. Unsupported or corrupt file?");
						}
						if (!scummVMConfigHandled) {
							// We copy the old file over the new one.
							// This will happen once during this installation, but on a subsequent one it will again copy that old config file
							// if we don't remove it
							copyFileUsingStream(oldCandidateScummVMConfig, _configScummvmFile);
							Log.d(ScummVM.LOG_TAG, "Old config " + oldConfigFileDescription + " ScummVM file was renamed and overwrote the new (empty) scummvm.ini");
							scummVMConfigHandled = true;
							existingConfigInScummVMDataDirReplacedOnce = true;
						}

						// Here we remove the old config
						if (oldCandidateScummVMConfig.delete()) {
							Log.d(ScummVM.LOG_TAG, "The old config " + oldConfigFileDescription + " ScummVM file is now deleted!");
						} else {
							Log.d(ScummVM.LOG_TAG, "Failed to delete the old config " + oldConfigFileDescription + " ScummVM file!");
						}
					} else {
						Log.d(ScummVM.LOG_TAG, "...not found!");
					}
				} else {
					Log.d(ScummVM.LOG_TAG, "...not found!");
				}
			}
		} catch(Exception e) {
			e.printStackTrace();
			new AlertDialog.Builder(this)
				.setTitle(R.string.no_config_file_title)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(R.string.no_config_file)
				.setNegativeButton(R.string.quit,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							finish();
						}
					})
				.show();
			return false;
		}

		if (maxOldVersionFound.compareTo(new Version("0")) != 0) {
			Log.d(ScummVM.LOG_TAG, "Maximum ScummVM version found and (re)used is: " + maxOldVersionFound.getDescription() +" (" + maxOldVersionFound.get() +")");
		} else {
			Log.d(ScummVM.LOG_TAG, "No viable existing ScummVM config version found");
		}

		//
		// TODO The assets cleanup upgrading system is not perfect but it will have to do
		//      A more efficient way would be to compare hash (when we deem that an upgrade is happening, so we will also still have to compare versions)
		// Note that isSideUpgrading is also true each time we re-launch the app
		// Also even during a side-upgrade we cleanup any redundant files (no longer part of our assets)

		// By first checking for isDirty() and then comparing the Version objects,
		// we don't need to also compare the Version descriptions (full version text) for a match too,
		// since, if the full versions text do not match, it's because at least one of them is dirty.
		// TODO: This does mean that "pre" (or similar) versions (eg. 2.2.1pre) will always be considered non-side-upgrades
		//        and will re-copy the assets upon each launch
		//       This should have a slight performance impact (for launch time) for those intermediate version releases,
		//       but it's better than the alternative (comparing MD5 hashes for all files), and it should go away with the next proper release.
		//       This solution should cover "git" versions properly
		//       (ie. developer builds, built with release configuration (eg 2.3.0git) or debug configuration (eg. 2.3.0git9272-gc71ac4748b))
		boolean isSideUpgrading = (!_currentScummVMVersion.isDirty()
		                           && !maxOldVersionFound.isDirty()
		                           && maxOldVersionFound.compareTo(_currentScummVMVersion) == 0);
		copyAssetsToInternalMemory(isSideUpgrading);

		//
		// Set global savepath
		//
		// First see in ScummVM if there is a saved "savepath" in the config file
		// This is the case where the user has set the global save path from the GUI, explicitly to something other than Default
		//
		// Main logic:
		// - Create an internal savepath ANYWAY if it does not exist
		// - If our internal savepath is empty (and only then):
		//    a. TODO maybe create a dummy file in it (to skip the process next time)
		//    b. we search for the largest save folder of a previous ScummVM version
		//       TODO we could take into account which versions tended to save in which locations, so as to prioritize a recent version
		//            but for now we will go with absolute size as the only comparison criteria
		//       So we store the path with max num of files (or none if all are empty)
		//    c. After the search, if we have a valid folder with non-zero size, we copy all files to our internal save path
		//
		// Then we look for two main cases:
		// 1. If there is a valid "savepath" persisted in the config file,
		//    and it is a directory that exists and we can list its contents (even if 0 contents)
		//    then we use that one as savepath
		// 2. If not, we fall back to our internal savepath
		//

		// TODO We should always keep in mind that *STORING* full paths (in general) in config files or elsewhere is considered bad practice on Android
		//      due to features like being able to switch storage for applications between internal and external
		//      or external storage not always being available (but then eg. a save file on the storage should be correctly shown as not available)
		//      or maybe among Android OS versions the same external storage could be mounted to a (somewhat) different path?
		//      However, it seems unavoidable when user has set paths explicitly (ie not using the defaults)
		//      We always set the default save path as a launch parameter
		//
		// By default choose to store savegames on app's internal storage, which is always available
		//
		File defaultScummVMSavesPath = new File(_actualScummVMDataDir, "saves");
		// By default use this as the saves path
		_usingScummVMSavesDir = new File(defaultScummVMSavesPath.getPath());

		if (defaultScummVMSavesPath.exists() && defaultScummVMSavesPath.isDirectory()) {
			try {
				Log.d(ScummVM.LOG_TAG, "ScummVM default saves path already exists: " + defaultScummVMSavesPath.getPath());
			} catch (Exception e) {
				Log.d(ScummVM.LOG_TAG, "ScummVM default saves path exception CAUGHT!");
			}
		} else if (!defaultScummVMSavesPath.exists() && defaultScummVMSavesPath.mkdirs()) {
			Log.d(ScummVM.LOG_TAG, "Created ScummVM default saves path: " + defaultScummVMSavesPath.getPath());
		} else {
			Log.e(ScummVM.LOG_TAG, "Could not create folder for ScummVM default saves path: " + defaultScummVMSavesPath.getPath());
			new AlertDialog.Builder(this)
				.setTitle(R.string.no_save_path_title)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(R.string.no_save_path_configured)
				.setNegativeButton(R.string.quit,
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							finish();
						}
					})
				.show();
			return false;
		}

		File[] defaultSaveDirFiles = defaultScummVMSavesPath.listFiles();
		if (defaultSaveDirFiles != null) {
			Log.d(ScummVM.LOG_TAG, "Size: "+ defaultSaveDirFiles.length);
			// Commented out listing of files in the default saves folder for debug purposes
			//if (defaultSaveDirFiles.length > 0) {
			//	Log.d(ScummVM.LOG_TAG, "Listing ScummVM save files in default saves path...");
			//	for (File savfile : defaultSaveDirFiles) {
			//		smallNodeDesc = "(F)";
			//		if (savfile.isDirectory()) {
			//			smallNodeDesc = "(D)";
			//		}
			//		Log.d(ScummVM.LOG_TAG, "Name: " + smallNodeDesc + " " + savfile.getName());
			//	}
			//}

			// patch for 2.2.1 Beta1: (additional check)
			//       if defaultSaveDirFiles size (num of files) is not 0
			//          and there was a config ini in the ScummVM data dir, with version 2.2.1
			//          and that config ini was replaced during the above process of recovering another ini
			//       Then: Scan for previous usable ScummVM folder (it will still only copy the larger one found)
			boolean scanOnlyInAuxExternalStorage = false;
			if (defaultSaveDirFiles.length == 0
			    || (existingConfigInScummVMDataDirReplacedOnce
			        && existingVersionFoundInScummVMDataDir.compareTo(version2_2_1_forPatch) == 0)
			) {
				if (existingConfigInScummVMDataDirReplacedOnce
					&& existingVersionFoundInScummVMDataDir.compareTo(version2_2_1_forPatch) == 0) {
					scanOnlyInAuxExternalStorage = true;
				}

				Log.d(ScummVM.LOG_TAG, "Scanning for a previous usable ScummVM Saves folder...");
				// Note: A directory named "Saves" is NOT the same as "saves" in internal storage.
				//       ie. paths and filenames in internal storage (including emulated external) are case sensitive!
				//       BUT: It could be the same in external SD card or other FAT formatted storage
				//
				// TODO add code here for creating a dummy place holder file in order to not repeat the process?

				File candidateOldScummVMSavesPath = null;
				int maxSavesFolderFoundSize = 0;

				LinkedHashMap<String, File> candidateOldLocationsOfScummVMSavesMap = new LinkedHashMap<>();

				// TODO some of these entries are an overkill, but better safe than sorry
				if (!scanOnlyInAuxExternalStorage) {
					// due to case sensitivity this is different than "saves"
					candidateOldLocationsOfScummVMSavesMap.put("A01", new File(_actualScummVMDataDir, "Saves"));
					// This is a potential one, when internal storage for app was used
					candidateOldLocationsOfScummVMSavesMap.put("A02", new File(_actualScummVMDataDir, ".local/share/scummvm/saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A03", new File(_actualScummVMDataDir, ".local/scummvm/saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A04", new File(_actualScummVMDataDir, "scummvm/saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A05", new File(_actualScummVMDataDir, "../.local/share/scummvm/saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A06", new File(_actualScummVMDataDir, "../.local/scummvm/saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A07", new File(_actualScummVMDataDir, "../saves"));
					candidateOldLocationsOfScummVMSavesMap.put("A08", new File(_actualScummVMDataDir, "../scummvm/saves"));
					if (_externalPathAvailableForReadAccess) {
						// this is a popular one
						candidateOldLocationsOfScummVMSavesMap.put("A09", new File(_possibleExternalScummVMDir, ".local/share/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A10", new File(_possibleExternalScummVMDir, ".local/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A11", new File(_possibleExternalScummVMDir, "saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A12", new File(_possibleExternalScummVMDir, "scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A13", new File(_possibleExternalScummVMDir, "../.local/share/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A14", new File(_possibleExternalScummVMDir, "../.local/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A15", new File(_possibleExternalScummVMDir, "../saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A16", new File(_possibleExternalScummVMDir, "../scummvm/saves"));
					}
					// this was for old Android plain port
					candidateOldLocationsOfScummVMSavesMap.put("A17", new File(Environment.getExternalStorageDirectory(), "ScummVM/Saves"));
				}

				// Add AUX external storage locations
				incKeyId = 0;
				for (int incIndx = 0; incIndx + 1 < listOfAuxExtStoragePaths.length; incIndx += 2) {
					// exclude identical matches for internal and emulated external app dir, since we take them into account below explicitly
					if (listOfAuxExtStoragePaths[incIndx + 1].compareToIgnoreCase(_actualScummVMDataDir.getPath()) != 0
						&& listOfAuxExtStoragePaths[incIndx + 1].compareToIgnoreCase(_possibleExternalScummVMDir.getPath()) != 0
					) {
						//
						// Possible for Saves dirs locations on top of paths returned by getAllStorageLocationsNoPermissionRequest
						//
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/ScummVM/Saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/.local/share/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/.local/scummvm/saves"));

						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/../.local/share/scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/../.local/scummvm/saves"));

						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/../saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/Android/data/" +  getPackageName() + "/files/../scummvm/saves"));
						candidateOldLocationsOfScummVMSavesMap.put("A-" + (++incKeyId) + "-" + listOfAuxExtStoragePaths[incIndx],
							new File(listOfAuxExtStoragePaths[incIndx + 1] + "/.scummvmrc"));
					}
				}

				for (String oldSavesPathDescription : candidateOldLocationsOfScummVMSavesMap.keySet()) {
					File iterCandidateScummVMSavesPath = candidateOldLocationsOfScummVMSavesMap.get(oldSavesPathDescription);
					Log.d(ScummVM.LOG_TAG, "Looking for old saves path " + oldSavesPathDescription + "...");
					try {
						if (iterCandidateScummVMSavesPath != null) {
							Log.d(ScummVM.LOG_TAG, "at Path: " + iterCandidateScummVMSavesPath.getPath() + "...");

							if (iterCandidateScummVMSavesPath.exists() && iterCandidateScummVMSavesPath.isDirectory()) {
								File[] sgfiles = iterCandidateScummVMSavesPath.listFiles();
								if (sgfiles != null) {
									Log.d(ScummVM.LOG_TAG, "Size: " + sgfiles.length);
									for (File sgfile : sgfiles) {
										smallNodeDesc = "(F)";
										if (sgfile.isDirectory()) {
											smallNodeDesc = "(D)";
										}
										Log.d(ScummVM.LOG_TAG, "Name: " + smallNodeDesc + " " + sgfile.getName());
									}

									if (sgfiles.length > maxSavesFolderFoundSize) {
										maxSavesFolderFoundSize = sgfiles.length;
										candidateOldScummVMSavesPath = iterCandidateScummVMSavesPath;
									}
								}
							} else {
								Log.d(ScummVM.LOG_TAG, "...not found.");
							}
						} else {
							Log.d(ScummVM.LOG_TAG, "...not found.");
						}

					} catch (Exception e) {
						Log.d(ScummVM.LOG_TAG, "ScummVM Saves path exception CAUGHT!");
					}
				}

				if (candidateOldScummVMSavesPath != null) {
					//
					Log.d(ScummVM.LOG_TAG, "Copying files from old saves folder: " + candidateOldScummVMSavesPath.getPath() + " to: " + defaultScummVMSavesPath.getPath());
					File[] sgfiles = candidateOldScummVMSavesPath.listFiles();
					if (sgfiles != null) {
						for (File sgfile : sgfiles) {
							String filename = sgfile.getName();
							if (!sgfile.isDirectory()) {
								Log.d(ScummVM.LOG_TAG, "Copying: " + filename);
								InputStream in = null;
								OutputStream out = null;
								try {
									in = new FileInputStream(sgfile);
									File outFile = new File(defaultScummVMSavesPath, filename);
									out = new FileOutputStream(outFile);
									copyStreamToStream(in, out);
								} catch (IOException e) {
									Log.e(ScummVM.LOG_TAG, "Failed to copy save file: " + filename);
								} finally {
									if (in != null) {
										try {
											in.close();
										} catch (IOException e) {
											// NOOP
										}
									}
									if (out != null) {
										try {
											out.close();
										} catch (IOException e) {
											// NOOP
										}
									}
								}
							} else {
								Log.d(ScummVM.LOG_TAG, "Not copying directory: " + filename);
							}
						}
					}
				}
			}
		}

		File persistentGlobalSavePath = null;
		if (_configScummvmFile.exists() && _configScummvmFile.isFile()) {
			Log.d(ScummVM.LOG_TAG, "Looking into config file for save path: " + _configScummvmFile.getPath());
			String persistentGlobalSavePathStr = getSavepathInfoFromScummvmConfiguration(_configScummvmFile.getPath());
			if (!TextUtils.isEmpty(persistentGlobalSavePathStr) && !TextUtils.isEmpty(persistentGlobalSavePathStr.trim()) ) {
				Log.d(ScummVM.LOG_TAG, "Found explicit save path: " + persistentGlobalSavePathStr);
				persistentGlobalSavePath = new File(persistentGlobalSavePathStr);
				if (persistentGlobalSavePath.exists() && persistentGlobalSavePath.isDirectory() && persistentGlobalSavePath.listFiles() != null) {
					try {
						Log.d(ScummVM.LOG_TAG, "ScummVM explicit saves path folder exists and it is list-able");
					} catch (Exception e) {
						persistentGlobalSavePath = null;
						Log.e(ScummVM.LOG_TAG, "ScummVM explicit saves path exception CAUGHT!");
					}
				} else {
					// We won't bother creating it, it's not in our scope to do that (and it would probably result in potential permission issues)
					Log.e(ScummVM.LOG_TAG, "Could not access explicit save folder for ScummVM: " + persistentGlobalSavePath.getPath());
					persistentGlobalSavePath = null;
					// We should *not* quit or return here,
					// TODO But, how do we override this explicit set path? Do we leave it to the user to reset it?
					new AlertDialog.Builder(this)
						.setTitle(R.string.no_save_path_title)
						.setIcon(android.R.drawable.ic_dialog_alert)
						.setMessage(R.string.bad_explicit_save_path_configured)
						.setPositiveButton(R.string.ok,
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int which) {

								}
							})
						.show();
				}
			} else {
				Log.d(ScummVM.LOG_TAG, "Could not find explicit save path info in ScummVM's config file");
			}
		}

		if (persistentGlobalSavePath != null) {
			// Use the persistent savepath
			_usingScummVMSavesDir = new File(persistentGlobalSavePath.getPath());
		}
		Log.d(ScummVM.LOG_TAG, "Resulting save path is: " + _usingScummVMSavesDir.getPath());

		return true;
	}


	private boolean containsStringEntry(@NonNull String[] stringItenary, String targetEntry) {
		for (String sourceEntry : stringItenary) {
			// Log.d(ScummVM.LOG_TAG, "Comparing filename: " + sourceEntry + " to filename: " + targetEntry);
			if (sourceEntry.compareToIgnoreCase(targetEntry) == 0) {
				return true;
			}
		}
		return false;
	}

	// clear up any possibly deprecated assets (when upgrading to a new version)
	// Don't remove the scummvm.ini file!
	// Remove any files not in the filesItenary, even in a sideUpgrade
	// Remove any files in the filesItenary only if not a sideUpgrade
	private void internalAppFolderCleanup(String[] filesItenary, boolean sideUpgrade) {
		if (_actualScummVMDataDir != null) {
			File[] extfiles = _actualScummVMDataDir.listFiles();
			if (extfiles != null) {
				Log.d(ScummVM.LOG_TAG, "Cleaning up files in internal app space");
				for (File extfile : extfiles) {
					if (extfile.isFile()) {
						if (extfile.getName().compareToIgnoreCase("scummvm.ini") != 0
							&& (!containsStringEntry(filesItenary, extfile.getName())
							|| !sideUpgrade)
						) {
							Log.d(ScummVM.LOG_TAG, "Deleting file:" + extfile.getName());
							if (!extfile.delete()) {
								Log.e(ScummVM.LOG_TAG, "Failed to delete file:" + extfile.getName());
							}
						}
					}
				}
			}
		}
	}

	// code based on https://stackoverflow.com/a/4530294
	// Note, the following assumptions are made (since they are true as of yet)
	// - We don't need to copy (sub)folders
	// - We copy all the files from our assets (not a subset of them)
	// Otherwise we would probably need to create a specifically named zip file with the selection of files we'd need to extract to the internal memory
	@RequiresApi(api = Build.VERSION_CODES.KITKAT)
	private void copyAssetsToInternalMemory(boolean sideUpgrade) {
		// sideUpgrade is set to true, if we upgrade to the same version -- just check for the files existence before copying
		if (_actualScummVMDataDir != null) {
			AssetManager assetManager = getAssets();
			String[] files = null;
			try {
				files = assetManager.list("");
			} catch (IOException e) {
				Log.e(ScummVM.LOG_TAG, "Failed to get asset file list.", e);
			}

			internalAppFolderCleanup(files, sideUpgrade);

			if (files != null) {
				for (String filename : files) {
					InputStream in = null;
					OutputStream out = null;
					try {
						in = assetManager.open(filename);
						File outFile = new File(_actualScummVMDataDir, filename);
						if (sideUpgrade && outFile.exists()) {
							Log.d(ScummVM.LOG_TAG, "Side-upgrade. No need to update asset file: " + filename);
						} else {
							Log.d(ScummVM.LOG_TAG, "Copying asset file: " + filename);
							out = new FileOutputStream(outFile);
							copyStreamToStream(in, out);
						}
					} catch (IOException e) {
						Log.e(ScummVM.LOG_TAG, "Failed to copy asset file: " + filename);
					} finally {
						if (in != null) {
							try {
								in.close();
							} catch (IOException e) {
								// NOOP
							}
						}
						if (out != null) {
							try {
								out.close();
							} catch (IOException e) {
								// NOOP
							}
						}
					}
				}
			}
		}
	}

	// -------------------------------------------------------------------------------------------
	// Start of SAF enabled code
	// Code borrows parts from open source project: OpenLaucher's SharedUtil class
	// https://github.com/OpenLauncherTeam/openlauncher
	// https://github.com/OpenLauncherTeam/openlauncher/blob/master/app/src/main/java/net/gsantner/opoc/util/ShareUtil.java
	// as well as StackOverflow threads:
	// https://stackoverflow.com/questions/43066117/android-m-write-to-sd-card-permission-denied
	// https://stackoverflow.com/questions/59000390/android-accessing-files-in-native-c-c-code-with-google-scoped-storage-api
	// -------------------------------------------------------------------------------------------
	public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
		if (resultCode != RESULT_OK)
			return;
		else {
			if (requestCode == REQUEST_SAF) {
				if (resultCode == RESULT_OK && resultData != null && resultData.getData() != null) {
					Uri treeUri = resultData.getData();
					//SharedPreferences sharedPref = getApplicationContext().getSharedPreferences(getApplicationContext().getPackageName() + "_preferences", Context.MODE_PRIVATE);
					SharedPreferences sharedPref = getPreferences(Context.MODE_PRIVATE);

					SharedPreferences.Editor editor = sharedPref.edit();
					editor.putString(getString(R.string.preference_saf_tree_key), treeUri.toString());
					editor.apply();

					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
						getContentResolver().takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
					}
					return;
				}
			}
		}
	}

	/***
	 * Request storage access. The user needs to press "Select storage" at the correct storage.
	 */
	public void requestStorageAccessFramework(String dirPathSample) {

		_scummvm.displayMessageOnOSD(getString(R.string.saf_request_prompt) + dirPathSample);

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
			Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
			intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
			                | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
			                | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
			                | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
			);
			startActivityForResult(intent, REQUEST_SAF);
		}
	}

	/**
	 * Get storage access framework tree uri. The user must have granted access via requestStorageAccessFramework
	 *
	 * @return Uri or null if not granted yet
	 */
	public Uri getStorageAccessFrameworkTreeUri() {
		SharedPreferences sharedPref = getPreferences(Context.MODE_PRIVATE);
		String treeStr = sharedPref.getString(getString(R.string.preference_saf_tree_key), null);

		if (!TextUtils.isEmpty(treeStr)) {
			try {
				Log.d(ScummVM.LOG_TAG, "getStorageAccessFrameworkTreeUri: " + treeStr);
				return Uri.parse(treeStr);
			} catch (Exception ignored) {
			}
		}
		return null;
	}

	public File getStorageRootFolder(final File file) {
		String filepath;
		try {
			filepath = file.getCanonicalPath();
		} catch (Exception ignored) {
			return null;
		}

		for (String storagePath : _scummvm.getAllStorageLocationsNoPermissionRequest() ) {
			if (filepath.startsWith(storagePath)) {
				return new File(storagePath);
			}
		}
		return null;
	}

	// TODO we need to implement support for reading access somewhere too
	@SuppressWarnings({"ResultOfMethodCallIgnored", "StatementWithEmptyBody"})
	public void writeFile(final File file, final boolean isDirectory, final boolean keepFileDescriptorOpen, final MyWriteFileCallback writeFileCallback ) {
		try {
			// TODO we need code for read access too (even though currently API28 reading works without SAF, just with the runtime permissions)
			String hackyFilename = "";

			ParcelFileDescriptor pfd = null;
			if (file.canWrite() || (!file.exists() && file.getParentFile().canWrite())) {
				if (isDirectory) {
					file.mkdirs();
				} else {
					// If we are here this means creating a new file can be done with fopen from native
					//fileOutputStream = new FileOutputStream(file);
					Log.d(ScummVM.LOG_TAG, "writeFile() file can be created normally -- (not created here)" );
					hackyFilename = "";
				}
			} else {
				DocumentFile dof = getDocumentFile(file, isDirectory);
				if (dof != null && dof.getUri() != null && dof.canWrite()) {
					if (isDirectory) {
						// Nothing more to do
					} else {
						pfd = getContentResolver().openFileDescriptor(dof.getUri(), "w");
						if (pfd != null) {
							// https://stackoverflow.com/questions/59000390/android-accessing-files-in-native-c-c-code-with-google-scoped-storage-api
							int fd = pfd.getFd();
							hackyFilename = "/proc/self/fd/" + fd;
							hackyNameToOpenFileDescriptorList.put(hackyFilename, pfd);
							Log.d(ScummVM.LOG_TAG, "writeFile() file created with SAF -- hacky name: " + hackyFilename );
						}
					}
				}
			}

			// TODO the idea of a callback is to work with the output (or input) streams, then return here and close the streams and the descriptors properly
			//      however since we are interacting with native this would not work for those cases

			if (writeFileCallback != null) {
				writeFileCallback.handle( (isDirectory && file.exists()) || (!isDirectory && file.exists() && file.isFile() ), hackyFilename);

			}

			// TODO We need to close the file descriptor when we are done with it from native
			//		- what if the call is not from native but from the activity?
			//      - directory operations don't create or need a file descriptor
			if (!keepFileDescriptorOpen && pfd != null) {
				if (hackyNameToOpenFileDescriptorList.containsKey(hackyFilename)) {
					hackyNameToOpenFileDescriptorList.remove(hackyFilename);
				}
				pfd.close();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Get a DocumentFile object out of a normal java File object.
	 * When used on a external storage (SD), use requestStorageAccessFramework()
	 * first to get access. Otherwise this will fail.
	 *
	 * @param file  The file/folder to convert
	 * @param isDir Whether or not file is a directory. For non-existing (to be created) files this info is not known hence required.
	 * @return A DocumentFile object or null if file cannot be converted
	 */
	@SuppressWarnings("RegExpRedundantEscape")
	public DocumentFile getDocumentFile(final File file, final boolean isDir) {
		// On older versions use fromFile
		if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
			return DocumentFile.fromFile(file);
		}

		// Get ContextUtils to find storageRootFolder
		File baseFolderFile = getStorageRootFolder(file);

		String baseFolder = baseFolderFile == null ? null : baseFolderFile.getAbsolutePath();
		boolean originalDirectory = false;
		if (baseFolder == null) {
			return null;
		}

		String relPath = null;
		try {
			String fullPath = file.getCanonicalPath();
			if (!baseFolder.equals(fullPath)) {
				relPath = fullPath.substring(baseFolder.length() + 1);
			} else {
				originalDirectory = true;
			}
		} catch (IOException e) {
			return null;
		} catch (Exception ignored) {
			originalDirectory = true;
		}
		Uri treeUri;
		if ((treeUri = getStorageAccessFrameworkTreeUri()) == null) {
			return null;
		}
		DocumentFile dof = DocumentFile.fromTreeUri(getApplicationContext(), treeUri);
		if (originalDirectory) {
			return dof;
		}
		String[] parts = relPath.split("\\/");
		for (int i = 0; i < parts.length; i++) {
			DocumentFile nextDof = dof.findFile(parts[i]);
			if (nextDof == null) {
				try {
					nextDof = ((i < parts.length - 1) || isDir) ? dof.createDirectory(parts[i]) : dof.createFile("image", parts[i]);
				} catch (Exception ignored) {
					nextDof = null;
				}
			}
			dof = nextDof;
		}
		return dof;
	}

	/**
	 * Check whether or not a file can be written.
	 * Requires storage access framework permission for external storage (SD)
	 *
	 * @param file  The file object (file/folder)
	 * @param isDirectory Whether or not the given file parameter is a directory
	 * @return Whether or not the file can be written
	 */
	public boolean canWriteFile(final File file, final boolean isDirectory) {
		if (file == null) {
			return false;
		} else if (file.getAbsolutePath().startsWith(Environment.getExternalStorageDirectory().getAbsolutePath())
		           || file.getAbsolutePath().startsWith(getFilesDir().getAbsolutePath())) {
			return (!isDirectory && file.getParentFile() != null) ? file.getParentFile().canWrite() : file.canWrite();
		} else {
			DocumentFile dof = getDocumentFile(file, isDirectory);
			return dof != null && dof.canWrite();
		}
	}
	// -------------------------------------------------------------------------------------------
	// End of SAF enabled code
	// -------------------------------------------------------------------------------------------


} // end of ScummVMActivity

// *** HONEYCOMB / ICS FIX FOR FULLSCREEN MODE, by lmak ***
// TODO DimSystemStatusBar may be redundant for us
abstract class DimSystemStatusBar {

	final boolean bGlobalsImmersiveMode = true;

	public static DimSystemStatusBar get() {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			return DimSystemStatusBarHoneycomb.Holder.sInstance;
		} else {
			return DimSystemStatusBarDummy.Holder.sInstance;
		}
	}

	public abstract void dim(final View view);

	private static class DimSystemStatusBarHoneycomb extends DimSystemStatusBar {
		private static class Holder {
			private static final DimSystemStatusBarHoneycomb sInstance = new DimSystemStatusBarHoneycomb();
		}

		public void dim(final View view) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT && bGlobalsImmersiveMode) {
				// Immersive mode, I already hear curses when system bar reappears mid-game from the slightest swipe at the bottom of the screen
				view.setSystemUiVisibility(android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | android.view.View.SYSTEM_UI_FLAG_FULLSCREEN);
			} else {
				view.setSystemUiVisibility(android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE);
			}
		}
	}

	private static class DimSystemStatusBarDummy extends DimSystemStatusBar {
		private static class Holder {
			private static final DimSystemStatusBarDummy sInstance = new DimSystemStatusBarDummy();
		}

		public void dim(final View view) { }
	}
}

abstract class SetLayerType {

	public static SetLayerType get() {
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB) {
			return SetLayerTypeHoneycomb.Holder.sInstance;
		} else {
			return SetLayerTypeDummy.Holder.sInstance;
		}
	}

	public abstract void setLayerType(final View view);

	private static class SetLayerTypeHoneycomb extends SetLayerType {
		private static class Holder {
			private static final SetLayerTypeHoneycomb sInstance = new SetLayerTypeHoneycomb();
		}

		public void setLayerType(final View view) {
			view.setLayerType(android.view.View.LAYER_TYPE_NONE, null);
			//view.setLayerType(android.view.View.LAYER_TYPE_HARDWARE, null);
		}
	}

	private static class SetLayerTypeDummy extends SetLayerType {
		private static class Holder {
			private static final SetLayerTypeDummy sInstance = new SetLayerTypeDummy();
		}

		public void setLayerType(final View view) { }
	}
}

// Used to define the interface for a callback after a write operation (via the method that is enhanced to use SAF if the normal way fails)
interface MyWriteFileCallback {
	public void handle(Boolean created, String hackyFilename);
}

// Used to define the interface for a callback after ScummVM thread has finished
interface MyScummVMDestroyedCallback {
	public void handle(int exitResult);
}
