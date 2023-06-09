#include "brick_selector_widget.h"
#include "map.h"
#include "types.h"
#include "globals.h"

BrickSelectorWidget::BrickSelectorWidget() {
    BrickSprites = nullptr;
    iCount = 0;
}

BrickSelectorWidget::~BrickSelectorWidget() {
    if(BrickSprites != nullptr) {
        delete[] BrickSprites;
    }
}

void BrickSelectorWidget::Init(Map &map) {    
    auto filenames = map.GetTextureFilenames();

    iCount = filenames.size();
    BrickSprites = new Eternal::Sprite[iCount];
    for(int i = 0;i < iCount;i++) {
        BrickSprites[i].Load(filenames[i]);
    }

    BlankSprite.Load("textures/blank.png");
}

void BrickSelectorWidget::Draw() {
    std::cout << "count: " << iCount << std::endl;


    const int ICON_SIZE = 48;
    Rect r, c;
    c.x = c.y = 0; c.w = c.h = 16;
    r.w = r.h = ICON_SIZE;
    r.y = WIN_H - (ICON_SIZE * 1.5);

    r.x = (WIN_W / 2) - ((ICON_SIZE * iCount) / 2);
    
    Rect backDrop = r;
    backDrop.x -= 2;
    backDrop.w = (ICON_SIZE+2) * iCount;
    backDrop.w += 2;
    backDrop.y -= 2;
    backDrop.h += 4;
    BlankSprite.Draw(backDrop, c);
    
    for(int x = 0;x < iCount;x++) {
        BrickSprites[x].Draw(r,c);
        r.x += ICON_SIZE+2;
    }
}
