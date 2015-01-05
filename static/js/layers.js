"use strict";

function RasterImageLayer(kernel, code) {
    this.kernel = kernel;
    this.pythonCode = code;
    this.alpha = 1.0;
    this.initShaders();
    this.initBuffers();
    this.initTexture();
    this.requestData();
}

RasterImageLayer.prototype.genColorMap = function(canvas) {
    var colors = new Uint8Array([0x00, 0x00, 0x00, 0x00,
      0xcc, 0xff, 0xff, 0xff,
      0xcc, 0x99, 0xcc, 0xff,
      0x99, 0x66, 0x99, 0xff,
      0x66, 0x33, 0x66, 0xff,
      0xcc, 0xcc, 0x99, 0xff,
      0x99, 0x99, 0x66, 0xff,
      0x64, 0x64, 0x64, 0xff,
      0x04, 0xe9, 0xe7, 0xff,
      0x01, 0x9f, 0xf4, 0xff,
      0x03, 0x00, 0xf4, 0xff,
      0x02, 0xfd, 0x02, 0xff,
      0x01, 0xc5, 0x01, 0xff,
      0x00, 0x8e, 0x00, 0xff,
      0xfd, 0xf8, 0x02, 0xff,
      0xe5, 0xbc, 0x00, 0xff,
      0xfd, 0x95, 0x00, 0xff,
      0xfd, 0x00, 0x00, 0xff,
      0xd4, 0x00, 0x00, 0xff,
      0xbc, 0x00, 0x00, 0xff,
      0xf8, 0x00, 0xfd, 0xff,
      0x98, 0x54, 0xc6, 0xff,
      0xfd, 0xfd, 0xfd, 0xff]);
    this.cmap = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.cmap);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, colors.length / 4, 1, 0, gl.RGBA,
        gl.UNSIGNED_BYTE, colors);

    // Wrapping and filtering settings important to supporting non-POT texture
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);

    gl.uniform1i(this.shader.cmapUniform, 1);
    gl.uniform1i(this.shader.useCmap, true);
}

RasterImageLayer.prototype.draw = function(canvas) {
    // Setup pos
    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapPositionBuffer);
    gl.vertexAttribPointer(this.shader.vertexPositionAttribute,
        this.mapPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    // gl.bindBuffer(gl.ARRAY_BUFFER, this.mapColorBuffer);
    // gl.vertexAttribPointer(this.shader.vertexColorAttribute,
    //     this.mapColorBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.mapTextureCoordBuffer);
    gl.vertexAttribPointer(this.shader.vertexTextureCoordAttribute,
        this.mapTextureCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.useProgram(this.shader);

    if (this.texture.initialized) {
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
    }

    if (this.cmap) {
        gl.activeTexture(gl.TEXTURE1);
        gl.bindTexture(gl.TEXTURE_2D, this.cmap);
    }

    gl.uniformMatrix4fv(this.shader.pMatrixUniform, false, canvas.pMatrix);
    gl.uniformMatrix4fv(this.shader.mvMatrixUniform, false, canvas.mvMatrix);
    gl.uniform1f(this.shader.alphaUniform, this.alpha);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, this.mapPositionBuffer.numItems);
}

RasterImageLayer.prototype.initShaders = function(view) {
    this.shader = new ShaderProgram("static/fs/textureMapping.fs",
        "static/vs/textureMapping.vs");
    gl.useProgram(this.shader);

    this.shader.vertexPositionAttribute = gl.getAttribLocation(this.shader, "aVertexPosition");
    gl.enableVertexAttribArray(this.shader.vertexPositionAttribute);
    
    // this.shader.vertexColorAttribute = gl.getAttribLocation(this.shader, "aVertexColor");
    // gl.enableVertexAttribArray(this.shader.vertexColorAttribute);

    this.shader.vertexTextureCoordAttribute = gl.getAttribLocation(this.shader, "aVertexTextureCoord");
    gl.enableVertexAttribArray(this.shader.vertexTextureCoordAttribute);

    this.shader.pMatrixUniform = gl.getUniformLocation(this.shader, "uPMatrix");
    this.shader.mvMatrixUniform = gl.getUniformLocation(this.shader, "uMVMatrix");
    this.shader.alphaUniform = gl.getUniformLocation(this.shader, "uAlpha");
    this.shader.cmapUniform = gl.getUniformLocation(this.shader, "uCmap");
    this.shader.useCmap = gl.getUniformLocation(this.shader, "uUseCmap");

    this.shader.samplerUniform = gl.getUniformLocation(this.shader, "uSampler");
    gl.uniform1i(this.shader.samplerUniform, 0);
}

RasterImageLayer.prototype.initBuffers = function() {
    this.mapTextureCoordBuffer = new Buffer(
        [[0.0,  0.0], [0.0,  1.0], [1.0,  0.0], [1.0,  1.0]]);

    // this.mapColorBuffer = new Buffer(
    //     [[0.0, 0.0, 0.0, 1.0], [0.0, 1.0, 0.0, 1.0],
    //      [0.0, 0.0, 1.0, 1.0], [0.0, 1.0, 1.0, 1.0]]);

    // Can we eliminate this placeholder?
    this.mapPositionBuffer = new Buffer([[0., 0.], [0., 0.], [0., 0.], [0., 0.]]);
}

RasterImageLayer.prototype.loadTextureData = function() {
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE,
        this.texture.image);

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
    // this.texture.image = new Image();
    // this.texture.image.onload = this.loadTextureData.bind(this);
}

RasterImageLayer.prototype.handleData = function(msg_type, content) {
    // callback for updating the texture with new data
    logit(msg_type, content)
    // console.log(content)
    if (msg_type == 'display_data') {
        var payload = content['data'];
        var data = parseImage(payload);
        if (data != null) {
            this.texture.image.src = data;
        } else if (payload['application/json'] != null) {
            var obj = JSON.parse(payload['application/json']);
            if (obj.bbox != null) {
                this.loadPosition(obj.bbox);
            }
            if (obj.image != null) {
                this.loadImageData(obj.image);
            }
        }
    }
}

RasterImageLayer.prototype.loadPosition = function(bbox) {
    console.log(bbox);
    this.mapPositionBuffer = new Buffer(bbox);
}

RasterImageLayer.prototype.loadImageData = function(image) {
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    // gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
    var imgData = new Uint8Array(image.data);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl[image.format], image.shape[1], image.shape[0],
        0, gl[image.format], gl[image.type], imgData);

    // Wrapping and filtering settings important to supporting non-POT texture
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.bindTexture(gl.TEXTURE_2D, null);
    this.texture.initialized = true;
}

RasterImageLayer.prototype.requestData = function(kernel) {
    this.kernel.execute(this.pythonCode,
        {'output': this.handleData.bind(this)});
}