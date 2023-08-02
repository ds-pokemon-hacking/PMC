#include "PMC_Print.h"

#include <stdarg.h>

#include "gfl/str/gfl_systemfont.h"
#include "gfl/str/gfl_textprint.h"
#include "gfl/core/gfl_heap.h"
#include "gfl/g2d/gfl_bg_sys.h"
#include "gfl/core/gfl_vrammgr.h"
#include "gfl/g2d/gfl_vhblank.h"
#include "system/game_input.h"
#include "data/heapid_def.h"
#include "nds/mem.h"
#include "nds/gx.h"
#include "nds/svc.h"
#include "nds/irq.h"
#include "nds/timer.h"

namespace pmc {
    namespace debug {
        static const unsigned char ASCII_CHAR_OFFSET = ' ';
        static const unsigned char GFL_CHAR_OFFSET = 4;
        static const unsigned char GFL_CHAR_BPT = 8;
        static const unsigned char TILE_DIM = 8;
        static const unsigned char CHAR_BPT = 32;
        static const unsigned char TILES_PER_LINE = 32;

        struct Gfl2AsciiMapping {
            unsigned char GflStart;
            unsigned char AsciiStart;
            unsigned char Count;
        };

        void swapbuffer_memcpy32(void* a, void* b, size_t size) {
            //Not really optimized since we're not copying large amounts
            u32* a_32 = static_cast<u32*>(a);
            u32* b_32 = static_cast<u32*>(b);
            u32 swap;
            u32* end = b_32 + (size >> 2);
            while (b_32 < end) {
                swap = *a_32;
                *a_32 = *b_32;
                *b_32 = swap;
                a_32++;
                b_32++;
            }
        }

        void swap16(void* a, void* b) {
            u16* a_16 = static_cast<u16*>(a);
            u16* b_16 = static_cast<u16*>(b);
            u16 temp = *a_16;
            *a_16 = *b_16;
            *b_16 = temp;
        }

        struct Printer {

            void* CharBuf;
            u32   CharBufSize;

            u16* MapBase;
            u16  MapSize;
            
            u16 MapIndex;

            bool IsShown;
            u16 EnabledBGs;
            u16 PaletteBGColor;
            u16 PaletteTextColor;
            BGCntReg BGCfg;
            u16 BGHOfs;
            u16 BGVOfs;
            G2DBGTransformRegs BGTransform;
            u16 BGPriority;
            u16 MasterBright;
            u16 BldCnt;
            u16 DispLayout;

            bool IsPrinting;

            void ResetRegStates() {
                BGCfg.Mosaic = 0;
                BGCfg.Priority = 0;
                BGCfg.ScreenSize = BGSIZE_T256x256_R128x128;
                BGCfg.ColorPaletteMode = 0;
                BGCfg.Bit12 = 0;
                BGCfg.CharBase = 0;
                BGCfg.ScreenBase = 2;
                BGHOfs = 0;
                BGVOfs = 0;
                BldCnt = 0;
                DispLayout = G2D_DISPLAYOUT_A_UP_B_DOWN;
                EnabledBGs = G2D_SCREENBG_1;
                MasterBright = 0;
                memset(&BGTransform, 0, sizeof(BGTransform));
            }

            void Reset() {
                IsPrinting = false;
                MapBase = nullptr;
                MapSize = 0;
                MapIndex = 0;
                IsShown = false;
                PaletteBGColor = 0x0000;
                PaletteTextColor = 0x7FFF;
                ResetRegStates();
            }

            void MergeBGConfig(BGCntReg src) {
                BGCfg.CharBase = src.CharBase;
                BGCfg.ScreenBase = src.ScreenBase;
            }

            void SetCharBuf(void* charBuf, u32 size) {
                CharBuf = charBuf;
                CharBufSize = size;
            }

            void SetSurface(u16* map, u32 lines) {
                MapBase = map;
                MapSize = lines * TILES_PER_LINE; //32 lines in BG
                MapIndex = 0;
            }

            void SwapRegs() {
                swap16(&MASTER_BRIGHT_B, &MasterBright);
                swap16(&LCDIO_B.BG1CNT, &BGCfg);
                swap16(&LCDIO_B.BG1HOFS, &BGHOfs);
                swap16(&LCDIO_B.BG1VOFS, &BGVOfs);
                swap16(&LCDIO_B.BLD.BLDCNT, &BldCnt);
                //swapbuffer_memcpy32(&LCDIO_B.BG3Transform, &BGTransform, sizeof(BGTransform));

                u16 bgs = DISPCNT_B.DispBGs;
                u16 dispLayout = POWCNT1.DispLayout;
                swap16(&EnabledBGs, &bgs);
                swap16(&DispLayout, &dispLayout);
                DISPCNT_B.DispBGs = bgs;
                POWCNT1.DispLayout = dispLayout;
            }

