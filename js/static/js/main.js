"use strict";
function Rectangle () {
}

Rectangle.prototype.width = function() {
    return this.right - this.left;
}

Rectangle.prototype.height = function() {
    return this.top - this.bottom;
}

function bound(min, val, max) {
    if (val < min) {
        return min;
    } else if (val > max ) {
        return max;
    }
    return val;
}

var cameraLoc = vec3.fromValues(0., 0., 1.);
var lookAt = vec3.fromValues(0., 0., 0.);
var up = vec3.fromValues(0., 1., 0.);

var wheelDelta = 0.;
var wheelZoomStep = 60.;
function handleWheel(e) {
    var pos = vec4.fromValues(e.pageX - canvas.offsetLeft,
        e.pageY - canvas.offsetTop, 0., 1.);
    vec4.scale(pos, pos, window.devicePixelRatio || 1);
    pos[3] = 1.;
    wheelDelta += e.wheelDelta;
    if (wheelDelta >= wheelZoomStep) {
        wheelDelta %= wheelZoomStep;
        zoomInTo(pos);
    } else if (wheelDelta < -wheelZoomStep) {
        wheelDelta = -(-wheelDelta % wheelZoomStep);
        zoomOutFrom(pos);
    }
}

var dragPos = vec4.create();
var dragging = false;
function handleDown(e) {
    if (e.button == 0) {
        dragging = true;
        vec4.set(dragPos, e.clientX, e.clientY, 0., 0.);
    }
}

function handleUp(e) {
    dragging = false;
}

function handleMove() {
    if (dragging) {
        var offset = vec4.fromValues(window.event.clientX, window.event.clientY, 0., 0.);
        vec4.subtract(offset, dragPos, offset);
        vec4.scale(offset, offset, window.devicePixelRatio || 1);
        vec4.transformMat4(offset, offset, screenToProj);
        shift(vec3.fromValues(offset[0], offset[1], offset[2]));
        vec4.set(dragPos, window.event.clientX, window.event.clientY, 0., 0.);
    }
}

var zoomIncrement = 1.5;
function setZoom(z) {
    zoom = bound(1., z, 100.);
    matrixChanged = true;
    return zoom;
}

function zoomInTo(pos) {
    var oldZoom = zoom;
    zoomFixedPoint(pos, oldZoom / setZoom(zoom * zoomIncrement));
}

function zoomOutFrom(pos) {
    var oldZoom = zoom;
    zoomFixedPoint(pos, oldZoom / setZoom(zoom / zoomIncrement));
}

function zoomFixedPoint(pt, factor) {
    var offset = vec4.create();
    var trans = vec4.create();
    vec4.transformMat4(trans, pt, screenToProj);
    vec4.subtract(offset, trans, cameraLoc);
    vec4.scale(offset, offset, 1 - factor);
    offset[3] = 1.;
    shift(offset);
}

function shift(vec) {
    vec3.add(cameraLoc, cameraLoc, vec);
    vec3.add(lookAt, lookAt, vec);
    limitCenter();
    matrixChanged = true;
}

function limitCenter() {
    var totalScale = scale();
    var scaledWidth = worldCoords.width() / totalScale;
    console.log("camera" + cameraLoc);
    console.log(domain.height());
    console.log(worldCoords.height());
    if (scaledWidth <= domain.width()) {
        cameraLoc[0] = bound(domain.left + 0.5 * scaledWidth, cameraLoc[0], domain.right - 0.5 * scaledWidth);
    } else {
        cameraLoc[0] = bound(domain.right - 0.5 * scaledWidth, cameraLoc[0], domain.left + 0.5 * scaledWidth);
    }
    var scaledHeight = worldCoords.height() * aspect / totalScale;
    console.log(domain.top - 0.5 * scaledHeight);
    console.log(scaledHeight);
    if (scaledHeight <= domain.height()) {
        cameraLoc[1] = bound(domain.bottom + 0.5 * scaledHeight, cameraLoc[1], domain.top - 0.5 * scaledHeight);
    } else {
        cameraLoc[1] = bound(domain.top - 0.5 * scaledHeight, cameraLoc[1], domain.bottom + 0.5 * scaledHeight);
    }
    lookAt[0] = cameraLoc[0];
    lookAt[1] = cameraLoc[1];
    console.log(cameraLoc);
    matrixChanged = true;
}

var gl;
function initGL(canvas) {
    gl = WebGLUtils.setupWebGL(canvas, {antialias: true})
    window.onresize = resize;
    gl.canvas.onmousewheel = handleWheel;
    gl.canvas.onmousedown = handleDown;
    gl.canvas.onmouseup = handleUp;
    gl.canvas.onmousemove = handleMove;
    resize();
    if (!gl) {
        alert("Could not initialise WebGL, sorry :-(");
    }
}

var worldCoords = new Rectangle();
worldCoords.left = -1.0;
worldCoords.right = 1.0;
worldCoords.bottom = -1.0;
worldCoords.top = 1.0;

var domain = new Rectangle();
domain.left = -180.;
domain.right = 180.;
domain.bottom = -90.;
domain.top = 90.;

var worldScale = 1.0;
function updateWorldScale() {
    var widthRat = worldCoords.width() / domain.width();
    var heightRat = aspect * worldCoords.height() / domain.height();
    worldScale = widthRat >= heightRat ? widthRat : heightRat;
    matrixChanged = true;
}

var zoom = 1.;
function scale() {
    return zoom * worldScale;
}

