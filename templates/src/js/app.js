window.addEventListener("hashchange", (event) => {
	switch (window.location.hash) {
		case "#index":
			window.location = "./";
			break;
		case "#gol":
			window.location = "./gol";
			break;
		case "#mandelbrot":
			window.location = "./mandelbrot";
			break;
		default:
			console.log("Page Not Found");
	}
});
