package org.scummvm.scummvm;

import android.content.Context;
import android.os.Environment;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import android.text.TextUtils;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Locale;
import java.util.regex.Pattern;
import android.util.Log;
import android.os.Build;


/**
 * Contains helper methods to get list of available media
 */
public class ExternalStorage {
	public static final String SD_CARD = "sdCard";
	public static final String EXTERNAL_SD_CARD = "externalSdCard";
	public static final String DATA_DIRECTORY = "ScummVM data";
	public static final String DATA_DIRECTORY_INT = "ScummVM data (Int)";
	public static final String DATA_DIRECTORY_EXT = "ScummVM data (Ext))";


	// Find candidate removable sd card paths
	// Code reference: https://stackoverflow.com/a/54411385
	private static final String ANDROID_DIR = File.separator + "Android";

	private static String ancestor(File dir) {
		// getExternalFilesDir() and getExternalStorageDirectory()
		// may return something app-specific like:
		//   /storage/sdcard1/Android/data/com.mybackuparchives.android/files
		// so we want the great-great-grandparent folder.
		if (dir == null) {
			return null;
		} else {
			String path = dir.getAbsolutePath();
			int i = path.indexOf(ANDROID_DIR);
			if (i == -1) {
				return path;
			} else {
				return path.substring(0, i);
			}
		}
	}

	// Pattern that SD card device should match
	private static final Pattern
		devicePattern = Pattern.compile("/dev/(block/.*vold.*|fuse)|/mnt/.*");
	// Pattern that SD card mount path should match
	private static final Pattern
		pathPattern = Pattern.compile("/(mnt|storage|external_sd|extsd|_ExternalSD|Removable|.*MicroSD).*", Pattern.CASE_INSENSITIVE);
	// Pattern that the mount path should not match.
	//' emulated' indicates an internal storage location, so skip it.
	// 'asec' is an encrypted package file, decrypted and mounted as a directory.
	private static final Pattern
		pathAntiPattern = Pattern.compile(".*(/secure|/asec|/emulated).*");
	// These are expected fs types, including vfat. tmpfs is not OK.
	// fuse can be removable SD card (as on Moto E or Asus ZenPad), or can be internal (Huawei G610).
	private static final Pattern
		fsTypePattern = Pattern.compile(".*(fat|msdos|ntfs|ext[34]|fuse|sdcard|esdfs).*");

	/** Common paths for microSD card. **/
	private static final String[] commonPaths = {
		// Some of these taken from
		// https://stackoverflow.com/questions/13976982/removable-storage-external-sdcard-path-by-manufacturers
		// These are roughly in order such that the earlier ones, if they exist, are more sure
		// to be removable storage than the later ones.
		"/mnt/Removable/MicroSD",
		"/storage/removable/" + SD_CARD + "1", // !< Sony Xperia Z1
		"/Removable/MicroSD", // Asus ZenPad C
		"/removable/microsd",
		"/external_sd", // Samsung
		"/_ExternalSD", // some LGs
		"/storage/extSdCard", // later Samsung
		"/storage/extsdcard", // Main filesystem is case-sensitive; FAT isn't.
		"/mnt/extsd", // some Chinese tablets, e.g. Zeki
		"/storage/" + SD_CARD + "1", // If this exists it's more likely than sdcard0 to be removable.
		"/mnt/extSdCard",
		"/mnt/" + SD_CARD + "/external_sd",
		"/mnt/external_sd",
		"/storage/external_SD",
		"/storage/ext_sd", // HTC One Max
		"/mnt/" + SD_CARD + "/_ExternalSD",
		"/mnt/" + SD_CARD + "-ext",

		"/" + SD_CARD + "2", // HTC One M8s
		"/" + SD_CARD + "1", // Sony Xperia Z
		"/mnt/media_rw/" + SD_CARD + "1",   // 4.4.2 on CyanogenMod S3
		"/mnt/" + SD_CARD, // This can be built-in storage (non-removable).
		"/" + SD_CARD,
		"/storage/" + SD_CARD +"0",
		"/emmc",
		"/mnt/emmc",
		"/" + SD_CARD + "/sd",
		"/mnt/" + SD_CARD + "/bpemmctest",
		"/mnt/external1",
		"/data/sdext4",
		"/data/sdext3",
		"/data/sdext2",
		"/data/sdext",
		"/storage/microsd" //ASUS ZenFone 2

		// If we ever decide to support USB OTG storage, the following paths could be helpful:
		// An LG Nexus 5 apparently uses usb://1002/UsbStorage/ as a URI to access an SD
		// card over OTG cable. Other models, like Galaxy S5, use /storage/UsbDriveA
		//        "/mnt/usb_storage",
		//        "/mnt/UsbDriveA",
		//        "/mnt/UsbDriveB",
	};

