"use strict";

var conn;
var main;
function Backend() {
    conn = new IpythonConnection({
        init_code: 'import wave',
        start: function() {
            main = new mainLoop(this.kernel);
        }
    });
}

function mainLoop(kernel) {
    this.canvas = new ViewCanvas(document.getElementById("canvas"));
    this.canvas.layers.push(new RasterImageLayer(kernel));

    this.initStats();
    this.tick();
}

mainLoop.prototype.initStats = function() {
    this.stats = new Stats();
    this.stats.setMode(1); // 0: fps, 1: ms

    // Align top-left
    this.stats.domElement.style.position = 'absolute';
    this.stats.domElement.style.left = '0px';
    this.stats.domElement.style.top = '0px';
    document.body.appendChild( this.stats.domElement );
}

mainLoop.prototype.tick = function() {
    requestAnimFrame(this.tick.bind(this));
    this.stats.begin();
    this.canvas.drawScene();
    this.stats.end();
}