            void SwapWithVRAM() {
                if (gfxGetCharAddrBG1B() && gfxGetScreenAddrBG1B()) {
                    swap16(&STD_PALETTE_BG_B.Colors[0xE], &PaletteBGColor);
                    swap16(&STD_PALETTE_BG_B.Colors[0xF], &PaletteTextColor);

                    if (!IsShown) {
                        SwapRegs();
                    }

                    swapbuffer_memcpy32(CharBuf, gfxGetCharAddrBG1B(), CharBufSize);
                    swapbuffer_memcpy32(MapBase, gfxGetScreenAddrBG1B(), MapSize << 1);

                    if (IsShown) {
                        SwapRegs();
                    }

                    IsShown = !IsShown;
                }
            }

            void Print(const char* str, size_t size) {
                IsPrinting = true;

                u16* out = IsShown ? static_cast<u16*>(gfxGetScreenAddrBG1B()) : MapBase;
                if (!out) {
                    IsPrinting = false;
                    return;
                }

                char c;
                while ((c = *str++) != 0) {
                    if (MapIndex >= MapSize) {
                        //Shift up one line
                        sys_memcpy32_fast(out + TILES_PER_LINE, out, (MapSize - TILES_PER_LINE) << 1);
                        sys_memset32_fast(0, out + MapSize - TILES_PER_LINE, TILES_PER_LINE << 1);
                        MapIndex -= TILES_PER_LINE;
                    }
                    if (c == '\r') {
                        continue;
                    }
                    else if (c == '\n') {
                        //fill remaining tiles in line with gap
                        u32 fillCount = TILES_PER_LINE - (MapIndex & 31);
                        sys_memset32_fast(0, out + MapIndex, fillCount << 1);
                        MapIndex += fillCount;
                    }
                    else {
                        out[MapIndex] = (unsigned char)c - ASCII_CHAR_OFFSET;
                        MapIndex++;
                    }
                    size--;
                    if (size == 0) {
                        break;
                    }
                }
                IsPrinting = false;
            }
        };

        Printer* g_DebugPrinter;

        void PrinterScreenSwapTCB(TCB* tcb, void* data) {
            static const InputButton ACTIVATE_KEYCOMBO = InputButton::KEY_L | InputButton::KEY_R;

            Printer* p = static_cast<Printer*>(data);

            if (HID_CHECK_COMBO(ACTIVATE_KEYCOMBO)) {
                u32 timer0Irq = IE.Timer0;
                u32 time0Value = TM0CNT_L;
                IE.Timer0 = 0; //disable clock IRQ for CPU clock counter
                game::HID::Update(); //clear R and B as "newly pressed keys" so that the game won't register them if the combo is pressed but surface is unavailable
                if (g_GfxVRAMBGBanksB == 0 || gfxGetCharAddrBG1B() == nullptr) {
                    //Suspend if surface unavailable
                    return;
                }
                if (p->IsPrinting) {
                    //Do not swap buffers if a print is still being processed
                    return;
                }
                p->SwapWithVRAM(); //popup debug print screen
                do {
                    game::HID::Update();
                    irq_waitFor(true, CPU_INTRB_VBLANK);
                } while (!HID_CHECK_KEY_PRESS(InputButton::KEY_X));
                do {
                    game::HID::Update();
                    irq_waitFor(true, CPU_INTRB_VBLANK);
                } while (!HID_CHECK_KEY_UP(InputButton::KEY_X)); //wait until key is fully released
                p->SwapWithVRAM(); //close debug print screen
                while (TM0CNT_L < time0Value || TM0CNT_L > (time0Value + 1)) {
                    ; //busy wait to sync clock
                }
                IE.Timer0 = timer0Irq;
            }
        }

        void UnpackChars(const char* charin, u32 inIndex, u32* charout, unsigned char outOffset, u32 count) {
            charin = charin + (inIndex - GFL_CHAR_OFFSET) * GFL_CHAR_BPT;
            charout = charout + (outOffset - ASCII_CHAR_OFFSET) * CHAR_BPT / sizeof(*charout);
            u32 unpackCount = count * GFL_CHAR_BPT;
            for (u32 unpackIdx = 0; unpackIdx < unpackCount; unpackIdx++) {
                *charout++ = GFL_SystemFontUnpackBits(*charin++) | 0xEEEEEEEE; //replace 0s with Es, keep Fs unchanged
            }
        }

