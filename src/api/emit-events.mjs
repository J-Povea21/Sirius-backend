/*
    In this script we have all the logic related to the emit events of the api. Here we're going to use the fns declared in
    the port-manager.mjs module to communicate with the Arduino
 */

import * as Port from "../hardware/port-manager.mjs";

let webSocket = null;
let dataIsBeingSent = false;

// This method combines the findArduino and the checkExperiment functions
// to make easier the process of starting an experiment in the frontend
async function checkConnection(experiment) {
    const arduinoFound = await Port.findArduino();

    // If the arduino wasn't found, we return the JSON
    if (!arduinoFound.status) {
        emitResponse('checkConn', arduinoFound);
    } else {
        const experimentCode = await Port.checkExperimentCode(experiment);
        emitResponse('checkConn', experimentCode);
    }

}

async function findArduino() {
    const res = await Port.findArduino();
    emitResponse('findArduino', res);
}

async function checkExperimentCode(experiment) {
    const res = await Port.checkExperimentCode(experiment);
    emitResponse('checkExperiment', res);
}

async function startExperiment(runExperiment, experiment) {
    const operationToExecute = (runExperiment) ? Port.PORT_OPERATIONS.INIT : Port.PORT_OPERATIONS.PAUSE;
    const res = await Port.executeOperation(operationToExecute);

    dataIsBeingSent = runExperiment // We'll use this to handle the different cases when the user logs out of the application

    if (!res.status) {
        emitResponse('startExperiment', res);
    } else if (!runExperiment) {
        removeListeners();
    } else if (res.status && runExperiment) {
        emitExperimentData(experiment);
    }

}

/*
    This function basically sends the ESC command to the Arduino. This command is used to stop the current experiment and
    put the arduino in a state where it can listen if the user wants to start another experience
 */
function changeExperiment() {

    if (Port.getPort() == null) return;

    //Before we execute the ESC operation,we need to make sure that the arduino is not going to send more data. With flush we remove all the possible data              that it's in the port

    Port.getPort().close();
    Port.resetAllVars();
}


function emitExperimentData(exp) {
    const dataParser = Port.getParser();

    dataParser.on('data', sensorData => {
        try {
            if (typeof sensorData !== 'string') sensorData = sensorData.toString();

            emitResponse(exp, sensorData);
        } catch (e) {
            emitResponse(exp, {status: false, message: `Error parsing data: ${e.message}`});
        }

    });

}

function emitResponse(event, response) {
    webSocket.emit(event, response);
}

function setSocket(socket) {
    webSocket = socket;
}

function removeListeners() {
    Port.getParser().removeAllListeners();
}

export {
    setSocket,
    findArduino,
    checkExperimentCode,
    startExperiment,
    changeExperiment,
    checkConnection,
}