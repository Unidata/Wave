"use strict";

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
function initShaders(view) {
    var fragmentShader = getShader(gl, "shader-fs");
    var vertexShader = getShader(gl, "shader-vs");

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        alert("Could not initialize shaders");
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


function setMatrixUniforms(view) {
    gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, view.pMatrix);
    gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, view.mvMatrix);
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
    // marbleTexture.image.src = "static/world.topo.bathy.200406.3x5400x2700.png";
}

function drawScene() {
    stats.begin();
    if (canvas.matrixChanged) {
        canvas.updateMatrix();
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

    setMatrixUniforms(canvas);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, mapPositionBuffer.numItems);
    
    stats.end();
}

function tick() {
    requestAnimFrame(tick);
    drawScene();
}

var canvas;
function webGLStart() {
    canvas = new ViewCanvas(document.getElementById("canvas"));
    initShaders();
    initBuffers();
    initTexture();
    initStats();
    initIPython();

    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);

    tick();
}

function init_namespace(kernel) {
    var code = "import json\n" +
    "from IPython.core.display import JSON, Image, display\n" +
    "from numpy import linspace\n" +
    "import wave\n" +
    "wave.foo()\n" +
    "def update_plot(n, xmax=10, npoints=200):\n" +
    "    print n, xmax, npoints\n" +
    "    display(Image('static/world.topo.bathy.200406.3x5400x2700.png'))";
    kernel.execute(code, {'output': logit});
}

function logit(msg_type, content) {
    if (msg_type == 'stream') {
        console.log(content['data']);
    }
}

function update_plot(msg_type, content){
    // callback for updating the plot with the output of the request
    if (msg_type !== 'display_data')
        return;
    var lines = content['data']['image/png'];//['application/image'];
    marbleTexture.image.src = "data:image/jpeg;base64," + lines;
    // marbleTexture.image.src = "static/world.topo.bathy.200406.3x5400x2700.png";
    // console.log(lines);
};

function do_request_update(kernel){
    var args = 5 + ", xmax=" + 10 + ", npoints=" + 15;
    kernel.execute("update_plot(" + args + ")", {'output': update_plot});
};

var request_update;
function initIPython() {
    $([IPython.events]).on('status_started.Kernel', function(evt, data) {
        setTimeout(function() {
            init_namespace(data.kernel);
            do_request_update(data.kernel);
        }, 500);
    });
    
    var kernel = new IPython.Kernel('/kernels');
    
    request_update = function() {
        do_request_update(kernel);
    }

    var ws_url = 'ws' + document.location.origin.substring(4);
    setTimeout(function() {
        kernel._kernel_started({kernel_id: '1', ws_url: ws_url});
    }, 500);
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