        void ConvGFLFontToChars(void* chars, gfl::str::SystemFont* sysfont) {
            static const Gfl2AsciiMapping GFL_TO_ASCII_MAP[] = {
                {218, 'a', 'z' -  'a' + 1},
                {192, 'A', 'Z' - 'A' + 1},
                {182, '0', '9' - '0' + 1},
                {244, '!', '/' - '!' + 1}
            };
            static const char EXTRA_ASCII_CHARS[] = {'?', '=', '_', '[', ']', '@', ':', ';', '<', '>', '^', '~', '\''};

            const char* fontBits = reinterpret_cast<char*>(sysfont->FontData) + sysfont->FontData->GlyphsOffset;

            u32* charout = static_cast<u32*>(chars); //ensure 32-bit memory writes to VRAM
            sys_memset32_fast(0xEEEEEEEE, charout, 0x60 * CHAR_BPT); //fill with background color
            
            for (u32 i = 0; i < NELEMS(GFL_TO_ASCII_MAP); i++) {
                const Gfl2AsciiMapping* mapping = &GFL_TO_ASCII_MAP[i];
                UnpackChars(fontBits, mapping->GflStart, charout, mapping->AsciiStart, mapping->Count);
            }

            for (u32 i = 0; i < NELEMS(EXTRA_ASCII_CHARS); i++) {
                UnpackChars(
                    fontBits,
                    GFL_TextPrintASCIIToIndex(EXTRA_ASCII_CHARS[i]),
                    charout,
                    EXTRA_ASCII_CHARS[i],
                    1
                );
            }
        }
        
        enum DllMainReason {
            DLL_MODULE_LOAD,
            DLL_MODULE_UNLOAD
        };

        namespace nkextra {
            class Printer {
            public:
                virtual void Print(const char* str) = 0;
                virtual void Printf(const char* format, va_list va) = 0;
            };

            typedef void (*SetPrinterProc)(Printer* printer);
        }

        void InitPrinter() {
            Printer* p = GFL_NEW(HEAPID_SYSTEM) Printer();
            p->Reset();
            size_t charBufsize = CHAR_BPT * 0x60;
            p->SetCharBuf(GFL_MALLOC(HEAPID_SYSTEM, charBufsize), charBufsize);
            size_t mapCount = TILES_PER_LINE * 24;
            p->SetSurface(GFL_NEW(HEAPID_SYSTEM) u16[mapCount], 24);
            sys_memset32_fast(0, p->MapBase, mapCount * sizeof(u16));
            GFL_VBlankTCBAdd(PrinterScreenSwapTCB, p, 0);

            ConvGFLFontToChars(p->CharBuf, g_SystemFont);

            g_DebugPrinter = p; //do this at end so that the vblank does not fire before ready
        }

        namespace EWL_C {
            typedef int (*StringWriteProc)(void *data, const char *str, size_t count);
            extern "C" int __pformatter(StringWriteProc writeProc, void *writeProcArg, const char *format_str, va_list arg, int is_secure);
        };

        class NKPrinter : public nkextra::Printer {
        private:
            pmc::debug::Printer* m_PMCPrinter;

        public:
            NKPrinter(pmc::debug::Printer* pmcPrinter) {
                m_PMCPrinter = pmcPrinter;
            }
            
            void Print(const char* str) override {
                m_PMCPrinter->Print(str, 0xFFFFFFFF);
            }

            static int PMCPrintStringWriteProc(void* printer, const char* str, size_t count) {
                static_cast<pmc::debug::Printer*>(printer)->Print(str, count);
                return 1;
            }

            void Printf(const char* format, va_list va) override {
                EWL_C::__pformatter(PMCPrintStringWriteProc, m_PMCPrinter, format, va, false);
            }
        };

        static NKPrinter* g_NKPrinter;

        void AttachToNK(void* nkPrinterSetFunc) {
            if (g_NKPrinter == nullptr) {
                g_NKPrinter = GFL_NEW(HEAPID_SYSTEM) NKPrinter(g_DebugPrinter);
            }
            reinterpret_cast<nkextra::SetPrinterProc>(nkPrinterSetFunc)(g_NKPrinter);
        }

        void Print(const char* str) {
            if (g_DebugPrinter) {
                g_DebugPrinter->Print(str, 0xFFFFFFFF);
            }
        }

        void Printf(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);

            if (g_NKPrinter) {
                g_NKPrinter->Printf(fmt, args);
            }
            va_end(args);
        }
    }
}