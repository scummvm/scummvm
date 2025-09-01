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

mergeInto(LibraryManager.library, {

    NetworkReadStreamEmscripten_init: function () {
        // Global storage for all fetch operations
        if (!window.scummvmFetches) {
            window.scummvmFetches = {};
            window.scummvmNextFetchId = 1;
        }

    },

    NetworkReadStreamEmscripten_fetch: function (methodPtr, urlPtr, headersPtr, requestDataPtr, requestDataSize, formFieldsPtr, formFilesPtr) {
        const method = UTF8ToString(methodPtr);
        const url = UTF8ToString(urlPtr);
        const fetchId = window.scummvmNextFetchId++;
        console.debug("Starting fetch #" + fetchId + " for URL: " + url + " with method: " + method);

        // Initialize fetch object
        const fetch = {
            id: fetchId,
            status: 0,
            statusText: 0,
            url: urlPtr,
            numBytes: 0,
            totalBytes: 0,
            responseHeaders: 0,
            active: true,
            completed: false,
            success: false,
            reader: null,
            buffer: null,
            bufferSize: 0
        };

        // Store in global registry
        window.scummvmFetches[fetchId] = fetch;

        // Create headers object from the headers array
        let headers = {};
        if (headersPtr) {
            const headersObj = new Headers();
            let i = 0;
            while (HEAP32[(headersPtr >> 2) + (i * 2)]) {
                const keyPtr = HEAP32[(headersPtr >> 2) + (i * 2)];
                const valuePtr = HEAP32[(headersPtr >> 2) + (i * 2 + 1)];
                const key = UTF8ToString(keyPtr);
                const value = UTF8ToString(valuePtr);
                // Don't set Content-Type for FormData - browser will set it with boundary
                if (!formFieldsPtr && !formFilesPtr || key.toLowerCase() !== 'content-type') {
                    console.debug(`Adding header: ${key} = ${value}`);
                    headersObj.append(key, value);
                }
                i++;
            }
            headers = headersObj;
        }

        const options = {
            method: method,
            headers: headers
        };

        // Add body if needed
        if (requestDataPtr && requestDataSize > 0) {
            const bodyData = new Uint8Array(HEAPU8.buffer, requestDataPtr, requestDataSize);
            options.body = bodyData;
            console.debug("Added request body, size:", requestDataSize);
        }

        // Handle FormData request
        if (formFieldsPtr || formFilesPtr) {
            const formData = new FormData();

            // Add form fields (array contains alternating name/value pairs)
            if (formFieldsPtr) {
                let i = 0;
                while (HEAP32[(formFieldsPtr >> 2) + i]) {
                    const namePtr = HEAP32[(formFieldsPtr >> 2) + i];
                    const valuePtr = HEAP32[(formFieldsPtr >> 2) + i + 1];
                    if (namePtr && valuePtr) {
                        const name = UTF8ToString(namePtr);
                        const value = UTF8ToString(valuePtr);
                        formData.append(name, value);
                        console.debug(`FormData field: ${name} = ${value}`);
                    }
                    i += 2;  // Skip both name and value
                }
            }

            // Add file fields (array contains alternating name/path pairs)
            if (formFilesPtr) {
                let i = 0;
                while (HEAP32[(formFilesPtr >> 2) + i]) {
                    const namePtr = HEAP32[(formFilesPtr >> 2) + i];
                    const pathPtr = HEAP32[(formFilesPtr >> 2) + i + 1];
                    if (namePtr && pathPtr) {
                        const name = UTF8ToString(namePtr);
                        const path = UTF8ToString(pathPtr);
                        console.debug(`FormData file field: ${name} from path ${path}`);

                        // Read file from Emscripten filesystem
                        try {
                            const fileData = FS.readFile(path);
                            const fileName = path.split('/').pop();
                            const blob = new Blob([fileData], { type: 'application/octet-stream' });
                            formData.append(name, blob, fileName);
                            console.debug(`Added file ${fileName} (${fileData.length} bytes)`);
                        } catch (e) {
                            console.error(`Failed to read file ${path}:`, e);
                        }
                    }
                    i += 2;  // Skip both name and path
                }
            }

            options.body = formData;
        }

        // Start the fetch
        window.fetch(url, options)
            .then(response => {
                fetch.status = response.status;
                fetch.statusText = response.statusText;

                // Store headers array for efficient access
                let responseHeadersArray = 0;
                if (response.headers) {
                    const headerPairs = [];
                    response.headers.forEach((value, key) => {
                        headerPairs.push(key, value);
                    });

                    // Allocate memory for the array of string pointers (pairs + null terminator)
                    const arraySize = (headerPairs.length + 1) * 4; // 4 bytes per pointer
                    const arrayPtr = _malloc(arraySize);

                    // Fill the array with pointers to the header strings
                    for (let i = 0; i < headerPairs.length; i++) {
                        const str = headerPairs[i];
                        const strLen = lengthBytesUTF8(str) + 1;
                        const strPtr = _malloc(strLen);
                        stringToUTF8(str, strPtr, strLen);
                        HEAP32[(arrayPtr >> 2) + i] = strPtr;
                    }

                    // Null terminate the array
                    HEAP32[(arrayPtr >> 2) + headerPairs.length] = 0;

                    responseHeadersArray = arrayPtr;
                }
                fetch.responseHeadersArray = responseHeadersArray;

                // Get total size if available
                const contentLength = response.headers.get('Content-Length');
                if (contentLength) {
                    fetch.totalBytes = parseInt(contentLength, 10);
                    fetch.bufferSize = fetch.totalBytes * 1.2; // Allocate 20% extra space for gzipped content
                } else {
                    fetch.bufferSize = 1024 * 1024; // 1MB
                }
                // Update fetch object
                fetch.buffer = _malloc(fetch.bufferSize);

                // Check for error status codes (4xx, 5xx)
                if (!response.ok) {
                    console.debug(`Fetch #${fetchId} received error status: ${response.status} ${response.statusText}`);
                    // For error responses, we still want to read the body for potential error details
                    // but mark the request as not successful
                    fetch.success = false;
                } else {
                    fetch.success = true;
                }
                console.debug("Fetch #" + fetchId + " headers received, status: " + response.status);

                // Start reading the data
                const reader = response.body.getReader();
                fetch.reader = reader; // Store reader for possible cancellation

                // Function to read all chunks (async)
                const readAllChunks = async () => {
                    try {
                        while (true) {
                            const { done, value } = await reader.read();
                            if (done) {
                                // Finished reading
                                fetch.completed = true;
                                fetch.active = false;
                                const date = new Date();
                                console.debug(`Fetch #${fetchId} complete: Read ${fetch.numBytes} bytes total at ${date.getMinutes()}:${date.getSeconds()}`);
                                break;
                            }

                            // Ensure we have enough buffer space
                            if (fetch.numBytes + value.length > fetch.bufferSize) { // totalBytes is based on content-length header which is off for gzipped content
                                console.debug(`Fetch #${fetchId} expanding buffer from ${fetch.bufferSize} to ${(fetch.numBytes + value.length) * 1.2}`);
                                fetch.bufferSize = (fetch.numBytes + value.length) * 1.2;
                                const newBuffer = _malloc(fetch.bufferSize);
                                if (fetch.numBytes > 0) {
                                    // Copy existing data if needed
                                    HEAPU8.set(HEAPU8.subarray(fetch.buffer, fetch.buffer + fetch.numBytes), newBuffer);
                                }
                                _free(fetch.buffer);
                                fetch.buffer = newBuffer;
                            }
                            if (fetch.totalBytes && fetch.numBytes + value.length > fetch.totalBytes) {
                                fetch.totalBytes = fetch.numBytes + value.length; // Adjust totalBytes if we underestimated due to gzip
                            }

                            // Copy data to the buffer
                            HEAPU8.set(value, fetch.buffer + fetch.numBytes);
                            fetch.numBytes += value.length;
                        }
                    } catch (error) {
                        console.error(`Error reading from fetch #${fetchId}:`, error);
                        fetch.completed = true;
                        fetch.active = false;
                        fetch.success = false;
                    }
                };

                // Start reading all chunks
                readAllChunks();
            })
            .catch(error => {
                console.error("Fetch #" + fetchId + " failed:", error);
                // this is a bit of a hack, but emscripten malloc returns a valid pointer for size 0
                fetch.buffer = _malloc(fetch.bufferSize);
                fetch.bufferSize = 0;
                fetch.completed = true;
                fetch.active = false;
                fetch.success = false;

                if (fetch.statusText) _free(fetch.statusText);
                const errorMsg = "Network error: " + error.message;
                const errorLen = lengthBytesUTF8(errorMsg) + 1;
                fetch.statusText = _malloc(errorLen);
                stringToUTF8(errorMsg, fetch.statusText, errorLen);
            });

        return fetchId;
    },

    NetworkReadStreamEmscripten_close: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return;

        console.debug("Closing fetch #" + fetchId);

        // Cancel reader if active
        if (fetch.reader) {
            try {
                fetch.reader.cancel();
            } catch (e) { }
        }

        // Free allocated memory
        if (fetch.statusText) _free(fetch.statusText);
        if (fetch.responseHeadersArray) {
            // Free the header strings in the array first
            let i = 0;
            while (HEAP32[(fetch.responseHeadersArray >> 2) + i] !== 0) {
                _free(HEAP32[(fetch.responseHeadersArray >> 2) + i]);
                i++;
            }
            // Free the array itself
            _free(fetch.responseHeadersArray);
        }
        if (fetch.buffer) _free(fetch.buffer);

        // Remove from registry
        delete window.scummvmFetches[fetchId];
    },

    // Getter functions properties

    NetworkReadStreamEmscripten_status: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return 0;
        return fetch.status;
    },



    NetworkReadStreamEmscripten_getNumBytes: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return 0;
        return fetch.numBytes;
    },

    NetworkReadStreamEmscripten_totalBytes: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return 0;
        return fetch.totalBytes;
    },

    NetworkReadStreamEmscripten_responseHeadersArray: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return 0;
        return fetch.responseHeadersArray;
    },

    NetworkReadStreamEmscripten_getDataPtr: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        //console.debug("Fetch #" + fetchId + " data pointer requested:", fetch ? fetch.buffer : "not found");
        if (!fetch) return 0;
        return fetch.buffer;
    },

    NetworkReadStreamEmscripten_completed: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return true;
        return fetch.completed;
    },

    NetworkReadStreamEmscripten_hasError: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return true;
        return !fetch.success;
    },

    NetworkReadStreamEmscripten_getErr: function (fetchId) {
        const fetch = window.scummvmFetches[fetchId];
        if (!fetch) return 0; // Return null pointer

        if (fetch.statusText) {
            // Return the statusText if available
            const ptr = _malloc(lengthBytesUTF8(fetch.statusText) + 1);
            stringToUTF8(fetch.statusText, ptr, lengthBytesUTF8(fetch.statusText) + 1);
            return ptr;
        }

        // Return a generic error if no specific message is available
        const errorMsg = "Unknown error";
        const ptr = _malloc(lengthBytesUTF8(errorMsg) + 1);
        stringToUTF8(errorMsg, ptr, lengthBytesUTF8(errorMsg) + 1);
        return ptr;
    }
});
