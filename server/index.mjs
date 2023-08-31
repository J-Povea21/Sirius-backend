import Fastify from "fastify";
import fastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import * as PortManager from "./hardware/port-manager.mjs";
import path from "node:path";




const app = Fastify({
    logger: true
});


app.register(fastifyIO);

/*
    Serve static files:

    app.register(fastifyStatic, {
    root: path.resolve('fastify', 'static'),
    });

 */


// Server routes. It's more organized to keep them in a separate file
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
    const arduinoConnected = await PortManager.findArduino();

    if (arduinoConnected) {
        app.log.info('Arduino connected');
        const portOpened = await PortManager.openPort();
        const correctCode =  await PortManager.checkExperimentCode('U');
        PortManager.executeOperation('I');

    }else{
        app.log.error('Arduino not connected');
    }

}