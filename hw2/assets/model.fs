#version 330 core

out vec4 o_frag_color;

in vec3 normal;
in vec3 position;

uniform vec3 camera_position;
uniform samplerCube cubemap;
uniform float refractive_index;
uniform bool is_refractive;

void main()
{
    vec3 N = normalize(normal);
    vec3 I = normalize(position - camera_position);
    vec3 refl = reflect(I, normalize(normal));
    vec3 reflect_texture = texture(cubemap, refl).rgb;

    if (!is_refractive) {
        o_frag_color = vec4(reflect_texture, 1.0);
        return;
    }

    vec3 refr = refract(I, normalize(normal), 1.0 / refractive_index);

    vec3 refract_texture = texture(cubemap, refr).rgb;

    float cosi = abs(dot(I, N));
    float cost = abs(dot(refr, -N));

    float etat = 1.0;
    float etai = refractive_index;
    float rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
    float rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

    float kr = (rs * rs + rp * rp) / 2;

    vec3 color = (1 - kr) * refract_texture + kr * reflect_texture;
    o_frag_color = vec4(color, 1.0);
}
