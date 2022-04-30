// loading spinner

var clickCount = 0;

function disableButton() {
    loading();
    
    if (clickCount == 1){
        var elements = document.getElementsByClassName("button");
        for(var i = 0; i < elements.length; i++) {
            elements[i].disabled = true;
        }
    }
    else {
        ;
    }
    clickCount += 1
}

function loading(){

    var loading = document.getElementsByClassName("loading")
    var loading = loading[0];
    loading.style.opacity = "100";

}