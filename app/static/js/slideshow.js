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
        document.getElementsByClassName("content")[0].style.display = "none";
        document.getElementsByClassName("slideshow")[0].style.display = "block";
        showSlides();
    }

    // resets the timer
    function resetTimer() {
        clearTimeout(time);
        //after 10 seconds of idling the slideshow will start
        time = setTimeout(slideshow, 10000); 
        if (reset == true){
            clearTimeout(slideTimeout);
            document.getElementsByClassName("content")[0].style.display = "block";
            document.getElementsByClassName("slideshow")[0].style.display = "none";
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
    slides[slideIndex-1].style.display = "block";
    
    // repeats every 10 seconds
    slideTimeout = setTimeout(showSlides, 10000);
}

inactivityTime();