#version 330 core

in vec2 textureCoords_o;

out vec4 color;

uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

//The format of my engine specifies the textures as
//
// texture_TYPEi
//
//where TYPE is the type of the texture, and i is the index.
//
//as an example, for a normal map, you would do
//
// texture_normals1, texture_normals2, texture_normals3, and so on.

void main() {
    color = vec4(texture(texture_diffuse1, textureCoords_o));
}
