#version 400

in vec4 vertex;
in vec4 texc;
uniform mat4 mvp;
out vec2 texCoord;

void main(void)
{
    gl_Position = mvp * vertex;
    texCoord = texc.st;
}

