package org.scummvm.scummvm;

import android.os.Environment;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * Contains helper methods to get list of available media
 */
public class ExternalStorage {
	public static final String SD_CARD = "sdCard";
	public static final String EXTERNAL_SD_CARD = "externalSdCard";
	public static final String DATA_DIRECTORY = "ScummVM data directory";

	/**
	 * @return True if the external storage is available. False otherwise.
	 */
	public static boolean isAvailable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state) || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
			return true;
		}
		return false;
	}

	public static String getSdCardPath() {
		return Environment.getExternalStorageDirectory().getPath() + "/";
	}

	/**
	 * @return True if the external storage is writable. False otherwise.
	 */
	public static boolean isWritable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			return true;
		}
		return false;

	}

	/**
	 * @return list of locations available. Odd elements are names, even are paths
	 */
	public static List<String> getAllStorageLocations() {
		List<String> map = new ArrayList<String>(20);

		List<String> mMounts = new ArrayList<String>(10);
		List<String> mVold = new ArrayList<String>(10);
		mMounts.add("/mnt/sdcard");
		mVold.add("/mnt/sdcard");

		try {
			File mountFile = new File("/proc/mounts");
			if (mountFile.exists()){
				Scanner scanner = new Scanner(mountFile);
				while (scanner.hasNext()) {
					String line = scanner.nextLine();
					if (line.startsWith("/dev/block/vold/")) {
						String[] lineElements = line.split(" ");
						String element = lineElements[1];

						// don't add the default mount path
						// it's already in the list.
						if (!element.equals("/mnt/sdcard"))
							mMounts.add(element);
					}
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		try {
			File voldFile = new File("/system/etc/vold.fstab");
			if (voldFile.exists()){
				Scanner scanner = new Scanner(voldFile);
				while (scanner.hasNext()) {
					String line = scanner.nextLine();
					if (line.startsWith("dev_mount")) {
						String[] lineElements = line.split(" ");
						String element = lineElements[2];

						if (element.contains(":"))
							element = element.substring(0, element.indexOf(":"));
						if (!element.equals("/mnt/sdcard"))
							mVold.add(element);
					}
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}


		for (int i = 0; i < mMounts.size(); i++) {
			String mount = mMounts.get(i);
			if (!mVold.contains(mount))
				mMounts.remove(i--);
		}
		mVold.clear();

		List<String> mountHash = new ArrayList<String>(10);

		for (String mount : mMounts){
			File root = new File(mount);
			if (root.exists() && root.isDirectory() && root.canRead()) {
				File[] list = root.listFiles();
				String hash = "[";
				if (list != null) {
					for (File f : list) {
						hash += f.getName().hashCode() + ":" + f.length() + ", ";
					}
				}
				hash += "]";
				if (!mountHash.contains(hash)){
					String key = SD_CARD + "_" + (map.size() / 2);
					if (map.size() == 0) {
						key = SD_CARD;
					} else if (map.size() == 2) {
						key = EXTERNAL_SD_CARD;
					}
					mountHash.add(hash);
					map.add(key);
					map.add(root.getAbsolutePath());
				}
			}
		}

		mMounts.clear();

		map.add(DATA_DIRECTORY);
		map.add(Environment.getDataDirectory().getAbsolutePath());

		if (map.isEmpty()) {
			map.add(SD_CARD);
			map.add(Environment.getExternalStorageDirectory().getAbsolutePath());
		}
		return map;
	}
}
