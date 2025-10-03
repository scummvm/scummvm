package org.scummvm.scummvm;

import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.annotation.RequiresApi;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import org.scummvm.scummvm.zip.ZipFile;

public class BackupManager {
	public static final int ERROR_CANCELLED = 1;
	public static final int ERROR_NO_ERROR = 0;
	public static final int ERROR_INVALID_BACKUP = -1;
	public static final int ERROR_INVALID_SAVES = -2;

	@RequiresApi(api = Build.VERSION_CODES.N)
	public static int exportBackup(Context context, Uri output) {
		try (ParcelFileDescriptor pfd = context.getContentResolver().openFileDescriptor(output, "wt")) {
			if (pfd == null) {
				return ERROR_INVALID_BACKUP;
			}
			return exportBackup(context, new FileOutputStream(pfd.getFileDescriptor()));
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}
	}

	public static int exportBackup(Context context, File output) {
		try {
			return exportBackup(context, new FileOutputStream(output, false));
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		}
	}

	@RequiresApi(api = Build.VERSION_CODES.N)
	public static int importBackup(ScummVMActivity context, Uri input) {
		try (ParcelFileDescriptor pfd = context.getContentResolver().openFileDescriptor(input, "r")) {
			if (pfd == null) {
				return ERROR_INVALID_BACKUP;
			}
			return importBackup(context, new FileInputStream(pfd.getFileDescriptor()));
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}
	}

	public static int importBackup(ScummVMActivity context, File input) {
		try {
			return importBackup(context, new FileInputStream(input));
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		}
	}

	private static int exportBackup(Context context, FileOutputStream output) {
		File configuration = new File(context.getFilesDir(), "scummvm.ini");

		Map<String, Map<String, String>> parsedIniMap;
		try (FileReader reader = new FileReader(configuration)) {
			parsedIniMap = INIParser.parse(reader);
		} catch(FileNotFoundException ignored) {
			parsedIniMap = null;
		} catch(IOException ignored) {
			parsedIniMap = null;
		}

		if (parsedIniMap == null) {
			return ERROR_INVALID_BACKUP;
		}

		ZipOutputStream zos = new ZipOutputStream(output);

		try (FileInputStream stream = new FileInputStream(configuration)) {
			ZipEntry entry = new ZipEntry("scummvm.ini");
			entry.setSize(configuration.length());
			entry.setTime(configuration.lastModified());

			zos.putNextEntry(entry);
			copyStream(zos, stream);
			zos.closeEntry();
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}

		try {
			ZipEntry entry;
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
				entry = new ZipEntry("saf");
				zos.putNextEntry(entry);
				if (!exportTrees(context, zos)) {
					return ERROR_INVALID_BACKUP;
				}
				zos.closeEntry();
			}

			entry = new ZipEntry("saves/");
			zos.putNextEntry(entry);
			zos.closeEntry();
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}

		File savesPath = INIParser.getPath(parsedIniMap, "scummvm", "savepath",
			new File(context.getFilesDir(), "saves"));
		boolean ret = exportSaves(context, savesPath, zos, "saves/");
		if (!ret) {
			try {
				zos.close();
			} catch(IOException ignored) {
			}
			return ERROR_INVALID_BACKUP;
		}

		HashSet<File> savesPaths = new HashSet<>();
		savesPaths.add(savesPath);

		int sectionId = -1;
		for (String section : parsedIniMap.keySet()) {
			sectionId++;
			if ("scummvm".equals(section)) {
				continue;
			}

			savesPath = INIParser.getPath(parsedIniMap, section, "savepath", null);
			if (savesPath == null) {
				continue;
			}

			if (savesPaths.contains(savesPath)) {
				continue;
			}
			savesPaths.add(savesPath);

			String folderName = "saves-" + sectionId + "/";
			ZipEntry entry = new ZipEntry(folderName);
			try {
				zos.putNextEntry(entry);
				zos.closeEntry();
			} catch(IOException ignored) {
				return ERROR_INVALID_BACKUP;
			}

			ret = exportSaves(context, savesPath, zos, folderName);
			if (!ret) {
				break;
			}
		}

		try {
			zos.close();
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}

