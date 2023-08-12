import { SerialPort } from 'serialport';
import { DelimiterParser } from '@serialport/parser-delimiter';

// Define tus puertos seriales
let portName2 = 'COM5';

/*
    let portName2 = 'COM6';
    const port1 = new SerialPort({
  path: portName1,
  baudRate:9600,
  
});
*/


// Crea los objetos SerialPort

const port2 = new SerialPort({
    path: portName2,
    baudRate: 9600,
});



// Crea los objetos Readline parser
const parser2 = port2.pipe(new DelimiterParser({ delimiter: '\r\n' }));
//const parser1 = port1.pipe(new DelimiterParser({ delimiter: '\r\n' }));


// Escucha los datos entrantes del puerto 1
/** 
parser1.on('data', (data) => {
    data = JSON.parse(data)
    console.log(`Puerto 1 (tiempo): ${data.ULTRASONIC.time}`);
    console.log(`Puerto 1 (distancia): ${data.ULTRASONIC.distance}`);
});

// Escucha los datos entrantes del puerto 2
parser2.on('data', (data) => {
    console.log(`Puerto 2: ${data}`);
});

// Manejo de errores
port1.on('error', function(err) {
    console.log('Error en Puerto 1: ', err.message);
})

port2.on('error', function(err) {
    console.log('Error en Puerto 2: ', err.message);
})
*/


// setTimeout(() => {
//     console.log('Cerrando Puerto 1...');
//     port1.close();
  
//     setTimeout(() => {
//       console.log('Reabriendo Puerto 1...');
//       port1.open();
//     }, 5000); // Reanudar después de 5 segundos
//   }, 10000); // Pausar después de 10 segundos

export {port2, parser2};