	/** Find path to removable SD card. */
	public static LinkedHashSet<File> findSdCardPath() {
		String[] mountFields;
		BufferedReader bufferedReader = null;
		String lineRead;

		// Possible SD card paths
		LinkedHashSet<File> candidatePaths = new LinkedHashSet<>();

		// Build a list of candidate paths, roughly in order of preference. That way if
		// we can't definitively detect removable storage, we at least can pick a more likely
		// candidate.

		// Could do: use getExternalStorageState(File path), with and without an argument, when
		// available. With an argument is available since API level 21.
		// This may not be necessary, since we also check whether a directory exists and has contents,
		// which would fail if the external storage state is neither MOUNTED nor MOUNTED_READ_ONLY.

		// I moved hard-coded paths toward the end, but we need to make sure we put the ones in
		// backwards order that are returned by the OS. And make sure the iterators respect
		// the order!
		// This is because when multiple "external" storage paths are returned, it's always (in
		// experience, but not guaranteed by documentation) with internal/emulated storage
		// first, removable storage second.

		// Add value of environment variables as candidates, if set:
		// EXTERNAL_STORAGE, SECONDARY_STORAGE, EXTERNAL_SDCARD_STORAGE
		// But note they are *not* necessarily *removable* storage! Especially EXTERNAL_STORAGE.
		// And they are not documented (API) features. Typically useful only for old versions of Android.

		String val = System.getenv("SECONDARY_STORAGE");
		if (!TextUtils.isEmpty(val)) {
			addPath(val, candidatePaths);
		}

		val = System.getenv("EXTERNAL_SDCARD_STORAGE");
		if (!TextUtils.isEmpty(val)) {
			addPath(val, candidatePaths);
		}

		// Get listing of mounted devices with their properties.
		ArrayList<File> mountedPaths = new ArrayList<>();
		try {
			// Note: Despite restricting some access to /proc (http://stackoverflow.com/a/38728738/423105),
			// Android 7.0 does *not* block access to /proc/mounts, according to our test on George's Alcatel A30 GSM.
			bufferedReader = new BufferedReader(new FileReader("/proc/mounts"));

			// Iterate over each line of the mounts listing.
			while ((lineRead = bufferedReader.readLine()) != null) {
//				Log.d(ScummVM.LOG_TAG, "\nMounts line: " + lineRead);
				mountFields = lineRead.split(" ");

				// columns: device, mountpoint, fs type, options... Example:
				// /dev/block/vold/179:97 /storage/sdcard1 vfat rw,dirsync,nosuid,nodev,noexec,relatime,uid=1000,gid=1015,fmask=0002,dmask=0002,allow_utime=0020,codepage=cp437,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro 0 0
				String device = mountFields[0], path = mountFields[1], fsType = mountFields[2];

				// The device, path, and fs type must conform to expected patterns.
				// mtdblock is internal, I'm told.
				// Check for disqualifying patterns in the path.
				// If this mounts line fails our tests, skip it.
				if (!(devicePattern.matcher(device).matches()
						&& pathPattern.matcher(path).matches()
						&& fsTypePattern.matcher(fsType).matches())
					|| device.contains("mtdblock")
					|| pathAntiPattern.matcher(path).matches()
				) {
					continue;
				}

				// TODO maybe: check options to make sure it's mounted RW?
				// The answer at http://stackoverflow.com/a/13648873/423105 does.
				// But it hasn't seemed to be necessary so far in my testing.

				// This line met the criteria so far, so add it to candidate list.
				addPath(path, mountedPaths);
			}
		} catch (IOException ignored) { }
		finally {
			if (bufferedReader != null) {
				try {
					bufferedReader.close();
				} catch (IOException ignored) { }
			}
		}

		// Append the paths from mount table to candidate list, in reverse order.
		if (!mountedPaths.isEmpty()) {
			// See https://stackoverflow.com/a/5374346/423105 on why the following is necessary.
			// Basically, .toArray() needs its parameter to know what type of array to return.
			File[] mountedPathsArray = mountedPaths.toArray(new File[0]);
			addAncestors(mountedPathsArray, candidatePaths);
		}

		// Add hard-coded known common paths to candidate list:
		addStrings(commonPaths, candidatePaths);

		// If the above doesn't work we could try the following other options, but in my experience they
		// haven't added anything helpful yet.

		// getExternalFilesDir() and getExternalStorageDirectory() typically something app-specific like
		//   /storage/sdcard1/Android/data/com.mybackuparchives.android/files
		// so we want the great-great-grandparent folder.

		// TODO Note, This method was deprecated in API level 29.
		//      To improve user privacy, direct access to shared/external storage devices is deprecated.
		//      When an app targets Build.VERSION_CODES.Q, the path returned from this method is no longer directly accessible to apps.
		//      Apps can continue to access content stored on shared/external storage by migrating to
		//      alternatives such as Context#getExternalFilesDir(String), MediaStore, or Intent#ACTION_OPEN_DOCUMENT.
		//
		// This may be non-removable.
		Log.d(ScummVM.LOG_TAG, "Environment.getExternalStorageDirectory():");
		addPath(ancestor(Environment.getExternalStorageDirectory()), candidatePaths);

		// TODO maybe use getExternalStorageState(File path), with and without an argument,
		//      when available. With an argument is available since API level 21.
		//      This may not be necessary, since we also check whether a directory exists,
		//      which would fail if the external storage state is neither MOUNTED nor MOUNTED_READ_ONLY.

		// A "public" external storage directory. But in my experience it doesn't add anything helpful.
		// Note that you can't pass null, or you'll get an NPE.
		final File publicDirectory = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MUSIC);
		// Take the parent, because we tend to get a path like /pathTo/sdCard/Music.
		if (publicDirectory.getParentFile() != null) {
			addPath(publicDirectory.getParentFile().getAbsolutePath(), candidatePaths);
		}

