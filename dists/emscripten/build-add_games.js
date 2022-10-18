
const http = require('http');
const fs = require('fs');
const puppeteer = require('puppeteer');
const static = require('node-static');

var file = new (static.Server)("./");
const server = http.createServer(function (req, res) {
    file.serve(req, res);
}).listen(8080, async () => {
    const browser = await puppeteer.launch({ headless: true });
    const page = await browser.newPage();

    await page.goto('http://localhost:8080/scummvm.html#--add --path=/games --recursive');

    await page.screenshot({ path: 'example.png' });
    const regex = /Added ([0-9]+) games/;
    page.on('console', async msg => {
        const text = msg.text()
        console.log(text)
        const match = text.match(regex);
        if (match != null && match.length > 0) {
            console.log("Detection finished, exporting ini file for " + match[1] + " detected games.")
            const localStorage = await page.evaluate(() => Object.assign({}, window.localStorage));

            const ini_inode_id = "1b4a97d1-4ce0-417f-985c-e0f22ca21aef"  // defined in custom_shell.html
            const ini_lines = Buffer.from(localStorage[ini_inode_id], 'base64').toString().split('\n');
            // GRIM games check data consistency by reading all files. That's an expensive operation over
            // the network. Since we anyway should have known good data at build time, this script disables
            // that check.
            for (var i = 0; i < ini_lines.length; i++) {
                if (ini_lines[i] == "engineid=grim") {
                    ini_lines[i] = "check_gamedata=false\n" + ini_lines[i]
                }
            }
            fs.writeFileSync("scummvm.ini", ini_lines.join('\n'));
            browser.close();
            server.close();

            console.log('Done');
        }
    });


});