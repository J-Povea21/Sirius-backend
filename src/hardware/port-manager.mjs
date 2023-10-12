/*
    In this script we're going to have all the logic related to the ports
 */

import {SerialPort} from "serialport";
import {DelimiterParser} from "@serialport/parser-delimiter";


// PORT AND PARSER
let portPath = '';
let port = null;
let dataParser = null;
let portOpen = false;
let experimentChecked = false;

// PORT OPERATIONS
const PORT_OPERATIONS = {PAUSE: 'PAUSE', INIT: 'INIT', ESC: 'ESC'};

// FUNCTIONS

async function findArduino() {

    try {
        const portList = await SerialPort.list();
        const devicesConnected = portList.length > 0;

        if (!devicesConnected){
            // If no devices are detected, we reset all the variables
            resetAllVars();
            return {status: false, message: 'No devices found. Please check the connection'};

        }else if (portOpen){
            return {status: true, message: 'Arduino detected'};
        }

        // We search if there's an Arduino port
        portPath = portList.find(port => {
            if (!port.manufacturer)
                return false;
             else
                 return port.manufacturer.includes('Arduino');
        });

        portPath = (portPath) ? portPath.path : '';

        if (!portPath) return {status: false, message: 'Arduino not found. Please check the connection'};

        // If there's a port, we try to open it and assign the returned value to portOpen
        portOpen = await openPort();

        return {status: portOpen, message: 'Arduino detected'};

    }catch (e) {
        return {status: false, message: `Error finding arduino: ${e.message}`};
    }


}

function executeOperation(operation) {

    if( !(operation in PORT_OPERATIONS) ) return {status: false, message: 'Invalid operation'};
    if (!port) return {status: false, message: 'No arduino detected. Please check the connection'};

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

    if (experimentChecked) return {status: true, message: 'Connection already established'};

    port.write(`${experiment}\n`, err => {
        if (err) return {status: false, message: `Error checking experiment: ${err.message}`};
    });

    const status = await waitResponse() == experiment // If the response is the same as the given experiment, the code is correct
    const message = (status)? 'OK' : 'The code doesn\'t match the experiment';

    if (status) experimentChecked = true;

    return {status, message};
}


function openPort(){

    return new Promise((resolve, reject) => {

        port = new SerialPort({path: portPath, baudRate: 9600});
        dataParser = port.pipe(new DelimiterParser({delimiter: '\r\n', encoding: 'utf8'}));

        // Here we start a setTimeOut so in case we don't receive any data from the arduino, we resolve the promise with false
        const timeoutID = setTimeout(() => resolve(false), 5000); // 5 seconds

        dataParser.once('data', () =>{
            clearTimeout(timeoutID);
            resolve(true);
        });

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

function setExperimentChecked(value){
    experimentChecked = value;
}


function resetAllVars(){
    portOpen = false;
    portPath = '';
    port = null;
    dataParser = null;
    experimentChecked = false;
}

export {
    findArduino,
    checkExperimentCode,
    PORT_OPERATIONS,
    executeOperation,
    getParser,
    setExperimentChecked,
};