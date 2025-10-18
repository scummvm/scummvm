package org.scummvm.scummvm.net;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class HTTPManager {
	protected ExecutorService _executor;
	protected ArrayBlockingQueue<Runnable> _queue;
	protected boolean _empty;

	/** @noinspection unused
	 * Called from JNI (main ScummVM thread)
	 */
	public HTTPManager() {
		TLSSocketFactory.init();

		_executor = Executors.newCachedThreadPool();
		// Use a capacity to make sure the queue is checked on a regular basis
		_queue = new ArrayBlockingQueue<>(50);
		_empty = true;
	}

	/** @noinspection unused
	 * Called from JNI (main ScummVM thread)
	 */
	public void startRequest(HTTPRequest request) {
		request._manager = this;
		_executor.execute(request);
	}

	/** @noinspection unused
	 * Called from JNI (main ScummVM thread)
	 */
	public void poll() {
		Runnable r;
		while((r = _queue.poll()) != null) {
			r.run();
		}
		// The read is never synchronized but at least we ensure here that we don't miss any event
		synchronized(this) {
			_empty = _queue.isEmpty();
		}
	}

	// Called from workers
	void enqueue(Runnable r) {
		while(true) {
			try {
				_queue.put(r);
				synchronized(this) {
					_empty = false;
				}
				return;
			} catch (InterruptedException ignored) {
			}
		}
	}
}
