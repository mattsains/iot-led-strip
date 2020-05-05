const fs = require('fs');
const express = require('express');
const path = require('path');

const app = express();

let server;

if (process.env.https) {
    server = require('https').createServer({
        key: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/fullchain.pem"),
        cert: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/privkey.pem"),
        ca: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/fullchain.pem"),
        secureProtocol: "TLSv1_2_method",
    }, app);
} else {
    server = require('http').createServer(app);
}

const expressWs = require('express-ws')(app, server);

let w = r = g = b = 0;

let strip_data = [];

// test what binary looks like in the websocket stream.
for (let a = 0; a < 256; a++) {
    strip_data.push(a)
}

function send_data(ws) {
    console.log("sending data to websocket client.");
    ws.send(`${w},${r},${g},${b}`);
    // TODO: we might need to combine these into a single tramission,
    // or include a way to tell them apart.
    ws.send(strip_data, {
        binary: true,
        fin: true,
    });
}

app.ws('/ws', ws => {
    console.log("new connection");
    // It looks like if you send right away, the esp32 doesn't catch the transmission.
    // So by moving the send into the event loop, we force the websocket library to send it later.
    setTimeout(() => send_data(ws), 1);

    ws.on('message', function incoming(message) {
        console.log('received: %s', message);
    });
});

app.get('/s', (req, res) => {
    console.log('saving led data');
    w = req.query.w;
    r = req.query.r;
    g = req.query.g;
    b = req.query.b;

    expressWs.getWss().clients.forEach(client => {
        console.log(client.readyState);
        if (client.readyState == 1) {
            send_data(client);
        }
    });
    res.end();
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'ui.html'));
});

server.listen(process.env.https ? 443 : 8080);
