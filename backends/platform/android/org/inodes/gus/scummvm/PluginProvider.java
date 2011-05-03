package org.inodes.gus.scummvm;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import java.util.ArrayList;

public class PluginProvider extends BroadcastReceiver {
	private final static String LOG_TAG = "ScummVM";

	public final static String META_UNPACK_LIB =
		"org.inodes.gus.scummvm.meta.UNPACK_LIB";

	public void onReceive(Context context, Intent intent) {
		if (!intent.getAction().equals(ScummVMApplication.ACTION_PLUGIN_QUERY))
			return;

		Bundle extras = getResultExtras(true);

		final ActivityInfo info;
		try {
			info = context.getPackageManager()
				.getReceiverInfo(new ComponentName(context, this.getClass()),
									PackageManager.GET_META_DATA);
		} catch (PackageManager.NameNotFoundException e) {
			Log.e(LOG_TAG, "Error finding my own info?", e);
			return;
		}

		String mylib = info.metaData.getString(META_UNPACK_LIB);
		if (mylib != null) {
			ArrayList<String> all_libs =
				extras.getStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS);
			all_libs.add(new Uri.Builder()
							.scheme("plugin")
							.authority(context.getPackageName())
							.path(mylib)
							.toString());

			extras.putStringArrayList(ScummVMApplication.EXTRA_UNPACK_LIBS,
										all_libs);
		}

		setResultExtras(extras);
	}
}

