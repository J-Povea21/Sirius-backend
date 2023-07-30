// Select the pause-resume button
const arduinoBtn = document.querySelector('#arduinoBtn');
let opened = false;

// Create the socket
const socket = io();

arduinoBtn.onclick = () => socket.emit('opened', changeState());


// We listen to the event 'arduino' and we print the data received
socket.on('arduino', (data) => {
    console.log(data);
});

function changeState(){
    arduinoBtn.innerText = (!opened)?'Pause':'Resume';
    opened = !opened;
    return opened;
}