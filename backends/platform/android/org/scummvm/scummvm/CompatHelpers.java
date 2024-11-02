package org.scummvm.scummvm;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Insets;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.DisplayCutout;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.accessibility.AccessibilityEvent;

import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;

class CompatHelpers {
	static class HideSystemStatusBar {

		public static void hide(final Window window) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				HideSystemStatusBarR.hide(window);
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
				HideSystemStatusBarKitKat.hide(window);
			} else {
				HideSystemStatusBarJellyBean.hide(window);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.JELLY_BEAN)
		@SuppressWarnings("deprecation")
		private static class HideSystemStatusBarJellyBean {
			public static void hide(final Window window) {
				View view = window.getDecorView();
				view.setSystemUiVisibility(
					view.getSystemUiVisibility() |
					View.SYSTEM_UI_FLAG_LOW_PROFILE);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.KITKAT)
		@SuppressWarnings("deprecation")
		private static class HideSystemStatusBarKitKat {
			public static void hide(final Window window) {
				View view = window.getDecorView();
				view.setSystemUiVisibility(
					(view.getSystemUiVisibility() & ~View.SYSTEM_UI_FLAG_IMMERSIVE) |
					View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
					View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_FULLSCREEN);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class HideSystemStatusBarR {
			public static void hide(final Window window) {
				WindowInsetsController insetsController = window.getInsetsController();
				insetsController.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
				insetsController.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
			}
		}
	}

	static class SystemInsets {
		public interface SystemInsetsListener {
			void systemInsetsUpdated(int[] gestureInsets, int[] systemInsets, int[] cutoutInsets);
		}

