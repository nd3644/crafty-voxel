#ifndef BRICK_SELECTOR_WIDGET_H
#define BRICK_SELECTOR_WIDGET_H

#include <vector>
#include <array>
#include "sprite.h"

class Map;
class BrickSelectorWidget {
    public:
        BrickSelectorWidget();
        ~BrickSelectorWidget();

        void Init(Map &map);
        void Draw();

        int GetSelectedBrickID() const;
    private:
        Eternal::Sprite BrickSprites[16];
        Eternal::Sprite BlankSprite, SelectedBrick;
        int iCount;
        int iBrickStartIndex;
        int SelectedIndex;

        std::vector<std::array<int,6>>BrickTextureIndices;
};

#endif
