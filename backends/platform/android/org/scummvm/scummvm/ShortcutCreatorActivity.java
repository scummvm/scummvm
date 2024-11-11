package org.scummvm.scummvm;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import org.scummvm.scummvm.zip.ZipEntry;
import org.scummvm.scummvm.zip.ZipFile;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;


public class ShortcutCreatorActivity extends Activity implements CompatHelpers.SystemInsets.SystemInsetsListener  {
	final protected static String LOG_TAG = "ShortcutCreatorActivity";

	private IconsCache _cache;

	static void pushShortcut(Context context, String gameId, Intent intent) {
		Map<String, Map<String, String>> parsedIniMap;
		try (FileReader reader = new FileReader(new File(context.getFilesDir(), "scummvm.ini"))) {
			parsedIniMap = INIParser.parse(reader);
		} catch(FileNotFoundException ignored) {
			parsedIniMap = null;
		} catch(IOException ignored) {
			parsedIniMap = null;
		}

		if (parsedIniMap == null) {
			return;
		}

		Game game = Game.loadGame(parsedIniMap, gameId);
		if (game == null) {
			return;
		}

		FileInputStream defaultStream = openFile(new File(context.getFilesDir(), "gui-icons.dat"));

		File iconsPath = INIParser.getPath(parsedIniMap, "scummvm", "iconspath",
			new File(context.getFilesDir(), "icons"));
		FileInputStream[] packsStream = openFiles(context, iconsPath, "gui-icons.*\\.dat");

		IconsCache cache = new IconsCache(context, defaultStream, packsStream);
		final Drawable icon = cache.getGameIcon(game);

		CompatHelpers.ShortcutCreator.pushDynamicShortcut(context, game.getTarget(), intent, game.getDescription(), icon, R.drawable.ic_no_game_icon);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shortcut_creator_activity);

		CompatHelpers.SystemInsets.registerSystemInsetsListener(findViewById(R.id.shortcut_creator_root), this);

		// We are only here to create a shortcut
		if (!Intent.ACTION_CREATE_SHORTCUT.equals(getIntent().getAction())) {
			finish();
			return;
		}

		List<Game> games;
		Map<String, Map<String, String>> parsedIniMap;
		try (FileReader reader = new FileReader(new File(getFilesDir(), "scummvm.ini"))) {
			parsedIniMap = INIParser.parse(reader);
		} catch(FileNotFoundException ignored) {
			parsedIniMap = null;
		} catch(IOException ignored) {
			parsedIniMap = null;
		}

		if (parsedIniMap == null) {
			Toast.makeText(this, R.string.ini_parsing_error, Toast.LENGTH_LONG).show();
			finish();
			return;
		}

		games = Game.loadGames(parsedIniMap);

		FileInputStream defaultStream = openFile(new File(getFilesDir(), "gui-icons.dat"));

		File iconsPath = INIParser.getPath(parsedIniMap, "scummvm", "iconspath",
			new File(getFilesDir(), "icons"));
		FileInputStream[] packsStream = openFiles(this, iconsPath, "gui-icons.*\\.dat");

		_cache = new IconsCache(this, defaultStream, packsStream);

		final GameAdapter listAdapter = new GameAdapter(this, games, _cache);

		ListView listView = findViewById(R.id.shortcut_creator_games_list);
		listView.setAdapter(listAdapter);
		listView.setEmptyView(findViewById(R.id.shortcut_creator_games_list_empty));
		listView.setOnItemClickListener(_gameClicked);

