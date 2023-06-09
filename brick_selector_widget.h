#ifndef BRICK_SELECTOR_WIDGET_H
#define BRICK_SELECTOR_WIDGET_H

#include <vector>
#include "sprite.h"

class Map;
class BrickSelectorWidget {
    public:
        BrickSelectorWidget();
        ~BrickSelectorWidget();

        void Init(Map &map);
        void Draw();
    private:
        Eternal::Sprite *BrickSprites;
        Eternal::Sprite BlankSprite;
        int iCount;
};

#endif
