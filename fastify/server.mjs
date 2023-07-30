import Fastify from "fastify";
import fastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import path from "node:path";

const app = Fastify({
    logger: true
});


app.register(fastifyIO);

// Serve static files
app.register(fastifyStatic, {
    root: path.resolve('fastify', 'static'),
});

// Server routes. It's more organized to keep them in a separate file
app.register(routes);


// Here we start the server

app.listen({port: 3000}, (err, address) => {
    if (err) {
        app.log.error(`Error: ${err}`);
        process.exit(1);
    }
    app.log.info(`server listening on ${address}`);

    // Listening to the event 'arduino' from the client
    app.io.on('connection', (socket) => {

        console.log('A user connected');

        socket.on('opened', (data) => {
            console.log(`The opened state is: ${data}`);
        });

    });

});