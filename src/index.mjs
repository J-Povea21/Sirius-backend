import Fastify from "fastify";
import cors from "@fastify/cors";
import FastifyIO from "fastify-socket.io";
import {setConnection} from "./api/on-events.mjs";

const {ENVIRONMENT = 'development',PORT = '3000', ADDRESS = 'localhost', ALLOWED_ORGIN} = process.env;

const app = Fastify({logger: ENVIRONMENT === 'development'});

// Server cors
app.register(cors, {
   origin: ALLOWED_ORGIN,
});

// Server api.io. It's important to note that the path must be the same as the one in
// the client side

app.register(FastifyIO, {
    serveClient: true,
    path: '/api/',
    cors: {
        origin: ALLOWED_ORGIN,
    },

    // We don't really know which transport is going to work, so we
    // let socket.io use both websocket and polling protocol
    transports: ['websocket', 'polling']
});

// Here we start the server

app.listen({host: ADDRESS ,port: parseInt(PORT, 10)}, (err, address) => {
    if (err) {
        app.log.error(`Error opening server: ${err}`);
        process.exit(1);
    }

    app.io.on('connection', socket => {
        app.log.info('Socket connected');
        setConnection(socket, app)
    });


});
