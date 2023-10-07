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
const PORT_OPERATIONS = {PAUSE: 'PAUSE', INIT: 'INIT', ESC: 'ESC'};

// FUNCTIONS

async function findArduino() {

    if (port) return {status: true, message: 'Arduino detected'};


    try {
        const portList = await SerialPort.list();

        if (portList.length === 0) return {status: false, message: 'No devices found. Please check the connection'};

        // We search if there's an Arduino port and save the path
        portPath = portList.find(port => {
            if (!port.manufacturer)
                return false;
             else
                 return port.manufacturer.includes('Arduino');
        });

        portPath = (portPath) ? portPath.path : '';

        if (!portPath) return {status: false, message: 'Arduino not found. Please check the connection'};

        // If there's a port, we open it and return the status
        return {status: await openPort(), message: 'Arduino detected'};

    }catch (e) {
        return {status: false, message: `Error finding arduino: ${e.message}`};
    }


}

function executeOperation(operation) {

    if( !(operation in PORT_OPERATIONS) ) return {status: false, message: 'Invalid operation'};

    try{
        port.write(`${operation}\n`, err => {
            if (err) return {status: false, message: `Communication error: ${err.message}`};
        });
        return {status: true, message: 'Operation executed'};
    }catch (e){
        return {status: false, message: `Error executing operation: ${e.message}`};
    }

}

async function checkExperimentCode(experiment){

    if (!port) return {status: false, message: 'No arduino detected.The experiment cant be checked'};

    port.write(`${experiment}\n`, err => {
        if (err) return {status: false, message: `Error checking experiment: ${err.message}`};
    });

    const status = await waitResponse() == experiment // If the response is the same as the given experiment, the code is correct
    const message = (status)? 'OK' : 'The code doesn\'t match the experiment';

    return {status, message};
}


function openPort(){

    return new Promise((resolve, reject) => {

        port = new SerialPort({path: portPath, baudRate: 9600});
        dataParser = port.pipe(new DelimiterParser({delimiter: '\r\n', encoding: 'utf8'}));

        dataParser.once('data', () => resolve(true));

        port.once('error', (err) => reject(err) );

    });

}

function waitResponse(){
    return new Promise((resolve, reject) => {
        dataParser.once('data', data => resolve(data));

        port.once('error', err => reject(err));
    });
}

function getParser() {
    return dataParser;
}

export {
    findArduino,
    checkExperimentCode,
    getParser,
    executeOperation,
    PORT_OPERATIONS
};