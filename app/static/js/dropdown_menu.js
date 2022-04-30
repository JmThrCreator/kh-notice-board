// dropdown menu

function dropdown() {
    var dropdown = document.getElementById("dropdown");
    if (dropdown.style.display == "block"){ 
        dropdown.style.display="none";  
    }
    else {
        dropdown.style.display="block";
    }
}

window.onclick = function(event) {
    var targetClass = event.target.className;
    if (targetClass != "bi bi-sort-down") {
        var dropdown = document.getElementById("dropdown");
        if (dropdown.style.display == "block"){
            dropdown.style.display="none";
        }
    }
}