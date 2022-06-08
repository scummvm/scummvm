const request = require('request');
const fs = require('fs');
const fsprocess = require('process');
const { url } = require('inspector');

process.on('uncaughtException', (err, origin) => {
    console.error(err)
    console.error(origin)
    process.exitCode = 2
})
const args_games = process.argv.slice(2);

/*
 Copied from https://github.com/scummvm/scummvm-web/blob/master/include/DataUtils.php
 */
const SHEET_URL = 'https://docs.google.com/spreadsheets/d/e/2PACX-1vQamumX0p-DYQa5Umi3RxX-pHM6RZhAj1qvUP0jTmaqutN9FwzyriRSXlO9rq6kR60pGIuPvCDzZL3s/pub?output=tsv';
const SHEET_IDS = {
    'platforms': '1061029686',
    'compatibility': '854570757',
    'games': '1946612063',
    'engines': '0',
    'companies': '226191984',
    'versions': '1225902887',
    'game_demos': '713475305',
    'series': '1095671818',
    'screenshots': '1985243204',
    'scummvm_downloads': '1057392663',
    'game_downloads': '1287892109',
    'director_demos': '1256563740',
}

// Small Helper function as having followRedirect:true sometimes lead to ECONNRESET errors 
function getGoogleSheet(url, callback) {
    request({
        url: url,
        followRedirect: function (response) {
            return false
        }
    }, function (error, response, body) {
        if (response.headers && response.headers.location) {
            request({
                url: response.headers.location,
                followRedirect: false
            }, callback);
        } else {
            callback(error, response, body)
        }
    });
}
function parseTSV(text) {
    const lines = text.split("\r\n")
    const headers = lines[0].split("\t")
    var ret = []
    for (var i = 1; i < lines.length; i++) {
        ret[i - 1] = {}
        lines[i].split("\t").forEach((value, col) => ret[i - 1][headers[col]] = value)
    }
    return ret
}

var games = {}
// Get Freeware Games
function get_freeware_games() {
    console.error("download_games.js: Fetching list of freeware games")
    return new Promise((resolve, reject) => {
        var url = SHEET_URL + "&gid=" + SHEET_IDS['game_downloads'];
        getGoogleSheet(url, (error, response, body) => {
            if (error) {
                reject(error)
                return
            }
            parseTSV(body).forEach((downloads) => {
                if (downloads['category'] == "games" && !(downloads['game_id'] in games)) {
                    games[downloads['game_id']] = "/frs/extras/" + downloads['url']
                }
                filename = downloads['url'].substring(downloads['url'].lastIndexOf("/"))
                games[downloads['game_id'] + filename] = "/frs/extras/" + downloads['url']
            })
            resolve()
        })
    });
}
// Get Demos Games
function get_demos() {
    console.error("download_games.js: Fetching list of game demos")
    return new Promise((resolve, reject) => {
        var url = SHEET_URL + "&gid=" + SHEET_IDS['game_demos'];
        getGoogleSheet(url, (error, response, body) => {
            if (error) {
                reject(error)
                return
            }
            parseTSV(body).forEach((downloads) => {
                if (!(downloads['id'] in games)) {
                    games[downloads['id']] = downloads['url']
                }
                filename = downloads['url'].substring(downloads['url'].lastIndexOf("/"))
                games[downloads['id'] + filename] = downloads['url']
            })
            resolve()
        })
    });
}
// Get Director Demos 
function get_director_demos() {
    console.error("download_games.js: Fetching list of director demos")
    return new Promise((resolve, reject) => {
        var url = SHEET_URL + "&gid=" + SHEET_IDS['director_demos'];
        getGoogleSheet(url, (error, response, body) => {
            if (error || body == undefined) {
                reject(error)
                return
            }
            parseTSV(body).forEach((downloads) => {
                if (!(downloads['id'] in games)) {
                    games[downloads['id']] = downloads['url']
                }
                filename = downloads['url'].substring(downloads['url'].lastIndexOf("/"))
                games[downloads['id'] + filename] = downloads['url']
            });
            resolve()
        });
    });
}

// Download a file
var download_file = function (uri, filename) {
    // TODO: Rewrite as promise to serialize this (easier to return status updates)
    return request.get(uri).on('error', function (err) {
        throw err
    }).on('response', function (response) {
        if (response.statusCode == 200) {
            console.error("download_games.js: Downloading " + uri)
        } else {
            console.error(response)
            throw new Error(response.statusCode)
        }
    }).pipe(fs.createWriteStream(filename))

}

const download_all_games = async (gameIds) => {
    for (var gameId of gameIds) {
        if (gameId.startsWith("http")) {
            var  url = gameId
            var filename = url.substring(url.lastIndexOf("/") + 1)

            console.log(filename)
            if (!fs.existsSync(filename)) {
                await download_file(url, filename)
            }
        } else if (!(gameId in games)) {
            console.error("download_games.js: GameID " + gameId + " not known")
            process.exit(1)
        } else {
            var url = "https://downloads.scummvm.org" + games[gameId]
            if (gameId.includes("/")) {
                gameId = gameId.substring(0, gameId.lastIndexOf("/"))
            }
            var filename = url.substring(url.lastIndexOf("/") + 1)
            if (!filename.startsWith(gameId)) { filename = gameId + "-" + filename }
            console.log(filename)
            if (!fs.existsSync(filename)) {
                await download_file(url, filename)
            }
        }
    }
}

// start everything
get_freeware_games()
    .then(get_demos)
    .then(get_director_demos)
    .then(() => {
        download_all_games(args_games)

    });
