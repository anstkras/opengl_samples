#version 330 core

in vec3 tex_coords;
uniform samplerCube cubemap;

void main()
{
    gl_FragColor = texture(cubemap, tex_coords);
}