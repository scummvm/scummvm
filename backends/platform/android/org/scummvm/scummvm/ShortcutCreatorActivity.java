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
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.text.Editable;
import android.text.TextWatcher;
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
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipInputStream;

public class ShortcutCreatorActivity extends Activity {
	private IconsCache _cache;
	private GameAdapter _listAdapter;

	private final ExecutorService _executor = new ThreadPoolExecutor(
		0, Runtime.getRuntime().availableProcessors(),
		1L, TimeUnit.SECONDS,
		new LinkedBlockingQueue<>());

	private ProgressBar _progressBar;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shortcut_creator_activity);

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

		OpenFileResult defaultStream = openFile(new File(getFilesDir(), "gui-icons.dat"));

		File iconsPath = INIParser.getPath(parsedIniMap, "scummvm", "iconspath",
			new File(getFilesDir(), "icons"));
		OpenFileResult[] packsStream = openFiles(iconsPath, "gui-icons.*\\.dat");

		_cache = new IconsCache(this, _cacheListener,
			games, defaultStream, packsStream,
			_executor, new Handler(Looper.getMainLooper()));
		_listAdapter = new GameAdapter(this, games, _cache);

		ListView listView = findViewById(R.id.shortcut_creator_games_list);
		listView.setAdapter(_listAdapter);
		listView.setEmptyView(findViewById(R.id.shortcut_creator_games_list_empty));
		listView.setOnItemClickListener(_gameClicked);

		_progressBar = findViewById(R.id.shortcut_creator_progress_bar);

		EditText searchEdit = findViewById(R.id.shortcut_creator_search_edit);
		searchEdit.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence cs, int arg1, int arg2, int arg3) {
				_listAdapter.getFilter().filter(cs.toString());
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
	protected void onDestroy() {
		super.onDestroy();

		_executor.shutdownNow();
	}

	private static class OpenFileResult {
		@NonNull
		public FileInputStream stream;
		public long streamSize;
		OpenFileResult(@NonNull FileInputStream stream, long streamSize) {
			this.stream = stream;
			this.streamSize = streamSize;
		}
	}

	private OpenFileResult openFile(File path) {
		 try {
			FileInputStream stream = new FileInputStream(path);
			return new OpenFileResult(stream, path.length());
		} catch (FileNotFoundException e) {
			return null;
		}
	}

	private OpenFileResult[] openFiles(File basePath, String regex) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N ||
			!basePath.getPath().startsWith("/saf/")) {
			// This is a standard filesystem path
			File[] children = basePath.listFiles((dir, name) -> name.matches(regex));
			if (children == null) {
				return new OpenFileResult[0];
			}
			Arrays.sort(children);
			OpenFileResult[] ret = new OpenFileResult[children.length];
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

		SAFFSTree tree = SAFFSTree.findTree(this, treeName);
		if (tree == null) {
			return new OpenFileResult[0];
		}
		SAFFSTree.SAFFSNode node = tree.pathToNode(path);
		if (node == null) {
			return new OpenFileResult[0];
		}
		SAFFSTree.SAFFSNode[] children = tree.getChildren(node);
		if (children == null) {
			return new OpenFileResult[0];
		}
		Arrays.sort(children);

		ArrayList<OpenFileResult> ret = new ArrayList<>();
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
			ret.add(new OpenFileResult(
				new ParcelFileDescriptor.AutoCloseInputStream(pfd),
				pfd.getStatSize()
			));
		}
		return ret.toArray(new OpenFileResult[0]);
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

				Intent shortcut = new Intent(Intent.ACTION_MAIN, Uri.fromParts("scummvm", game.getTarget(), null),
					ShortcutCreatorActivity.this, SplashActivity.class);
				Intent result = CompatHelpers.ShortcutCreator.createShortcutResultIntent(ShortcutCreatorActivity.this, game.getTarget(), shortcut,
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

	private final IconsCache.IconsCacheListener _cacheListener = new IconsCache.IconsCacheListener() {
		@Override
		public void onIconUpdated(List<Game> games) {
			_listAdapter.notifyDataSetChanged();
		}

		@Override
		public void onLoadProgress(int percent) {
			if (percent == 100) {
				_progressBar.setVisibility(View.GONE);
			}
			_progressBar.setProgress(percent);
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
		 * This kind of mimics Common::generateZipSet with asynchronous feature
		 */
		public interface IconsCacheListener {
			void onIconUpdated(List<Game> games);
			void onLoadProgress(int percent);
		}

		private final Context _context;
		private final IconsCacheListener _listener;
		private final Map<String, byte[]> _icons = new HashMap<>();
		private final Map<String, List<Game>> _candidates = new HashMap<>();

		private long _totalSize;
		private final long[] _totalSizes;
		private final long[] _readSizes;

		public IconsCache(Context context, IconsCacheListener listener,
		                  List<Game> games,
		                  OpenFileResult defaultStream, OpenFileResult[] packsStream,
		                  Executor executor, Handler handler) {
			_context = context;
			_listener = listener;

			// Establish a list of candidates
			for (Game game : games) {
				for (String candidate : game.getIconCandidates()) {
					List<Game> v = _candidates.get(candidate);
					if (v == null) {
						v = new ArrayList<>();
						_candidates.put(candidate, v);
					}
					v.add(game);
				}
			}

			_totalSizes = new long[1 + packsStream.length];
			_readSizes = new long[1 + packsStream.length];

			// Iterate over the files starting with default and continuing with packs
			// This will let us erase outdated versions
			if (defaultStream != null) {
				_totalSizes[0] = defaultStream.streamSize;
				_totalSize += _totalSizes[0];
				executor.execute(() -> loadZip(defaultStream.stream, 0, handler));
			}
			int i = 1;
			for (final OpenFileResult packStream : packsStream) {
				if (packStream == null) {
					continue;
				}

				_totalSizes[i] = packStream.streamSize;
				_totalSize += _totalSizes[i];
				// Make it final for lambda
				int argI = i;
				executor.execute(() -> loadZip(packStream.stream, argI, handler));
				i += 1;
			}

			if (_totalSize == 0) {
				handler.post(() -> _listener.onLoadProgress(100));
			}
		}

		public Drawable getGameIcon(Game game) {
			for (String name : game.getIconCandidates()) {
				byte[] data = _icons.get(name);
				if (data == null) {
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

		private void loadZip(@NonNull FileInputStream zipStream, int id, @NonNull Handler handler) {
			try (ZipInputStream zip = new ZipInputStream(zipStream)) {
				ZipEntry entry;
				while ((entry = zip.getNextEntry()) != null) {
					_readSizes[id] = zipStream.getChannel().position();
					handler.post(() -> {
						long readSize = 0;
						for (long pos : _readSizes) {
							readSize += pos;
						}
						_listener.onLoadProgress((int)(readSize * 100 / _totalSize));
					});

					String name = entry.getName().toLowerCase();
					if (entry.isDirectory()) {
						zip.closeEntry();
						continue;
					}
					if (!_candidates.containsKey(name)) {
						zip.closeEntry();
						continue;
					}

					int sz = (int) entry.getSize();
					byte[] buffer = new byte[sz];
					int off = 0;
					while (off < buffer.length && (sz = zip.read(buffer, off, buffer.length - off)) > 0) {
						off += sz;
					}
					if (off != buffer.length) {
						throw new IOException();
					}

					_icons.put(name, buffer);
					handler.post(() -> _listener.onIconUpdated(_candidates.get(name)));

					zip.closeEntry();
				}
				_readSizes[id] = _totalSizes[id];
				handler.post(() -> {
					long readSize = 0;
					for (long pos : _readSizes) {
						readSize += pos;
					}
					_listener.onLoadProgress((int)(readSize * 100 / _totalSize));
				});
			} catch (ZipException ignored) {
			} catch (IOException ignored) {
			}
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
