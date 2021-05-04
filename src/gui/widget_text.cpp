#include <DepthsOfPower/gui/widget_text.h>
#include <DepthsOfPower/engine.h>

extern engine* Engine;

widget_text::widget_text(glm::vec2 pos, glm::vec2 _fontSize, const char* text) : widget(pos, { 0.f, 0.f })
{
    characterSpacing = 0.f;
    fontSize = _fontSize;
    lineSpacing = _fontSize.y;
    wordSpacing = _fontSize.x;
    SetText(text);
}

glm::vec2 widget_text::Draw(bool isParent, glm::vec2 currentOffset, glm::vec2 parentBounds)
{
    if (words.size() > 0)
    {
        std::vector<int> newLineWordIndexes;

        // split up words into each line and add the index of the word that should start the new line into newLineWordIndexes
        f32 tempSize = 0.f;
        for (int i = 0; i < words.size(); i++)
        {
            f32 wordSize = words[i].length() * (fontSize.x + characterSpacing) - characterSpacing;
            if (i != 0 && wordSize + tempSize > parentBounds.x)
            {
                tempSize = 0;
                newLineWordIndexes.push_back(i);
            }
            tempSize += wordSize + wordSpacing; // account for space between words
        }

        const int characterCountX = 16; // based on characters in font atlas
        const int characterCountY = 8;
        const f32 variationStepX = 1.f / characterCountX;
        const f32 variationStepY = 1.f / characterCountY;
        const glm::vec4 baseTextureCoords = { 0.0f, 0.0f, 1.0f / characterCountX, 1.0f / characterCountY }; // min, max for elem 0
        glm::vec2 scaledFontSize = { fontSize.x * MetersToPixels(ScreenMetersX), fontSize.y * MetersToPixels(ScreenMetersX) / Engine->GetRenderer().GetAspectRatio() };
        f32 scaledSpacing = characterSpacing * MetersToPixels(ScreenMetersX);
        f32 scaledWordSpacing = wordSpacing * MetersToPixels(ScreenMetersX);
        f32 scaledLineSpacing = lineSpacing * MetersToPixels(ScreenMetersX) / Engine->GetRenderer().GetAspectRatio();
        int newLineIndex = 0;
        int charIndex = 0;
        glm::vec2 posOffset = { 0.f, 0.f };
        for (int i = 0; i < words.size(); i++)
        {
            // new line if new line word is hit
            if (newLineIndex < newLineWordIndexes.size() && newLineWordIndexes[newLineIndex] == i)
            {
                posOffset = { 0.f, posOffset.y - scaledLineSpacing };
                newLineIndex++;
            }

            for (int c = 0; c < words[i].length(); c++)
            {
                // get char in atlas
                int atlasX = (words[i][c] - 33) % characterCountX;
                int atlasY = (words[i][c] - 33) / characterCountX;

                glm::vec4 offsetScale = {
                    posOffset.x + scaledFontSize.x * 0.5f, posOffset.y - scaledFontSize.y * 0.5f,
                    scaledFontSize
                };
                glm::vec4 texCoord = {
                    baseTextureCoords.x + atlasX * variationStepX,
                    baseTextureCoords.y + atlasY * variationStepY,
                    baseTextureCoords.z + atlasX * variationStepX,
                    baseTextureCoords.w + atlasY * variationStepY,
                };

                posOffset.x += scaledFontSize.x;
                if (c < words[i].length() - 1)
                    posOffset.x += scaledSpacing;

                offsetScales[charIndex] = offsetScale;
                texCoords[charIndex] = texCoord;
                textureIndexes[charIndex] = textureId;
                colors[charIndex] = color;
                charIndex++;
            }

            posOffset.x += scaledWordSpacing;
        }

        glm::vec2 cameraPos = Engine->GetCamera().GetPosition();
        diamond_transform parentTransform = diamond_transform();
        parentTransform.location = { currentOffset.x * MetersToPixels(ScreenMetersX) + cameraPos.x, -currentOffset.y * MetersToPixels(ScreenMetersX) / Engine->GetRenderer().GetAspectRatio() + cameraPos.y }; // transform location
        parentTransform.location += glm::vec2(-MetersToPixels(ScreenMetersX) * 0.5f, MetersToPixels(ScreenMetersX) * 0.5f / Engine->GetRenderer().GetAspectRatio()); // offset location

        Engine->GetRenderer().DrawQuadsOffsetScale(textureIndexes.data(), offsetScales.data(), characterCount, parentTransform, colors.data(), texCoords.data());

        if (newLineWordIndexes.size() == 0)
            return { characterCount * (fontSize.x + characterSpacing) - characterSpacing, fontSize.y };
        else
            return { parentBounds.x, fontSize.y * newLineWordIndexes.size() };
    }

    return { 0.f, 0.f };
}

void widget_text::SetText(const char* newText)
{
    std::string textString = newText;
    words.clear();
    characterCount = 0;

    // separate into words
    std::string temp;
    for (int i = 0; i < textString.length(); i++)
    {
        if ((textString[i] == ' ' && temp.length() > 0) || i == textString.length() - 1)
        {
            if (textString[i] != ' ')
                temp += textString[i];

            words.push_back(temp);
            characterCount += temp.length();
            temp = "";
        }
        else
            temp += textString[i];
    }

    offsetScales.clear();
    textureIndexes.clear();
    texCoords.clear();
    colors.clear();
    textureIndexes.resize(characterCount);
    offsetScales.resize(characterCount);
    texCoords.resize(characterCount);
    colors.resize(characterCount);
}

const char* widget_text::GetText()
{
    std::string temp;
    for (int i = 0; i < words.size(); i++)
    {
        temp += words[i];
        if (i < words.size() - 1)
            temp += ' ';
    }
    return temp.c_str();
}