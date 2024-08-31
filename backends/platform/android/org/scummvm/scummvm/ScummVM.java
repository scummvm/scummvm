package org.scummvm.scummvm;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Scanner;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

public abstract class ScummVM implements SurfaceHolder.Callback,
	   CompatHelpers.SystemInsets.SystemInsetsListener, Runnable {
	public static final int SHOW_ON_SCREEN_MENU = 1;
	public static final int SHOW_ON_SCREEN_INPUT_MODE = 2;

	final protected static String LOG_TAG = "ScummVM";
	final private AssetManager _asset_manager;
	final private Object _sem_surface;
	final private MyScummVMDestroyedCallback _svm_destroyed_callback;

	private EGL10 _egl;
	private EGLDisplay _egl_display = EGL10.EGL_NO_DISPLAY;
	private EGLConfig _egl_config;
	private EGLContext _egl_context = EGL10.EGL_NO_CONTEXT;
	private EGLSurface _egl_surface = EGL10.EGL_NO_SURFACE;

	private SurfaceHolder _surface_holder;
	private int bitsPerPixel;
	private AudioTrack _audio_track;
	private int _sample_rate = 0;
	private int _buffer_size = 0;

	private String[] _args;

	private native void create(AssetManager asset_manager,
	                           EGL10 egl,
							   EGLDisplay egl_display,
	                           AudioTrack audio_track,
							   int sample_rate,
							   int buffer_size);
	private native void destroy();
	private native void setSurface(int width, int height, int bpp);
	private native int main(String[] args);

	// pause the engine and all native threads
	final public native void setPause(boolean pause);
	// Feed an event to ScummVM.  Safe to call from other threads.
	final public native void pushEvent(int type, int arg1, int arg2, int arg3,
										int arg4, int arg5, int arg6);
	// Update the 3D touch controls
	final public native void setupTouchMode(int oldValue, int newValue);
	final public native void updateTouch(int action, int ptr, int x, int y);

	final public native void syncVirtkeyboardState(boolean newState);

	final public native String getNativeVersionInfo();

	// CompatHelpers.WindowInsets.SystemInsetsListener interface
	@Override
	final public native void systemInsetsUpdated(int insets[]);

	// Callbacks from C++ peer instance
	abstract protected void getDPI(float[] values);
	abstract protected void displayMessageOnOSD(String msg);
	abstract protected void openUrl(String url);
	abstract protected boolean hasTextInClipboard();
	abstract protected String getTextFromClipboard();
	abstract protected boolean setTextInClipboard(String text);
	abstract protected boolean isConnectionLimited();
	abstract protected void setWindowCaption(String caption);
	abstract protected void showVirtualKeyboard(boolean enable);
	abstract protected void showOnScreenControls(int enableMask);
	abstract protected void setTouchMode(int touchMode);
	abstract protected int getTouchMode();
	abstract protected void setOrientation(int orientation);
	abstract protected String getScummVMBasePath();
	abstract protected String getScummVMConfigPath();
	abstract protected String getScummVMLogPath();
	abstract protected void setCurrentGame(String target);
	abstract protected String[] getSysArchives();
	abstract protected String[] getAllStorageLocations();
	abstract protected String[] getAllStorageLocationsNoPermissionRequest();
	abstract protected SAFFSTree getNewSAFTree(boolean folder, boolean write, String initialURI, String prompt);
	abstract protected SAFFSTree[] getSAFTrees();
	abstract protected SAFFSTree findSAFTree(String name);

	public ScummVM(AssetManager asset_manager, SurfaceHolder holder, final MyScummVMDestroyedCallback scummVMDestroyedCallback) {
		_asset_manager = asset_manager;
		_sem_surface = new Object();
		_svm_destroyed_callback = scummVMDestroyedCallback;
		holder.addCallback(this);
	}

	final public String getInstallingScummVMVersionInfo() {
		return getNativeVersionInfo();
	}

	// SurfaceHolder callback
	final public void surfaceCreated(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceCreated");

		// no need to do anything, surfaceChanged() will be called in any case
	}

	// SurfaceHolder callback
	final public void surfaceChanged(SurfaceHolder holder, int format,
										int width, int height) {

		PixelFormat pixelFormat = new PixelFormat();
		PixelFormat.getPixelFormatInfo(format, pixelFormat);
		bitsPerPixel = pixelFormat.bitsPerPixel;

		Log.d(LOG_TAG, String.format(Locale.ROOT, "surfaceChanged: %dx%d (%d: %dbpp)",
										width, height, format, bitsPerPixel));

		// store values for the native code
		// make sure to do it before notifying the lock
		// as it leads to a race condition otherwise
		setSurface(width, height, bitsPerPixel);

		synchronized(_sem_surface) {
			_surface_holder = holder;
			_sem_surface.notifyAll();
		}
	}

	// SurfaceHolder callback
	final public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceDestroyed");

		synchronized(_sem_surface) {
			_surface_holder = null;
			_sem_surface.notifyAll();
		}

		// clear values for the native code
		setSurface(0, 0, 0);
	}

	final public void setArgs(String[] args) {
		_args = args;
	}

	final public void run() {
		try {
			// wait for the surfaceChanged callback
			synchronized(_sem_surface) {
				while (_surface_holder == null)
					_sem_surface.wait();
			}

			initAudio();
			initEGL();
		} catch (Exception e) {
			deinitEGL();
			deinitAudio();

			throw new RuntimeException("Error preparing the ScummVM thread", e);
		}

		create(_asset_manager, _egl, _egl_display,
				_audio_track, _sample_rate, _buffer_size);

		int res = main(_args);

		destroy();

		deinitEGL();
		deinitAudio();

		// Don't exit force-ably here!
		if (_svm_destroyed_callback != null) {
			_svm_destroyed_callback.handle(res);
		}
	}

	private void initEGL() throws Exception {
		_egl = (EGL10)EGLContext.getEGL();
		_egl_display = _egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

		int[] version = new int[2];
		_egl.eglInitialize(_egl_display, version);
		Log.d(LOG_TAG, String.format(Locale.ROOT, "EGL version %d.%d initialized", version[0], version[1]));

		int[] num_config = new int[1];
		_egl.eglGetConfigs(_egl_display, null, 0, num_config);

		final int numConfigs = num_config[0];

		if (numConfigs <= 0)
			throw new IllegalArgumentException("No EGL configs");

		EGLConfig[] configs = new EGLConfig[numConfigs];
		_egl.eglGetConfigs(_egl_display, configs, numConfigs, num_config);

		// Android's eglChooseConfig is busted in several versions and
		// devices so we have to filter/rank the configs ourselves.
		_egl_config = chooseEglConfig(configs, version);

		int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
		int[] attrib_list = { EGL_CONTEXT_CLIENT_VERSION, 2,
		                      EGL10.EGL_NONE };
		_egl_context = _egl.eglCreateContext(_egl_display, _egl_config,
		                                     EGL10.EGL_NO_CONTEXT, attrib_list);

		if (_egl_context == EGL10.EGL_NO_CONTEXT)
			throw new Exception(String.format(Locale.ROOT, "Failed to create context: 0x%x",
												_egl.eglGetError()));
	}

	// Callback from C++ peer instance
	final protected EGLSurface initSurface() throws Exception {
		_egl_surface = _egl.eglCreateWindowSurface(_egl_display, _egl_config,
													_surface_holder, null);

		if (_egl_surface == EGL10.EGL_NO_SURFACE)
			throw new Exception(String.format(Locale.ROOT,
				"eglCreateWindowSurface failed: 0x%x", _egl.eglGetError()));

		_egl.eglMakeCurrent(_egl_display, _egl_surface, _egl_surface,
							_egl_context);

		GL10 gl = (GL10)_egl_context.getGL();

		Log.i(LOG_TAG, String.format(Locale.ROOT, "Using EGL %s (%s); GL %s/%s (%s)",
						_egl.eglQueryString(_egl_display, EGL10.EGL_VERSION),
						_egl.eglQueryString(_egl_display, EGL10.EGL_VENDOR),
						gl.glGetString(GL10.GL_VERSION),
						gl.glGetString(GL10.GL_RENDERER),
						gl.glGetString(GL10.GL_VENDOR)));

		return _egl_surface;
	}

	// Callback from C++ peer instance
	final protected void deinitSurface() {
		if (_egl_display != EGL10.EGL_NO_DISPLAY) {
			_egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (_egl_surface != EGL10.EGL_NO_SURFACE)
				_egl.eglDestroySurface(_egl_display, _egl_surface);
		}

		_egl_surface = EGL10.EGL_NO_SURFACE;
	}

	// Callback from C++ peer instance
	final protected int eglVersion() {
		String version = _egl.eglQueryString(_egl_display, EGL10.EGL_VERSION);
		if (version == null) {
			// 1.0
			return 0x00010000;
		}

		Scanner versionScan = new Scanner(version).useLocale(Locale.ROOT).useDelimiter("[ .]");
		int versionInt = versionScan.nextInt() << 16;
		versionInt |= versionScan.nextInt() & 0xffff;
		return versionInt;
	}

	private void deinitEGL() {
		if (_egl_display != EGL10.EGL_NO_DISPLAY) {
			_egl.eglMakeCurrent(_egl_display, EGL10.EGL_NO_SURFACE,
								EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);

			if (_egl_surface != EGL10.EGL_NO_SURFACE)
				_egl.eglDestroySurface(_egl_display, _egl_surface);

			if (_egl_context != EGL10.EGL_NO_CONTEXT)
				_egl.eglDestroyContext(_egl_display, _egl_context);

			_egl.eglTerminate(_egl_display);
		}

		_egl_surface = EGL10.EGL_NO_SURFACE;
		_egl_context = EGL10.EGL_NO_CONTEXT;
		_egl_config = null;
		_egl_display = EGL10.EGL_NO_DISPLAY;
		_egl = null;
	}

	private void initAudio() throws Exception {
		_sample_rate = AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_MUSIC);
		_buffer_size = AudioTrack.getMinBufferSize(_sample_rate,
		                                           AudioFormat.CHANNEL_OUT_STEREO,
		                                           AudioFormat.ENCODING_PCM_16BIT);

		// ~50ms
		int buffer_size_want = (_sample_rate * 2 * 2 / 20) & ~1023;

		if (_buffer_size < buffer_size_want) {
			Log.w(LOG_TAG, String.format(Locale.ROOT,
				"adjusting audio buffer size (was: %d)", _buffer_size));

			_buffer_size = buffer_size_want;
		}

		Log.i(LOG_TAG, String.format(Locale.ROOT, "Using %d bytes buffer for %dHz audio",
										_buffer_size, _sample_rate));

		CompatHelpers.AudioTrackCompat.AudioTrackCompatReturn audioTrackRet =
			CompatHelpers.AudioTrackCompat.make(_sample_rate, _buffer_size);
		_audio_track = audioTrackRet.audioTrack;
		_buffer_size = audioTrackRet.bufferSize;

		if (_audio_track.getState() != AudioTrack.STATE_INITIALIZED)
			throw new Exception(
				String.format(Locale.ROOT, "Error initializing AudioTrack: %d",
								_audio_track.getState()));
	}

	private void deinitAudio() {
		if (_audio_track != null)
			_audio_track.stop();

		_audio_track = null;
		_buffer_size = 0;
		_sample_rate = 0;
	}

	private static final int[] s_eglAttribs = {
		EGL10.EGL_CONFIG_ID,
		EGL10.EGL_BUFFER_SIZE,
		EGL10.EGL_RED_SIZE,
		EGL10.EGL_GREEN_SIZE,
		EGL10.EGL_BLUE_SIZE,
		EGL10.EGL_ALPHA_SIZE,
		EGL10.EGL_CONFIG_CAVEAT,
		EGL10.EGL_DEPTH_SIZE,
		EGL10.EGL_LEVEL,
		EGL10.EGL_MAX_PBUFFER_WIDTH,
		EGL10.EGL_MAX_PBUFFER_HEIGHT,
		EGL10.EGL_MAX_PBUFFER_PIXELS,
		EGL10.EGL_NATIVE_RENDERABLE,
		EGL10.EGL_NATIVE_VISUAL_ID,
		EGL10.EGL_NATIVE_VISUAL_TYPE,
		EGL10.EGL_SAMPLE_BUFFERS,
		EGL10.EGL_SAMPLES,
		EGL10.EGL_STENCIL_SIZE,
		EGL10.EGL_SURFACE_TYPE,
		EGL10.EGL_TRANSPARENT_TYPE,
		EGL10.EGL_TRANSPARENT_RED_VALUE,
		EGL10.EGL_TRANSPARENT_GREEN_VALUE,
		EGL10.EGL_TRANSPARENT_BLUE_VALUE,
		EGL10.EGL_RENDERABLE_TYPE
	};
	final private static int EGL_OPENGL_ES_BIT = 1;
	final private static int EGL_OPENGL_ES2_BIT = 4;

	final private class EglAttribs  {

		LinkedHashMap<Integer, Integer> _lhm;

		public EglAttribs(EGLConfig config) {
			_lhm = new LinkedHashMap<>(s_eglAttribs.length);

			int[] value = new int[1];

			// prevent throwing IllegalArgumentException
			if (_egl_display == null || config == null) {
				return;
			}

			for (int i : s_eglAttribs) {
				_egl.eglGetConfigAttrib(_egl_display, config, i, value);

				_lhm.put(i, value[0]);
			}
		}

		private int weightBits(int attr, int size) {
			final int value = get(attr);

			int score = 0;

			if (value == size || (size > 0 && value > size))
				score += 10;

			// penalize for wasted bits
			if (value > size)
				score -= value - size;

			return score;
		}

		public int weight() {
			int score = 10000;

			if (get(EGL10.EGL_CONFIG_CAVEAT) != EGL10.EGL_NONE)
				score -= 1000;

			// If there is a config with EGL_OPENGL_ES2_BIT it must be favored
			// This attribute can only be checked with EGL 1.3 but it may be present on older versions
			if ((get(EGL10.EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT) > 0)
				score += 5000;

			// less MSAA is better
			score -= get(EGL10.EGL_SAMPLES) * 100;

			// Must be at least 565, but then smaller is better
			score += weightBits(EGL10.EGL_RED_SIZE, 5);
			score += weightBits(EGL10.EGL_GREEN_SIZE, 6);
			score += weightBits(EGL10.EGL_BLUE_SIZE, 5);
			score += weightBits(EGL10.EGL_ALPHA_SIZE, 0);
			// Prefer 24 bits depth
			score += weightBits(EGL10.EGL_DEPTH_SIZE, 24);
			score += weightBits(EGL10.EGL_STENCIL_SIZE, 8);

			return score;
		}

		@NonNull
		public String toString() {
			String s;

			if (get(EGL10.EGL_ALPHA_SIZE) > 0)
				s = String.format(Locale.ROOT, "[%d] RGBA%d%d%d%d",
									get(EGL10.EGL_CONFIG_ID),
									get(EGL10.EGL_RED_SIZE),
									get(EGL10.EGL_GREEN_SIZE),
									get(EGL10.EGL_BLUE_SIZE),
									get(EGL10.EGL_ALPHA_SIZE));
			else
				s = String.format(Locale.ROOT, "[%d] RGB%d%d%d",
									get(EGL10.EGL_CONFIG_ID),
									get(EGL10.EGL_RED_SIZE),
									get(EGL10.EGL_GREEN_SIZE),
									get(EGL10.EGL_BLUE_SIZE));

			if (get(EGL10.EGL_DEPTH_SIZE) > 0)
				s += String.format(Locale.ROOT, " D%d", get(EGL10.EGL_DEPTH_SIZE));

			if (get(EGL10.EGL_STENCIL_SIZE) > 0)
				s += String.format(Locale.ROOT, " S%d", get(EGL10.EGL_STENCIL_SIZE));

			if (get(EGL10.EGL_SAMPLES) > 0)
				s += String.format(Locale.ROOT, " MSAAx%d", get(EGL10.EGL_SAMPLES));

			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) > 0)
				s += " W";
			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PBUFFER_BIT) > 0)
				s += " P";
			if ((get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_PIXMAP_BIT) > 0)
				s += " X";

			if ((get(EGL10.EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES_BIT) > 0)
				s += " ES";
			if ((get(EGL10.EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT) > 0)
				s += " ES2";


			switch (get(EGL10.EGL_CONFIG_CAVEAT)) {
			case EGL10.EGL_NONE:
				break;

			case EGL10.EGL_SLOW_CONFIG:
				s += " SLOW";
				break;

			case EGL10.EGL_NON_CONFORMANT_CONFIG:
				s += " NON_CONFORMANT";

			default:
				s += String.format(Locale.ROOT, " unknown CAVEAT 0x%x",
									get(EGL10.EGL_CONFIG_CAVEAT));
			}

			return s;
		}

		public Integer get(Integer key) {
			if (_lhm.containsKey(key) && _lhm.get(key) != null) {
				return _lhm.get(key);
			} else {
				return 0;
			}
		}
	}

	private EGLConfig chooseEglConfig(EGLConfig[] configs, int[] version) {
		EGLConfig res = configs[0];
		int bestScore = -1;

		Log.d(LOG_TAG, "EGL configs:");

		for (EGLConfig config : configs) {
			if (config != null) {
				boolean good = true;

				EglAttribs attr = new EglAttribs(config);

				// must have
				if ((attr.get(EGL10.EGL_SURFACE_TYPE) & EGL10.EGL_WINDOW_BIT) == 0)
					good = false;

				if (version[0] >= 2 ||
					(version[0] == 1 && version[1] >= 3)) {
					// EGL_OPENGL_ES2_BIT is only supported since EGL 1.3
					if ((attr.get(EGL10.EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT) == 0)
						good = false;
				}
				if (attr.get(EGL10.EGL_BUFFER_SIZE) < bitsPerPixel)
					good = false;

				// Force a config with a depth buffer and a stencil buffer when rendering directly on backbuffer
				if ((attr.get(EGL10.EGL_DEPTH_SIZE) == 0) || (attr.get(EGL10.EGL_STENCIL_SIZE) == 0))
					good = false;

				int score = attr.weight();

				Log.d(LOG_TAG, String.format(Locale.ROOT, "%s (%d, %s)", attr.toString(), score, good ? "OK" : "NOK"));

				if (!good) {
					continue;
				}

				if (score > bestScore) {
					res = config;
					bestScore = score;
				}
			}
		}

		if (bestScore < 0)
			Log.e(LOG_TAG,
					"Unable to find an acceptable EGL config, expect badness.");

		Log.d(LOG_TAG, String.format(Locale.ROOT, "Chosen EGL config: %s",
										new EglAttribs(res).toString()));

		return res;
	}

	static {
//		// For grabbing with gdb...
//		final boolean sleep_for_debugger = false;
//		if (sleep_for_debugger) {
//			try {
//				Thread.sleep(20 * 1000);
//			} catch (InterruptedException ignored) {
//			}
//		}

		System.loadLibrary("scummvm");
	}
}
