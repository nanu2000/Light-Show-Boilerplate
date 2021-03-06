#include "GuiTextCharacter.h"

void GuiTextCharacter::render(Shader& shader, Texture& texture, const Glyph& glyph, const glm::vec2& scale) {
    quad.render3D(
        shader,
        texture,
        glm::vec4(glyph.x, glyph.y, glyph.getWidth(), glyph.getHeight()),
        glm::vec2(xPosition, yPosition),
        glm::vec2(glyph.getWidth() * scale.x, glyph.getHeight() * scale.y));
}
