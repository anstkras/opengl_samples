#version 330 core

in vec4 gl_FragCoord;

out vec4 frag_color;

uniform int max_iterations;
uniform vec2 screen_size;
uniform vec2 offset;
uniform float zoom;
uniform sampler1D u_tex;

int get_iterations(vec2 v)
{
    float real = v.x;
    float imag = v.y;

    int i = 0;
    float const_real = real;
    float const_imag = imag;

    while (i < max_iterations)
    {
        float tmp_real = real;
        real = (real * real - imag * imag) + const_real;
        imag = (2.0 * tmp_real * imag) + const_imag;

        float dist = real * real + imag * imag;

        if (dist > 100.0)
        break;

        ++i;
    }
    return i;
}

vec4 return_color(vec2 v)
{
    int iter = get_iterations(v);
    if (iter == max_iterations)
    {
        gl_FragDepth = 0.0f;
        return vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    float iterations = float(iter) / max_iterations;
    return texture(u_tex, iterations);
}

void main()
{
    vec2 coord = vec2(gl_FragCoord.xy);
    frag_color = return_color(((coord - screen_size/ 2)/ zoom) - offset);
}