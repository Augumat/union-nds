#include <nds.h>
#include "sprite.h"
// grit imports (resources)
#include "top.h"
#include "bottom.h"
#include "object.h"

// set a constant for background copying
static const int BG_DMA_CHANNEL = 3;

void initVideo()
{
    // map VRAM
    vramSetMainBanks(
        VRAM_A_MAIN_BG_0x06000000,
        VRAM_B_MAIN_BG_0x06020000,
        VRAM_C_SUB_BG_0x06200000,
        VRAM_D_LCD
    );
    vramSetBankE(VRAM_E_MAIN_SPRITE);

    // set the video mode on the main screen (bottom screen)
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);

    // set the video mode on the sub screen (top screen)
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
}

void initBackgrounds()
{
    // set the main screen background to 16-bit color at low priority
    REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
    // set the main screen affine to the identity matrix
    REG_BG3PA = 1 << 8;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3PD = 1 << 8;
    // set the main screen's position to the origin (top left)
    REG_BG3X = 0;
    REG_BG3Y = 0;

    // set the sub screen background to 16-bit color at low priority
    REG_BG3CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
    // set the sub screen affine to the identity matrix
    REG_BG3PA_SUB = 1 << 8;
    REG_BG3PB_SUB = 0;
    REG_BG3PC_SUB = 0;
    REG_BG3PD_SUB = 1 << 8;
    // set the sub screen's position to the origin (top left)
    REG_BG3X_SUB = 0;
    REG_BG3Y_SUB = 0;
}

void initObjects(OAMTable* oam, SpriteInfo* info)
{
    // set sprite config constants
    static const int BYTES_PER_16_COLOR_TILE = 32;
    static const int COLORS_PER_PALETTE = 16;
    static const int BOUNDARY_VALUE = 32;
    static const int OFFSET_MULTIPLIER = BOUNDARY_VALUE / sizeof(SPRITE_GFX[0]);

    // TODO do this for all 4 "players" / make it expandable

    // create sprite
    SpriteInfo* objectInfo = &info[0];
    SpriteEntry* object = &oam->oamBuffer[0];

    // initialize objectInfo
    objectInfo->oamId  = 0;
    objectInfo->width  = 32;
    objectInfo->height = 32;
    objectInfo->angle  = 0;
    objectInfo->entry  = object;

    // set the object's starting position in the OAM table
    object->x = (SCREEN_WIDTH  / 2) - (objectInfo->width  / 2);
    object->y = (SCREEN_HEIGHT / 2) - (objectInfo->height / 2);

    // specify an affine, 16-color sprite
    object->isRotateScale = true;
    object->isSizeDouble = false;
    object->blendMode = OBJMODE_NORMAL;
    object->isMosaic = false;
    object->colorMode = OBJCOLOR_16;
    object->shape = OBJSHAPE_SQUARE;

    // specify the location of the affine matrix for this object
    object->rotationIndex = objectInfo->oamId;
    object->size = OBJSIZE_32;

    // specify the tile index, layer, and palette to use
    object->gfxIndex = 0;
    object->priority = OBJPRIORITY_0;
    object->palette = objectInfo->oamId;

    // rotate the sprite to its starting angle.
    rotateSprite(&oam->matrixBuffer[objectInfo->oamId], objectInfo->angle);

    // copy over the palette
    dmaCopyHalfWords(
        SPRITE_DMA_CHANNEL,
        objectPal,
        &SPRITE_PALETTE[objectInfo->oamId * COLORS_PER_PALETTE],
        objectPalLen
    );
    // display the sprite
    dmaCopyHalfWords(
        SPRITE_DMA_CHANNEL,
        objectTiles,
        &SPRITE_GFX[object->gfxIndex * OFFSET_MULTIPLIER],
        objectTilesLen
    );
}

void displayBackgrounds()
{
    // display top screen
    dmaCopyHalfWords(
        BG_DMA_CHANNEL,
        topBitmap,
        (uint16*)BG_BMP_RAM_SUB(0),
        topBitmapLen
    );
    // display bottom screen
    dmaCopyHalfWords(
        BG_DMA_CHANNEL,
        bottomBitmap,
        (uint16*)BG_BMP_RAM(0),
        bottomBitmapLen
    );
}

int main()
{
    // turn on and initialize the graphics engine
    powerOn(POWER_ALL_2D);
    lcdMainOnBottom();
    initVideo();
    initBackgrounds();

    // initialize the OAM table
    SpriteInfo spriteInfo[SPRITE_COUNT];
    OAMTable* oam = new OAMTable();
    initOAM(oam);

    // display static sprites
    displayBackgrounds();

    // initialize dynamic sprites
    static const int MAX_OBJECTS = 4;
    static int numObjects = 1;
    initObjects(oam, spriteInfo);

    // create structures for easy access to sprite data
    SpriteEntry* objectEntry[MAX_OBJECTS];
    SpriteRotation* objectRotation[MAX_OBJECTS];
    objectEntry[0] = &oam->oamBuffer[0];
    objectRotation[0] = &oam->matrixBuffer[0];

    // TODO remove this for a better system
    int angle = 0;

    // enter the program loop
    while (true) {
        // TODO read from connection

        // TODO update state from connected instances

        // update input
        scanKeys();

        // update local state
        if (keysHeld() & KEY_LEFT ) objectEntry[0]->x -= 1;
        if (keysHeld() & KEY_RIGHT) objectEntry[0]->x += 1;
        if (keysHeld() & KEY_UP   ) objectEntry[0]->y -= 1;
        if (keysHeld() & KEY_DOWN ) objectEntry[0]->y += 1;
        if (keysHeld() & KEY_A) {
            angle += 546;
            if (angle >= 32768) angle -= 32768;
            rotateSprite(objectRotation[0], angle);
        }
        if (keysHeld() & KEY_B) {
            angle -= 546;
            if (angle < 0) angle += 32768;
            rotateSprite(objectRotation[0], angle);
        }

        // TODO transmit local state

        // update display and loop
        swiWaitForVBlank();
        updateOAM(oam);
    } return 0;
}
