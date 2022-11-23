// dropdown menu

function dropdown() {
    var dropdown = document.getElementById("dropdown");
    var buttons = document.getElementsByClassName("sort_button");
    console.log(buttons)
    if (dropdown.style.opacity == "100") {
        dropdown.style.opacity = "0";
        dropdown.style.scale = "95%";
        for (var i = 0; i < buttons.length; i++) {
            buttons[i].style.pointerEvents = "none";
        }
    }
    else {
        dropdown.style.opacity = "100";
        dropdown.style.scale = "100%";
        for (var i = 0; i < buttons.length; i++) {
            buttons[i].style.pointerEvents = "auto";
        }
    }
}

window.onclick = function(event) {
    var targetId = event.target.id
    if (targetId !== "dropdown_button" && targetId !== "dropdown") {
        var dropdown = document.getElementById("dropdown");
        var buttons = document.getElementsByClassName("sort_button");
        if (dropdown.style.opacity == "0") {
            dropdown.style.opacity = "0";
            dropdown.style.scale = "95%";
            dropdown.style.pointerEvents = "none";
            for (var i = 0; i < buttons.length; i++) {
                buttons[i].style.pointerEvents = "none";
            }
        }
    }
}

function showPopup() {
    items = document.getElementById("popup");
}

function clearPopup() {
    items = document.getElementById("popup");
}