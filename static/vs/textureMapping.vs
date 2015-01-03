attribute vec3 aVertexPosition;
// attribute vec4 aVertexColor;
attribute vec2 aVertexTextureCoord;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

varying vec2 vTextureCoord;
varying vec4 vColor;

void main(void) {
    gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
    vTextureCoord = aVertexTextureCoord;
    // vColor = aVertexColor;
}