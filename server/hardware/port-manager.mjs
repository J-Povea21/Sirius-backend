/*
    In this script we're going to have all the logic related to the ports
 */

import {SerialPort} from "serialport";
import {DelimiterParser} from "@serialport/parser-delimiter";


// PORT AND PARSER
let portPath = '';
let port = null;
let dataParser = null;

// PORT OPERATIONS
const PORT_OPERATIONS = {PAUSE: 'P', INIT: 'I'}

// FUNCTIONS

async function findArduino() {
   const portList = await SerialPort.list();

   if(portList.length === 0)
       return false;
   else{
       // We search if there's an Arduino port and save the path
       portPath = portList.find(port => port.manufacturer.includes('Arduino')).path;
       return portPath !== undefined;
   }

}

function executeOperation(operation) {

    if(operation !== 'P' && operation !== 'I') return false;

    try{
        port.write(`${operation}\n`);
        dataParser.on('data', (data) => {
            data = JSON.parse(data);
            console.log(`Time: ${data.ULTRASONIC.time}`);
            console.log(`Distance: ${data.ULTRASONIC.distance}`);
        });

    }catch (e){
        return false;
    }

}

async function checkExperimentCode(experiment){

    port.write(`${experiment}\n`);

    const status = await waitForResponse('data');

    return status == '1';
}

function openPort(){

    return new Promise((resolve, reject) => {
        port = new SerialPort({path: portPath, baudRate: 9600});
        dataParser = port.pipe(new DelimiterParser({delimiter: '\r\n', encoding: 'utf8'}));

        dataParser.once('data', () => resolve(true));

        port.on('error', (err) => reject(err) );

    });

}

function getPort() {
    return port;
}

function getParser() {
    return dataParser;
}

function waitForResponse(event){
    return new Promise((resolve, reject) => {
    dataParser.once(event, (data) => {
      resolve(data);
    });

    dataParser.once('error', (error) => {
      reject(error);
    });
  });
}

export {
    findArduino,
    checkExperimentCode,
    getPort,
    getParser,
    openPort,
    executeOperation,
    PORT_OPERATIONS
};