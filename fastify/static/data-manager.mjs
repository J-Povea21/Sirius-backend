// Import the charts module
import {chart, updateChart} from "./charts.mjs";


// Select the pause-resume button
const arduinoBtn = document.querySelector('#arduinoBtn');
let opened = false;

// Create the socket
const socket = io();

arduinoBtn.onclick = () => socket.emit('opened', changeState());


// We listen to the event 'arduino' and we print the data received
socket.on('ULTRASONIC', (data) => {
    updateChart(chart, data, 0);
});

function changeState(){
    arduinoBtn.innerText = (!opened)?'Pause':'Resume';
    opened = !opened;
    return opened;
}