		EditText searchEdit = findViewById(R.id.shortcut_creator_search_edit);
		searchEdit.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence cs, int arg1, int arg2, int arg3) {
				listAdapter.getFilter().filter(cs.toString());
			}

			@Override
			public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
										  int arg3) {
			}

			@Override
			public void afterTextChanged(Editable arg0) {
			}
		});
		if (games.isEmpty()) {
			searchEdit.setVisibility(View.GONE);
		}

		setResult(RESULT_CANCELED);
	}

	@Override
	public void systemInsetsUpdated(int[] gestureInsets, int[] systemInsets, int[] cutoutInsets) {
		LinearLayout root = findViewById(R.id.shortcut_creator_root);
		// Ignore bottom as we have our list which can overflow
		root.setPadding(
			Math.max(systemInsets[0], cutoutInsets[0]),
			Math.max(systemInsets[1], cutoutInsets[1]),
			Math.max(systemInsets[2], cutoutInsets[2]), 0);
	}

	static private FileInputStream openFile(File path) {
		 try {
			return new FileInputStream(path);
		} catch (FileNotFoundException e) {
			return null;
		}
	}

	static private FileInputStream[] openFiles(Context context, File basePath, String regex) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N ||
			!basePath.getPath().startsWith("/saf/")) {
			// This is a standard filesystem path
			File[] children = basePath.listFiles((dir, name) -> name.matches(regex));
			if (children == null) {
				return new FileInputStream[0];
			}
			Arrays.sort(children);
			FileInputStream[] ret = new FileInputStream[children.length];
			int i = 0;
			for (File f: children) {
				ret[i] = openFile(f);
				i += 1;
			}
			return ret;
		}
		// This is a SAF fake mount point
		String baseName = basePath.getPath();
		int slash = baseName.indexOf('/', 5);
		if (slash == -1) {
			slash = baseName.length();
		}
		String treeName = baseName.substring(5, slash);
		String path = baseName.substring(slash);

		SAFFSTree tree = SAFFSTree.findTree(context, treeName);
		if (tree == null) {
			return new FileInputStream[0];
		}
		SAFFSTree.SAFFSNode node = tree.pathToNode(path);
		if (node == null) {
			return new FileInputStream[0];
		}
		SAFFSTree.SAFFSNode[] children = tree.getChildren(node);
		if (children == null) {
			return new FileInputStream[0];
		}
		Arrays.sort(children);

		ArrayList<FileInputStream> ret = new ArrayList<>();
		for (SAFFSTree.SAFFSNode child : children) {
			if ((child._flags & SAFFSTree.SAFFSNode.DIRECTORY) != 0) {
				continue;
			}
			String component = child._path.substring(child._path.lastIndexOf('/') + 1);
			if (!component.matches(regex)) {
				continue;
			}
			ParcelFileDescriptor pfd = tree.createFileDescriptor(child, "r");
			if (pfd == null) {
				continue;
			}
			ret.add(new ParcelFileDescriptor.AutoCloseInputStream(pfd));
		}
		return ret.toArray(new FileInputStream[0]);
	}

	private final OnItemClickListener _gameClicked = new OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> a, View v, int position, long id) {
			Game game = (Game)a.getItemAtPosition(position);
			final Drawable icon = _cache.getGameIcon(game);

			// Display a customization dialog to let the user change (shorten?) the title
			AlertDialog.Builder builder = new AlertDialog.Builder(ShortcutCreatorActivity.this);
			builder.setTitle("Title");
			View fragment = LayoutInflater.from(ShortcutCreatorActivity.this).inflate(R.layout.shortcut_creator_customize, null);
			final EditText desc = fragment.findViewById(R.id.shortcut_creator_customize_game_description);
			desc.setText(game.getDescription());
			final ImageView iconView = fragment.findViewById(R.id.shortcut_creator_customize_game_icon);
			Drawable displayedIcon = icon;
			if (displayedIcon == null) {
				displayedIcon = CompatHelpers.DrawableCompat.getDrawable(ShortcutCreatorActivity.this, R.drawable.ic_no_game_icon);
			}
			iconView.setImageDrawable(displayedIcon);
			builder.setView(fragment);

			builder.setPositiveButton(android.R.string.ok, (dialog, which) -> {
				dialog.dismiss();

				String label = desc.getText().toString();
				// Generate an id which depends on the user description
				// Without this, if the user changes the description but already has the same shortcut (also in the dynamic ones), the other label will be reused
				String shortcutId = game.getTarget() + String.format("-%08x", label.hashCode());

				Intent shortcut = new Intent(Intent.ACTION_MAIN, Uri.fromParts("scummvm", game.getTarget(), null),
					ShortcutCreatorActivity.this, SplashActivity.class);
				Intent result = CompatHelpers.ShortcutCreator.createShortcutResultIntent(ShortcutCreatorActivity.this, shortcutId, shortcut,
					desc.getText().toString(), icon, R.drawable.ic_no_game_icon);
				setResult(RESULT_OK, result);

				finish();
			});
			builder.setNegativeButton(android.R.string.cancel, (dialog, which) ->
				dialog.cancel());

			final AlertDialog dialog = builder.create();
			desc.setOnEditorActionListener((TextView tv, int actionId, KeyEvent event) -> {
				if (actionId == EditorInfo.IME_ACTION_DONE) {
					dialog.getButton(DialogInterface.BUTTON_POSITIVE).performClick();
					return true;
				}
				return false;
			});

			dialog.show();
		}
	};

	private static class Game {
		@NonNull
		private final String _target;
		private final String _engineid;
		private final String _gameid;
		@NonNull
		private final String _description;

		private Game(@NonNull String target, String engineid, String gameid, @NonNull String description) {
			_target = target;
			_engineid = engineid;
			_gameid = gameid;
			_description = description;
		}

		@NonNull
		public String getTarget() {
			return _target;
		}

		@NonNull
		public String getDescription() {
			return _description;
		}

		public Collection<String> getIconCandidates() {
			if (_engineid == null) {
				return new ArrayList<>();
			}

			ArrayList<String> ret = new ArrayList<>();
			if (_gameid != null) {
				ret.add(String.format("icons/%s-%s.png", _engineid, _gameid).toLowerCase());
			}
			ret.add(String.format("icons/%s.png", _engineid).toLowerCase());

			return ret;
		}

		public static Game loadGame(@NonNull Map<String, Map<String, String>> parsedIniMap, String target) {
			Map<String, String> domain = parsedIniMap.get(target);
			if (domain == null) {
				return null;
			}
			String engineid = domain.get("engineid");
			String gameid = domain.get("gameid");
			String description = domain.get("description");
			if (description == null) {
				return null;
			}
			return new Game(target, engineid, gameid, description);
		}

		public static List<Game> loadGames(@NonNull Map<String, Map<String, String>> parsedIniMap) {
			List<Game> games = new ArrayList<>();
			for (Map.Entry<String, Map<String, String>> entry : parsedIniMap.entrySet()) {
				final String domain = entry.getKey();
				if (domain == null ||
					"scummvm".equals(domain) ||
					"cloud".equals(domain) ||
					"keymapper".equals(domain)) {
					continue;
				}
				String engineid = entry.getValue().get("engineid");
				String gameid = entry.getValue().get("gameid");
				String description = entry.getValue().get("description");
				if (description == null) {
					continue;
				}
				games.add(new Game(domain, engineid, gameid, description));
			}
			return games;
		}

		@NonNull
		@Override
		public String toString() {
			return _description;
		}
	}

	private static class IconsCache {
		/**
		 * This kind of mimics Common::generateZipSet
		 */
		private final Context _context;
		private final Map<String, byte[]> _icons = new LinkedHashMap<String, byte[]>(16,0.75f, true) {
			@Override
			protected boolean removeEldestEntry(Map.Entry<String, byte[]> eldest) {
				return size() > 128;
			}
		};
		private static final byte[] _noIconSentinel = new byte[0];

		private final List<ZipFile> _zipFiles = new ArrayList<>();

		public IconsCache(Context context,
		                  FileInputStream defaultStream,
		                  FileInputStream[] packsStream) {
			_context = context;

			for (int i = packsStream.length - 1; i >= 0; i--) {
				final FileInputStream packStream = packsStream[i];
				if (packStream == null) {
					continue;
				}
				try {
					ZipFile zf = new ZipFile(packStream);
					_zipFiles.add(zf);
				} catch (IOException e) {
					Log.e(LOG_TAG, "Error while loading pack ZipFile: " + i, e);
				}
			}
			if (defaultStream != null) {
				try {
					ZipFile zf = new ZipFile(defaultStream);
					_zipFiles.add(zf);
				} catch (IOException e) {
					Log.e(LOG_TAG, "Error while loading default ZipFile", e);
				}
			}
		}

		public Drawable getGameIcon(Game game) {
			for (String name : game.getIconCandidates()) {
				byte[] data = _icons.get(name);
				if (data == null) {
					data = loadIcon(name);
				}
				if (data == _noIconSentinel) {
					continue;
				}

				Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
				if (bitmap == null) {
					continue;
				}

				return new BitmapDrawable(_context.getResources(), bitmap);
			}

			return null;
		}

		private byte[] loadIcon(String name) {
			int zfi = 0;
			for(ZipFile zf : _zipFiles) {
				final ZipEntry ze = zf.getEntry(name);
				if (ze == null) {
					zfi++;
					continue;
				}

				int sz = (int) ze.getSize();
				byte[] buffer = new byte[sz];

				try (InputStream is = zf.getInputStream(ze)) {
					if (is.read(buffer) != buffer.length) {
						throw new IOException();
					}
				} catch (IOException e) {
					Log.e(LOG_TAG, "Error while uncompressing: " + name + " from zip file " + zfi, e);
					zfi++;
					continue;
				}

				_icons.put(name, buffer);
				return buffer;
			}

			// Register failure
			_icons.put(name, _noIconSentinel);
			return _noIconSentinel;
		}
	}

	private static class GameAdapter extends ArrayAdapter<Game> {
		private final IconsCache _cache;

		public GameAdapter(Context context,
		                   List<Game> items,
		                   IconsCache cache) {
			super(context, 0, items);
			Collections.sort(items, (lhs, rhs) -> lhs.getDescription().compareToIgnoreCase(rhs.getDescription()));
			_cache = cache;
		}

		@NonNull
		@Override
		public View getView(int position, View convertView, @NonNull ViewGroup parent)
		{
			if (convertView == null) {
				convertView = LayoutInflater.from(getContext()).inflate(R.layout.shortcut_creator_game_list_item, parent, false);
			}
			final Game game = getItem(position);
			assert game != null;

			final TextView desc = convertView.findViewById(R.id.shortcut_creator_game_item_description);
			desc.setText(game.getDescription());
			final ImageView iconView = convertView.findViewById(R.id.shortcut_creator_game_item_icon);
			Drawable icon = _cache.getGameIcon(game);
			if (icon == null) {
				icon = CompatHelpers.DrawableCompat.getDrawable(getContext(), R.drawable.ic_no_game_icon);
			}
			iconView.setImageDrawable(icon);
			return convertView;
		}
	}
}
