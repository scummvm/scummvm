package org.inodes.gus.scummvm;

import android.util.Log;
import android.content.res.AssetManager;
import android.view.SurfaceHolder;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import java.io.File;
import java.util.Map;
import java.util.LinkedHashMap;

public abstract class ScummVM implements SurfaceHolder.Callback, Runnable {
	final protected static String LOG_TAG = "ScummVM";
	final private AssetManager asset_manager;
	final private Object sem_surface;

	private EGL10 egl;
	private EGLDisplay eglDisplay = EGL10.EGL_NO_DISPLAY;
	private EGLConfig eglConfig;
	private EGLContext eglContext = EGL10.EGL_NO_CONTEXT;
	private EGLSurface eglSurface = EGL10.EGL_NO_SURFACE;

	private SurfaceHolder surface_holder;
	private AudioTrack audio_track;
	private int sample_rate = 0;
	private int buffer_size = 0;

	private String[] args;

	final private native void create(AssetManager asset_manager,
										EGL10 egl, EGLDisplay eglDisplay,
										AudioTrack audio_track,
										int sample_rate, int buffer_size);
	final private native void destroy();
	final private native void setSurface(int width, int height);
	final private native int main(String[] args);
	final private native void pauseEngine(boolean pause);

	// Set scummvm config options
	final public native void enableZoning(boolean enable);
	// Feed an event to ScummVM.  Safe to call from other threads.
	final public native void pushEvent(Event e);

	// Callbacks from C++ peer instance
	abstract protected void displayMessageOnOSD(String msg);
	abstract protected void setWindowCaption(String caption);
	abstract protected String[] getPluginDirectories();
	abstract protected void showVirtualKeyboard(boolean enable);
	abstract protected String[] getSysArchives();

	public ScummVM(AssetManager asset_manager, SurfaceHolder holder) {
		this.asset_manager = asset_manager;
		sem_surface = new Object();

		holder.addCallback(this);
	}

	// SurfaceHolder callback
	final public void surfaceCreated(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceCreated");

		// no need to do anything, surfaceChanged() will be called in any case
	}

	// SurfaceHolder callback
	final public void surfaceChanged(SurfaceHolder holder, int format,
										int width, int height) {
		Log.d(LOG_TAG, String.format("surfaceChanged: %dx%d (%d)",
										width, height, format));

		synchronized(sem_surface) {
			surface_holder = holder;
			sem_surface.notifyAll();
		}

		// store values for the native code
		setSurface(width, height);
	}

	// SurfaceHolder callback
	final public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceDestroyed");

		synchronized(sem_surface) {
			surface_holder = null;
			sem_surface.notifyAll();
		}

