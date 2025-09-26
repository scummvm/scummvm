package org.scummvm.scummvm.net;

import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import org.scummvm.scummvm.SAFFSTree;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.SequenceInputStream;
import java.math.BigInteger;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.Charset;
import java.security.SecureRandom;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

public class HTTPRequest implements Runnable {
	final static String LOG_TAG = "ScummVM.HTTP";
	private static final int DEFAULT_BUFFER_SIZE = 16384;

	HTTPManager _manager;

	protected String _method;
	protected AtomicReference<String> _url;
	protected TreeMap<String, String> _requestHeaders;
	protected InputStream _uploadBuffer;
	protected int _uploadBufferLength;

	protected long _nativePointer;

	protected AtomicBoolean _cancelled;

	protected native void gotHeaders(long nativePointer, String[] headers);
	protected native void gotData(long nativePointer, byte[] data, int size, int totalSize);
	protected native void finished(long nativePointer, int errorCode, String errorMsg);

	/** @noinspection unused
	 * Called from JNI
	 */
	public HTTPRequest(long nativePointer, String url, String[] requestHeaders, byte[] uploadBuffer, boolean uploading, boolean usingPatch, boolean post) {
		init(nativePointer, url);
		setupUploadBuffer(uploadBuffer, uploading, usingPatch, post);
		setupHeaders(requestHeaders);
	}

	/** @noinspection unused
	 * Called from JNI
	 */
	public HTTPRequest(long nativePointer, String url, String[] requestHeaders, String[] formFields, String[] formFiles) {
		init(nativePointer, url);
		setupMultipartForm(formFields, formFiles);
		setupHeaders(requestHeaders);
	}

	private void init(long nativePointer, String url) {
		_nativePointer = nativePointer;
		_url = new AtomicReference<>(url);
		_requestHeaders = new TreeMap<>(String.CASE_INSENSITIVE_ORDER);
		_cancelled = new AtomicBoolean(false);
		_method = "GET";
	}

	private void setupUploadBuffer(byte[] uploadBuffer, boolean uploading, boolean usingPatch, boolean post) {
		_uploadBuffer = null;
		_uploadBufferLength = 0;
		if (uploading) {
			if (uploadBuffer == null) {
				uploadBuffer = new byte[0];
			}
			_uploadBuffer = new ByteArrayInputStream(uploadBuffer);
			_uploadBufferLength = uploadBuffer.length;
			_method = "PUT";
			return;
		}

		if (usingPatch) {
			_method = "PATCH";
			return;
		}

		if (uploadBuffer != null && uploadBuffer.length > 0) {
			_uploadBuffer = new ByteArrayInputStream(uploadBuffer);
			_uploadBufferLength = uploadBuffer.length;
			post = true;
		}

		if (post) {
			_method = "POST";
			_requestHeaders.put("content-type", "application/x-www-form-urlencoded");
		}
	}

	private void setupMultipartForm(String[] formFields, String[] formFiles) {
		if ((formFields.length & 1) != 0) {
			throw new IllegalArgumentException("formFields has odd length");
		}
		if ((formFiles.length & 1) != 0) {
			throw new IllegalArgumentException("formFiles has odd length");
		}

		SecureRandom rnd = new SecureRandom();
		String boundary = "ScummVM-Boundary-" + (new BigInteger(128, rnd)).toString(10);

		int contentLength = 0;
		Vector<InputStream> bodyParts = new Vector<>(formFiles.length * 2 + 2);

		_method = "POST";
		_requestHeaders.put("content-type", String.format("multipart/form-data; boundary=%s", boundary));

		StringBuilder formFieldsContent = new StringBuilder();
		for (int i = 0; i < formFields.length; i += 2) {
			formFieldsContent.append(String.format("\r\n--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n", boundary, formFields[i]));
			formFieldsContent.append(formFields[i+1]);
		}
		for (int i = 0; i < formFiles.length; i += 2) {
			File file = new File(formFiles[i+1]);
			formFieldsContent.append(String.format("\r\n--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n\r\n", boundary, formFiles[i], file.getName()));

			byte[] textContent = formFieldsContent.toString().getBytes(Charset.defaultCharset());
			bodyParts.add(new ByteArrayInputStream(textContent));
			if (contentLength >= 0) {
				try {
					contentLength = Math.addExact(contentLength, textContent.length);
				} catch (ArithmeticException e) {
					contentLength = -1;
				}
			}
			formFieldsContent = new StringBuilder();

			try {
				SAFFSTree.PathResult pr = SAFFSTree.fullPathToNode(null, file.getPath(), false);
				if (pr == null || Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
					bodyParts.add(new FileInputStream(file));
					long length = file.length();
					if (!file.isFile() || length >= Integer.MAX_VALUE) {
						contentLength = -1;
					} else if (contentLength >= 0) {
						try {
							contentLength = Math.addExact(contentLength, (int)length);
						} catch (ArithmeticException e) {
							contentLength = -1;
						}
					}
				} else {
					ParcelFileDescriptor pfd = pr.tree.createFileDescriptor(pr.node, "r");
					bodyParts.add(new ParcelFileDescriptor.AutoCloseInputStream(pfd));
					contentLength = -1;
				}
			} catch (FileNotFoundException ignored) {
				// We can't trigger an error now: we will make sure we call finished later with an error
				bodyParts.add(null);
				break;
			}
		}
		// Now we only have to close the multipart with an ending boundary
		formFieldsContent.append(String.format("\r\n--%s--\r\n", boundary));
		byte[] textContent = formFieldsContent.toString().getBytes(Charset.defaultCharset());
		bodyParts.add(new ByteArrayInputStream(textContent));
		if (contentLength >= 0) {
			try {
				contentLength = Math.addExact(contentLength, textContent.length);
			} catch (ArithmeticException e) {
				contentLength = -1;
			}
		}
		_uploadBuffer = new SequenceInputStream(bodyParts.elements());
		_uploadBufferLength = contentLength;
	}

