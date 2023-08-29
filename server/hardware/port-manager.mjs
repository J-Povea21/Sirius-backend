/*
    In this script we're going to have all the logic related to the ports
 */

import {SerialPort} from "serialport";
import {DelimiterParser} from "@serialport/parser-delimiter";

// PORT AND PARSER
let portPath = '';
let port = null;
let dataParser = new DelimiterParser({delimiter: '\r\n'});

// PORT OPERATIONS
const PORT_OPERATIONS = {PAUSE: 'P', INIT: 'I'}

// FUNCTIONS

function findArduino() {
    return new Promise((resolve, reject) => {

        SerialPort.list()
            .then(ports => {

            const arduino = ports.find(port => port.manufacturer.includes('Arduino'));

            if(arduino){
                portPath = arduino.path;
                openPort();
                resolve(true);
            }else{
                resolve(false);
            }

            })
            .catch(err => reject(err));

    });
}

async function executeOperation(operation, callback) {

    if(operation !== 'P' && operation !== 'I') return callback(false, 'Operation not recognized');

    try{
        port.write(`${operation}\n`);
        callback(true);
    }catch (e){
        callback(false, e);
    }

}

function checkExperimentCode(experiment){

    return new Promise((resolve, reject) => {

        try {
            port.write(`${experiment}\n`);

            dataParser.once('data', async (status)=> {
                const isCorrect = status == '1'; // 1 means the experiment code is correct
                resolve(isCorrect);
            });

            dataParser.once('error', err => reject(err));
        }catch (e){
            reject(e);
        }

    });

}

function openPort(){

    if(port) return; // If the port is already opened, we just return

     port = new SerialPort({path: portPath, baudRate: 9600});
     port.pipe(dataParser);

     port.once('error', err => {
         console.log(`Error opening port: ${err}`);
         port = null;
     });

}

function getPort() {
    return port;
}

function getParser() {
    return dataParser;
}


export {
    findArduino,
    checkExperimentCode,
    getPort,
    getParser,
    executeOperation,
    PORT_OPERATIONS
};