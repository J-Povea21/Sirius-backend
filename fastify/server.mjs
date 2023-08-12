import Fastify from "fastify";
import fastifyIO from "fastify-socket.io";
import fastifyStatic from "@fastify/static"
import routes from "./routes.mjs";
import path from "node:path";
import {port2, parser2} from "./tester.mjs";
import { Socket } from "socket.io";
import { SerialPort } from 'serialport';

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

        //->Envia dato
        socket.on('check', (arduinoType) =>{
            //¿Hay sensores conectados?
            async function findArduinos(){
                let serialList = await SerialPort.list().then(data => data)
                return serialList
            }
            if(findArduinos().length==0){
               console.log("HEY BROU NO HAY UNA VERGAAAA")
            } else {
                port2.write("SENSORTYPE\n");
                parser2.on("data", status => {
                    if(status == "1"){
                        console.log("SI EXISTOOO");
                    }else{
                        console.log("TAS MAL")
                    }
                });
            }

        });

        socket.on('opened', (state) => {
            if (state === true) {

                port2.write('I\n');
                parser2.on('data', data => {
                    const parsedData = JSON.parse(data);
                    console.log(parsedData);
                    socket.emit("arduino", parsedData.ULTRASONIC.time);
                });

            }else{
                port2.write('P\n');
            }

        });

    });

});