/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