		return ret ? ERROR_NO_ERROR : ERROR_INVALID_SAVES;
	}

	@RequiresApi(api = Build.VERSION_CODES.N)
	static private boolean exportTrees(Context context, ZipOutputStream zos) throws IOException {
		ObjectOutputStream oos = new ObjectOutputStream(zos);
		// Version 1
		oos.writeInt(1);

		SAFFSTree[] trees = SAFFSTree.getTrees(context);

		oos.writeInt(trees.length);
		for (SAFFSTree tree : trees) {
			oos.writeObject(tree.getTreeName());
			oos.writeObject(tree.getTreeId());
			oos.writeObject(tree.getTreeDocumentUri().toString());
		}

		// Don't close as it would close the underlying ZipOutputStream
		oos.flush();

		return true;
	}

	static private boolean exportSaves(Context context, File folder, ZipOutputStream zos, String folderName) {
		SAFFSTree.PathResult pr;
		try {
			pr = SAFFSTree.fullPathToNode(context, folder.getPath(), false);
		} catch (FileNotFoundException e) {
			return false;
		}

		// This version check is only to make Android Studio linter happy
		if (pr == null || Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
			// This is a standard filesystem path
			File[] children = folder.listFiles();
			if (children == null) {
				return true;
			}
			Arrays.sort(children);
			for (File f: children) {
				if ("timestamps".equals(f.getName())) {
					continue;
				}
				try (FileInputStream stream = new FileInputStream(f)) {
					ZipEntry entry = new ZipEntry(folderName + f.getName());
					entry.setSize(f.length());
					entry.setTime(f.lastModified());

					zos.putNextEntry(entry);
					copyStream(zos, stream);
					zos.closeEntry();
				} catch(FileNotFoundException ignored) {
					return false;
				} catch(IOException ignored) {
					return false;
				}
			}
			return true;
		}

		// This is a SAF fake mount point
		SAFFSTree.SAFFSNode[] children = pr.tree.getChildren(pr.node);
		if (children == null) {
			return false;
		}
		Arrays.sort(children);

		for (SAFFSTree.SAFFSNode child : children) {
			if ((child._flags & SAFFSTree.SAFFSNode.DIRECTORY) != 0) {
				continue;
			}
			String component = child._path.substring(child._path.lastIndexOf('/') + 1);
			if ("timestamps".equals(component)) {
				continue;
			}
			try (ParcelFileDescriptor pfd = pr.tree.createFileDescriptor(child, "r")) {
				ZipEntry entry = new ZipEntry(folderName + component);

				zos.putNextEntry(entry);
				copyStream(zos, new ParcelFileDescriptor.AutoCloseInputStream(pfd));
				zos.closeEntry();
			} catch(FileNotFoundException ignored) {
				return false;
			} catch(IOException ignored) {
				return false;
			}
		}
		return true;
	}

	private static int importBackup(ScummVMActivity context, FileInputStream input) {
		ZipFile zf;
		try {
			zf = new ZipFile(input);
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}

		// Load configuration
		org.scummvm.scummvm.zip.ZipEntry ze = zf.getEntry("scummvm.ini");
		if (ze == null) {
			// No configuration file, not a backup
			return ERROR_INVALID_BACKUP;
		}

		// Avoid using tmp suffix as it's used by atomic file support
		File configurationTmp = new File(context.getFilesDir(), "scummvm.ini.tmp2");

		try (FileOutputStream os = new FileOutputStream(configurationTmp);
			InputStream is = zf.getInputStream(ze)) {
			copyStream(os, is);
		} catch(FileNotFoundException ignored) {
			return ERROR_INVALID_BACKUP;
		} catch(IOException ignored) {
			return ERROR_INVALID_BACKUP;
		}

		// Load the new configuration to know where to put saves
		Map<String, Map<String, String>> parsedIniMap;
		try (FileReader reader = new FileReader(configurationTmp)) {
			parsedIniMap = INIParser.parse(reader);
		} catch(FileNotFoundException ignored) {
			parsedIniMap = null;
		} catch(IOException ignored) {
			parsedIniMap = null;
		}

		if (parsedIniMap == null) {
			configurationTmp.delete();
			return ERROR_INVALID_BACKUP;
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
			// Restore the missing SAF trees
			ze = zf.getEntry("saf");
			if (ze == null) {
				// No configuration file, not a backup
				return ERROR_INVALID_BACKUP;
			}
			try (InputStream is = zf.getInputStream(ze)) {
				if (importTrees(context, is) == ERROR_INVALID_BACKUP) {
					// Try to continue except obvious error
					return ERROR_INVALID_BACKUP;
				}
			} catch(FileNotFoundException ignored) {
				return ERROR_INVALID_BACKUP;
			} catch(IOException ignored) {
				return ERROR_INVALID_BACKUP;
			} catch (ClassNotFoundException e) {
				return ERROR_INVALID_BACKUP;
			}
		}

		// Move the configuration back now that we know it's parsable and that SAF is set up
		Log.i(ScummVM.LOG_TAG, "Writing new ScummVM configuration");
		File configuration = new File(context.getFilesDir(), "scummvm.ini");
		if (!configurationTmp.renameTo(configuration)) {
			try (FileOutputStream os = new FileOutputStream(configuration);
				FileInputStream is = new FileInputStream(configurationTmp)) {
				copyStream(os, is);
			} catch(FileNotFoundException ignored) {
				return ERROR_INVALID_BACKUP;
			} catch(IOException ignored) {
				return ERROR_INVALID_BACKUP;
			}
			configurationTmp.delete();
		}

		File savesPath = INIParser.getPath(parsedIniMap, "scummvm", "savepath",
			new File(context.getFilesDir(), "saves"));

		boolean ret = importSaves(context, savesPath, zf, "saves/");
		if (!ret) {
			try {
				zf.close();
			} catch(IOException ignored) {
			}
			return ERROR_INVALID_BACKUP;
		}

		HashSet<File> savesPaths = new HashSet<>();
		savesPaths.add(savesPath);

		int sectionId = -1;
		for (String section : parsedIniMap.keySet()) {
			sectionId++;
			if ("scummvm".equals(section)) {
				continue;
			}

			savesPath = INIParser.getPath(parsedIniMap, section, "savepath", null);
			if (savesPath == null) {
				continue;
			}

			if (savesPaths.contains(savesPath)) {
				continue;
			}
			savesPaths.add(savesPath);

			String folderName = "saves-" + sectionId + "/";
			ret = importSaves(context, savesPath, zf, folderName);
			if (!ret) {
				break;
			}
		}

		try {
			zf.close();
		} catch(IOException ignored) {
		}

		return ret ? ERROR_NO_ERROR : ERROR_INVALID_SAVES;
	}

	@RequiresApi(api = Build.VERSION_CODES.N)
	static private int importTrees(ScummVMActivity context, InputStream is) throws IOException, ClassNotFoundException {
		boolean failed = false;

		ObjectInputStream ois = new ObjectInputStream(is);
		// Version 1
		if (ois.readInt() != 1) {
			// Invalid version
			return ERROR_INVALID_BACKUP;
		}

		for (int length = ois.readInt(); length > 0; length--) {
			String treeName = (String)ois.readObject();
			String treeId = (String)ois.readObject();
			String treeUri = (String)ois.readObject();

			if (SAFFSTree.findTree(context, treeId) != null) {
				continue;
			}

			Uri uri = context.selectWithNativeUI(true, true, Uri.parse(treeUri), treeName, null, null);
			if (uri == null) {
				failed = true;
				continue;
			}

			// Register the new selected tree
			SAFFSTree.newTree(context, uri);
		}

		ois.close();

		return failed ? ERROR_CANCELLED : ERROR_NO_ERROR;
	}

	static private boolean importSaves(Context context, File folder, ZipFile zf, String folderName) {
		SAFFSTree.PathResult pr;
		try {
			pr = SAFFSTree.fullPathToNode(context, folder.getPath(), true);
		} catch (FileNotFoundException e) {
			return false;
		}

		// This version check is only to make Android Studio linter happy
		if (pr == null || Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
			// This is a standard filesystem path
			if (!folder.isDirectory() && !folder.mkdirs()) {
				return false;
			}

			Enumeration<? extends org.scummvm.scummvm.zip.ZipEntry> entries = zf.entries();
			while (entries.hasMoreElements()) {
				org.scummvm.scummvm.zip.ZipEntry entry = entries.nextElement();
				String name = entry.getName();
				if (!name.startsWith(folderName)) {
					continue;
				}

				// Get the base name (this avoids directory traversal)
				name = name.substring(name.lastIndexOf("/") + 1);
				if (name.isEmpty() || "timestamps".equals(name)) {
					continue;
				}

				File f = new File(folder, name);
				try (InputStream is = zf.getInputStream(entry);
					FileOutputStream os = new FileOutputStream(f)) {
					copyStream(os, is);
				} catch(FileNotFoundException ignored) {
					return false;
				} catch(IOException ignored) {
					return false;
				}
			}
			return true;
		}

		// This is a SAF fake mount point
		Enumeration<? extends org.scummvm.scummvm.zip.ZipEntry> entries = zf.entries();
		while (entries.hasMoreElements()) {
			org.scummvm.scummvm.zip.ZipEntry entry = entries.nextElement();
			String name = entry.getName();
			if (!name.startsWith(folderName)) {
				continue;
			}

			// Get the base name (this avoids directory traversal)
			name = name.substring(name.lastIndexOf("/") + 1);
			if (name.isEmpty() || "timestamps".equals(name)) {
				continue;
			}

			SAFFSTree.SAFFSNode child = pr.tree.getChild(pr.node, name);
			if (child == null) {
				child = pr.tree.createFile(pr.node, name);
			}
			if (child == null) {
				return false;
			}

			try (InputStream is = zf.getInputStream(entry);
				ParcelFileDescriptor pfd = pr.tree.createFileDescriptor(child, "wt")) {
				copyStream(new FileOutputStream(pfd.getFileDescriptor()), is);
			} catch(FileNotFoundException ignored) {
				return false;
			} catch(IOException ignored) {
				return false;
			}
		}
		return true;
	}

	public static void copyStream(OutputStream out, InputStream in) throws IOException {
		byte[] buffer = new byte[4096];
		int sz;
		while((sz = in.read(buffer)) != -1) {
			out.write(buffer, 0, sz);
		}
	}
}
