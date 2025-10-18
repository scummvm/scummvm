package org.scummvm.scummvm.net;

/*
 * Customized from https://blog.dev-area.net/2015/08/13/android-4-1-enable-tls-1-1-and-tls-1-2/
 * Added TLS1.3 support and keep old protocols enabled for maximum compatibility
 */

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

/**
 * @author fkrauthan
 */
public class TLSSocketFactory extends SSLSocketFactory {

	private final SSLSocketFactory _factory;
	private static String[] _protocols;

	private static boolean _init;

	public static void init() {
		if (_init) {
			return;
		}
		try {
			HttpsURLConnection.setDefaultSSLSocketFactory(new TLSSocketFactory());
		} catch (RuntimeException ignored) {
		}
		_init = true;
	}

	public TLSSocketFactory() {
		SSLContext context = null;
		try {
			context = LETrustManager.getContext();
		} catch (NoSuchAlgorithmException e) {
			throw new RuntimeException(e);
		} catch (KeyManagementException e) {
			throw new RuntimeException(e);
		}
		_factory = context.getSocketFactory();
	}

	@Override
	public String[] getDefaultCipherSuites() {
		return _factory.getDefaultCipherSuites();
	}

	@Override
	public String[] getSupportedCipherSuites() {
		return _factory.getSupportedCipherSuites();
	}

	@Override
	public Socket createSocket() throws IOException {
		return enableTLSOnSocket(_factory.createSocket());
	}

	@Override
	public Socket createSocket(Socket s, String host, int port, boolean autoClose) throws IOException {
		return enableTLSOnSocket(_factory.createSocket(s, host, port, autoClose));
	}

	@Override
	public Socket createSocket(String host, int port) throws IOException, UnknownHostException {
		return enableTLSOnSocket(_factory.createSocket(host, port));
	}

	@Override
	public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException, UnknownHostException {
		return enableTLSOnSocket(_factory.createSocket(host, port, localHost, localPort));
	}

	@Override
	public Socket createSocket(InetAddress host, int port) throws IOException {
		return enableTLSOnSocket(_factory.createSocket(host, port));
	}

	@Override
	public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort) throws IOException {
		return enableTLSOnSocket(_factory.createSocket(address, port, localAddress, localPort));
	}

	private Socket enableTLSOnSocket(Socket socket) {
		if(socket instanceof SSLSocket) {
			SSLSocket sslSocket = (SSLSocket)socket;

			if (_protocols == null) {
				String[] newProtocols = {"TLSv1", "TLSv1.1", "TLSv1.2", "TLSv1.3"};

				// Build the list of protocols to enable
				Set<String> protocols = new HashSet<>(Arrays.asList(sslSocket.getEnabledProtocols()));
				Set<String> supported = new HashSet<>(Arrays.asList(sslSocket.getSupportedProtocols()));
				for (String protocol : newProtocols) {
					if (protocols.contains(protocol)) {
						continue;
					}
					if (!supported.contains(protocol)) {
						continue;
					}
					protocols.add(protocol);
				}
				_protocols = protocols.toArray(new String[]{});
			}

			sslSocket.setEnabledProtocols(_protocols);
		}
		return socket;
	}
}
