var gameOfLife,
canvas,
ctx,
generationNumberSpan
offBlackStyle = 'rgb(10, 10, 15)',
greyStyle = 'rgb(128, 128, 135)',
whiteStyle = 'rgb(305, 305, 305)',
sqSz = 10;

function getClickedCell(e) {
  var x, y;

  if (e.pageX || e.pageY) {
    x = e.pageX;
    y = e.pageY;
  } else {
    x = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
    y = e.clientY + document.body.scrollTop + document.documentElement.scrollTop;
  }

  x -= canvas.offsetLeft;
  y -= canvas.offsetTop;

  return [Math.floor(x / sqSz), Math.floor(y / sqSz)];
}
    

function onCanvasClick(e) {
  var cell = getClickedCell(e);
  gameOfLife.toggleCell(cell[0], cell[1]);
}

function drawRectOnCell(x, y, style) {
  style = style || offBlackStyle;
  ctx.fillStyle = style;
  ctx.fillRect(x * sqSz, y * sqSz, sqSz, sqSz);
}

function drawLines() {
  var currentX, currentY;
  var numberOfLinesX = canvas.width / sqSz - 1;
  var numberOfLinesY = canvas.height / sqSz - 1;
  ctx.lineWidth = 1;
  for (currentX = 1; currentX < canvas.height / sqSz; currentX++) {
    ctx.moveTo(0, sqSz * currentX + 0.5);
    ctx.lineTo(canvas.width, sqSz * currentX + 0.5);
  }
  for (currentY = 1; currentY < canvas.width / sqSz; currentY++) {
    ctx.moveTo(sqSz * currentY + 0.5, 0);
    ctx.lineTo(sqSz * currentY + 0.5, canvas.height);
  }
  ctx.stroke();
}

function prepareGameOfLife() {
    // initialize canvas for cells
  canvas = document.getElementById('gameOfLifeWorld');
  ctx = canvas.getContext('2d');
  ctx.fillStyle = 'rgb(10, 10, 15)';

  // Create the object grid for the game
  gameOfLife = GameOfLife(canvas.width / sqSz, canvas.height / sqSz);
  console.log(gameOfLife);
  generationNumberSpan = document.getElementById('generation');

  drawLines();

  gameOfLife.onStep(function() {
    var cx, cy;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    for (cx = 0; cx < gameOfLife.get_width(); cx += 1) {
      for (cy = 0; cy < gameOfLife.get_height(); cy += 1) {
        if (gameOfLife.isLiving(cx, cy)) {
          drawRectOnCell(cx, cy, offBlackStyle);
        } else {
          drawRectOnCell(cx, cy, whiteStyle);
        }
      }
    }

    drawLines();
    generationNumberSpan.innerText = gameOfLife.get_generation();
  });

  gameOfLife.onCellPopulated(function(x, y) {
    drawRectOnCell(x, y, offBlackStyle);
  });

  gameOfLife.onCellKilled(function(x, y) {
    drawRectOnCell(x, y, whiteStyle);
  });

  canvas.addEventListener('click', onCanvasClick, false);
}

function startGameOfLife() {
  gameOfLife.start(40, -1);
}

function seedBoard(newMatrix) {
    gameOfLife.populateCells(newMatrix);
}

function foobar() {
    seedBoard([[70, 60], [71, 60], [72, 60], [72, 61], [74, 61], [74, 62], [74, 58], [74, 57], [74, 56], [75, 60], [75, 62], [75, 58], [76, 60], [76, 61], [78, 60], [78, 61], [79, 60], [79, 62], [79, 58], [80, 62], [80, 61], [80, 58], [80, 57], [80, 56], [82, 60], [82, 61], [83, 60], [84, 60]]);
}