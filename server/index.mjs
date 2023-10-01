import Fastify from "fastify";
import FastifyIO from "fastify-socket.io";
import cors from "@fastify/cors";
import {setConnection} from "./socket/onEvents.mjs";





const app = Fastify({
    logger: true
});

// Server cors
app.register(cors, {
   origin: '*',
    methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH'],
});

// Server socket.io. It's important to note that the path must be the same as the one in the client side in the script src
app.register(FastifyIO, {
    serveClient: true,
    path: '/socket.io/',
    cors: {
        origin: 'http://localhost:5173',
        methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH'],
    }
});

// Here we start the server

app.listen({port: 3000}, (err, address) => {
    if (err) {
        app.log.error(`Error opening server: ${err}`);
        process.exit(1);
    }

    app.io.once('connection', socket => setConnection(socket, app));

});
