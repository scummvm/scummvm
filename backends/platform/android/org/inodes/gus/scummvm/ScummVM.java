package org.inodes.gus.scummvm;

import android.content.Context;
import android.content.res.AssetManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import java.io.File;
import java.util.concurrent.Semaphore;


// At least in Android 2.1, eglCreateWindowSurface() requires an
// EGLNativeWindowSurface object, which is hidden deep in the bowels
// of libui.  Until EGL is properly exposed, it's probably safer to
// use the Java versions of most EGL functions :(

public class ScummVM implements SurfaceHolder.Callback {
	private final static String LOG_TAG = "ScummVM.java";

	private final int AUDIO_FRAME_SIZE = 2 * 2;	 // bytes. 16bit audio * stereo
	public static class AudioSetupException extends Exception {}

	private long nativeScummVM;	// native code hangs itself here
	boolean scummVMRunning = false;

	private native void create(AssetManager am);

	public ScummVM(Context context) {
		create(context.getAssets());  // Init C++ code, set nativeScummVM
	}

	private native void nativeDestroy();

	public synchronized void destroy() {
		if (nativeScummVM != 0) {
			nativeDestroy();
			nativeScummVM = 0;
		}
	}
	protected void finalize() {
		destroy();
	}

	// Surface creation:
	// GUI thread: create surface, release lock
	// ScummVM thread: acquire lock (block), read surface
	//
	// Surface deletion:
	// GUI thread: post event, acquire lock (block), return
	// ScummVM thread: read event, free surface, release lock
	//
	// In other words, ScummVM thread does this:
	//	acquire lock
	//	setup surface
	//	when SCREEN_CHANGED arrives:
	//	 destroy surface
	//	 release lock
	//	back to acquire lock
	static final int configSpec[] = {
		EGL10.EGL_RED_SIZE, 5,
		EGL10.EGL_GREEN_SIZE, 5,
		EGL10.EGL_BLUE_SIZE, 5,
		EGL10.EGL_DEPTH_SIZE, 0,
		EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
		EGL10.EGL_NONE,
	};
	EGL10 egl;
	EGLDisplay eglDisplay = EGL10.EGL_NO_DISPLAY;
	EGLConfig eglConfig;
	EGLContext eglContext = EGL10.EGL_NO_CONTEXT;
	EGLSurface eglSurface = EGL10.EGL_NO_SURFACE;
	Semaphore surfaceLock = new Semaphore(0, true);
	SurfaceHolder nativeSurface;

	public void surfaceCreated(SurfaceHolder holder) {
		nativeSurface = holder;
		surfaceLock.release();
	}

	public void surfaceChanged(SurfaceHolder holder, int format,
							   int width, int height) {
		// Disabled while I debug GL problems
		//pushEvent(new Event(Event.EVENT_SCREEN_CHANGED));
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		pushEvent(new Event(Event.EVENT_SCREEN_CHANGED));
		try {
			surfaceLock.acquire();
		} catch (InterruptedException e) {
			Log.e(this.toString(),
				  "Interrupted while waiting for surface lock", e);
		}
	}

	// Called by ScummVM thread (from initBackend)
	private void createScummVMGLContext() {
		egl = (EGL10)EGLContext.getEGL();
		eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
		int[] version = new int[2];
		egl.eglInitialize(eglDisplay, version);
		int[] num_config = new int[1];
		egl.eglChooseConfig(eglDisplay, configSpec, null, 0, num_config);

		final int numConfigs = num_config[0];
		if (numConfigs <= 0)
			throw new IllegalArgumentException("No configs match configSpec");

		EGLConfig[] configs = new EGLConfig[numConfigs];
		egl.eglChooseConfig(eglDisplay, configSpec, configs, numConfigs,
							num_config);
		eglConfig = configs[0];

		eglContext = egl.eglCreateContext(eglDisplay, eglConfig,
										  EGL10.EGL_NO_CONTEXT, null);
	}

	// Called by ScummVM thread
	static private boolean _log_version = true;
	protected void setupScummVMSurface() {
		try {
			surfaceLock.acquire();
		} catch (InterruptedException e) {
			Log.e(this.toString(),
				  "Interrupted while waiting for surface lock", e);
			return;
		}
		eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig,
												nativeSurface, null);
		egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

		GL10 gl = (GL10)eglContext.getGL();

		if (_log_version) {
			Log.i(LOG_TAG, String.format("Using EGL %s (%s); GL %s/%s (%s)",
										 egl.eglQueryString(eglDisplay, EGL10.EGL_VERSION),
										 egl.eglQueryString(eglDisplay, EGL10.EGL_VENDOR),
										 gl.glGetString(GL10.GL_VERSION),
										 gl.glGetString(GL10.GL_RENDERER),
										 gl.glGetString(GL10.GL_VENDOR)));
			_log_version = false; // only log this once
		}

