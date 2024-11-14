package org.scummvm.scummvm;

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

import java.io.FileNotFoundException;
import java.lang.ref.SoftReference;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.HashMap;

/**
 * SAF primitives for C++ FSNode
 */
@RequiresApi(api = Build.VERSION_CODES.N)
public class SAFFSTree {
	private static HashMap<String, SAFFSTree> _trees;

	public static void loadSAFTrees(Context context) {
		final ContentResolver resolver = context.getContentResolver();

		_trees = new HashMap<>();
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

	@RequiresApi(api = 0)
	public static void clearCaches() {
		if (_trees == null) {
			return;
		}
		for (SAFFSTree tree : _trees.values()) {
			tree.clearCache();
		}
	}

	public static class SAFFSNode implements Comparable<SAFFSNode> {
		public static final int DIRECTORY = 0x01;
		public static final int WRITABLE  = 0x02;
		public static final int READABLE  = 0x04;
		public static final int DELETABLE = 0x08;
		public static final int REMOVABLE = 0x10;

		public SAFFSNode _parent;
		public String _path;
		public String _documentId;
		public int _flags;

		private HashMap<String, SoftReference<SAFFSNode>> _children;
		private boolean _dirty;

		private SAFFSNode() {
		}

		private SAFFSNode reset(SAFFSNode parent, String path, String documentId, int flags) {
			_parent = parent;
			_path = path;
			_documentId = documentId;
			_flags = flags;

			_children = null;
			return this;
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

		@Override
		public int compareTo(SAFFSNode o) {
			if (o == null) {
				throw new NullPointerException();
			}
			return _path.compareTo(o._path);
		}
	}

	private Context _context;
	private Uri _treeUri;

	private SAFFSNode _root;
	private String _treeName;

	public SAFFSTree(Context context, Uri treeUri) {
		_context = context;
		_treeUri = treeUri;

		_root = new SAFFSNode().reset(null, "", DocumentsContract.getTreeDocumentId(treeUri), 0);
		// Update flags and get name
		_treeName = stat(_root);
	}

	public String getTreeId() {
		return Uri.encode(DocumentsContract.getTreeDocumentId(_treeUri));
	}

	private void clearCache() {
		ArrayDeque<SAFFSNode> stack = new ArrayDeque<>();
		stack.push(_root);
		while (stack.size() > 0) {
			SAFFSNode node = stack.pop();
			node._dirty = true;
			if (node._children == null) {
				continue;
			}
			for (SoftReference<SAFFSNode> ref : node._children.values()) {
				node = ref.get();
				if (node != null) {
					stack.push(node);
				}
			}
		}
	}

	public SAFFSNode pathToNode(String path) {
		String[] components = path.split("/");

		SAFFSNode node = _root;
		for (String component : components) {
			if (component.isEmpty() || ".".equals(component)) {
				continue;
			}
			if ("..".equals(component)) {
				if (node._parent != null) {
					node = node._parent;
				}
				continue;
			}

			node = getChild(node, component);
			if (node == null) {
				return null;
			}
		}
		return node;
	}

	public SAFFSNode[] getChildren(SAFFSNode node) {
		Collection<SAFFSNode> results = null;
		if (node._children != null && !node._dirty) {
			results = new ArrayDeque<>();
			for (SoftReference<SAFFSNode> ref : node._children.values()) {
				if (ref == null) {
					continue;
				}
				SAFFSNode newnode = ref.get();
				if (newnode == null) {
					// Some reference went stale: refresh
					results = null;
					break;
				}
				results.add(newnode);
			}
		}

		if (results == null) {
			try {
				results = fetchChildren(node);
			} catch (Exception e) {
				Log.w(ScummVM.LOG_TAG, "Failed to get children: " + e);
				return null;
			}
		}

		return results.toArray(new SAFFSNode[0]);
	}

	public Collection<SAFFSNode> fetchChildren(SAFFSNode node) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri searchUri = DocumentsContract.buildChildDocumentsUriUsingTree(_treeUri, node._documentId);
		final ArrayDeque<SAFFSNode> results = new ArrayDeque<>();

		// Keep the old children around to reuse them: this will help to keep one SAFFSNode instance for each node
		HashMap<String, SoftReference<SAFFSNode>> oldChildren = node._children;
		node._children = null;

		// When _children will be set, it will be clean
		node._dirty = false;
		HashMap<String, SoftReference<SAFFSNode>> newChildren = new HashMap<>();

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

				SAFFSNode newnode = null;
				SoftReference<SAFFSNode> oldnodeRef = null;
				if (oldChildren != null) {
					oldnodeRef = oldChildren.remove(displayName);
					if (oldnodeRef != null) {
						newnode = oldnodeRef.get();
					}
				}
				if (newnode == null) {
					newnode = new SAFFSNode();
				}

				newnode.reset(node, node._path + "/" + displayName, documentId, ourFlags);
				newChildren.put(displayName, new SoftReference<>(newnode));

				results.add(newnode);
			}

			// Success: store the cache
			node._children = newChildren;
		} finally {
			if (c != null) {
				c.close();
			}
		}

