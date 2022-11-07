// loading spinner

var clicked = false;

function disableButton(id) { 
    loading();

    var elements = document.getElementsByTagName("button");

    for (var i = 0; i < elements.length; i++) {
        if (elements[i].id === id) ;
        else elements[i].disabled = true;
    }

    if (clicked === true) document.getElementById(id).disabled = true;

    clicked = true;
}

function loading(){
    var spinner = document.getElementById("reset")
    spinner.style.animation = "loading 1s linear infinite"
}