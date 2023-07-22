#include "brick_selector_widget.h"
#include "map.h"
#include "types.h"
#include "globals.h"

BrickSelectorWidget::BrickSelectorWidget() {
    iCount = 0;
    iBrickStartIndex = 0;
    SelectedIndex = 0;
}

BrickSelectorWidget::~BrickSelectorWidget() {
}

void BrickSelectorWidget::Init(Map &map) {    
    auto filenames = map.GetTextureFilenames();

    for(size_t i = 0;i < filenames.size();i++) {
        BrickSprites[i].Load(filenames[i]);
    }
    std::cout << "loaded " << filenames.size() << " sprites into BrickSelectorWidget" << std::endl;

    BlankSprite.Load("textures/blank.png");
    SelectedBrick.Load("textures/selected_brick.png");

    BrickTextureIndices = map.GetLookupArr();
    iCount = BrickTextureIndices.size() - 1;
}

void BrickSelectorWidget::Draw() {
    const int ICON_SIZE = 48;
    const int MAX_ICONS_SHOWN = iCount;

    SelectedIndex = (SelectedIndex - mouseWheelDelta);
    if(SelectedIndex < 0)
        SelectedIndex = 0;
    if(SelectedIndex >= iCount)
        SelectedIndex = iCount - 1;

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
    iBrickStartIndex = (SelectedIndex > MAX_ICONS_SHOWN) ? (SelectedIndex - MAX_ICONS_SHOWN) : 0;
    int EndIndex = (iBrickStartIndex + MAX_ICONS_SHOWN);
    if(EndIndex > iCount)
        EndIndex = iCount;

    for(int x = iBrickStartIndex;x < EndIndex;x++) {
        // Use the front face of the cube
        int index = BrickTextureIndices[x][2];
        BrickSprites[index].Draw(r,c);
        r.x += ICON_SIZE+2;
    }

    // Draw the selected brick marker
    r.w = r.h = ICON_SIZE;
    r.y = WIN_H - (ICON_SIZE * 1.5);

    r.x = (WIN_W / 2) - ((ICON_SIZE * MAX_ICONS_SHOWN) / 2);
    r.x += SelectedIndex * ((ICON_SIZE == 0) ? ICON_SIZE : ICON_SIZE+2);
    SelectedBrick.Draw(r,c);
}

int BrickSelectorWidget::GetSelectedBrickID() const {
    return SelectedIndex+1;
}