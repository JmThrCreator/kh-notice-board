// loading spinner

var clicked = false;

function disableButton(id) {

    loading();
    
    var elements = document.getElementsByTagName("button");

    for (var i = 0; i < elements.length; i++) {
        if (elements[i].id === id) ;
        else (elements[i].disabled = true);
    } 

    if (clicked === true) {
        document.getElementById(id).disabled = true;
    }

    clicked = true;

}

function loading(){

    var loading = document.getElementsByClassName("loading")
    var loading = loading[0];
    loading.style.opacity = "100";

}