		// EXTERNAL_STORAGE: may not be removable.
		val = System.getenv("EXTERNAL_STORAGE");
		if (!TextUtils.isEmpty(val)) {
			addPath(val, candidatePaths);
		}

		if (candidatePaths.isEmpty()) {
			Log.w(ScummVM.LOG_TAG, "No removable microSD card found.");
			return candidatePaths;
		} else {
			Log.i(ScummVM.LOG_TAG, "\nFound potential removable storage locations: " + candidatePaths);
		}

		// Accept or eliminate candidate paths if we can determine whether they're removable storage.
		// In Lollipop and later, we can check isExternalStorageRemovable() status on each candidate.
		if (Build.VERSION.SDK_INT >= 21) {
			Iterator<File> itf = candidatePaths.iterator();
			while (itf.hasNext()) {
				File dir = itf.next();
				// handle illegalArgumentException if the path is not a valid storage device.
				try {
					if (Environment.isExternalStorageRemovable(dir)) {
						Log.i(ScummVM.LOG_TAG, dir.getPath() + " is removable external storage");
						addPath(dir.getAbsolutePath(), candidatePaths);
					} else if (Environment.isExternalStorageEmulated(dir)) {
						Log.d(ScummVM.LOG_TAG, "Removing emulated external storage dir " + dir);
						itf.remove();
					}
				} catch (IllegalArgumentException e) {
					Log.d(ScummVM.LOG_TAG, "isRemovable(" + dir.getPath() + "): not a valid storage device.", e);
				}
			}
		}

		// Continue trying to accept or eliminate candidate paths based on whether they're removable storage.
		// On pre-Lollipop, we only have singular externalStorage. Check whether it's removable.
		if (Build.VERSION.SDK_INT >= 9) {
			File externalStorage = Environment.getExternalStorageDirectory();
			Log.d(ScummVM.LOG_TAG, String.format(Locale.ROOT, "findSDCardPath: getExternalStorageDirectory = %s", externalStorage.getPath()));
			if (Environment.isExternalStorageRemovable()) {
				// Make sure this is a candidate.
				// TODO: Does this contains() work? Should we be canonicalizing paths before comparing?
				if (candidatePaths.contains(externalStorage)) {
					Log.d(ScummVM.LOG_TAG, "Using externalStorage dir " + externalStorage);
					// return externalStorage;
					addPath(externalStorage.getAbsolutePath(), candidatePaths);
				}
			} else if (Build.VERSION.SDK_INT >= 11 && Environment.isExternalStorageEmulated()) {
				Log.d(ScummVM.LOG_TAG, "Removing emulated external storage dir " + externalStorage);
				candidatePaths.remove(externalStorage);
			}
		}

