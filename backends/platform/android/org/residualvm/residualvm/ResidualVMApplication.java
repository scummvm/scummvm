package org.residualvm.residualvm;

import android.app.Application;

import java.io.File;

public class ResidualVMApplication extends Application {
	public final static String ACTION_PLUGIN_QUERY = "org.residualvm.residualvm.action.PLUGIN_QUERY";
	public final static String EXTRA_UNPACK_LIBS = "org.residualvm.residualvm.extra.UNPACK_LIBS";
	public final static String EXTRA_VERSION = "org.residualvm.residualvm.extra.VERSION";

	private static File _cache_dir;

	@Override
	public void onCreate() {
		super.onCreate();

		// This is still on /data :(
		_cache_dir = getCacheDir();
		// This is mounted noexec :(
		//cache_dir = new File(Environment.getExternalStorageDirectory(),
		//				"/.ResidualVM.tmp");
		// This is owned by download manager and requires special
		// permissions to access :(
		//cache_dir = Environment.getDownloadCacheDirectory();
	}

	public static File getLastCacheDir() {
		return _cache_dir;
	}
}
