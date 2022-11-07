window.onclick = function(event) {
    var targetId = event.target.id;
    if (targetId == "numpad") {
        inputValue = event.target.value;
        value = document.getElementById("input_box").value;
        if (inputValue == "backspace") {
            value = value.slice(0, -1);
        }
        else if (value == "0") {
            value = inputValue
        }
        else {
            value = value.concat(inputValue);
        }
        document.getElementById("input_box").value = value;
    } 
}