		public static void registerSystemInsetsListener(View v, SystemInsetsListener l) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				v.setOnApplyWindowInsetsListener(new OnApplyWindowInsetsListenerR(l));
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.Q) {
				v.setOnApplyWindowInsetsListener(new OnApplyWindowInsetsListenerQ(l));
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
				v.setOnApplyWindowInsetsListener(new OnApplyWindowInsetsListenerP(l));
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
				v.setOnApplyWindowInsetsListener(new OnApplyWindowInsetsListenerLollipop(l));
			} else {
				// Not available
				int[] gestureInsets = new int[] { 0, 0, 0, 0 };
				int[] systemInsets = new int[] { 0, 0, 0, 0 };
				int[] cutoutInsets = new int[] { 0, 0, 0, 0 };
				l.systemInsetsUpdated(gestureInsets, systemInsets, cutoutInsets);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
		@SuppressWarnings("deprecation")
		private static class OnApplyWindowInsetsListenerLollipop implements View.OnApplyWindowInsetsListener {
			final private SystemInsetsListener l;

			public OnApplyWindowInsetsListenerLollipop(SystemInsetsListener l) {
				this.l = l;
			}

			@Override
			public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
				// No system gestures inset before Android Q
				int[] gestureInsets = new int[] {
					insets.getStableInsetLeft(),
					insets.getStableInsetTop(),
					insets.getStableInsetRight(),
					insets.getStableInsetBottom()
				};
				int[] systemInsets = new int[] {
					insets.getSystemWindowInsetLeft(),
					insets.getSystemWindowInsetTop(),
					insets.getSystemWindowInsetRight(),
					insets.getSystemWindowInsetBottom()
				};
				// No cutouts before Android P
				int[] cutoutInsets = new int[] { 0, 0, 0, 0 };
				l.systemInsetsUpdated(gestureInsets, systemInsets, cutoutInsets);
				return v.onApplyWindowInsets(insets);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.P)
		@SuppressWarnings("deprecation")
		private static class OnApplyWindowInsetsListenerP implements View.OnApplyWindowInsetsListener {
			final private SystemInsetsListener l;

			public OnApplyWindowInsetsListenerP(SystemInsetsListener l) {
				this.l = l;
			}

			@Override
			public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
				// No system gestures inset before Android Q
				int[] gestureInsets = new int[] {
					insets.getStableInsetLeft(),
					insets.getStableInsetTop(),
					insets.getStableInsetRight(),
					insets.getStableInsetBottom()
				};
				int[] systemInsets = new int[] {
					insets.getSystemWindowInsetLeft(),
					insets.getSystemWindowInsetTop(),
					insets.getSystemWindowInsetRight(),
					insets.getSystemWindowInsetBottom()
				};
				int[] cutoutInsets;
				DisplayCutout cutout = insets.getDisplayCutout();
				if (cutout == null) {
					cutoutInsets = new int[] { 0, 0, 0, 0 };
				} else {
					cutoutInsets = new int[] {
						cutout.getSafeInsetLeft(),
						cutout.getSafeInsetTop(),
						cutout.getSafeInsetRight(),
						cutout.getSafeInsetBottom()
					};
				}
				l.systemInsetsUpdated(gestureInsets, systemInsets, cutoutInsets);
				return v.onApplyWindowInsets(insets);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.Q)
		@SuppressWarnings("deprecation")
		private static class OnApplyWindowInsetsListenerQ implements View.OnApplyWindowInsetsListener {
			final private SystemInsetsListener l;

			public OnApplyWindowInsetsListenerQ(SystemInsetsListener l) {
				this.l = l;
			}

			@Override
			public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
				Insets insetsStruct = insets.getSystemGestureInsets();
				int[] gestureInsets = new int[] {
					insetsStruct.left,
					insetsStruct.top,
					insetsStruct.right,
					insetsStruct.bottom,
				};
				insetsStruct = insets.getSystemWindowInsets();
				int[] systemInsets = new int[] {
					insetsStruct.left,
					insetsStruct.top,
					insetsStruct.right,
					insetsStruct.bottom,
				};
				int[] cutoutInsets;
				DisplayCutout cutout = insets.getDisplayCutout();
				if (cutout == null) {
					cutoutInsets = new int[] { 0, 0, 0, 0 };
				} else {
					cutoutInsets = new int[] {
						cutout.getSafeInsetLeft(),
						cutout.getSafeInsetTop(),
						cutout.getSafeInsetRight(),
						cutout.getSafeInsetBottom()
					};
				}
				l.systemInsetsUpdated(gestureInsets, systemInsets, cutoutInsets);
				return v.onApplyWindowInsets(insets);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class OnApplyWindowInsetsListenerR implements View.OnApplyWindowInsetsListener {
			final private SystemInsetsListener l;

			public OnApplyWindowInsetsListenerR(SystemInsetsListener l) {
				this.l = l;
			}

			@Override
			public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
				Insets insetsStruct = insets.getInsetsIgnoringVisibility(WindowInsets.Type.systemGestures());
				int[] gestureInsets = new int[] {
					insetsStruct.left,
					insetsStruct.top,
					insetsStruct.right,
					insetsStruct.bottom,
				};
				insetsStruct = insets.getInsetsIgnoringVisibility(WindowInsets.Type.systemBars());
				int[] systemInsets = new int[] {
					insetsStruct.left,
					insetsStruct.top,
					insetsStruct.right,
					insetsStruct.bottom,
				};
				int[] cutoutInsets;
				DisplayCutout cutout = insets.getDisplayCutout();
				if (cutout == null) {
					cutoutInsets = new int[] { 0, 0, 0, 0 };
				} else {
					cutoutInsets = new int[] {
						cutout.getSafeInsetLeft(),
						cutout.getSafeInsetTop(),
						cutout.getSafeInsetRight(),
						cutout.getSafeInsetBottom()
					};
				}
				l.systemInsetsUpdated(gestureInsets, systemInsets, cutoutInsets);
				return v.onApplyWindowInsets(insets);
			}
		}
	}

	static class AudioTrackCompat {
		public static class AudioTrackCompatReturn {
			public AudioTrack audioTrack;
			public int bufferSize;
		}

		public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
				return AudioTrackCompatM.make(sample_rate, buffer_size);
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
				return AudioTrackCompatLollipop.make(sample_rate, buffer_size);
			} else {
				return AudioTrackCompatOld.make(sample_rate, buffer_size);
			}
		}

