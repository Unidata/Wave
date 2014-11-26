"use strict";

// Class to handle a rectangular bounding box
function Rectangle () {
}

Rectangle.prototype.width = function() {
    return this.right - this.left;
}

Rectangle.prototype.height = function() {
    return this.top - this.bottom;
}


// Helper function for bounding a value
function bound(min, val, max) {
    if (val < min) {
        return min;
    } else if (val > max ) {
        return max;
    }
    return val;
}

// Handle to WebGL stuff
var gl;
function ViewCanvas(canvas) {
    // Grab reference to the canvas we're using
    this.canvas = canvas;

    // Initialize camera vectors
    this.cameraLoc = vec3.fromValues(0., 0., 1.);
    this.lookAt = vec3.fromValues(0., 0., 0.);
    this.up = vec3.fromValues(0., 1., 0.);
    this.zoom = 1.;

    // Initial accumualted wheel events
    this.wheelDelta = 0.;

    // Initial values for dragging
    this.dragPos = vec4.create();
    this.dragging = false;

    // Matrices
    this.matrixChanged = true;
    this.screenToProj = mat4.create();
    this.normMatrix = mat4.create();
    this.pMatrix = mat4.create();
    this.mvMatrix = mat4.create();

    // Set up bounding boxes
    this.worldCoords = new Rectangle();
    this.worldCoords.left = -1.0;
    this.worldCoords.right = 1.0;
    this.worldCoords.bottom = -1.0;
    this.worldCoords.top = 1.0;

    this.domain = new Rectangle();
    this.domain.left = -180.;
    this.domain.right = 180.;
    this.domain.bottom = -90.;
    this.domain.top = 90.;

    this.worldScale = 1.0;

    // Screen information
    this.size = {};
    this.aspect = 1.0;

    // Iniitialize draw layers
    this.layers = [];

    // Initialze WebGL
    this.initGL();

    // Set-up some permanent WebGL state
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);
}

ViewCanvas.prototype.initGL = function() {
    gl = WebGLUtils.setupWebGL(this.canvas, {antialias: true})
    window.onresize = this.resize.bind(this);
    gl.canvas.onmousewheel = this.handleWheel.bind(this);
    gl.canvas.onmousedown = this.handleDown.bind(this);
    gl.canvas.onmouseup = this.handleUp.bind(this);
    gl.canvas.onmousemove = this.handleMove.bind(this);
    this.resize();
    if (!gl) {
        alert("Could not initialize WebGL, sorry :-(");
    }
}

ViewCanvas.prototype.wheelZoomStep = 60.;
ViewCanvas.prototype.handleWheel = function(e) {
    // Calculate the position of the wheel event
    var pos = vec4.fromValues(e.pageX - this.canvas.offsetLeft,
        e.pageY - this.canvas.offsetTop, 0., 1.);

    // pos = pos * scale
    vec4.scale(pos, pos, window.devicePixelRatio || 1);
    pos[3] = 1.; // Force z to 1.

    // Accumulate the wheel delta and zoom if necessary
    this.wheelDelta += e.wheelDelta;
    if (this.wheelDelta >= this.wheelZoomStep) {
        this.wheelDelta %= this.wheelZoomStep;
        this.zoomInTo(pos);
    } else if (this.wheelDelta < -this.wheelZoomStep) {
        this.wheelDelta = -(-this.wheelDelta % this.wheelZoomStep);
        this.zoomOutFrom(pos);
    }
}

ViewCanvas.prototype.handleDown = function(e) {
    if (e.button == 0) {
        this.dragging = true;
        vec4.set(this.dragPos, e.clientX, e.clientY, 0., 0.);
    }
}

ViewCanvas.prototype.handleUp = function(e) {
    this.dragging = false;
}

ViewCanvas.prototype.handleMove = function() {
    if (this.dragging) {
        var offset = vec4.fromValues(window.event.clientX, window.event.clientY, 0., 0.);
        vec4.subtract(offset, this.dragPos, offset);
        vec4.scale(offset, offset, window.devicePixelRatio || 1);
        vec4.transformMat4(offset, offset, this.screenToProj);
        this.shift(vec3.fromValues(offset[0], offset[1], offset[2]));
        vec4.set(this.dragPos, window.event.clientX, window.event.clientY, 0., 0.);
    }
}

ViewCanvas.prototype.zoomIncrement = 1.5;
ViewCanvas.prototype.setZoom = function(z) {
    this.zoom = bound(1., z, 100.);
    this.matrixChanged = true;
    return this.zoom;
}

ViewCanvas.prototype.zoomInTo = function(pos) {
    var oldZoom = this.zoom;
    this.zoomFixedPoint(pos, oldZoom / this.setZoom(this.zoom * this.zoomIncrement));
}

