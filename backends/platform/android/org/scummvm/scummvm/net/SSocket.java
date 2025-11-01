package org.scummvm.scummvm.net;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.Proxy;
import java.net.ProxySelector;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import java.util.List;
import java.util.Locale;

import javax.net.ssl.SSLSocketFactory;

/** @noinspection unused*/
public class SSocket {
	final static String LOG_TAG = "ScummVM";

	protected Socket _socket;

	protected int _buffer = -2;

	public SSocket(String url_) {
		final URL url;
		try {
			url = new URL(url_);
		} catch (MalformedURLException e) {
			throw new RuntimeException(e);
		}

		String scheme = url.getProtocol().toLowerCase();
		if (!scheme.equals("http") && !scheme.equals("https")) {
			throw new RuntimeException("Unsupported protocol");
		}

		String host = url.getHost();
		if (host == null) {
			throw new RuntimeException("Missing host name");
		}
		if (host.contains(":")) {
			host = String.format("[%s]", host);
		}
		int port = url.getPort();
		if (port == -1) {
			port = url.getDefaultPort();
		}

		Socket socket;
		try {
			socket = proxyConnect(url, host, port);

			if (scheme.equals("https")) {
				SSLSocketFactory ssf = new TLSSocketFactory();
				socket = ssf.createSocket(socket, host, port, true);
			}

			_socket = socket;
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	private static Socket proxyConnect(URL url, String host, int port) throws IOException {
		Socket ret;

		final URI uri;
		try {
			uri = url.toURI();
		} catch (URISyntaxException e) {
			throw new RuntimeException(e);
		}
		SocketAddress sa = new InetSocketAddress(host, port);

		final ProxySelector proxySelector = ProxySelector.getDefault();
		final List<Proxy> proxies = proxySelector.select(uri);

		IOException lastExc = null;
		for (Proxy proxy : proxies) {
			final Proxy.Type proxyType = proxy.type();
			try {
				if (proxyType != Proxy.Type.HTTP) {
					ret = new Socket(proxy);
					ret.connect(sa);
					return ret;
				}

				// HTTP proxy with Socket is not supported on Android
				// Let's do it ourselves with a CONNECT method

				// First, resolve the proxy address: it's not resolved in Proxy
				InetSocketAddress proxyAddress = (InetSocketAddress)proxy.address();
				InetAddress addr = proxyAddress.getAddress();
				String proxyHost;
				if (addr != null) {
					proxyHost = addr.getHostName();
				} else {
					proxyHost = proxyAddress.getHostName();
				}
				int proxyPort = proxyAddress.getPort();
				proxyAddress = new InetSocketAddress(proxyHost, proxyPort);

				ret = new Socket();
				ret.connect(proxyAddress);

				proxyHTTPConnect(ret, host, port);
				return ret;
			} catch (IOException e) {
				Log.e(LOG_TAG, "Got an exception while connecting", e);
				if (proxy.address() != null) {
					proxySelector.connectFailed(uri, proxy.address(), e);
				}
				lastExc = e;
			}
		}
		if (lastExc == null) {
			throw new RuntimeException("No proxy specified");
		}
		throw lastExc;
	}

	private static void proxyHTTPConnect(Socket socket, String host, int port) throws IOException {
		String requestLine = String.format(Locale.ROOT, "CONNECT %s:%d HTTP/1.0\r\n\r\n", host, port);
		socket.getOutputStream().write(requestLine.getBytes());
		byte[] buffer = readLine(socket);

		// HTTP/1.x SP 2xx SP
		if (buffer.length < 13 ||
			buffer[0] != 'H' ||
			buffer[1] != 'T' ||
			buffer[2] != 'T' ||
			buffer[3] != 'P' ||
			buffer[4] != '/' ||
			buffer[5] != '1' ||
			buffer[6] != '.' ||
			(buffer[7] != '0' && buffer[7] != '1')  ||
			buffer[8] != ' ' ||
			buffer[9] != '2' ||
			!Character.isDigit(buffer[10]) ||
			!Character.isDigit(buffer[11]) ||
			buffer[12] != ' ') {
			throw new IOException("Invalid proxy reply");
		}

		for (int i = 0; i < 64 && buffer.length > 0; i++) {
			buffer = readLine(socket);
		}
		if (buffer.length > 0) {
			throw new IOException("Invalid proxy reply: too much headers");
		}
	}

	private static byte[] readLine(Socket socket) throws IOException {
		InputStream is = socket.getInputStream();
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		int b;
		while (true) {
			b = is.read();
			if (b == -1) {
				return baos.toByteArray();
			}
			if (b == '\r') {
				continue;
			}
			if (b == '\n') {
				return baos.toByteArray();
			}
			baos.write(b);
		}
	}

	public int ready() {
		if (_buffer != -2) {
			// We have at least one byte or an EOF
			return 1;
		}
		try {
			// Set receive timeout to something ridiculously low to mimic a non-blocking socket
			_socket.setSoTimeout(1);
			_buffer = _socket.getInputStream().read();
			return 1;
		} catch (SocketTimeoutException e) {
			// Nothing was ready to consume
			return 0;
		} catch (IOException e) {
			Log.e(LOG_TAG, "Got an exception while checking ready status", e);
			// Make it like if there was something ready
			return 1;
		}
	}

	public int send(byte[] data) {
		try {
			// Setup unlimited read timeout to allow for SSL exchanges to work
			_socket.setSoTimeout(0);
			_socket.getOutputStream().write(data);
			return data.length;
		} catch (IOException e) {
			Log.e(LOG_TAG, "Got an exception while sending socket data", e);
			// This likely failed
			return 0;
		}
	}

	public int recv(byte[] data) {
		if (data.length == 0) {
			return 0;
		}
		if (_buffer == -1) {
			_buffer = -2;
			return -1;
		}
		int offset = 0;
		if (_buffer != -2) {
			data[0] = (byte)_buffer;
			offset = 1;
			_buffer = -2;
		}
		try {
			int recvd = 0;
			long end = System.currentTimeMillis() + 5000;
			while (true) {
				try {
					// Allow for some timeout but not too much
					_socket.setSoTimeout(500);
					recvd = _socket.getInputStream().read(data, offset, data.length - offset);
					break;
				} catch (SocketTimeoutException e1) {
					if (System.currentTimeMillis() >= end) {
						break;
					}
				}
			}
			if (offset == 0) {
				// Nothing was buffered
				return recvd;
			}
			if (recvd == -1) {
				// Buffer the EOF and return the previous buffered data;
				_buffer = -1;
				return offset;
			}
			return offset + recvd;
		} catch (IOException e) {
			Log.e(LOG_TAG, "Got an exception while receiving socket data", e);
			return offset;
		}
	}

	public void close() {
		try {
			_socket.close();
		} catch (IOException e) {
			Log.e(LOG_TAG, "Got an exception while closing socket", e);
		}
		_socket = null;
	}
}
