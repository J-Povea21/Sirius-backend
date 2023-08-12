// Import the charts module
import {updateChart} from "./charts.mjs";


// Select the pause-resume button
const arduinoBtn = document.querySelector('#arduinoBtn');
const checkButton = document.querySelector('#checkButton');
let opened = false;

// Create the socket
const socket = io();

arduinoBtn.onclick = () => socket.emit('opened', changeState());
checkButton.onclick = () => socket.emit('check', 'SENSORTYPE') //NECESITO EL ARDUINO DE SENSOR HAXELL


// We listen to the event 'arduino' and we print the data received
socket.on('arduino', (data) => {
    updateChart(data, 0);
});

function changeState(){
    arduinoBtn.innerText = (!opened)?'Pause':'Resume';
    opened = !opened;
    return opened;
}