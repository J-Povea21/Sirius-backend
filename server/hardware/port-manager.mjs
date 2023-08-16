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

function executeOperation(operation){

    let operationExecuted = false;

    if(port == null || !port.isOpen){
        console.log(`[Error executing operation]: Port is not open`);
        return operationExecuted;
    }

    if(!(operation in PORT_OPERATIONS)){
        console.log(`Operation \'${operation}\' not recognized`);
    }else{
        port.write(`${PORT_OPERATIONS[operation]}\n`);
        operationExecuted = true;

        // In case an error happens, we set operationExecuted to false
        port.on('error', err => {
               console.log(`Error executing operation: ${err}`);
               operationExecuted = false;
        });
    }

    return operationExecuted;

}

function checkExperimentCode(experiment){
    let isCorrect = false;

    try{
        !port && openPort(); // If the port is not open, we open it
        port.write(experiment);
        dataParser.on('data', status => isCorrect = (status == '1') );
    }catch (e) {
        console.log(`An error happened while checking code: ${e}`);
    }
    return isCorrect;
}

function openPort(){
    try{
        port = new SerialPort({path: portPath, baudRate: 9600});
        port.pipe(dataParser);

        port.on('error', err => console.log(`[Error opening port]: ${err.message}`));
    }catch(e){
        console.log(`Error creating port instance: ${e}`);
    }
}

function getPort() {
    return port;
}



export {findArduino, checkExperimentCode, getPort, executeOperation, PORT_OPERATIONS};