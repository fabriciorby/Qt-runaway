#version 410

uniform sampler2D colorTexture;

in vec2 v2fcolor;

out vec4 myfragcolor;

void main()
{
    myfragcolor = texture(colorTexture, v2fcolor);
}
