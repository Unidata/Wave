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