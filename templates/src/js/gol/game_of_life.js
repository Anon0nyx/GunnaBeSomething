function initialiseGrid(w, h) {
	var x, y, result = [];

	for (x = 0; x < w; x += 1) {
		result[x] = [];
	}
	return result;
}

function GameOfLife(w, h) {
	var width = w,
	height = h,
	world = initialiseGrid(w, h),
	newWorld = initialiseGrid(w, h),
	interval,
	iterations = 0,
	onStepCallback,
	onCellPopulatedCallback,
	onCellKilledCallback,
	generation = 0;

	this.populateCell = function populateCell (x, y) {
		if (x >= width || y >= height) {
			throw {
				name : 'OutOfBoundsError',
				message : 'There is no such position in the world as [' + x + ',' + y + ']'
			}
		}
		world[x][y] = true;

		if (onCellPopulatedCallback) {
			onCellPopulatedCallback(x, y);
		}
	};

	this.populateCells = function populateCells (coordinateArray) {
		var i;
		for (i = 0; i < coordinateArray.length; i++) {
			this.populateCell(coordinateArray[i][0], coordinateArray[i][1]);
		}
	};

	this.toggleCell = function (x, y) {
		if (x >= width || y >= height) {
			throw {
				name : 'OutOfBoundsError',
				message : 'There is no such position in the world as [' + x + ',' + y + ']'
			}
		}
		if (isLiving(x, y)) {
			world[x][y] = false;
			if (onCellKilledCallback) {
				onCellKilledCallback(x, y);
			}
		} else {
			populateCell(x, y);
		}
	};

	this.isLiving = function isLiving(x, y) {
		return world[x] && world[x][y];
	}

	function numberOfLivingNeighbours(x, y) {
		var cx,
		cy,
		livingNeighbours = 0;

		for (cx = x - 1; cx <= x + 1; cx += 1) {
			for (cy = y - 1; cy <= y + 1; cy += 1) {
				if ((cx !== x || cy !== y) && isLiving(cx, cy)) {
					livingNeighbours += 1;
				}
			}
		}

		return livingNeighbours;
	}

	this.step = function step() {
		var cx, cy, livingNeighbours;

		newWorld = initialiseGrid(width, height);

		generation += 1;

		for (cx = 0; cx < w; cx += 1) {
			for (cy = 0; cy < h; cy += 1) {
				livingNeighbours = numberOfLivingNeighbours(cx, cy);
				
				if (livingNeighbours < 2) {
					// Any live cell with fewer than two live neighbours dies, as if caused by under-population
					newWorld[cx][cy] = false;
				} else if (isLiving(cx, cy) && (livingNeighbours === 2 || livingNeighbours === 3)) {
					// Any live cell with two or three live neighbours lives on to the next generation.
					newWorld[cx][cy] = true;
				} else if (livingNeighbours > 3 && isLiving(cx, cy)) {
					// Any live cell with more than three live neighbours dies, as if by overcrowding
					newWorld[cx][cy] = false;
				} else if (!isLiving(cx, cy) && livingNeighbours === 3) {
					// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction
					newWorld[cx][cy] = true;
				}
			}
		}
		world = newWorld;

		if (onStepCallback) {
			onStepCallback();
		}

		if (iterations > 0) {
			iterations -= 1;
		}

		if (iterations > 0 || iterations === -1) {
			if (interval > 0) {
				setTimeout(step, interval);
			} else {
				step();
			}
		}
	}

	this.onStep = function(callback) {
		onStepCallback = callback;
	};

	this.onCellPopulated = function(callback) {
		onCellPopulatedCallback = callback;
	};

	this.onCellKilled = function(callback) {
		onCellKilledCallback = callback;
	};

	this.get_world = function () {
		return world;
	};

	this.get_width = function () {
		return width;
	};

	this.get_height = function () {
		return height;
	};

	this.get_generation = function () {
		return generation;
	};

	this.start = function (iv, it) {
		interval = iv || 0;
		iterations = it;

		generation = 0;

		step();
	};

	return this;
}