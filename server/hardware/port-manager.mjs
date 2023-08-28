/*
    In this script we're going to have all the logic related to the ports
 */

import {SerialPort} from "serialport";
import {DelimiterParser} from "@serialport/parser-delimiter";
import {resolve} from "chart.js/helpers";

// PORT AND PARSER
let portPath = '';
let port = null;
let dataParser = new DelimiterParser({delimiter: '\r\n'});

// PORT OPERATIONS
const PORT_OPERATIONS = {PAUSE: 'P', INIT: 'I'}

// FUNCTIONS

async function findArduino(){

    try{
        const ports = await SerialPort.list();
        portPath = ports[0].path || ''; // If the list is empty and there's no path at ports[0], we just assign an empty string
    }catch (e) {
        console.log(`An error happened: ${e}`);
    }

   // If we have a portPath, we have an Arduino connected
   return (portPath !== '');
}

async function executeOperation(operation) {
    if (port === null || !port.isOpen) {
        console.log(`[Error executing operation]: Port is not open`);
        return false;
    }

    if (!(operation in PORT_OPERATIONS)) {
        console.log(`Operation '${operation}' not recognized`);
    } else {
        return new Promise((resolve, reject) => {
            port.write(`${PORT_OPERATIONS[operation]}\n`);

            port.once('error', err => {
                console.log(`Error executing operation: ${err}`);
                resolve(false); // Resolve with false when an error occurs
            });

            resolve(true); // Resolve with true when the operation is executed
        });
    }
}

async function checkExperimentCode(experiment){

    if(!port)
        await openPort();

    return new Promise((resolve, reject) => {
        port.write(experiment);

        dataParser.once('data', status => resolve(status == '1') ); // 1 means the experiment code is correct

        port.on('error', err => reject(err.message));

    });

}

function openPort(){

    if(port) return true // If the port is already opened, we just return true

    return new Promise((resolve, reject) => {
        port = new SerialPort({path: portPath, baudRate: 9600});
        port.pipe(dataParser);

        port.once('open', () => resolve(true));

        port.on('error', err => reject(err.message));
    });
}

function getPort() {
    return port;
}


export {
    openPort,
    findArduino,
    checkExperimentCode,
    getPort,
    executeOperation,
    PORT_OPERATIONS
};