		// clear values for the native code
		setSurface(0, 0);
	}

	final public void setArgs(String[] args) {
		this.args = args;
	}

	final public void run() {
		try {
			initAudio();
			initEGL();

			// wait for the surfaceChanged callback
			synchronized(sem_surface) {
				while (surface_holder == null)
					sem_surface.wait();
			}
		} catch (Exception e) {
			deinitEGL();
			deinitAudio();

			throw new RuntimeException("Error preparing the ScummVM thread", e);
		}

		create(asset_manager, egl, eglDisplay,
				audio_track, sample_rate, buffer_size);

		int res = main(args);

		destroy();

		deinitEGL();
		deinitAudio();

		// On exit, tear everything down for a fresh restart next time.
		System.exit(res);
	}

	public void pause(boolean pause) {
		if (audio_track != null && !pause)
			audio_track.play();

		pauseEngine(pause);

		if (audio_track != null && pause)
			audio_track.stop();
	}

	final private void initEGL() throws Exception {
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

		if (false) {
			Log.d(LOG_TAG, String.format("Found %d EGL configurations.",
											numConfigs));
			for (EGLConfig config : configs)
				dumpEglConfig(config);
		}

		// Android's eglChooseConfig is busted in several versions and
		// devices so we have to filter/rank the configs again ourselves.
		eglConfig = chooseEglConfig(configs);

		if (false) {
			Log.d(LOG_TAG, String.format("Chose from %d EGL configs",
											numConfigs));
			dumpEglConfig(eglConfig);
		}

		eglContext = egl.eglCreateContext(eglDisplay, eglConfig,
											EGL10.EGL_NO_CONTEXT, null);

		if (eglContext == EGL10.EGL_NO_CONTEXT)
			throw new Exception(String.format("Failed to create context: 0x%x",
												egl.eglGetError()));
	}

	// Callback from C++ peer instance
	final protected EGLSurface initSurface() throws Exception {
		eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig,
												surface_holder, null);

		if (eglSurface == EGL10.EGL_NO_SURFACE)
			throw new Exception(String.format(
					"eglCreateWindowSurface failed: 0x%x", egl.eglGetError()));

		egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

		GL10 gl = (GL10)eglContext.getGL();

		Log.i(LOG_TAG, String.format("Using EGL %s (%s); GL %s/%s (%s)",
						egl.eglQueryString(eglDisplay, EGL10.EGL_VERSION),
						egl.eglQueryString(eglDisplay, EGL10.EGL_VENDOR),
						gl.glGetString(GL10.GL_VERSION),
						gl.glGetString(GL10.GL_RENDERER),
						gl.glGetString(GL10.GL_VENDOR)));

		return eglSurface;
	}

	// Callback from C++ peer instance
	final protected void deinitSurface() {
		if (eglDisplay != EGL10.EGL_NO_DISPLAY) {
			egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (eglSurface != EGL10.EGL_NO_SURFACE)
				egl.eglDestroySurface(eglDisplay, eglSurface);
		}

		eglSurface = EGL10.EGL_NO_SURFACE;
	}

	final private void deinitEGL() {
		if (eglDisplay != EGL10.EGL_NO_DISPLAY) {
			egl.eglMakeCurrent(eglDisplay, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (eglSurface != EGL10.EGL_NO_SURFACE)
				egl.eglDestroySurface(eglDisplay, eglSurface);

			if (eglContext != EGL10.EGL_NO_CONTEXT)
				egl.eglDestroyContext(eglDisplay, eglContext);

			egl.eglTerminate(eglDisplay);
		}

		eglSurface = EGL10.EGL_NO_SURFACE;
		eglContext = EGL10.EGL_NO_CONTEXT;
		eglConfig = null;
		eglDisplay = EGL10.EGL_NO_DISPLAY;
		egl = null;
	}

	final private void initAudio() throws Exception {
		sample_rate = AudioTrack.getNativeOutputSampleRate(
									AudioManager.STREAM_MUSIC);
		buffer_size = AudioTrack.getMinBufferSize(sample_rate,
									AudioFormat.CHANNEL_CONFIGURATION_STEREO,
									AudioFormat.ENCODING_PCM_16BIT);

		// ~100ms
		int buffer_size_want = (sample_rate * 2 * 2 / 10) & ~1023;

		if (buffer_size < buffer_size_want) {
			Log.w(LOG_TAG, String.format(
				"adjusting audio buffer size (was: %d)", buffer_size));

			buffer_size = buffer_size_want;
		}

		Log.i(LOG_TAG, String.format("Using %d bytes buffer for %dHz audio",
										buffer_size, sample_rate));

		audio_track = new AudioTrack(AudioManager.STREAM_MUSIC,
									sample_rate,
									AudioFormat.CHANNEL_CONFIGURATION_STEREO,
									AudioFormat.ENCODING_PCM_16BIT,
									buffer_size,
									AudioTrack.MODE_STREAM);

		if (audio_track.getState() != AudioTrack.STATE_INITIALIZED)
			throw new Exception(
				String.format("Error initialising AudioTrack: %d",
								audio_track.getState()));
	}

	final private void deinitAudio() {
		if (audio_track != null)
			audio_track.stop();

		audio_track = null;
		buffer_size = 0;
		sample_rate = 0;
	}

	static final int configSpec[] = {
		EGL10.EGL_RED_SIZE, 5,
		EGL10.EGL_GREEN_SIZE, 5,
		EGL10.EGL_BLUE_SIZE, 5,
		EGL10.EGL_DEPTH_SIZE, 0,
		EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
		EGL10.EGL_NONE,
	};

	// For debugging
	private static final Map<String, Integer> attribs;

	static {
		attribs = new LinkedHashMap<String, Integer>();
		attribs.put("CONFIG_ID", EGL10.EGL_CONFIG_ID);
		attribs.put("BUFFER_SIZE", EGL10.EGL_BUFFER_SIZE);
		attribs.put("RED_SIZE", EGL10.EGL_RED_SIZE);
		attribs.put("GREEN_SIZE", EGL10.EGL_GREEN_SIZE);
		attribs.put("BLUE_SIZE", EGL10.EGL_BLUE_SIZE);
		attribs.put("ALPHA_SIZE", EGL10.EGL_ALPHA_SIZE);
		//attribs.put("BIND_TO_RGB", EGL10.EGL_BIND_TO_TEXTURE_RGB);
		//attribs.put("BIND_TO_RGBA", EGL10.EGL_BIND_TO_TEXTURE_RGBA);
		attribs.put("CONFIG_CAVEAT", EGL10.EGL_CONFIG_CAVEAT);
		attribs.put("DEPTH_SIZE", EGL10.EGL_DEPTH_SIZE);
		attribs.put("LEVEL", EGL10.EGL_LEVEL);
		attribs.put("MAX_PBUFFER_WIDTH", EGL10.EGL_MAX_PBUFFER_WIDTH);
		attribs.put("MAX_PBUFFER_HEIGHT", EGL10.EGL_MAX_PBUFFER_HEIGHT);
		attribs.put("MAX_PBUFFER_PIXELS", EGL10.EGL_MAX_PBUFFER_PIXELS);
		//attribs.put("MAX_SWAP_INTERVAL", EGL10.EGL_MAX_SWAP_INTERVAL);
		//attribs.put("MIN_SWAP_INTERVAL", EGL10.EGL_MIN_SWAP_INTERVAL);
		attribs.put("NATIVE_RENDERABLE", EGL10.EGL_NATIVE_RENDERABLE);
		attribs.put("NATIVE_VISUAL_ID", EGL10.EGL_NATIVE_VISUAL_ID);
		attribs.put("NATIVE_VISUAL_TYPE", EGL10.EGL_NATIVE_VISUAL_TYPE);
		attribs.put("SAMPLE_BUFFERS", EGL10.EGL_SAMPLE_BUFFERS);
		attribs.put("SAMPLES", EGL10.EGL_SAMPLES);
		attribs.put("STENCIL_SIZE", EGL10.EGL_STENCIL_SIZE);
		attribs.put("SURFACE_TYPE", EGL10.EGL_SURFACE_TYPE);
		attribs.put("TRANSPARENT_TYPE", EGL10.EGL_TRANSPARENT_TYPE);
		attribs.put("TRANSPARENT_RED_VALUE", EGL10.EGL_TRANSPARENT_RED_VALUE);
		attribs.put("TRANSPARENT_GREEN_VALUE", EGL10.EGL_TRANSPARENT_GREEN_VALUE);
		attribs.put("TRANSPARENT_BLUE_VALUE", EGL10.EGL_TRANSPARENT_BLUE_VALUE);
	}

	final private void dumpEglConfig(EGLConfig config) {
		int[] value = new int[1];

		for (Map.Entry<String, Integer> entry : attribs.entrySet()) {
			egl.eglGetConfigAttrib(eglDisplay, config,
									entry.getValue(), value);

			if (value[0] == EGL10.EGL_NONE)
				Log.d(LOG_TAG, entry.getKey() + ": NONE");
			else
				Log.d(LOG_TAG, String.format("%s: %d", entry.getKey(), value[0]));
		}
	}

	final private EGLConfig chooseEglConfig(EGLConfig[] configs) {
		int best = 0;
		int bestScore = -1;
		int[] value = new int[1];

		for (int i = 0; i < configs.length; i++) {
			EGLConfig config = configs[i];
			int score = 10000;

			egl.eglGetConfigAttrib(eglDisplay, config,
									EGL10.EGL_SURFACE_TYPE, value);

			// must have
			if ((value[0] & EGL10.EGL_WINDOW_BIT) == 0)
				continue;

			egl.eglGetConfigAttrib(eglDisplay, config,
									EGL10.EGL_CONFIG_CAVEAT, value);

			if (value[0] != EGL10.EGL_NONE)
				score -= 1000;

			// Must be at least 555, but then smaller is better
			final int[] colorBits = { EGL10.EGL_RED_SIZE,
										EGL10.EGL_GREEN_SIZE,
										EGL10.EGL_BLUE_SIZE,
										EGL10.EGL_ALPHA_SIZE
									};

			for (int component : colorBits) {
				egl.eglGetConfigAttrib(eglDisplay, config, component, value);

				// boost if >5 bits accuracy
				if (value[0] >= 5)
					score += 10;

				// penalize for wasted bits
				score -= value[0];
			}

			egl.eglGetConfigAttrib(eglDisplay, config,
									EGL10.EGL_DEPTH_SIZE, value);

			// penalize for wasted bits
			score -= value[0];

			if (score > bestScore) {
				best = i;
				bestScore = score;
			}
		}

		if (bestScore < 0) {
			Log.e(LOG_TAG, "Unable to find an acceptable EGL config, expect badness.");
			return configs[0];
		}

		return configs[best];
	}

	static {
		// For grabbing with gdb...
		final boolean sleep_for_debugger = false;
		if (sleep_for_debugger) {
			try {
				Thread.sleep(20 * 1000);
			} catch (InterruptedException e) {
			}
		}

		File cache_dir = ScummVMApplication.getLastCacheDir();
		String libname = System.mapLibraryName("scummvm");
		File libpath = new File(cache_dir, libname);

		System.load(libpath.getPath());
	}
}

