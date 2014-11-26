"use strict";

function RasterImageLayer(kernel) {
    this.kernel = kernel;
    this.initShaders();
    this.initBuffers();
    this.initTexture();
    this.requestData();
}

RasterImageLayer.prototype.draw = function(canvas) {
    // Setup pos
    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapPositionBuffer);
    gl.vertexAttribPointer(this.shaderProgram.vertexPositionAttribute,
        this.mapPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapColorBuffer);
    gl.vertexAttribPointer(this.shaderProgram.vertexColorAttribute,
        this.mapColorBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapTextureCoordBuffer);
    gl.vertexAttribPointer(this.shaderProgram.vertexTextureCoordAttribute,
        this.mapTextureCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.useProgram(this.shaderProgram);

    if (this.texture.initialized) {
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
    }

    gl.uniformMatrix4fv(this.shaderProgram.pMatrixUniform, false, canvas.pMatrix);
    gl.uniformMatrix4fv(this.shaderProgram.mvMatrixUniform, false, canvas.mvMatrix);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, this.mapPositionBuffer.numItems);
}

RasterImageLayer.prototype.initShaders = function(view) {
    var fragmentShader = getShader(gl, "shader-fs");
    var vertexShader = getShader(gl, "shader-vs");

    this.shaderProgram = gl.createProgram();
    gl.attachShader(this.shaderProgram, vertexShader);
    gl.attachShader(this.shaderProgram, fragmentShader);
    gl.linkProgram(this.shaderProgram);

    if (!gl.getProgramParameter(this.shaderProgram, gl.LINK_STATUS)) {
        alert("Could not initialize shaders");
    }

    gl.useProgram(this.shaderProgram);
    this.shaderProgram.vertexPositionAttribute = gl.getAttribLocation(this.shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(this.shaderProgram.vertexPositionAttribute);
    
    this.shaderProgram.vertexColorAttribute = gl.getAttribLocation(this.shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(this.shaderProgram.vertexColorAttribute);
    this.shaderProgram.vertexTextureCoordAttribute = gl.getAttribLocation(this.shaderProgram, "aVertexTextureCoord");
    gl.enableVertexAttribArray(this.shaderProgram.vertexTextureCoordAttribute);

    this.shaderProgram.pMatrixUniform = gl.getUniformLocation(this.shaderProgram, "uPMatrix");
    this.shaderProgram.mvMatrixUniform = gl.getUniformLocation(this.shaderProgram, "uMVMatrix");
    this.shaderProgram.samplerUniform = gl.getUniformLocation(this.shaderProgram, "uSampler");                                                        

    gl.uniform1i(this.shaderProgram.samplerUniform, 0);
}

RasterImageLayer.prototype.initBuffers = function() {
    this.mapTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapTextureCoordBuffer);
    var textureCoords = [
         0.0,  0.0,
         0.0,  1.0,
         1.0,  0.0,
         1.0,  1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoords), gl.STATIC_DRAW);
    this.mapTextureCoordBuffer.itemSize = 2;
    this.mapTextureCoordBuffer.numItems = 4;

    this.mapColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapColorBuffer);
    var colors = [
        0.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0,
        0.0, 1.0, 1.0, 1.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    this.mapColorBuffer.itemSize = 4;
    this.mapColorBuffer.numItems = 4;

    this.mapPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapPositionBuffer);
    var vertices = [
        -180.0, -90.0,
        -180.0,  90.0,
         180.0, -90.0,
         180.0,  90.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    this.mapPositionBuffer.itemSize = 2;
    this.mapPositionBuffer.numItems = 4;
}

RasterImageLayer.prototype.loadTextureData = function() {
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.texture.image);

    // Wrapping and filtering settings important to supporting non-POT texture
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);
    this.texture.initialized = true;
}

RasterImageLayer.prototype.initTexture = function() {
    this.texture = gl.createTexture();
    this.texture.image = new Image();
    this.texture.image.onload = this.loadTextureData.bind(this);
}

RasterImageLayer.prototype.handleBinary = function(msg_type, content) {
    // callback for updating the texture with new data
    logit(msg_type, content)
    if (msg_type == 'display_data') {
        this.texture.image.src = parseImage(content['data']);
    }
}

RasterImageLayer.prototype.requestData = function(kernel) {
    this.kernel.execute('wave.blueMarble()',
        {'output': this.handleBinary.bind(this)});
}