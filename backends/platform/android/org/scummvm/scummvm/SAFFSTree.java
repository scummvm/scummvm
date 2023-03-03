package org.scummvm.scummvm;

import java.io.FileNotFoundException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Map;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.UriPermission;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import androidx.annotation.RequiresApi;

/**
 * SAF primitives for C++ FSNode
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class SAFFSTree {
	private static HashMap<String, SAFFSTree> _trees;

	public static void loadSAFTrees(Context context) {
		final ContentResolver resolver = context.getContentResolver();

		_trees = new HashMap<String, SAFFSTree>();
		for (UriPermission permission : resolver.getPersistedUriPermissions()) {
			final Uri uri = permission.getUri();
			if (!DocumentsContract.isTreeUri(uri)) {
				continue;
			}

			SAFFSTree tree = new SAFFSTree(context, uri);
			_trees.put(tree.getTreeId(), tree);
		}
	}

	public static SAFFSTree newTree(Context context, Uri uri) {
		if (_trees == null) {
			loadSAFTrees(context);
		}
		SAFFSTree tree = new SAFFSTree(context, uri);
		_trees.put(tree.getTreeId(), tree);
		return tree;
	}

	public static SAFFSTree[] getTrees(Context context) {
		if (_trees == null) {
			loadSAFTrees(context);
		}
		return _trees.values().toArray(new SAFFSTree[0]);
	}

	public static SAFFSTree findTree(Context context, String name) {
		if (_trees == null) {
			loadSAFTrees(context);
		}
		return _trees.get(name);
	}

	public static void clearCaches() {
		if (_trees == null) {
			return;
		}
		for (SAFFSTree tree : _trees.values()) {
			tree.clearCache();
		}
	}

	public static class SAFFSNode {
		public static final int DIRECTORY = 0x01;
		public static final int WRITABLE  = 0x02;
		public static final int READABLE  = 0x04;
		public static final int DELETABLE = 0x08;
		public static final int REMOVABLE = 0x10;

		public SAFFSNode _parent;
		public String _path;
		public String _documentId;
		public int _flags;

		private SAFFSNode() {
		}

		private SAFFSNode(SAFFSNode parent, String path, String documentId, int flags) {
			_parent = parent;
			_path = path;
			_documentId = documentId;
			_flags = flags;
		}

		private static int computeFlags(String mimeType, int flags) {
			int ourFlags = 0;
			if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType)) {
				ourFlags |= SAFFSNode.DIRECTORY;
			}
			if ((flags & (DocumentsContract.Document.FLAG_SUPPORTS_WRITE | DocumentsContract.Document.FLAG_DIR_SUPPORTS_CREATE)) != 0) {
				ourFlags |= SAFFSNode.WRITABLE;
			}
			if ((flags & DocumentsContract.Document.FLAG_VIRTUAL_DOCUMENT) == 0) {
				ourFlags |= SAFFSNode.READABLE;
			}
			if ((flags & DocumentsContract.Document.FLAG_SUPPORTS_DELETE) != 0) {
				ourFlags |= SAFFSNode.DELETABLE;
			}
			if ((flags & DocumentsContract.Document.FLAG_SUPPORTS_REMOVE) != 0) {
				ourFlags |= SAFFSNode.REMOVABLE;
			}
			return ourFlags;
		}
	}

	// Sentinel object
	private static final SAFFSNode NOT_FOUND_NODE = new SAFFSNode();

	private static class SAFCache extends LinkedHashMap<String, SAFFSNode> {
		private static final int MAX_ENTRIES = 10000;

		public SAFCache() {
			super(16, 0.75f, true);
		}

		@Override
		protected boolean removeEldestEntry(Map.Entry<String, SAFFSNode> eldest) {
			return size() > MAX_ENTRIES;
		}
	}

	private Context _context;
	private Uri _treeUri;

	private SAFFSNode _root;
	private String _treeName;

	private SAFCache _cache;

	public SAFFSTree(Context context, Uri treeUri) {
		_context = context;
		_treeUri = treeUri;

		_cache = new SAFCache();

		_root = new SAFFSNode(null, "", DocumentsContract.getTreeDocumentId(treeUri), 0);
		// Update flags and get name
		_treeName = stat(_root);
		clearCache();
	}

	public String getTreeId() {
		return Uri.encode(DocumentsContract.getTreeDocumentId(_treeUri));
	}

	private void clearCache() {
		_cache.clear();
		_cache.put("/", _root);
		_cache.put("", _root);
	}

	private static String[] normalizePath(String path) {
		LinkedList<String> components = new LinkedList<String>(Arrays.asList(path.split("/")));
		ListIterator<String> it = components.listIterator();
		while(it.hasNext()) {
			final String component = it.next();
			if (component.isEmpty()) {
				it.remove();
				continue;
			}
			if (".".equals(component)) {
				it.remove();
				continue;
			}
			if ("..".equals(component)) {
				it.remove();
				if (it.hasPrevious()) {
					it.previous();
					it.remove();
				}
			}
		}
		return components.toArray(new String[0]);
	}

	public SAFFSNode pathToNode(String path) {
		SAFFSNode node = null;

		// Short-circuit
		node = _cache.get(path);
		if (node != null) {
			if (node == NOT_FOUND_NODE) {
				return null;
			} else {
				return node;
			}
		}

		String[] components = normalizePath(path);

		int pivot = components.length;
		String wpath = "/" + String.join("/", components);

		while(pivot > 0) {
			node = _cache.get(wpath);
			if (node != null) {
				break;
			}

			// Try without last component
			pivot--;
			int newidx = wpath.length() - components[pivot].length() - 1;
			wpath = wpath.substring(0, newidx);
		}

		// We found a negative result in cache for a point in the path
		if (node == NOT_FOUND_NODE) {
			wpath = "/" + String.join("/", components);
			_cache.put(wpath, NOT_FOUND_NODE);
			_cache.put(path, NOT_FOUND_NODE);
			return null;
		}

		// Start from the last cached result (if any)
		if (pivot == 0) {
			node = _root;
		}
		while(pivot < components.length) {
			node = getChild(node, components[pivot]);
			if (node == null) {
				// Cache as much as we can
				wpath = "/" + String.join("/", components);
				_cache.put(wpath, NOT_FOUND_NODE);
				_cache.put(path, NOT_FOUND_NODE);
				return null;
			}

			pivot++;
		}

		_cache.put(path, node);
		return node;
	}

	public SAFFSNode[] getChildren(SAFFSNode node) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri searchUri = DocumentsContract.buildChildDocumentsUriUsingTree(_treeUri, node._documentId);
		final LinkedList<SAFFSNode> results = new LinkedList<>();

		Cursor c = null;
		try {
			c = resolver.query(searchUri, new String[] { DocumentsContract.Document.COLUMN_DISPLAY_NAME,
				DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_MIME_TYPE,
				DocumentsContract.Document.COLUMN_FLAGS }, null, null, null);
			while (c.moveToNext()) {
				final String displayName = c.getString(0);
				final String documentId = c.getString(1);
				final String mimeType = c.getString(2);
				final int flags = c.getInt(3);

				final int ourFlags = SAFFSNode.computeFlags(mimeType, flags);

				SAFFSNode newnode = new SAFFSNode(node, node._path + "/" + displayName, documentId, ourFlags);
				_cache.put(newnode._path, newnode);
				results.add(newnode);
			}
		} catch (Exception e) {
			Log.w(ScummVM.LOG_TAG, "Failed query: " + e);
		} finally {
			if (c != null) {
				c.close();
			}
		}

		return results.toArray(new SAFFSNode[0]);
	}

	public SAFFSNode getChild(SAFFSNode node, String name) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri searchUri = DocumentsContract.buildChildDocumentsUriUsingTree(_treeUri, node._documentId);

		String childPath = node._path + "/" + name;
		SAFFSNode newnode;

		newnode = _cache.get(childPath);
		if (newnode == null) {
			// Child is not cached: fetch all files in the parent
			// They will end up in cache
			getChildren(node);
			newnode = _cache.get(childPath);
		}

		if (newnode == null) {
			// The child hasn't been found: don't look it up again
			_cache.put(childPath, NOT_FOUND_NODE);
			return null;
		} else if (newnode == NOT_FOUND_NODE) {
			return null;
		} else {
			return newnode;
		}
	}

	public SAFFSNode createDirectory(SAFFSNode node, String name) {
		return createDocument(node, name, DocumentsContract.Document.MIME_TYPE_DIR);
	}

	public SAFFSNode createFile(SAFFSNode node, String name) {
		return createDocument(node, name, "application/octet-stream");
	}

	public int createReadStream(SAFFSNode node) {
		return createStream(node, "r");
	}

	public int createWriteStream(SAFFSNode node) {
		return createStream(node, "wt");
	}

	public boolean removeNode(SAFFSNode node) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri uri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._documentId);

		if ((node._flags & SAFFSNode.REMOVABLE) != 0) {
			final Uri parentUri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._parent._documentId);
			try {
				if (!DocumentsContract.removeDocument(resolver, uri, parentUri)) {
					return false;
				}
			} catch(FileNotFoundException e) {
				return false;
			}
		} else if ((node._flags & SAFFSNode.DELETABLE) != 0) {
			try {
				if (!DocumentsContract.deleteDocument(resolver, uri)) {
					return false;
				}
			} catch(FileNotFoundException e) {
				return false;
			}
		} else {
			return false;
		}

		for(Map.Entry<String, SAFFSNode> e : _cache.entrySet()) {
			if (e.getValue() == node) {
				e.setValue(NOT_FOUND_NODE);
			}
		}

		return true;
	}

	public void removeTree() {
		final ContentResolver resolver = _context.getContentResolver();

		String treeId = getTreeId();

		resolver.releasePersistableUriPermission(_treeUri,
			Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

		if (_trees == null || _trees.remove(treeId) == null) {
			loadSAFTrees(_context);
		}
	}

	private SAFFSNode createDocument(SAFFSNode node, String name, String mimeType) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri parentUri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._documentId);
		Uri newDocUri;

		try {
			newDocUri = DocumentsContract.createDocument(resolver, parentUri, mimeType, name);
		} catch(FileNotFoundException e) {
			return null;
		}
		if (newDocUri == null) {
			return null;
		}

		final String documentId = DocumentsContract.getDocumentId(newDocUri);

		final SAFFSNode newnode = new SAFFSNode(node, node._path + "/" + name, documentId, 0);
		// Update flags
		final String realName = stat(_root);
		if (realName == null) {
			return null;
		}
		// Unlikely but...
		if (!realName.equals(name)) {
			newnode._path = node._path + "/" + realName;
		}

		_cache.put(newnode._path, newnode);

		return newnode;
	}

	private int createStream(SAFFSNode node, String mode) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri uri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._documentId);

		ParcelFileDescriptor pfd;
		try {
			pfd = resolver.openFileDescriptor(uri, mode);
		} catch(FileNotFoundException e) {
			return -1;
		}
		if (pfd == null) {
			return -1;
		}

		return pfd.detachFd();
	}

	private String stat(SAFFSNode node) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri uri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._documentId);

		Cursor c = null;
		try {
			c = resolver.query(uri, new String[] { DocumentsContract.Document.COLUMN_DISPLAY_NAME,
				DocumentsContract.Document.COLUMN_MIME_TYPE, DocumentsContract.Document.COLUMN_FLAGS }, null, null, null);
			while (c.moveToNext()) {
				final String displayName = c.getString(0);
				final String mimeType = c.getString(1);
				final int flags = c.getInt(2);

				node._flags = SAFFSNode.computeFlags(mimeType, flags);

				return displayName;
			}
		} catch (Exception e) {
			Log.w(ScummVM.LOG_TAG, "Failed query: " + e);
		} finally {
			if (c != null) {
				try {
					c.close();
				} catch (RuntimeException e) {
					throw e;
				} catch (Exception e) {
				}
			}
		}
		// We should never end up here...
		return null;
	}
}
