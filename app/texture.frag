#version 400

out vec4 color;
uniform sampler2D tex;
in vec2 texCoord;

void main(void)
{
    color = texture(tex, texCoord);
}
