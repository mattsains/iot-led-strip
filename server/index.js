const https = require('https');
const fs = require('fs');
const express = require('express');
const path = require('path');

const app = express();

const server = https.createServer({
  key: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/privkey.pem"),
  cert: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/cert.pem"),
  ca: fs.readFileSync("/etc/letsencrypt/live/iot.sainsbury.io/fullchain.pem"),
  secureProtocol: "TLSv1_2_method"
}, app);

const expressWs = require('express-ws')(app, server);

let w, r, g, b;

app.get('/s', (req, res) => {
    console.log('hello');
    w = req.query.w;
    r = req.query.r;
    g = req.query.g;
    b = req.query.b;

    expressWs.getWss().clients.forEach(client => {
        console.log(client.readyState);
        if (client.readyState == 1) {
            console.log("sending");
            client.send(`${w},${r},${g},${b}`);
        }
    });
    res.end();
})

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'ui.html'));
})

app.ws('/ws', ws => {
  console.log("new connection");
  // It looks like if you send right away, the esp32 doesn't catch the transmission.
  // So by moving the send into the event loop, we force the websocket library to send it later.
  setTimeout(() => ws.send(`${w},${r},${g},${b}`), 1);

  ws.on('message', function incoming(message) {
      console.log('received: %s', message);
  });
});

server.listen(443);
