.type	FULL_COPY_ARM9_0x02005050_InterceptOverlayLoad, %function
.type	FULL_COPY_ARM9_0x02005268_InjectInterceptor, %function

.thumb

FULL_COPY_ARM9_0x02005050_InterceptOverlayLoad:
	PUSH {LR}
	MOVS R4, R0 @remember intercepted parameter
	MOVS R0, #0 @ARM9 processor
	LDR R1, OVL_344
	BL sys_load_overlay @overlay loaded, now call system initializers
	BL _PMCSystemInit
	MOVS R0, R4 @get back the original parameter
	BL GFL_OvlLoad @use intercepted overlay loading routine
	POP {PC}
	.align 2
OVL_344: .word 0x158
	.size	FULL_COPY_ARM9_0x02005050_InterceptOverlayLoad, .-FULL_COPY_ARM9_0x02005050_InterceptOverlayLoad

FULL_COPY_ARM9_0x02005268_InjectInterceptor:
	BL 0x02005051
	.size	FULL_COPY_ARM9_0x02005268_InjectInterceptor, .-FULL_COPY_ARM9_0x02005268_InjectInterceptor

FULL_COPY_ARM9_0x0207B3F0_ResizeMemoryForOvl:
	.word 0x02205080
	.size	FULL_COPY_ARM9_0x0207B3F0_ResizeMemoryForOvl, .-FULL_COPY_ARM9_0x0207B3F0_ResizeMemoryForOvl

FULL_COPY_ARM9_0x02071054_UncapOverlayMaximum:
	.word 0x7FFFFFFF
	.size	FULL_COPY_ARM9_0x02071054_UncapOverlayMaximum, .-FULL_COPY_ARM9_0x02071054_UncapOverlayMaximum
	