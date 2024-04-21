package org.scummvm.scummvm;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.accessibility.AccessibilityEvent;

import androidx.annotation.RequiresApi;

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
}
