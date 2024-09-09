
/*
 * ScummvmFS - A custom Emscripten filesystem for ScummVM
 * 
 * This is the filesystem used to load any read-only files used by ScummVM: data, games and 
 * plugins. It supports range-requests and caches data in memory to minimize latency when loading
 * data from the network. 
 * 
 * Adapted from Emscripten's NodeFS and BrowserFS' EmscriptenFS + XHR backend:
 * https://github.com/emscripten-core/emscripten/blob/main/src/library_nodefs.js
 * https://github.com/jvilk/BrowserFS/blob/master/src/generic/emscripten_fs.ts
 * https://github.com/jvilk/BrowserFS/blob/master/src/generic/xhr.ts
 */
const DIR_MODE = 16895; // 040777
const FILE_MODE = 33206; // 100666
const SEEK_SET = 0;
const SEEK_CUR = 1;
const SEEK_END = 2;
const RANGE_REQUEST_BLOCK_SIZE = 1024 * 1024
const ERRNO_CODES = {
    // TODO: We should get these from Emscripten - see https://github.com/emscripten-core/emscripten/issues/10061 and https://github.com/emscripten-core/emscripten/issues/14783
    EPERM: 1, // Operation not permitted
    ENOENT: 2, // No such file or directory
    EINVAL: 22 // I©nvalid argument
};


const DEBUG = false


export class ScummvmFS {
    url;
    fs_index;
    stream_ops;
    node_ops;
    FS;
    constructor(_FS, _url) {
        this.FS = _FS;
        this.url = _url
        var req = new XMLHttpRequest(); // a new request
        req.open("GET", _url + "/index.json", false);
        req.send(null);
        var json_index = JSON.parse(req.responseText)
        this.fs_index = {}
        var walk_index = function (path, dir) {
            logger(path, "walk_index")
            this.fs_index[path] = null
            if (path != "/") {
                path = path + "/"
            }
            for (var key in dir) {
                if (typeof dir[key] === 'object') {
                    walk_index(path + key, dir[key]) // toLowerCase to simulate a case-insensitive filesystem
                } else {
                    if (key !== "index.json") {
                        this.fs_index[path + key] = dir[key] // toLowerCase to simulate a case-insensitive filesystem
                    }
                }

            }
        }.bind(this)

        walk_index("/", json_index)
    }

    listDirectory(_path) {
        const path = _path.path
        var result = []
        for (var node in this.fs_index) {
            if (node.startsWith(path) && node.lastIndexOf("/") <= path.length && node !== path && node.substr(path.length + 1).length > 0 && node.charAt(path.length) == "/") {
                result.push(node.substr(path.length + 1))
            }
        }
        return { ok: true, data: result };
    }

    // used for open
    get(_path) {
        const path = _path.path
        logger(path, "get")
        if (path in this.fs_index) {
            // if  this.fs_index[path] is still a integer (hence a file), we now initialize the array to store any file data
            if (Number.isInteger(this.fs_index[path])) { // if not a number we either already have iniitalized the data or it's a folder
                const size = this.fs_index[path];
                var data;
                data = new Array(Math.ceil(size / RANGE_REQUEST_BLOCK_SIZE)) // data will be an array of blocks

                this.fs_index[path] = { size: this.fs_index[path], data: data }
                return { ok: true, data: data, size: size };
            } else if (typeof this.fs_index[path] == "object" && this.fs_index[path] !== null) {
                return { ok: true, data: this.fs_index[path].data, size: this.fs_index[path].size }; // already initialized
            } else {
                return { ok: true, data: null }; // directory
            }
        } else {
            return { ok: false }
        }
    }

    // used for close, mknod
    put(args) {
        const path = args.path
        logger(path, "put")
        if (!this.fs_index[path]) {
            throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
        }

        return { ok: true, data: this.fs_index[path].data };
    }

