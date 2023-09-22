.type	FULL_COPY_ARM9_0x0200400C_PMCBootCode, %function
.type	FULL_COPY_main_0x6, %function

.thumb

FULL_COPY_ARM9_0x0200400C_PMCBootCode: @Inject to garbage data in secure area
	PUSH {LR}
	BL GFLAppInit @Call the earlier-overwritten GFLAppInit
	MOVS R0, #0 @ARM9 processor
	LDR R1, OVL_344
	BL sys_load_overlay @overlay loaded, now call system initializers
	BL _PMCSystemInit
	POP {PC}
	.align 2
OVL_344: .word 0x158
	.size	FULL_COPY_ARM9_0x0200400C_PMCBootCode, .-FULL_COPY_ARM9_0x0200400C_PMCBootCode

FULL_COPY_main_0x6: @hook instead of GFLAppInit in main(). The reason why we are not doing this in SystemInit is that a lot of patches might want to use GFL functions in their DllMain.
	BL 0x0200400D
	.size	FULL_COPY_main_0x6, .-FULL_COPY_main_0x6

FULL_COPY_GetUserMemRegionDefaultStart_0x7C_AdjustHeapStart:
	.word 0x022050C0
	.size	FULL_COPY_GetUserMemRegionDefaultStart_0x7C_AdjustHeapStart, .-FULL_COPY_GetUserMemRegionDefaultStart_0x7C_AdjustHeapStart

FULL_COPY_sys_read_overlay_header_0xFC_UncapOverlayMaximum:
	.word 0x7FFFFFFF
	.size	FULL_COPY_sys_read_overlay_header_0xFC_UncapOverlayMaximum, .-FULL_COPY_sys_read_overlay_header_0xFC_UncapOverlayMaximum
	