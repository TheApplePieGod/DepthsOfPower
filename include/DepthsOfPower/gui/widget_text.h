#pragma once
#include <DepthsOfPower/gui/widget.h>
#include <string>

// this widget will not render children
class widget_text: public widget
{
public:
    widget_text(glm::vec2 pos, glm::vec2 fontSize, const char* text);

    void Draw(bool isParent, glm::vec2 currentOffset = { 0.f, 0.f }, glm::vec2 parentBounds = { 0.f, 0.f }) override; // todo: propogate resize down to simplify draw calculations

    void SetText(const char* newText);
    inline void SetFontSize(glm::vec2 newSize) { fontSize = newSize; };

    // defaults to blocked and even spacing. Can be adjusted both positively and negatively
    inline void SetCharacterSpacing(f32 newSpacing) { characterSpacing = newSpacing; };
    inline void SetWordSpacing(f32 newSpacing) { wordSpacing = newSpacing; };
    inline void SetLineSpacing(f32 newSpacing) { lineSpacing = newSpacing; };

    const char* GetText();
    inline glm::vec2 GetFontSize() { return fontSize; };
    inline f32 GetCharacterSpacing() { return characterSpacing; };
    inline f32 GetWordSpacing() { return wordSpacing; };
    inline f32 GetLineSpacing() { return lineSpacing; };

private:
    std::vector<std::string> words;
    std::vector<glm::vec4> offsetScales;
    std::vector<glm::vec4> texCoords;
    std::vector<int> textureIndexes;

    int characterCount;
    glm::vec2 fontSize;
    f32 characterSpacing;
    f32 wordSpacing;
    f32 lineSpacing;

};