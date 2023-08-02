#include <stdio.h>

#include "PMC_ExceptionPrint.h"
#include "nds/gx.h"
#include "gfl/g2d/gfl_bitmap.h"
#include "data/heapid_def.h"
#include "gfl/str/gfl_textprint.h"
#include "gfl/g2d/gfl_bg_sys.h"

extern "C" void(*g_ExcUndefAbortCallback)(int*, void*);

#define ARMFUNC __attribute__((target("arm"))) \
    __attribute__ ((noinline))

namespace pmc {
    namespace debug {
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
            DISPCNT_B.RawBits &= 0xFFFFE0FF;
            gfxAcquireBGBanksB();
            gfxSetBGBanksB(VRAM_MASK_C);
            gfl::g2d::SetDisplayLayout(G2D_DISPLAYOUT_A_UP_B_DOWN);
            gfxSetBGModeB(G2D_BGMODE_HTTE);
            gfl::g2d::SetEnabledBGsB(G2D_SCREENBG_0);
            DISPCNT_B.DisplayMode = G2D_DISPMODE_GRAPHICS;
            LCDIO_B.BLD.BLDCNT = 0;
            gfxRegSetMasterBrightness(&MASTER_BRIGHT_B, 0);
            
            LCDIO_B.BG0CNT.CharBase = 4;
            LCDIO_B.BG0CNT.ScreenBase = 0;
            LCDIO_B.BG0CNT.Mosaic = 0;
            LCDIO_B.BG0CNT.ColorPaletteMode = 0;
            LCDIO_B.BG0CNT.Bit12 = 0;
            LCDIO_B.BG0CNT.Priority = 0;
            LCDIO_B.BG0CNT.ScreenSize = G2DBGScreenSize::BGSIZE_T256x256_R128x128;
            LCDIO_B.BG0HOFS = 0;
            LCDIO_B.BG0VOFS = 0;
            STD_PALETTE_BG_B.Colors[1] = 0x7FFF;
            STD_PALETTE_BG_B.Colors[2] = 0x60E1;
            gfl::str::TextPrint::Init(nullptr);
            gfl::str::TextPrintState* tps = &g_DebugPrintState;
            tps->m_Bitmap = gfl::g2d::Bitmap::WrapVRAM(gfxGetCharAddrBG0B(), 32, 24, 32, HEAPID_SYSTEM);
            tps->OutOffsX = 0;
            tps->OutOffsY = 0;
            tps->LetterSpacing = 1;
            tps->LineSpacing = 1;
            tps->GlyphColorIndex = 1;
            tps->BGColorIndex = 2;
            tps->m_Bitmap->Fill(2);
            u16* map = (u16*)gfxGetScreenAddrBG0B();
            for (u32 i = 0; i < 32 * 24; i++)
            {
                map[i] = i;
            }
        }

        extern "C" void GFL_DebugPrintOutputMessage(const char* message);

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
        }
    }
}