		return candidatePaths;
	}


	/** Add each path to the collection. */
	private static void addStrings(String[] newPaths, LinkedHashSet<File> candidatePaths) {
		for (String path : newPaths) {
			addPath(path, candidatePaths);
		}
	}

	/** Add ancestor of each File to the collection. */
	private static void addAncestors(File[] files, LinkedHashSet<File> candidatePaths) {
		for (int i = files.length - 1; i >= 0; i--) {
			addPath(ancestor(files[i]), candidatePaths);
		}
	}

	/**
	 * Add a new candidate directory path to our list, if it's not obviously wrong.
	 * Supply path as either String or File object.
	 * @param strNew path of directory to add
	 */
	private static void addPath(String strNew, Collection<File> paths) {
		// If one of the arguments is null, fill it in from the other.
		if (!TextUtils.isEmpty(strNew)) {
			File fileNew = new File(strNew);

			if (!paths.contains(fileNew) &&
				// Check for paths known not to be removable SD card.
				// The antipattern check can be redundant, depending on where this is called from.
				!pathAntiPattern.matcher(strNew).matches()) {

				// Eliminate candidate if not a directory or not fully accessible.
				if (fileNew.exists() && fileNew.isDirectory() && fileNew.canExecute()) {
					Log.d(ScummVM.LOG_TAG, "  Adding candidate path " + strNew);
					paths.add(fileNew);
				} else {
					Log.d(ScummVM.LOG_TAG, String.format(Locale.ROOT, "  Invalid path %s: exists: %b isDir: %b canExec: %b canRead: %b",
					      strNew, fileNew.exists(), fileNew.isDirectory(), fileNew.canExecute(), fileNew.canRead()));
				}
			}
		}
	}



	/**
	 * @return True if the external storage is available. False otherwise.
	 */
	public static boolean isAvailable() {
		String state = Environment.getExternalStorageState();
		return Environment.MEDIA_MOUNTED.equals(state) || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state);
	}

	public static String getSdCardPath() {
		return Environment.getExternalStorageDirectory().getPath() + "/";
	}

	/**
	 * @return True if the external storage is writable. False otherwise.
	 */
	public static boolean isWritable() {
		String state = Environment.getExternalStorageState();
		return Environment.MEDIA_MOUNTED.equals(state);

	}

	/**
	 * @return list of locations available. Odd elements are names, even are paths
	 */
	public static List<String> getAllStorageLocations(Context ctx) {
		List<String> map = new ArrayList<>(20);

		List<String> mMounts = new ArrayList<>(10);
		List<String> mVold = new ArrayList<>(10);
		mMounts.add("/mnt/sdcard");
		mVold.add("/mnt/sdcard");

		try {
			File mountFile = new File("/proc/mounts");
			if (mountFile.exists()) {
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

		List<String> mountHash = new ArrayList<>(10);

		for (String mount : mMounts) {
			File root = new File(mount);
			if (root.exists() && root.isDirectory() && root.canRead()) {
				File[] list = root.listFiles();
				StringBuilder hash = new StringBuilder("[");
				if (list != null) {
					for (File f : list) {
						hash.append(f.getName().hashCode()).append(":").append(f.length()).append(", ");
					}
				}
				hash.append("]");
				if (!mountHash.contains(hash.toString())) {
					String key = SD_CARD + "_" + (map.size() / 2);
					if (map.size() == 0) {
						key = SD_CARD;
					} else if (map.size() == 2) {
						key = EXTERNAL_SD_CARD;
					}
					mountHash.add(hash.toString());
					map.add(key);
					map.add(root.getAbsolutePath());
				}
			}
		}

		mMounts.clear();

		if (Environment.getDataDirectory() != null
			&& !TextUtils.isEmpty(Environment.getDataDirectory().getAbsolutePath())) {
			File dataFilePath = new File(Environment.getDataDirectory().getAbsolutePath());
			if (dataFilePath.exists() && dataFilePath.isDirectory()) {
				map.add(DATA_DIRECTORY);
				map.add(Environment.getDataDirectory().getAbsolutePath());
			}
		}
		map.add(DATA_DIRECTORY_INT);
		map.add(ctx.getFilesDir().getPath());
		map.add(DATA_DIRECTORY_EXT);
		if (ctx.getExternalFilesDir(null) != null) {
			map.add(ctx.getExternalFilesDir(null).getPath());
		}

		// Now go through the external storage
		if (isAvailable()) {  // we can read the External Storage...
			// Retrieve the primary External Storage:
			File primaryExternalStorage = Environment.getExternalStorageDirectory();

			// Retrieve the External Storages root directory:
			String externalStorageRootDir;
			if ((externalStorageRootDir = primaryExternalStorage.getParent()) == null) {  // no parent...
				String key = primaryExternalStorage.getAbsolutePath();
				if (!map.contains(key)) {
					map.add(key); // Make name as directory
					map.add(key);
				}
			} else {
				File externalStorageRoot = new File(externalStorageRootDir);
				File[] files = externalStorageRoot.listFiles();

				if (files != null) {
					for (final File file : files) {
						// Check if it is a real directory (not a USB drive)...
						if (file.isDirectory()
						    && file.canRead()
						    && file.listFiles() != null
						    && (file.listFiles().length > 0)) {
							String key = file.getAbsolutePath();
							if (!map.contains(key)) {
								map.add(key); // Make name as directory
								map.add(key);
							}
						}
					}
				}
			}
		}

		// Get candidates for removable external storage
		LinkedHashSet<File> candidateRemovableSdCardPaths = findSdCardPath();
		for (final File file : candidateRemovableSdCardPaths) {
			String key = file.getAbsolutePath();
			if (!map.contains(key)) {
				map.add(key); // Make name as directory
				map.add(key);
			}
		}

		return map;
	}
}
