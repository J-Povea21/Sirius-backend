import Fastify from "fastify";
import cors from "@fastify/cors";
import FastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import {setConnection} from "./socket/onEvents.mjs";
import path from "node:path";




const app = Fastify({
    logger: true
});

// Server static files' directory:
app.register(fastifyStatic, {
    root: path.resolve('server', 'static'),
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

// Server routes
app.register(routes);


// Here we start the server

app.listen({port: 3000}, (err, address) => {
    if (err) {
        app.log.error(`Error opening server: ${err}`);
        process.exit(1);
    }

    app.io.once('connection', socket => {
        setConnection(socket, app);
    });


});
