/*
    In this script we have all the logic related to the emit events of the api. Here we're going to use the fns declared in
    the port-manager.mjs module to communicate with the Arduino
 */

import * as Port from "../hardware/port-manager.mjs";

let webSocket = null;

function setSocket(socket){
    webSocket = socket;
}

// This method combines the findArduino and the checkExperiment functions
// to make easier the process of starting an experiment in the frontend
async function checkConnection(experiment){
    const arduinoFound = await Port.findArduino();

    // If the arduino wasn't found, we return the JSON
    if (!arduinoFound.status){
        emitResponse('checkConn', arduinoFound);
    }else{
        const experimentCode = await Port.checkExperimentCode(experiment);
        emitResponse('checkConn', experimentCode);
    }

}

async function findArduino(){
    const res = await Port.findArduino();
    emitResponse('findArduino', res);
}

async function checkExperimentCode(experiment){
    const res = await Port.checkExperimentCode(experiment);
    emitResponse('checkExperiment', res);
}

async function startExperiment(runExperiment, experiment){
    const operationToExecute = (runExperiment) ? Port.PORT_OPERATIONS.INIT : Port.PORT_OPERATIONS.PAUSE;
    const res = await Port.executeOperation(operationToExecute);

    emitResponse('operationResponse', res);
    if (res.status) emitExperimentData(experiment); //If the operation was executed successfully, we start emitting data
}

/*
    This function basically sends the ESC command to the Arduino. This command is used to stop the current experiment and
    put the arduino in a state where it can listen if the user wants to start another experience
 */
async function changeExperiment(){
    // We remove all the listeners from the parser
    getParser().removeAllListeners();

    const response = await Port.executeOperation(Port.PORT_OPERATIONS.ESC);
    emitResponse('operationResponse', response);
}


function emitResponse(event, response){
    webSocket.emit(event, response);
}

function emitExperimentData(exp){
    const dataParser = Port.getParser();

    dataParser.on('data', sensorData => {
       const parsedData = JSON.parse(sensorData);
       webSocket.emit(exp, parsedData);
    });

}


export {
    setSocket,
    findArduino,
    checkExperimentCode,
    startExperiment  ,
    changeExperiment,
    checkConnection,
}