	private void setupHeaders(String[] requestHeaders) {
		if ((requestHeaders.length & 1) != 0) {
			throw new IllegalArgumentException("requestHeaders has odd length");
		}
		for(int i = 0; i < requestHeaders.length; i += 2) {
			if (requestHeaders[i] == null) {
				// If there were invalid headers passed in native code
				// we end up with null entries at the end of the array
				return;
			}
			_requestHeaders.put(requestHeaders[i], requestHeaders[i+1]);
		}
	}

	/** @noinspection unused
	 * Called from JNI
	 */
	public void cancel() {
		_cancelled.set(true);
		// Don't notify the native object if we got cancelled: it may have been reused
		_nativePointer = 0;
	}

	public String getURL() {
		return _url.get();
	}

	private void cleanup() {
		if (_uploadBuffer != null) {
			try {
				_uploadBuffer.close();
			} catch (IOException ignored) {
			}
		}
	}

	// Runs on HTTPManager thread pool
	@Override
	public void run() {
		if (_cancelled.get()) {
			cleanup();
			return;
		}

		URL url;
		HttpURLConnection urlConnection;
		try {
			url = new URL(_url.get());
			Log.d(LOG_TAG, String.format("Will make HTTP request to %s with method %s", url, _method));
			urlConnection = (HttpURLConnection) url.openConnection();
			urlConnection.setRequestMethod(_method);
		} catch (IOException e) {
			final String errorMsg = e.getMessage();
			_manager.enqueue(() -> finished(_nativePointer, -1, errorMsg));
			cleanup();
			return;
		}

		if (_cancelled.get()) {
			cleanup();
			return;
		}

		urlConnection.setInstanceFollowRedirects(true);
		for (Map.Entry<String, String> e : _requestHeaders.entrySet()) {
			urlConnection.addRequestProperty(e.getKey(), e.getValue());
		}
		if (_uploadBuffer != null) {
			urlConnection.setDoOutput(true);
			if (_uploadBufferLength != -1) {
				urlConnection.setFixedLengthStreamingMode(_uploadBufferLength);
			}
		}

		try {
			urlConnection.connect();
			if (_cancelled.get()) {
				urlConnection.disconnect();
				cleanup();
				return;
			}

			if (_uploadBuffer != null) {
				try (OutputStream out = urlConnection.getOutputStream()) {
					byte[] buffer = new byte[DEFAULT_BUFFER_SIZE];
					int read;
					while ((read = _uploadBuffer.read(buffer, 0, DEFAULT_BUFFER_SIZE)) >= 0) {
						out.write(buffer, 0, read);
					}
				} catch (NullPointerException e) {
					// We failed to open some file when building the buffer
					_manager.enqueue(() -> finished(_nativePointer, -1, "Can't open file"));
					cleanup();
					return;
				}
			}
			if (_cancelled.get()) {
				urlConnection.disconnect();
				cleanup();
				return;
			}

			Map<String, java.util.List<String>> headersField = urlConnection.getHeaderFields();
			if (_cancelled.get()) {
				urlConnection.disconnect();
				cleanup();
				return;
			}

			final Vector<String> headers = new Vector<>(headersField.size() * 2);
			for (Map.Entry<String, java.util.List<String>> e : headersField.entrySet()) {
				String key = e.getKey();
				if (key == null) {
					// The status line is placed in the map with a null key: ignore it
					continue;
				}
				List<String> values = e.getValue();
				headers.add(key.toLowerCase(Locale.ROOT));
				headers.add(values.get(values.size() - 1));
			}
			_manager.enqueue(() -> gotHeaders(_nativePointer, headers.toArray(new String[0])));

			int contentLength = urlConnection.getContentLength();

			InputStream in = urlConnection.getInputStream();
			if (_cancelled.get()) {
				cleanup();
				return;
			}

			boolean finished = false;
			while(!finished) {
				final byte[] inputData = new byte[DEFAULT_BUFFER_SIZE];
				int offset = 0;
				while(offset < DEFAULT_BUFFER_SIZE) {
					final int size = in.read(inputData, offset, DEFAULT_BUFFER_SIZE - offset);
					if (size == -1) {
						finished = true;
						break;
					}
					if (_cancelled.get()) {
						cleanup();
						return;
					}
					offset += size;
				}

				final int offset_ = offset;
				_manager.enqueue(() -> gotData(_nativePointer, inputData, offset_, contentLength));
			}
			// Update URL field
			url = urlConnection.getURL();
			_url.set(url.toExternalForm());

			final int responseCode = urlConnection.getResponseCode();
			_manager.enqueue(() -> finished(_nativePointer, responseCode, null));
		} catch (FileNotFoundException e) {
			// The server returned an error, return the error code and no data
			int responseCode = -1;
			try {
				responseCode = urlConnection.getResponseCode();
			} catch (IOException ignored) {
			}

			final int responseCode_ = responseCode;
			final String errorMsg = e.getMessage();
			_manager.enqueue(() -> finished(_nativePointer, responseCode_, errorMsg));
			cleanup();
		} catch (IOException e) {
			Log.w(LOG_TAG, "Error when making HTTP request", e);
			final String errorMsg = e.getMessage();
			_manager.enqueue(() -> finished(_nativePointer, -1, errorMsg));
			cleanup();
		} finally {
			urlConnection.disconnect();
		}
	}
}
