import express from 'express';
import {createServer} from "http";
import {Server} from "socket.io";
import path from "node:path";


const app = express();
const httpServer = createServer(app);
const io = new Server(httpServer);

/*
 With this statement we're setting where the static files are located.
 Now it's not necessary to specify the path to the static files
 */
app.use(express.static('express/static'));

// We set the port to listen to
app.set('port', 3000);

// We register socket.io
app.use('/socket.io', express.static('../node_modules/socket.io/client-dist'));


app.get('/', (req, res) => {
    res.sendFile(path.resolve('index.html'));
});

io.on('connection', (socket) => {
    console.log('A user connected');
});

httpServer.listen(3000, () => {
    console.log('Listening on port 3000');
});