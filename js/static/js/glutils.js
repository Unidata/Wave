"use strict";

function matrixStack() {
	this.stack = [];
}

matrixStack.prototype.push = function(matrix) {
	var copy = mat4.create();
	mat4.set(matrix, copy);
	stack.push(copy);
}

matrixStack.prototype.pop = function() {
    if (this.stack.length == 0) {
        throw "Invalid pop matrix!";
    }
    return stack.pop();
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


function ShaderProgram(fs, vs) {
    var fragmentShader = getShader(gl, fs);
    var vertexShader = getShader(gl, vs);

    var prog = gl.createProgram();
    gl.attachShader(prog, vertexShader);
    gl.attachShader(prog, fragmentShader);
    gl.linkProgram(prog);

    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
        alert("Error linking program!");
    }

    return prog;
}

function Buffer(data, Converter) {
	// Default to making Float32Arrays
	if (Converter == undefined) { Converter = Float32Array }

	// Flatten data for handing to WebGL
	var flattened = data.reduce(function(a, b) { return a.concat(b); }, []);

	// Create new buffer and load data
	var buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Converter(flattened), gl.STATIC_DRAW);

    // Store data size and stride for easy use later
    buffer.itemSize = data[0].length;
    buffer.numItems = data.length;
    return buffer;
}