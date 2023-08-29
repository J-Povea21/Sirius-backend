import Fastify from "fastify";
import fastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import * as ports from "./hardware/port-manager.mjs";
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

    // We check if there's an Arduino connected
    const arduinoConnected = await ports.findArduino();

    if (!arduinoConnected) {
        console.log('No Arduino found');
        process.exit(1);
    }

    const correctCode =   await ports.checkExperimentCode('U');

    if (correctCode)
        console.log('Correct experiment code');
    else
        console.log('Incorrect experiment code');



}