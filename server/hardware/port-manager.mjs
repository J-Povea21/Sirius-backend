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
const PORT_OPERATIONS = {PAUSE: 'PAUSE', INIT: 'INIT', ESCAPE: 'ESC'};

// FUNCTIONS

async function findArduino() {

    if (port) return {response: true, message: 'Arduino detected'};

    try {
        const portList = await SerialPort.list();

        if (portList.length === 0) return {response: false, message: 'No devices found. Please check the connection'};

        // We search if there's an Arduino port and save the path
        portPath = portList.find(port => port.manufacturer.includes('Arduino')).path;

        if (!portPath) return {response: false, message: 'Arduino not found. Please check the connection'};

        // If there's a port, we open it and return the status
        return {response: await openPort(), message: 'Arduino detected'};

    }catch (e) {
        return {response: false, message: `Error finding arduino: ${e.message}`};
    }


}

function executeOperation(operation) {

    if( !(operation in PORT_OPERATIONS) ) return {response: false, message: 'Invalid operation'};

    try{
        port.write(`${operation}\n`, err => {
            if (err) return {response: false, message: `Communication error: ${err.message}`};
        });
        return {response: true, message: 'Operation executed'};
    }catch (e){
        return {response: false, message: `Error executing operation: ${e.message}`};
    }

}

async function checkExperimentCode(experiment){

    if (!port) return {response: false, message: 'No arduino detected.The experiment cant be checked'};

    port.write(`${experiment}\n`, err => {
        if (err) return {response: false, message: `Error checking experiment: ${err.message}`};
    });

    const response = await waitResponse() == experiment // 1 means the code matches the experiment
    const message = (response)? 'OK' : 'The code doesn\'t match the experiment';

    return {response, message};
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