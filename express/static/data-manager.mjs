// We create a socket
const socket = io('/');
// We listen to the event 'arduino' and we print the data received
socket.on('arduino', (data) => {
    console.log(data);
});