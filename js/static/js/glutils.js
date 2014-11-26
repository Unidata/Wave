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


function loadShader(gl, url) {
	var shader;
	function loadSource() {
		if (this.readyState == this.DONE && this.status == 200) {
			var type = url.slice(-2);
			if (type == 'fs') {
				shader = gl.createShader(gl.FRAGMENT_SHADER);
			} else if (type == 'vs') {
				shader = gl.createShader(gl.VERTEX_SHADER);
			} else {
				console.warn('Could not determine shader type for: ' + url);
				return;
			}

			gl.shaderSource(shader, this.responseText);
			gl.compileShader(shader);

			if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
				alert(gl.getShaderInfoLog(shader));
				return null;
			}
		}
	}
	var xhr = new XMLHttpRequest();
	xhr.open('get', url, false);
	xhr.send(null);
	loadSource.call(xhr);

    return shader;
}


function ShaderProgram(fs, vs) {
    var fragmentShader = loadShader(gl, fs);
    var vertexShader = loadShader(gl, vs);

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