		int[] value = new int[1];
		egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_WIDTH, value);
		int width = value[0];
		egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_HEIGHT, value);
		int height = value[0];
		Log.i(LOG_TAG, String.format("New surface is %dx%d", width, height));
		setSurfaceSize(width, height);
	}

	// Called by ScummVM thread
	protected void destroyScummVMSurface() {
		if (eglSurface != null) {
			egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE,
							   EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			egl.eglDestroySurface(eglDisplay, eglSurface);
			eglSurface = EGL10.EGL_NO_SURFACE;
		}

		surfaceLock.release();
	}

	public void setSurface(SurfaceHolder holder) {
		holder.addCallback(this);
	}

	final public boolean swapBuffers() {
		if (!egl.eglSwapBuffers(eglDisplay, eglSurface)) {
			int error = egl.eglGetError();
			Log.w(LOG_TAG, String.format("eglSwapBuffers exited with error 0x%x", error));
			if (error == EGL11.EGL_CONTEXT_LOST)
				return false;
		}
		return true;
	}

	// Set scummvm config options
	final public native static void loadConfigFile(String path);
	final public native static void setConfMan(String key, int value);
	final public native static void setConfMan(String key, String value);
	final public native void enableZoning(boolean enable);
	final public native void setSurfaceSize(int width, int height);

	// Feed an event to ScummVM.  Safe to call from other threads.
	final public native void pushEvent(Event e);

	final private native void audioMixCallback(byte[] buf);

	// Runs the actual ScummVM program and returns when it does.
	// This should not be called from multiple threads simultaneously...
	final public native int scummVMMain(String[] argv);

	// Callbacks from C++ peer instance
	//protected GraphicsMode[] getSupportedGraphicsModes() {}
	protected void displayMessageOnOSD(String msg) {}
	protected void setWindowCaption(String caption) {}
	protected void showVirtualKeyboard(boolean enable) {}
	protected String[] getSysArchives() { return new String[0]; }
	protected String[] getPluginDirectories() { return new String[0]; }
	protected void initBackend() throws AudioSetupException {
		createScummVMGLContext();
		initAudio();
	}

	private static class AudioThread extends Thread {
		final private int buf_size;
		private boolean is_paused = false;
		final private ScummVM scummvm;
		final private AudioTrack audio_track;

		AudioThread(ScummVM scummvm, AudioTrack audio_track, int buf_size) {
			super("AudioThread");
			this.scummvm = scummvm;
			this.audio_track = audio_track;
			this.buf_size = buf_size;
			setPriority(Thread.MAX_PRIORITY);
			setDaemon(true);
		}

		public void pauseAudio() {
			synchronized (this) {
				is_paused = true;
			}
			audio_track.pause();
		}

		public void resumeAudio() {
			synchronized (this) {
				is_paused = false;
				notifyAll();
			}
			audio_track.play();
		}

		public void run() {
			byte[] buf = new byte[buf_size];
			audio_track.play();
			int offset = 0;
			try {
				while (true) {
					synchronized (this) {
						while (is_paused)
							wait();
					}

					if (offset == buf.length) {
						// Grab new audio data
						scummvm.audioMixCallback(buf);
						offset = 0;
					}
					int len = buf.length - offset;
					int ret = audio_track.write(buf, offset, len);
					if (ret < 0) {
						Log.w(LOG_TAG, String.format(
							"AudioTrack.write(%dB) returned error %d",
							buf.length, ret));
						break;
					} else if (ret != len) {
						Log.w(LOG_TAG, String.format(
							"Short audio write.	 Wrote %dB, not %dB",
							ret, buf.length));
						// Buffer is full, so yield cpu for a while
						Thread.sleep(100);
					}
					offset += ret;
				}
			} catch (InterruptedException e) {
				Log.e(this.toString(), "Audio thread interrupted", e);
			}
		}
	}
	private AudioThread audio_thread;

	final public int audioSampleRate() {
		return AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_MUSIC);
	}

	private void initAudio() throws AudioSetupException {
		int sample_rate = audioSampleRate();
		int buf_size =
			AudioTrack.getMinBufferSize(sample_rate,
										AudioFormat.CHANNEL_CONFIGURATION_STEREO,
										AudioFormat.ENCODING_PCM_16BIT);
		if (buf_size < 0) {
			int guess = AUDIO_FRAME_SIZE * sample_rate / 100;  // 10ms of audio
			Log.w(LOG_TAG, String.format(
										 "Unable to get min audio buffer size (error %d). Guessing %dB.",
										 buf_size, guess));
			buf_size = guess;
		}
		Log.d(LOG_TAG, String.format("Using %dB buffer for %dHZ audio",
									 buf_size, sample_rate));
		AudioTrack audio_track =
			new AudioTrack(AudioManager.STREAM_MUSIC,
						   sample_rate,
						   AudioFormat.CHANNEL_CONFIGURATION_STEREO,
						   AudioFormat.ENCODING_PCM_16BIT,
						   buf_size,
						   AudioTrack.MODE_STREAM);
		if (audio_track.getState() != AudioTrack.STATE_INITIALIZED) {
			Log.e(LOG_TAG, "Error initialising Android audio system.");
			throw new AudioSetupException();
		}

		audio_thread = new AudioThread(this, audio_track, buf_size);
		audio_thread.start();
	}

	public void pause() {
		audio_thread.pauseAudio();
		// TODO: need to pause engine too
	}

	public void resume() {
		// TODO: need to resume engine too
		audio_thread.resumeAudio();
	}

	static {
		// For grabbing with gdb...
		final boolean sleep_for_debugger = false;
		if (sleep_for_debugger) {
			try {
				Thread.sleep(20*1000);
			} catch (InterruptedException e) {
			}
		}

		//System.loadLibrary("scummvm");
		File cache_dir = ScummVMApplication.getLastCacheDir();
		String libname = System.mapLibraryName("scummvm");
		File libpath = new File(cache_dir, libname);
		System.load(libpath.getPath());
	}
}
