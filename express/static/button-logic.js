const arduinoBtn = document.querySelector('#arduinoBtn');
let clicked = false;


arduinoBtn.onclick = () =>{
    arduinoBtn.innerText = (clicked)?'Pause':'Resume';
    clicked = !clicked;
}