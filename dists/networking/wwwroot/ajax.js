// the following is snippet from http://stackoverflow.com/a/18078705
// I changed a few things though

var ajax = {};
ajax.x = function () { return new XMLHttpRequest(); }; // "no one uses IE6"

ajax.send = function (url, callback, errorCallback, method, data, async) {
    if (async === undefined) async = true;

    var x = ajax.x();
    x.open(method, url, async);
    x.onreadystatechange = function () {
        if (x.readyState == XMLHttpRequest.DONE) {
            if (x.status == 200)
                callback(x.responseText);
            else
                errorCallback(x);
        }
    };
    if (method == 'POST') {
        x.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    }
    x.send(data)
};

ajax.get = function (url, data, callback, errorCallback, async) {
    var query = [];
    for (var key in data) {
        query.push(encodeURIComponent(key) + '=' + encodeURIComponent(data[key]));
    }
    ajax.send(url + (query.length ? '?' + query.join('&') : ''), callback, errorCallback, 'GET', null, async)
};

ajax.post = function (url, data, callback, errorCallback, async) {
    var query = [];
    for (var key in data) {
        query.push(encodeURIComponent(key) + '=' + encodeURIComponent(data[key]));
    }
    ajax.send(url, callback, errorCallback, 'POST', query.join('&'), async)
};

ajax.getAndParseJson = function (url, data, callback) {
    ajax.get(
        url, data,
        function (responseText) { callback(JSON.parse(responseText)); },
        function (x) { console.log("error: " + x.status); }
    );
};