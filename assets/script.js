function runSpinner(id) {
    var spinner = document.getElementById(id);
        spinner.style.display = "block";
        spinner.style.animation = "loading 1s linear infinite";
        var elements = document.getElementsByTagName("button");
        for (var i = 0; i < elements.length; i++) {
            if (elements[i].id === id) ;
            else elements[i].disabled = true;
        }
        document.getElementById(id).disabled = true;
}

function resizeImage(img, size) {
    const perimeter = 1250*size;
    const aspectRatio = img.width / img.height;

    const height = perimeter / (2 * (aspectRatio + 1));
    const width = aspectRatio * height;

    img.style.width = width + 'px';
    img.style.height = height + 'px';
};