ViewCanvas.prototype.zoomOutFrom = function(pos) {
    var oldZoom = this.zoom;
    this.zoomFixedPoint(pos, oldZoom / this.setZoom(this.zoom / this.zoomIncrement));
}

ViewCanvas.prototype.zoomFixedPoint = function(pt, factor) {
    var offset = vec4.create();
    var trans = vec4.create();
    vec4.transformMat4(trans, pt, this.screenToProj);
    vec4.subtract(offset, trans, this.cameraLoc);
    vec4.scale(offset, offset, 1 - factor);
    offset[3] = 1.;
    this.shift(offset);
}

ViewCanvas.prototype.shift = function(vec) {
    vec3.add(this.cameraLoc, this.cameraLoc, vec);
    vec3.add(this.lookAt, this.lookAt, vec);
    this.limitCenter();
    this.matrixChanged = true;
}

ViewCanvas.prototype.limitCenter = function() {
    var totalScale = this.scale();
    var scaledWidth = this.worldCoords.width() / totalScale;
    if (scaledWidth <= this.domain.width()) {
        this.cameraLoc[0] = bound(this.domain.left + 0.5 * scaledWidth, this.cameraLoc[0], this.domain.right - 0.5 * scaledWidth);
    } else {
        this.cameraLoc[0] = bound(this.domain.right - 0.5 * scaledWidth, this.cameraLoc[0], this.domain.left + 0.5 * scaledWidth);
    }
    var scaledHeight = this.worldCoords.height() * this.aspect / totalScale;
    if (scaledHeight <= this.domain.height()) {
        this.cameraLoc[1] = bound(this.domain.bottom + 0.5 * scaledHeight, this.cameraLoc[1], this.domain.top - 0.5 * scaledHeight);
    } else {
        this.cameraLoc[1] = bound(this.domain.top - 0.5 * scaledHeight, this.cameraLoc[1], this.domain.bottom + 0.5 * scaledHeight);
    }
    this.lookAt[0] = this.cameraLoc[0];
    this.lookAt[1] = this.cameraLoc[1];
    this.matrixChanged = true;
}

ViewCanvas.prototype.updateWorldScale = function() {
    var widthRat = this.worldCoords.width() / this.domain.width();
    var heightRat = this.aspect * this.worldCoords.height() / this.domain.height();
    this.worldScale = widthRat >= heightRat ? widthRat : heightRat;
    this.matrixChanged = true;
}

ViewCanvas.prototype.scale = function() {
    return this.zoom * this.worldScale;
}

ViewCanvas.prototype.resize = function() {
    var ratio = window.devicePixelRatio || 1;
    gl.canvas.width = gl.canvas.clientWidth * ratio;
    gl.canvas.height = gl.canvas.clientHeight * ratio;

    this.size.width = gl.canvas.width;
    this.size.height = gl.canvas.height;
    this.aspect = this.size.height / this.size.width;

    // Set viewport and update world scale
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    this.updateWorldScale();

    // Regenerate matrix for getting normalized coords
    mat4.identity(this.normMatrix);
    var vec = vec4.fromValues(-1., 1., 0., 0.);
    mat4.translate(this.normMatrix, this.normMatrix, vec);
    vec4.set(vec, 2. / this.size.width, -2. / this.size.height, 1., 1.);
    mat4.scale(this.normMatrix, this.normMatrix, vec);
    this.matrixChanged = true;
}

ViewCanvas.prototype.updateMatrix = function() {
    var scaler = vec3.create();
    vec3.set(scaler, this.scale(), this.scale(), 1.);
    mat4.ortho(this.pMatrix, this.worldCoords.left, this.worldCoords.right,
        this.aspect * this.worldCoords.bottom, this.aspect * this.worldCoords.top, 10., -10.);
    mat4.scale(this.pMatrix, this.pMatrix, scaler);

    mat4.lookAt(this.mvMatrix, this.cameraLoc, this.lookAt, this.up);

    mat4.multiply(this.screenToProj, this.pMatrix, this.mvMatrix);
    mat4.invert(this.screenToProj, this.screenToProj);
    mat4.multiply(this.screenToProj, this.screenToProj, this.normMatrix);
    this.matrixChanged = false;
}

ViewCanvas.prototype.drawScene = function() {
    // Flush matrix changes if necessary
    if (this.matrixChanged) {
        this.updateMatrix();
    }

    // Clear before drawing
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Draw all the items
    var numLayers = this.layers.length;
    for (var ind=0; ind < numLayers; ++ind) {
        var layer = this.layers[ind];
        layer.draw.call(layer, this);
    }
}