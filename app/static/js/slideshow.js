// slideshow

var inactivityTime = function () {
    var time;
    var reset = false;

    // reset timer on the following events:
    window.onload = resetTimer;
    document.onmousemove = resetTimer;
    document.onkeydown = resetTimer;
    document.onmousedown = resetTimer;

    function slideshow() {
        reset = true;
        slideIndex = 0;
        document.getElementById("content").style.display = "none";
        document.getElementById("slideshow").style.display = "block";
        showSlides();
    }

    // reset the timer
    function resetTimer() {
        clearTimeout(time);

        //after 10 seconds of idling the slideshow will start
        time = setTimeout(slideshow, 10000); 
        if (reset == true){
            clearTimeout(slideTimeout);
            document.getElementById("content").style.display = "block";
            document.getElementById("slideshow").style.display = "none";
            reset = false;
        }
    }
}

function showSlides() {
    let i;
    let slides = document.getElementsByClassName("slide");
    for (i = 0; i < slides.length; i++) {
        slides[i].style.display = "none"; 
    }
    slideIndex++;
    if (slideIndex > slides.length) {slideIndex = 1}
    slides[slideIndex-1].style.display = "flex";
    
    // repeats every 10 seconds
    slideTimeout = setTimeout(showSlides, 10000);
}

inactivityTime();