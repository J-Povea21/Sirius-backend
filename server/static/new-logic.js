const socket = io();

//FIRST COMMAND
socket.emit('ESC');

const checkExperiment = document.querySelector('#check-experiment');
const startExperiment = document.querySelector('#start-experiment');

let experimentRunning = false;

checkExperiment.onclick = () => {
    const experiment = document.querySelector('#experiment').value;
    socket.emit('checkExperiment', experiment);
};

startExperiment.onclick = () => socket.emit('startExperiment', changeState(), 'MRUA');

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