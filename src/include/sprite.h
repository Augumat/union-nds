#pragma once
#include <nds.h>

static const int SPRITE_DMA_CHANNEL = 3;

struct SpriteInfo {
    int oamId;
    int width;
    int height;
    int angle;
    SpriteEntry* entry;
};

/* OAM utility methods. */
void initOAM(OAMTable* oam);
void updateOAM(OAMTable* oam);

/* Modify sprite visibility. */
void setSpriteVisibility(
    SpriteEntry* entry,
    bool hidden,
    bool affine = false,
    bool doubleBound = false
);

/* Rotate an affine sprite. */
void rotateSprite(SpriteRotation* rot, int angle);