		/**
		 * Support for Android KitKat or lower
		 */
		@SuppressWarnings("deprecation")
		private static class AudioTrackCompatOld {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					AudioManager.STREAM_MUSIC,
					sample_rate,
					AudioFormat.CHANNEL_OUT_STEREO,
					AudioFormat.ENCODING_PCM_16BIT,
					buffer_size,
					AudioTrack.MODE_STREAM);
				ret.bufferSize = buffer_size;
				return ret;
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
		private static class AudioTrackCompatLollipop {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					new AudioAttributes.Builder()
						.setUsage(AudioAttributes.USAGE_MEDIA)
						.setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
						.build(),
					new AudioFormat.Builder()
						.setSampleRate(sample_rate)
						.setEncoding(AudioFormat.ENCODING_PCM_16BIT)
						.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build(),
					buffer_size,
					AudioTrack.MODE_STREAM,
					AudioManager.AUDIO_SESSION_ID_GENERATE);
				ret.bufferSize = buffer_size;
				return ret;
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.M)
		private static class AudioTrackCompatM {
			public static AudioTrackCompatReturn make(int sample_rate, int buffer_size) {
				AudioTrackCompatReturn ret = new AudioTrackCompatReturn();
				ret.audioTrack = new AudioTrack(
					new AudioAttributes.Builder()
						.setUsage(AudioAttributes.USAGE_MEDIA)
						.setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
						.build(),
					new AudioFormat.Builder()
						.setSampleRate(sample_rate)
						.setEncoding(AudioFormat.ENCODING_PCM_16BIT)
						.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build(),
					buffer_size,
					AudioTrack.MODE_STREAM,
					AudioManager.AUDIO_SESSION_ID_GENERATE);
				// Keep track of the actual obtained audio buffer size, if supported.
				// We just requested 16 bit PCM stereo pcm so there are 4 bytes per frame.
				ret.bufferSize = ret.audioTrack.getBufferSizeInFrames() * 4;
				return ret;
			}
		}
	}

	static class AccessibilityEventConstructor {
		public static AccessibilityEvent make(int eventType) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				return AccessibilityEventConstructorR.make(eventType);
			} else {
				return AccessibilityEventConstructorOld.make(eventType);
			}
		}

		@SuppressWarnings("deprecation")
		private static class AccessibilityEventConstructorOld {
			public static AccessibilityEvent make(int eventType) {
				return AccessibilityEvent.obtain(eventType);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class AccessibilityEventConstructorR {
			public static AccessibilityEvent make(int eventType) {
				return new AccessibilityEvent(eventType);
			}
		}
	}

	static class ShortcutCreator {
		public static Intent createShortcutResultIntent(@NonNull Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N_MR1) {
				return ShortcutCreatorN_MR1.createShortcutResultIntent(context, id, intent, label, icon, fallbackIconId);
			} else {
				return ShortcutCreatorOld.createShortcutResultIntent(context, id, intent, label, icon, fallbackIconId);
			}
		}

		public static void pushDynamicShortcut(@NonNull Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
				ShortcutCreatorR.pushDynamicShortcut(context, id, intent, label, icon, fallbackIconId);
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N_MR1) {
				ShortcutCreatorN_MR1.pushDynamicShortcut(context, id, intent, label, icon, fallbackIconId);
			}
			// No support for older versions
		}

		@SuppressWarnings("deprecation")
		private static class ShortcutCreatorOld {
			public static Intent createShortcutResultIntent(@NonNull Context context, String ignoredId, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				Intent result = new Intent();

				if (icon == null) {
					icon = DrawableCompat.getDrawable(context, fallbackIconId);
				}
				Objects.requireNonNull(icon);
				Bitmap bmIcon = drawableToBitmap(icon);

				addToIntent(result, intent, label, bmIcon);
				return result;
			}

			public static void addToIntent(Intent result, @NonNull Intent intent, @NonNull String label, @NonNull Bitmap icon) {
				result.putExtra(Intent.EXTRA_SHORTCUT_INTENT, intent);
				result.putExtra(Intent.EXTRA_SHORTCUT_NAME, label);
				result.putExtra(Intent.EXTRA_SHORTCUT_ICON, icon);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.N_MR1)
		private static class ShortcutCreatorN_MR1 {
			public static ShortcutInfo createShortcutInfo(Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Icon icon) {
				ShortcutInfo.Builder builder = new ShortcutInfo.Builder(context, id);
				builder.setIntent(intent);
				builder.setShortLabel(label);
				builder.setIcon(icon);
				HashSet<String> categories = new HashSet<>(1);
				categories.add("actions.intent.START_GAME_EVENT");
				builder.setCategories(categories);
				return builder.build();
			}

			public static Intent createShortcutResultIntent(Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				Bitmap bm;
				Icon ic;
				if (icon != null) {
					bm = drawableToBitmap(icon);
					ic = Icon.createWithBitmap(bm);
				} else {
					icon = DrawableCompat.getDrawable(context, fallbackIconId);
					Objects.requireNonNull(icon);
					bm = drawableToBitmap(icon);
					ic = Icon.createWithResource(context, fallbackIconId);
				}
				ShortcutInfo si = createShortcutInfo(context, id, intent, label, ic);

				Intent result = null;
				if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
					final ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);
					result = shortcutManager.createShortcutResultIntent(si);
				}
				if (result == null) {
					result = new Intent();
				}
				ShortcutCreatorOld.addToIntent(result, intent, label, bm);
				return result;
			}

			public static void pushDynamicShortcut(@NonNull Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				Icon ic;
				if (icon != null) {
					ic = Icon.createWithBitmap(drawableToBitmap(icon));
				} else {
					ic = Icon.createWithResource(context, fallbackIconId);
				}
				ShortcutInfo si = createShortcutInfo(context, id, intent, label, ic);

				final ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);
				if (shortcutManager.isRateLimitingActive()) {
					return;
				}
				List<ShortcutInfo> shortcuts = shortcutManager.getDynamicShortcuts();
				// Sort shortcuts by rank, timestamp and id
				Collections.sort(shortcuts, new Comparator<ShortcutInfo>() {
					@Override
					public int compare(ShortcutInfo a, ShortcutInfo b) {
						int ret = Integer.compare(a.getRank(), b.getRank());
						if (ret != 0) {
							return ret;
						}

						ret = Long.compare(a.getLastChangedTimestamp(), b.getLastChangedTimestamp());
						if (ret != 0) {
							return ret;
						}

						return a.getId().compareTo(b.getId());
					}
				});

				// In old Android versions, only 4 shortcuts are displayed but 5 maximum are supported
				// Problem: the last one added is not displayed... so stick to 4
				int maxSize = Math.min(shortcutManager.getMaxShortcutCountPerActivity(), 4);
				if (shortcuts.size() >= maxSize) {
					int toRemove = shortcuts.size() - maxSize + 1;
					ArrayList<String> toRemoveList = new ArrayList<>(toRemove);
					// Remove the lowest rank, oldest shortcut if we need it
					for(ShortcutInfo oldSi : shortcuts) {
						if (oldSi.getId().equals(id)) {
							// We will update it: no need to make space
							toRemoveList.clear();
							break;
						}
						if (toRemove > 0) {
							toRemoveList.add(oldSi.getId());
							toRemove -= 1;
						}
					}
					shortcutManager.removeDynamicShortcuts(toRemoveList);
				}
				shortcuts = new ArrayList<>(1);
				shortcuts.add(si);
				shortcutManager.addDynamicShortcuts(shortcuts);
				shortcutManager.reportShortcutUsed(id);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.R)
		private static class ShortcutCreatorR {
			public static void pushDynamicShortcut(@NonNull Context context, String id, @NonNull Intent intent, @NonNull String label, @Nullable Drawable icon, @DrawableRes int fallbackIconId) {
				Icon ic;
				if (icon != null) {
					ic = Icon.createWithBitmap(drawableToBitmap(icon));
				} else {
					ic = Icon.createWithResource(context, fallbackIconId);
				}
				ShortcutInfo si = ShortcutCreatorN_MR1.createShortcutInfo(context, id, intent, label, ic);

				final ShortcutManager shortcutManager = context.getSystemService(ShortcutManager.class);
				shortcutManager.pushDynamicShortcut(si);
				// pushDynamicShortcut already reports usage
			}
		}

		private static Bitmap drawableToBitmap(@NonNull Drawable drawable) {
			// We resize to 128x128 to avoid having too big bitmaps for Binder
			if (drawable instanceof BitmapDrawable) {
				Bitmap bm = ((BitmapDrawable)drawable).getBitmap();
				bm = Bitmap.createScaledBitmap(bm, 128, 128, true);
				return bm.copy(bm.getConfig(), false);
			}

			Bitmap bitmap = Bitmap.createBitmap(128, 128, Bitmap.Config.ARGB_8888);
			Canvas canvas = new Canvas(bitmap);
			drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
			drawable.draw(canvas);

			// Create an immutable bitmap
			return bitmap.copy(bitmap.getConfig(), false);
		}
	}

	static class DrawableCompat {
		public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
				return DrawableCompatLollipop.getDrawable(context, id);
			} else {
				return DrawableCompatOld.getDrawable(context, id);
			}
		}

		@RequiresApi(android.os.Build.VERSION_CODES.LOLLIPOP)
		private static class DrawableCompatLollipop {
			@SuppressLint("UseCompatLoadingForDrawables")
			public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
				return context.getDrawable(id);
			}
		}

		@SuppressWarnings("deprecation")
		private static class DrawableCompatOld {
			@SuppressLint("UseCompatLoadingForDrawables")
			public static Drawable getDrawable(@NonNull Context context, @DrawableRes int id) throws Resources.NotFoundException {
				return context.getResources().getDrawable(id);
			}
		}
	}
}
