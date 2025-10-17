package org.scummvm.scummvm.net;

import android.util.Base64;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Locale;

/*
 * Inspiration taken from http://blog.novoj.net/2016/02/29/how-to-make-apache-httpclient-trust-lets-encrypt-certificate-authority/
 */

class LETrustManager implements X509TrustManager {
	private static final String[] derLECerts = {
		/* ISRG Root X1 */ "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygch77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6UA5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sWT8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyHB5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UCB5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUvKBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWnOlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTnjh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbwqHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CIrU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkqhkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZLubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KKNFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7UrTkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdCjNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVcoyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPAmRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57demyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=",
		/* ISRG Root X2 */ "MIICGzCCAaGgAwIBAgIQQdKd0XLq7qeAwSxs6S+HUjAKBggqhkjOPQQDAzBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMjAeFw0yMDA5MDQwMDAwMDBaFw00MDA5MTcxNjAwMDBaME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBTZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgyMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0HttwW+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9ItgKbppbd9/w+kHsOdx1ymgHDB/qo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUfEKWrt5LSDv6kviejM9ti6lyN5UwCgYIKoZIzj0EAwMDaAAwZQIwe3lORlCEwkSHRhtFcP9Ymd70/aTSVaYgLXTWNLxBo1BfASdWtL4ndQavEi51mI38AjEAi/V3bNTIZargCyzuFJ0nN6T5U6VR5CmD1/iQMVtCnwr1/q4AaOeMSQ+2b1tbFfLn",
	};

	private static LETrustManager instance;

	private final X509TrustManager _systemTrustManager;
	private final X509TrustManager _leTrustManager;

	 static SSLContext getContext() throws NoSuchAlgorithmException, KeyManagementException {
		try {
			if (instance == null) {
				instance = new LETrustManager();
			}
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace(System.err);
			return SSLContext.getDefault();
		} catch (KeyStoreException e) {
			e.printStackTrace(System.err);
			return SSLContext.getDefault();
		} catch (CertificateException e) {
			e.printStackTrace(System.err);
			return SSLContext.getDefault();
		} catch (IOException e) {
			e.printStackTrace(System.err);
			return SSLContext.getDefault();
		}

		SSLContext sslContext = SSLContext.getInstance("TLS");
		sslContext.init(null, new TrustManager[]{instance}, null);
		return sslContext;
	 }

	public LETrustManager() throws NoSuchAlgorithmException, KeyStoreException, CertificateException, IOException {
		final TrustManagerFactory mainTrustFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
		mainTrustFactory.init((KeyStore)null);
		this._systemTrustManager = (X509TrustManager)mainTrustFactory.getTrustManagers()[0];

		KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
		ks.load(null);

		CertificateFactory cf = CertificateFactory.getInstance("X.509");
		int i = 1;
		for (String derCert : derLECerts) {
			ByteArrayInputStream is = new ByteArrayInputStream(Base64.decode(derCert, Base64.DEFAULT));
			Certificate cert = cf.generateCertificate(is);
			ks.setCertificateEntry(String.format(Locale.getDefault(), "%d", i), cert);
			i++;
		}

		final TrustManagerFactory leTrustFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
		leTrustFactory.init(ks);
		this._leTrustManager = (X509TrustManager)leTrustFactory.getTrustManagers()[0];
	}

	@Override
	public void checkClientTrusted(final X509Certificate[] x509Certificates, final String authType) throws CertificateException {
		// LE doesn't issue client certificates
		_systemTrustManager.checkClientTrusted(x509Certificates, authType);
	}

	@Override
	public void checkServerTrusted(final X509Certificate[] x509Certificates, final String authType) throws CertificateException {
		try {
			_systemTrustManager.checkServerTrusted(x509Certificates, authType);
		} catch(CertificateException ignored) {
			this._leTrustManager.checkServerTrusted(x509Certificates, authType);
		}
	}

	@Override
	public X509Certificate[] getAcceptedIssuers() {
		X509Certificate[] systemAccepted = this._systemTrustManager.getAcceptedIssuers();
		X509Certificate[] leAccepted = this._leTrustManager.getAcceptedIssuers();

		X509Certificate[] allAccepted = Arrays.copyOf(systemAccepted, systemAccepted.length + leAccepted.length);
		System.arraycopy(leAccepted, 0, allAccepted, systemAccepted.length, leAccepted.length);

		return allAccepted;
	}
}
