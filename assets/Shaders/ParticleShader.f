#version 330 core

struct Material {
    sampler2D texture_diffuse1;
};

uniform Material material;

in vec2 textureCoords_o;
in vec4 particleColor_o;

void main() {
    gl_FragColor = texture2D(material.texture_diffuse1, textureCoords_o) * particleColor_o;
}
