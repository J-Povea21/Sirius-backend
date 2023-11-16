/*
    In this script we have all the logic related to the on events of the api
*/

import * as EmitEvents from "./emit-events.mjs";

export function setConnection(socket, app) {

    EmitEvents.setSocket(socket);
    socket.on('disconnect', () => EmitEvents.changeExperiment());

    socket.on('findArduino', () => EmitEvents.findArduino());
    socket.on('checkExperiment', experiment => EmitEvents.checkExperimentCode(experiment));

    socket.on('startExperiment', (runExperiment, experiment) => EmitEvents.startExperiment(runExperiment, experiment));
    socket.on('changeExperiment', () => EmitEvents.changeExperiment());

    socket.on('checkConn', experiment => EmitEvents.checkConnection(experiment));

    // FAKE
    socket.on('MD', () => EmitEvents.fakeMD());
    socket.on('checkConnFake', () => EmitEvents.fakeCheckConnection());
    socket.on('pause', () => EmitEvents.fakePause());
}