    read(args) {
        const path = args.path;
        logger(path, "read, args:" + JSON.stringify(args))

        if (typeof this.fs_index[path] !== "object") {
            console.error("File hasn't been opened yet")
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
        }
        const start = args.start;
        const end = (args.end > (this.fs_index[path].size)) ? (this.fs_index[path].size) : args.end // sometimes we get requests beyond the end of the file (????)
        var first_block = Math.floor(start / RANGE_REQUEST_BLOCK_SIZE)
        var last_block = Math.floor(end / RANGE_REQUEST_BLOCK_SIZE)
        if (start > end) {
            return { ok: true, data: [] };
        }
        var alreadyLoaded = false;
        if (Array.isArray(this.fs_index[path].data)) {
            alreadyLoaded = true
            for (var idx = first_block; idx <= last_block; idx++) {
                if (this.fs_index[path].data[idx] == undefined) {
                    logger(path, "block " + idx + " missing")
                    alreadyLoaded = false
                    break;
                }
            }
        }

        let data = null;
        logger(path, "file alreadyLoaded=" + alreadyLoaded)
        if (alreadyLoaded) {
            data = new Uint8Array(end - start + 1);
            for (var block = first_block; block <= last_block; block++) {
                // TODO: we should start at start at the request start and not block start (same for end)
                for (var idx = Math.max(start, block * RANGE_REQUEST_BLOCK_SIZE); idx <= Math.min(end, block * RANGE_REQUEST_BLOCK_SIZE + this.fs_index[path].data[block].length - 1); idx++) {
                    if (idx >= start && idx <= end) {
                        data[idx - start] = this.fs_index[path].data[block][idx - block * RANGE_REQUEST_BLOCK_SIZE]
                    }
                }
            }
            logger(path, "cache loaded ")
        } else {

            data = this.download(path, this.url, first_block, last_block, start, end);
        }
        return { ok: true, data: data };
    }
    download(path, _url, first_block, last_block, start, end) {
        self = this;
        let data = null;
        const req = new XMLHttpRequest();
        const url = _url + path;
        req.open('GET', url, false);

        let err = null;
        // On most platforms, we cannot set the responseType of synchronous downloads.
        // Classic hack to download binary data as a string.
        req.overrideMimeType('text/plain; charset=x-user-defined');

        // Trying to use range requests where possible
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Range
        var range_start = first_block * RANGE_REQUEST_BLOCK_SIZE
        var range_end = Math.min((last_block + 1) * RANGE_REQUEST_BLOCK_SIZE, this.fs_index[path].size) - 1
        if (this.fs_index[path].size > (range_start - range_end + 1)) {
            req.setRequestHeader('Range', 'bytes=' + range_start + '-' + range_end);
        }

        if (this.fs_index[path].data === null) {
            this.fs_index[path].data = new Array(Math.ceil(this.fs_index[path].size / RANGE_REQUEST_BLOCK_SIZE));
        }
        req.onreadystatechange = function (e) {
            if (req.readyState === 4) {
                var text = req.responseText;
                logger(path, "Downloaded " + text.length + " bytes");
                data = new Uint8Array(end - start + 1);
                if (req.status === 200) { // range request wasn't respected or requested
                    for (let i = 0; i < text.length; i++) {
                        if (i >= start && i <= end) {
                            // This will automatically throw away the upper bit of each
                            // character for us.
                            data[i - start] = text.charCodeAt(i);
                        }
                        var block = Math.floor(i / RANGE_REQUEST_BLOCK_SIZE)
                        if (self.fs_index[path].data[block] === undefined) {
                            self.fs_index[path].data[block] = new Uint8Array(RANGE_REQUEST_BLOCK_SIZE)
                        }
                        self.fs_index[path].data[block][i - block * RANGE_REQUEST_BLOCK_SIZE] = text.charCodeAt(i)
                    }
                    logger(path, "Downloaded [full download]");
                } else if (req.status === 206) { // partial response to range request
                    var start_offset = start - range_start
                    var end_offset = range_end - end

                    logger(path, "First block: " + first_block + " last block: " + last_block + "Text length: " + text.length)
                    var char_length = Math.round((range_end - range_start + 1) / text.length);
                    if (char_length == 2 && text.length == (range_end - range_start + 1) / 2 - 1) {
                        // The above hack to get binary data as text breaks if the first two bytes of the range are U+FEFF which is a BOM 
                        // for UTF16 and the browsers convert the data into a UTF16 string. I initially tied to fix this by breaking up 
                        // the UTF16 characters into 2 bytes and prepend the stripped BOM again,  but it turned out that there were other 
                        // issues how browsers handle UTF16 (e.g. 0xDFC3, 0xDFAD, 0xDFFB, 0xDF5B all somehow getting converted to 0xFFFD 
                        // - i.e. "REPLACEMENT CHARACTER") so this now just reruns shifts the start of the download. 
                        // That's wasting some data, but it's a rare enough occurrence
                        //
                        // TODO: The only proper fix for this is to implement a asynchronous filesystem for emscripten. Something which currently
                        // isn't possible
                        data = self.download(path, _url, first_block - 1, last_block, start, end)
                    } else if (char_length == 1) {

                        for (let i = 0; i < (range_end - range_start + 1); i++) {
                            var block = Math.floor((range_start + i) / RANGE_REQUEST_BLOCK_SIZE)
                            if (self.fs_index[path].data[block] === undefined) {
                                self.fs_index[path].data[block] = new Uint8Array(RANGE_REQUEST_BLOCK_SIZE)
                            }
                            var block_pos = (range_start + i) - (block * RANGE_REQUEST_BLOCK_SIZE)
                            // This will automatically throw away the upper bit of each
                            // character for us.
                            self.fs_index[path].data[block][block_pos] = text.charCodeAt(i)



                        }
                        logger(path, "First block length: " + self.fs_index[path].data[block] + " last block length: " + self.fs_index[path].data[block] + "Text length: " + text.length)
                        for (var block = first_block; block <= last_block; block++) {
                            // TODO: we should start at start at the request start and not block start (same for end)
                            for (var idx = Math.max(start, block * RANGE_REQUEST_BLOCK_SIZE); idx <= Math.min(end, block * RANGE_REQUEST_BLOCK_SIZE + self.fs_index[path].data[block].length - 1); idx++) {
                                if (idx >= start && idx <= end) {
                                    data[idx - start] = self.fs_index[path].data[block][idx - block * RANGE_REQUEST_BLOCK_SIZE]
                                }
                            }
                        }
                        logger(path, "Downloaded [range request]");
                    }
                } else {
                    console.error(req);
                    throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
                }
            }
        };
        req.send();
        if (err) {
            throw err;
        }
        return data;
    }

