#include <nds.h>
#include "includes/sprite.h"
// grit imports (resources)
#include "top.h"
#include "bottom.h"

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

    // set the video mode on the main screen
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    // TODO add these back ^^^ when adding sprites
    // DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D

    // set the video mode on the sub screen
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

    // initialize sprites in the OAM table
    SpriteInfo spriteInfo[SPRITE_COUNT];
    OAMTable* oam = new OAMTable();
    initOAM(oam);
    // TODO init other sprites (the ones that will move in the demo)

    // display static sprites
    displayBackgrounds();

    // TODO initialize input handling

    // enter the program loop
    while (true) {
        // TODO handle input and update state
        swiWaitForVBlank();
        updateOAM(oam);
    }

    return 0;
}
