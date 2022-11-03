#include <stdio.h>

#include "PMC_ExceptionPrint.h"
#include "nds/gx.h"
#include "gfl/g2d/gfl_bitmap.h"
#include "data/heapid_def.h"
#include "gfl/str/gfl_textprint.h"
#include "gfl/g2d/gfl_bg_sys.h"

extern "C" void(*g_ExcUndefAbortCallback)(int*, void*);

extern "C" void GFL_DebugPrintCreateSurface();
extern "C" void GFL_DebugPrintOutputMessage(const char *message);
extern "C" void GFL_DebugPrintCommit();

#define ARMFUNC __attribute__((target("arm"))) \
    __attribute__ ((noinline))

namespace pmc {
    enum CPUMode {
        MODE_USER = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_MON = 0x16,
        MODE_ABT = 0x17,
        MODE_HYP = 0x1A,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    void ExceptionPrintInit() {
        g_ExcUndefAbortCallback = PrintException;
    }

    void CreateDebugPrintSurface(){
        DISPCNT_B &= 0xFFFFE0FF;
        gfxAcquireBGBanksB();
        gfxSetBGBanksB(VRAM_MASK_C);
        gfl::g2d::SetDisplayLayout(G2D_DISPLAYOUT_A_UP_B_DOWN);
        gfxSetBGModeB(G2D_BGMODE_HTTE);
        gfl::g2d::SetEnabledBGsB(G2D_SCREENBG_0);
        DISPCNT_B |= (G2D_DISPMODE_GRAPHICS << 16);
        LCDIO_B.BLD.BLDCNT = 0;
        gfxRegSetMasterBrightness(&MASTER_BRIGHT_B, 0);
        
        LCDIO_B.BG0CNT = (LCDIO_B.BG0CNT & 0x43 | 0x10) & 0xFFFCu;
        LCDIO_B.BG0HOFS = 0;
        LCDIO_B.BG0VOFS = 0;
        STD_PALETTE_BG_B.Colors[1] = 0x7FFF;
        STD_PALETTE_BG_B.Colors[2] = 0x60E1;
        GFL_TextPrintInit(0);
        g_DebugPrintState.m_Bitmap = GFL_BitmapWrapVRAM(gfxGetCharAddrBG0B(), 32, 32, 32, HEAPID_SYSTEM);
        g_DebugPrintState.OutOffsX = 0;
        g_DebugPrintState.OutOffsY = 0;
        g_DebugPrintState.LetterSpacing = 1;
        g_DebugPrintState.LineSpacing = 1;
        g_DebugPrintState.ColorIndices = g_DebugPrintState.ColorIndices & 0xF0 | 1;
        g_DebugPrintState.ColorIndices = g_DebugPrintState.ColorIndices & 0xF | 0x20;
        GFL_BitmapFill(g_DebugPrintState.m_Bitmap, 2u);
    }

    ARMFUNC
    void PrintException(int* regDump, void* data) {
        CreateDebugPrintSurface();
        
        int cpsr;
        asm volatile("MRS %0, CPSR" : "=r" (cpsr) : :);
        int mode = cpsr & 0x1F;

        if (mode == MODE_ABT) {
            GFL_DebugPrintOutputMessage("Exception / Abort\n");
        }
        else {
            GFL_DebugPrintOutputMessage("Exception / Undefined\n");
        }

        static const char* REG_NAMES[] = {
            "CPSR", "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "SP", "LR", "PC"
        };

        char printBuffer[32];

        for (int i = 0; i <= 16; i++) {
            sprintf(printBuffer, "%-4s: 0x%08X\n", REG_NAMES[i], regDump[i]);
            GFL_DebugPrintOutputMessage(printBuffer);
        }

        u16* ScreenAddrBG0B = (u16*)gfxGetScreenAddrBG0B();
        u32 tileIndex = 0;
        for ( int i = 0; i < 32; ++i )
        {
            for ( int j = 0; j < 32; ++j )
            {
                ScreenAddrBG0B[j] = tileIndex++;
            }
            ScreenAddrBG0B += 32;
        }
    }
}