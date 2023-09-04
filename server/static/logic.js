// We start the socket connection in the client side
const socket = io();

const findArduino = document.querySelector('#find-arduino');
const checkExperiment = document.querySelector('#check-experiment');
const startExperiment = document.querySelector('#start-experiment');

let experimentRunning = false;


// SOCKET EMIT EVENTS

findArduino.onclick = () => socket.emit('findArduino');

checkExperiment.onclick = () => {
    const experiment = document.querySelector('#experiment').value;
    socket.emit('checkExperiment', experiment);
};

startExperiment.onclick = () => socket.emit('startExperiment', changeState(), 'MRUA');

// SOCKET ON EVENTS
socket.on('findArduino', res => {
    const arduinoFound = res.response;

    if (arduinoFound)
        document.querySelectorAll('.checks').forEach(elem => elem.style.display = 'block');

    alert(res.message);
});

socket.on('checkExperiment', res => {
    const isValidCode = res.response;

    if (isValidCode)
        document.querySelectorAll('.operations').forEach(elem => elem.style.display = 'block');

    alert(res.message);

});

socket.on('MRUA', res => {
    console.log(res);
});

socket.on('operationResponse', data => {
    console.log(data);
});


// UTILS
function changeState(){
    startExperiment.innerText = (!experimentRunning) ? 'Pause' : 'Resume';
    experimentRunning = !experimentRunning;
    return experimentRunning;
}