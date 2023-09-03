import Fastify from "fastify";
import FastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import * as PortManager from "./hardware/port-manager.mjs";
import path from "node:path";
import {emitData} from "./hardware/port-manager.mjs";




const app = Fastify({
    logger: true
});

// Server static files' directory:
app.register(fastifyStatic, {
    root: path.resolve('server', 'static'),
});

// Server socket.io. It's important to note that the path must be the same as the one in the client side in the script src
app.register(FastifyIO, {
    serveClient: true,
    path: '/socket.io/',
});

// Server routes
app.register(routes);


// Here we start the server

app.listen({port: 3000}, (err, address) => {
    if (err) {
        app.log.error(`Error opening server: ${err}`);
        process.exit(1);
    }
    main();

});

// MAIN function
async function main() {

    app.io.on('connection', socket => {

        socket.on('findArduino', async () => {

            const response = await PortManager.findArduino();

            socket.emit('findArduino', response);

        });

        socket.on('checkExperiment', async experiment => {

                const response = await PortManager.checkExperimentCode(experiment);

                socket.emit('checkExperiment', response);
        });

        socket.on('startExperiment', async experimentRunning => {
            const operation = (experimentRunning) ? PortManager.PORT_OPERATIONS.INIT : PortManager.PORT_OPERATIONS.PAUSE;
            const response = await PortManager.executeOperation(operation);

            operation === 'I' && emitData(socket, 'data');

        });


    });

}