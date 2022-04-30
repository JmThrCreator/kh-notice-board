window.onclick = function(event) {
    var targetClass = event.target.className;
    if (targetClass == "numpad") {
        inputValue = event.target.value;

        value = document.getElementsByClassName("input_box")[0].value;
        console.log(value)
        if (inputValue == "backspace") {
            value = value.slice(0, -1);
        }
        else if (value == "0") {
            value = inputValue
        }
        else {
            value = value.concat(inputValue);
        }
        document.getElementsByClassName("input_box")[0].value = value;
    } 
}