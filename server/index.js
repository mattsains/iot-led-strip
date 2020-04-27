const express = require('express');
const path = require('path');

const app = express();

const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

let w, r, g, b;

wss.on('connection', ws => {
    console.log("new connection");
    ws.send(`${w},${r},${g},${b}`);
});

app.get('/s', (req, res) => {
    console.log('hello');
    w = req.query.w;
    r = req.query.r;
    g = req.query.g;
    b = req.query.b;

    wss.clients.forEach(client =>{
        if (client.readyState === WebSocket.OPEN) {
            console.log("sending");
            client.send(`${w},${r},${g},${b}`)
        }
    });
    res.end();
})

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'ui.html'));
})

app.listen(8081);