var size = {};
var aspect = 1.0;
function resize() {
    var ratio = window.devicePixelRatio || 1;
    gl.canvas.width = gl.canvas.clientWidth * ratio;
    gl.canvas.height = gl.canvas.clientHeight * ratio;

    size.width = gl.canvas.width;
    size.height = gl.canvas.height;
    aspect = size.height / size.width;

    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    updateWorldScale();
    mat4.identity(normMatrix);
    var vec = vec4.fromValues(-1., 1., 0., 0.);
    mat4.translate(normMatrix, normMatrix, vec);
    vec4.set(vec, 2. / size.width, -2. / size.height, 1., 1.);
    mat4.scale(normMatrix, normMatrix, vec);
    matrixChanged = true;
}    

function getShader(gl, id) {
    var shaderScript = document.getElementById(id);
    if (!shaderScript) {
        return null;
    }

    var str = "";
    var k = shaderScript.firstChild;
    while (k) {
        if (k.nodeType == 3) {
            str += k.textContent;
        }
        k = k.nextSibling;
    }

    var shader;
    if (shaderScript.type == "x-shader/x-fragment") {
        shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (shaderScript.type == "x-shader/x-vertex") {
        shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
        return null;
    }

    gl.shaderSource(shader, str);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        alert(gl.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}


var shaderProgram;

function initShaders() {
    var fragmentShader = getShader(gl, "shader-fs");
    var vertexShader = getShader(gl, "shader-vs");

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        alert("Could not initialise shaders");
    }

    gl.useProgram(shaderProgram);

    shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);
    
    shaderProgram.vertexColorAttribute = gl.getAttribLocation(shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);
    shaderProgram.vertexTextureCoordAttribute = gl.getAttribLocation(shaderProgram, "aVertexTextureCoord");
    gl.enableVertexAttribArray(shaderProgram.vertexTextureCoordAttribute);

    shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
    shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
    shaderProgram.samplerUniform = gl.getUniformLocation(shaderProgram, "uSampler");                                                        
}


var mvMatrix = mat4.create();
var mvMatrixStack = [];
var pMatrix = mat4.create();
var screenToProj = mat4.create();
var normMatrix = mat4.create();

function setMatrixUniforms() {
    gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
    gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
}

function mvPushMatrix() {
    var copy = mat4.create();
    mat4.set(mvMatrix, copy);
    mvMatrixStack.push(copy);
}

function mvPopMatrix() {
    if (mvMatrixStack.length == 0) {
        throw "Invalid popMatrix!";
    }
    mvMatrix = mvMatrixStack.pop();
}

var mapPositionBuffer;
var mapTextureCoordBuffer;
var mapColorBuffer;

function initBuffers() {
    mapTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, mapTextureCoordBuffer);
    var textureCoords = [
         0.0,  0.0,
         0.0,  1.0,
         1.0,  0.0,
         1.0,  1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoords), gl.STATIC_DRAW);
    mapTextureCoordBuffer.itemSize = 2;
    mapTextureCoordBuffer.numItems = 4;

    mapColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, mapColorBuffer);
    var colors = [
        0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    mapColorBuffer.itemSize = 4;
    mapColorBuffer.numItems = 4;

    mapPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, mapPositionBuffer);
    var vertices = [
        -180.0, -90.0,
        -180.0,  90.0,
         180.0, -90.0,
         180.0,  90.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    mapPositionBuffer.itemSize = 2;
    mapPositionBuffer.numItems = 4;
}

function handleLoadedTexture(texture) {
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture.image);
    // Wrapping and filtering settings important to supporting non-POT texture
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);
    texture.initialized = true;
}

var marbleTexture;
function initTexture() {
    marbleTexture = gl.createTexture();
    marbleTexture.image = new Image();
    marbleTexture.image.onload = function() {
        handleLoadedTexture(marbleTexture)
    }
    marbleTexture.image.src = "static/world.topo.bathy.200406.3x5400x2700.png";
}

var matrixChanged = true;
function updateMatrix() {
    var scaler = vec3.create();
    vec3.set(scaler, scale(), scale(), 1.);
    mat4.ortho(pMatrix, worldCoords.left, worldCoords.right,
        aspect * worldCoords.bottom, aspect * worldCoords.top, 10., -10.);
    mat4.scale(pMatrix, pMatrix, scaler);

    mat4.lookAt(mvMatrix, cameraLoc, lookAt, up);

    mat4.multiply(screenToProj, pMatrix, mvMatrix);
    mat4.invert(screenToProj, screenToProj);
    mat4.multiply(screenToProj, screenToProj, normMatrix);
    matrixChanged = false;
}

function drawScene() {
    stats.begin();
    if (matrixChanged) {
        updateMatrix();
    }

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Setup pos
    gl.bindBuffer(gl.ARRAY_BUFFER, mapPositionBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, mapPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, mapColorBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexColorAttribute, mapColorBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, mapTextureCoordBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexTextureCoordAttribute, mapTextureCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);

    if (marbleTexture.initialized) {
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, marbleTexture);
        gl.uniform1i(shaderProgram.samplerUniform, 0);
    }

    setMatrixUniforms();
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, mapPositionBuffer.numItems);
    
    stats.end();
}

var lastTime = 0;
function animate() {
    var timeNow = new Date().getTime();
    if (lastTime != timeNow) {
        var elapsed = timeNow - lastTime;
    }
    lastTime = timeNow;
}

function tick() {
    requestAnimFrame(tick);
    drawScene();
    // animate();
}

function webGLStart() {
    var canvas = document.getElementById("canvas");
    initGL(canvas);
    initShaders();
    initBuffers();
    initTexture();
    initStats();

    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);

    tick();
}


var stats;
function initStats() {
    stats = new Stats();
    stats.setMode(1); // 0: fps, 1: ms

    // Align top-left
    stats.domElement.style.position = 'absolute';
    stats.domElement.style.left = '0px';
    stats.domElement.style.top = '0px';
    document.body.appendChild( stats.domElement );
}