    mount(mount) {
        return this.createNode(null, "/", DIR_MODE, 0);
    }

    createNode(parent, name, mode, size) {
        logger(name, "createNode")
        if (!this.FS.isDir(mode) && !this.FS.isFile(mode)) {
            throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        let node = this.FS.createNode(parent, name, mode);
        node.node_ops = this.node_ops;
        node.stream_ops = this.stream_ops;
        node.size = size
        return node;
    }

    convertResult(result) {
        if (result.ok) {
            return result.data;
        }
        else {
            let error;
            if (result.status === 404) {
                error = new FS.ErrnoError(ERRNO_CODES.ENOENT);
            }
            else {
                error = new FS.ErrnoError(ERRNO_CODES.EPERM);
            }
            error.cause = result.error;
            throw error;
        }
    }

    node_ops = {
        getattr: (node) => {
            return {
                dev: 1,
                ino: node.id,
                mode: node.mode,
                nlink: 1,
                uid: 0,
                gid: 0,
                rdev: 0,
                size: node.size,
                atime: new Date(),
                mtime: new Date(),
                ctime: new Date(),
                blksize: 4096,
                blocks: 0,
            };
        },
        setattr: (node, attr) => {
            // Doesn't really do anything
            if (attr.mode !== undefined) {
                node.mode = attr.mode;
            }
            if (attr.timestamp !== undefined) {
                node.timestamp = attr.timestamp;
            }
        },
        lookup: (parent, name) => {
            logger(name, "lookup ")
            if (parent instanceof FS.FSStream) { //sometimes we get a stream instead of a node
                parent = parent.node;
            }
            const path = realPath(parent, name);
            const result = this.get({ path });
            if (!result.ok) {
                // I wish Javascript had inner exceptions
                throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
            }
            return this.createNode(parent, name, result.data === null ? DIR_MODE : FILE_MODE, result.data ? result.size : null);
        },
        mknod: (parent, name, mode, dev) => {
            logger(name, "mknod ")
            const node = this.createNode(parent, name, mode, 0);
            const path = realPath(node);
            if (this.FS.isDir(node.mode)) {
                this.convertResult(this.put({ path, value: null }));
            }
            else {
                this.convertResult(this.put({ path, value: "" }));
            }
            return node;
        },

        rename: (oldNode, newDir, newName) => {
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
            const oldPath = realPath(oldNode);
            const newPath = realPath(newDir, newName);
            this.convertResult(this.move({ path: oldPath, newPath: newPath }));
            oldNode.name = newName;
        },

        unlink: (parent, name) => {
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
            const path = realPath(parent, name);
            this.convertResult(this.delete({ path }));
        },

        rmdir: (parent, name) => {
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
            const path = realPath(parent, name);
            this.convertResult(this.delete({ path }));
        },

        readdir: (node) => {
            const path = realPath(node);
            let result = this.convertResult(this.listDirectory({ path }));
            if (!result.includes(".")) {
                result.push(".");
            }
            if (!result.includes("..")) {
                result.push("..");
            }
            return result;
        },

        symlink: (parent, newName, oldPath) => {
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
        },

        readlink: (node) => {
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
        }
    }

    stream_ops = {
        open: (stream) => {
            logger(stream.path, "Open stream ")
            const path = realPath(stream.node);
            if (FS.isFile(stream.node.mode)) {
                const result = this.get({ path });
                if (result.data === null || result.data === undefined) {
                    return;
                }
                stream.fileData = result.data;
                stream.fileSize = result.size;
            }
        },

        close: (stream) => {
            logger(stream.path, "close stream ")
            const path = realPath(stream.node);
            if (FS.isFile(stream.node.mode) && stream.fileData) {
                const fileData = stream.fileData
                // TODO: Track open/closed files differently so we can warn but don't lose the cached data
                //stream.fileData = undefined;
                this.convertResult(this.put({ path, value: fileData }));
            }
        },

        read: (stream, buffer, offset, length, position) => {
            if (!position) {
                position = stream.position
            }
            // logger(stream.path, "read stream - offset:" + offset + " length:" + length + " position:" + position)
            const path = realPath(stream.node);
            var _a, _b;
            if (length <= 0)
                return 0;

            var size = length
            if (typeof stream.fileData === 'object' && stream.fileSize < position + length) {
                size = stream.fileSize - position
            }

            // logger(stream.path, "Length, Position " + length + "," + position)
            // logger(stream.path, "Size " + size)
            // logger(stream.path, "stream.fileSize " + stream.fileSize)
            if (size < 0) {
                throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
            }
            if (size > 0) {
                var fileData = this.convertResult(this.read({ path: path, start: position, end: position + size - 1 }));
                logger(stream.path, "fileData (start: " + (position) + " end: " + (position + size - 1).toString() + " (length: " + fileData.length + ")")

                if (DEBUG) {
                    logger(stream.path, Uint8Array2hex(fileData))
                }
                buffer.set(fileData, offset);
            }
            //    buffer.set(stream.fileData.subarray(position, position + size), offset);

            return size;
        },

        write: (stream, buffer, offset, length, position) => {
            // this FS actually can't write
            throw new FS.ErrnoError(ERRNO_CODES.EPERM);
        },

        llseek: (stream, offset, whence) => {
            let position = offset; // SEEK_SET
            if (whence === SEEK_CUR) {
                position += stream.position;
            }
            else if (whence === SEEK_END) {
                if (this.FS.isFile(stream.node.mode)) {
                    position += stream.fileSize;
                }
            } else if (whence !== SEEK_SET) {
                console.error("Illegal Whence: " + whence)
                throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
            }
            if (position < 0) {
                console.error("CRITICAL: Position < 0")
                throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
            }
            stream.position = position
            return position;
        }
    }


}

function realPath(node, fileName) {
    const parts = [];
    while (node.parent !== node) {
        parts.push(node.name);
        node = node.parent;
    }
    parts.push(node.mount.opts.root);
    parts.reverse();
    if (fileName !== undefined && fileName !== null) {
        parts.push(fileName);
    }
    return parts.join("/");
}


function logger(path, message) {
    if (DEBUG) {
        console.log(path + ": " + message)
    }
}
function Uint8Array2hex(byteArray) {
    return Array.prototype.map.call(byteArray, function (byte) {
        return ('0' + (byte & 0xFF).toString(16)).slice(-2).toUpperCase();
    }).join(' ');
}
