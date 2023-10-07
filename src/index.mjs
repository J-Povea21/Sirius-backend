import Fastify from "fastify";
import cors from "@fastify/cors";
import FastifyIO from "fastify-socket.io";
import {setConnection} from "./api/onEvents.mjs";

const app = Fastify({
    logger: true
});

// Server cors
app.register(cors, {
   origin: 'http://localhost:5173',
});

// Server api.io. It's important to note that the path must be the same as the one in
// the client side

app.register(FastifyIO, {
    serveClient: true,
    path: '/api/',
    cors: {
        origin: 'http://localhost:5173',
    },

    // We don't really know which transport is going to work, so we
    // let socket.io use both websocket and polling protocol
    transports: ['websocket', 'polling']
});

// Here we start the server

app.listen({port: 3000}, (err, address) => {
    if (err) {
        app.log.error(`Error opening server: ${err}`);
        process.exit(1);
    }

    app.io.on('connection', socket => {
        console.log('Socket connected');
        setConnection(socket, app)
    });


});