		return results;
	}

	public SAFFSNode getChild(SAFFSNode node, String name) {
		final Uri searchUri = DocumentsContract.buildChildDocumentsUriUsingTree(_treeUri, node._documentId);

		SAFFSNode newnode;

		// This variable is used to hold a strong reference on every children nodes
		Collection<SAFFSNode> children;

		if (node._children == null || node._dirty) {
			try {
				children = fetchChildren(node);
			} catch (Exception e) {
				Log.w(ScummVM.LOG_TAG, "Failed to get children: " + e);
				return null;
			}
		}

		SoftReference<SAFFSNode> ref = node._children.get(name);
		if (ref == null) {
			return null;
		}

		newnode = ref.get();
		if (newnode != null) {
			return newnode;
		}

		// Node reference was stale, force a refresh
		try {
			children = fetchChildren(node);
		} catch (Exception e) {
			Log.w(ScummVM.LOG_TAG, "Failed to get children: " + e);
			return null;
		}

		ref = node._children.get(name);
		if (ref == null) {
			return null;
		}

		newnode = ref.get();
		if (newnode == null) {
			Log.e(ScummVM.LOG_TAG, "Failed to keep a reference on object");
		}

		return newnode;
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

		// Cleanup node
		node._parent._dirty = true;
		node.reset(null, null, null, 0);

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

		// Make sure _children is OK
		if (node._children == null || node._dirty) {
			try {
				fetchChildren(node);
			} catch (Exception e) {
				Log.w(ScummVM.LOG_TAG, "Failed to get children: " + e);
				return null;
			}
		}

		try {
			newDocUri = DocumentsContract.createDocument(resolver, parentUri, mimeType, name);
		} catch(FileNotFoundException e) {
			return null;
		}
		if (newDocUri == null) {
			return null;
		}

		final String documentId = DocumentsContract.getDocumentId(newDocUri);

		SAFFSNode newnode = null;

		SoftReference<SAFFSNode> oldnodeRef = node._children.remove(name);
		if (oldnodeRef != null) {
			newnode = oldnodeRef.get();
		}
		if (newnode == null) {
			newnode = new SAFFSNode();
		}

		newnode.reset(node, node._path + "/" + name, documentId, 0);
		// Update flags
		final String realName = stat(newnode);
		if (realName == null) {
			return null;
		}
		// Unlikely but...
		if (!realName.equals(name)) {
			node._children.remove(realName);
			newnode._path = node._path + "/" + realName;
		}

		node._children.put(realName, new SoftReference<>(newnode));

		return newnode;
	}

	public ParcelFileDescriptor createFileDescriptor(SAFFSNode node, String mode) {
		final ContentResolver resolver = _context.getContentResolver();
		final Uri uri = DocumentsContract.buildDocumentUriUsingTree(_treeUri, node._documentId);

		ParcelFileDescriptor pfd;
		try {
			pfd = resolver.openFileDescriptor(uri, mode);
		} catch(FileNotFoundException e) {
			return null;
		}

		return pfd;
	}

	private int createStream(SAFFSNode node, String mode) {
		ParcelFileDescriptor pfd = createFileDescriptor(node, mode);
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
