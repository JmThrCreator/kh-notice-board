// dropdown menu

function dropdown() {
    var dropdown = document.getElementById("dropdown");
    if (dropdown.style.opacity == "100") {
        dropdown.style.opacity = "0";
        dropdown.style.scale = "95%";
    }
    else {
        dropdown.style.opacity = "100";
        dropdown.style.scale = "100%";
    }
}

window.onclick = function(event) {
    var targetId = event.target.id
    if (targetId !== "dropdown_button" && targetId !== "dropdown") {
        var dropdown = document.getElementById("dropdown");
        if (dropdown.style.opacity == "0") {
            dropdown.style.opacity = "0";
            dropdown.style.scale = "95%";
        }
    }
}

function showPopup() {
    items = document.getElementById("popup");
}

function clearPopup() {
    items = document.getElementById("popup");
}