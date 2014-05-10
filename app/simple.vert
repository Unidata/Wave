#version 400

in vec4 vertex;
in vec4 color;

uniform mat4 mvp;
out vec4 fragColor;

void main(void)
{
    gl_Position = mvp * vertex;
//    gl_Position = vertex;
    fragColor = color;
}
