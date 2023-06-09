#include "brick_selector_widget.h"
#include "map.h"
#include "types.h"
#include "globals.h"

BrickSelectorWidget::BrickSelectorWidget() {
    BrickSprites = nullptr;
    iCount = 0;
    iBrickStartIndex = 0;
    SelectedIndex = 0;
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
    SelectedBrick.Load("textures/selected_brick.png");
}

void BrickSelectorWidget::Draw() {
    const int ICON_SIZE = 48;
    const int MAX_ICONS_SHOWN = 3;

    SelectedIndex = (SelectedIndex + mouseWheelDelta) % iCount;

    Rect r, c;
    c.x = c.y = 0; c.w = c.h = 16;
    r.w = r.h = ICON_SIZE;
    r.y = WIN_H - (ICON_SIZE * 1.5);

    r.x = (WIN_W / 2) - ((ICON_SIZE * MAX_ICONS_SHOWN) / 2);
    

    // Draw the backdrop
    Rect backDrop = r;
    backDrop.x -= 2;
    backDrop.w = (ICON_SIZE+2) * MAX_ICONS_SHOWN;
    backDrop.w += 2;
    backDrop.y -= 2;
    backDrop.h += 4;
    BlankSprite.Draw(backDrop, c);

    // Draw the icons   
    int EndIndex = (iBrickStartIndex + MAX_ICONS_SHOWN) > iCount ? iCount : (iBrickStartIndex + MAX_ICONS_SHOWN);
    for(int x = iBrickStartIndex;x < EndIndex;x++) {
        BrickSprites[x].Draw(r,c);
        r.x += ICON_SIZE+2;
    }

    // Draw the selected brick marker
    r.w = r.h = ICON_SIZE;
    r.y = WIN_H - (ICON_SIZE * 1.5);

    r.x = (WIN_W / 2) - ((ICON_SIZE * MAX_ICONS_SHOWN) / 2);
    r.x += SelectedIndex * ((ICON_SIZE == 0) ? ICON_SIZE : ICON_SIZE+2);
    SelectedBrick.Draw(r,c);
}
