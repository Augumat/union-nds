#include <nds.h>
#include <nds/arm9/trig_lut.h>
#include "sprite.h"

void initOAM(OAMTable* oam)
{
    // clear old sprite data
    for (int i = 0; i < SPRITE_COUNT; i++) {
        oam->oamBuffer[i].attribute[0] = ATTR0_DISABLED;
        oam->oamBuffer[i].attribute[1] = 0;
        oam->oamBuffer[i].attribute[2] = 0;
    }
    // set transformation matrix to the identity matrix
    for (int i = 0; i < MATRIX_COUNT; i++) {
        oam->matrixBuffer[i].hdx = 1 << 8;
        oam->matrixBuffer[i].hdy = 0;
        oam->matrixBuffer[i].vdx = 0;
        oam->matrixBuffer[i].vdy = 1 << 8;
    }
    // update the initialized sprite
    updateOAM(oam);
}

void updateOAM(OAMTable* oam)
{
    DC_FlushAll();
    dmaCopyHalfWords(
        SPRITE_DMA_CHANNEL,
        oam->oamBuffer,
        OAM,
        SPRITE_COUNT * sizeof(SpriteEntry)
    );
}

void setSpriteVisibility(SpriteEntry* entry, bool hidden, bool affine, bool doubleBound)
{
    if (hidden) {
        entry->isRotateScale = false; // disable rotation (affine sprites cannot be hidden)
        entry->isHidden = true; // hide the sprite
    } else {
        if (affine) {
            entry->isRotateScale = true; // re-enable rotation (this also unhides the sprite)
            entry->isSizeDouble = doubleBound; // set double bound if specified
        } else {
            entry->isHidden = false; // unhide the sprite
        }
    }
}

void rotateSprite(SpriteRotation* rot, int angle)
{
    s16 s = sinLerp(angle) >> 4;
    s16 c = cosLerp(angle) >> 4;
    rot->hdx =  c; rot->hdy =  s;
    rot->vdx = -s; rot->vdy =  c;
}
