/*
    In this script we have all the logic related to the on events of the socket
*/

import * as EmitEvents from "./emitEvents.mjs";

export function setConnection(socket, app){

    EmitEvents.setSocket(socket);
    socket.on('findArduino', () => EmitEvents.findArduino());
    socket.on('checkExperiment', experiment => EmitEvents.checkExperimentCode(experiment));
    socket.on('startExperiment', (runExperiment, experiment) => EmitEvents.startExperiment(runExperiment, experiment));
    socket.on('changeExperiment', () => EmitEvents.changeExperiment());
}



