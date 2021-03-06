#version 330 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 textureCoords;
layout(location = 5) in vec3 particlePosition;
layout(location = 6) in float scale;
layout(location = 7) in vec4 particleColor;

uniform mat4 view;
uniform mat4 projection;
out vec2 textureCoords_o;
out vec4 particleColor_o;

void main() {
    mat4 model = mat4(1.0);

    model[3][0] = particlePosition.x;
    model[3][1] = particlePosition.y;
    model[3][2] = particlePosition.z;
    model[3][3] = 1.0;

    mat4 modelView = view * model;

    //This is fine, but we could be doing this in the source rather than this shader.
    //The alternative is making the model's 3x3 the transpose of the view matrix's 3x3. (upper left 3x3)

    //This makes the particle always face you.

    //First row.
    modelView[0][0] = 1.0;
    modelView[0][1] = 0.0;
    modelView[0][2] = 0.0;

    //Second row.
    modelView[1][0] = 0.0;
    modelView[1][1] = 1.0;
    modelView[1][2] = 0.0;

    //Thrid row.
    modelView[2][0] = 0.0;
    modelView[2][1] = 0.0;
    modelView[2][2] = 1.0;

    gl_Position     = projection * modelView * vec4(position * scale, 1.0);
    textureCoords_o = vec2(textureCoords.x, 1.0 - textureCoords.y);
    particleColor_o = particleColor;
}
