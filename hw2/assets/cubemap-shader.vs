#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 tex_coords;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    tex_coords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}