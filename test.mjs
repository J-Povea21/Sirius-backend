import { SerialPort } from 'serialport';

let serialList = SerialPort.list().then(data => data)
export default serialList;