/*
    In this script we have all the logic related to the emit events of the socket. Here we're going to use the fns declared in
    the port-manager.mjs module to communicate with the Arduino
 */

import * as PortManager from "../hardware/port-manager.mjs";

let webSocket = null;

function setSocket(socket){
    webSocket = socket;
}

async function findArduino(){
    const res = await PortManager.findArduino();
    emitResponse('findArduino', res);
}

async function checkExperimentCode(experiment){
    const res = await PortManager.checkExperimentCode(experiment);
    emitResponse('checkExperiment', res);
}

async function startExperiment(runExperiment, experiment){
    const operationToExecute = (runExperiment) ? PortManager.PORT_OPERATIONS.INIT : PortManager.PORT_OPERATIONS.PAUSE;
    const res = await PortManager.executeOperation(operationToExecute);

    emitResponse('operationResponse', res);
    if (res.response) emitExperimentData(experiment); //If the operation was executed successfully, we start emitting data

}


function emitResponse(event, response){
    webSocket.emit(event, response);
}

function emitExperimentData(exp){
    const dataParser = PortManager.getParser();

    dataParser.on('data', sensorData => {
       const parsedData = JSON.parse(sensorData);
       webSocket.emit(exp, parsedData);
    });

}

export {
    setSocket,
    findArduino,
    checkExperimentCode,
    startExperiment
}