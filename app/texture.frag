#version 400

out vec4 color;
uniform sampler2D texture;
in vec2 texCoord;

void main(void)
{
    color = texture2D(texture, texCoord);
}
