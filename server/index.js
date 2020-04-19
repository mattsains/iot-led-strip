const express = require('express');
const path = require('path');

const app = express();

let w, r, g, b;

app.get('/t', (req, res) => {
    res.send(`${w},${r},${g},${b}`);
});

app.get('/s', (req, res) => {
    w = req.query.w;
    r = req.query.r;
    g = req.query.g;
    b = req.query.b;
    res.end()
})

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'ui.html'));
})

app.listen(8080);