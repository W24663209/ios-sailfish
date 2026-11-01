/*
 * Board profile selected at build time.  Board-specific MMIO and RAM
 * reservations live here so a marlin build can never accidentally inherit a
 * sailfish-only screen geometry or a Pixel 6/QEMU fallback.
 *
 * Values shared by the first-generation Pixel pair are corroborated by the
 * Android msm8996 HTC common DT: LK's 0x82600000 DTB handoff, the continuous
 * splash reservation at 0xc0001000 and ramoops' console region at 0x83540000.
 * The MARLIN profile deliberately does not enable the direct panel kickoff:
 * the XL's panel command/timing sequence needs separate bring-up.  Early
 * diagnostics go to ramoops and are read after the automatic warm reset.
 */
#pragma once

#if defined(PIXEL1)
#define BOARD_NAME              "pixel1 (sailfish)"
#define TAGS_ADDR               0x82600000UL
#define PSTORE_CONSOLE_BASE     0x83540000UL
#define FB_BASE                 0xc0001000UL
#define FB_W                    1080
#define FB_H                    1920
#define FB_BPP                  3               /* RGB888, YSTRIDE 0xca8 */
#define FB_CAN_KICK             1
#elif defined(MARLIN)
#define BOARD_NAME              "pixel xl (marlin)"
/* Observed in this device's LK log: tags/device tree @ 0x82000000. */
#define TAGS_ADDR               0x82000000UL
#define PSTORE_CONSOLE_BASE     0x83540000UL
/*
 * The XL shares the 0xc0001000 continuous-splash reservation with sailfish,
 * but has a 1440x2560 WQHD panel.  Keep panel kickoff disabled until the
 * Marlin DSI mode is verified on hardware; logging remains available through
 * the shared ramoops console.
 */
#define FB_BASE                 0xc0001000UL
#define FB_W                    1440
#define FB_H                    2560
#define FB_BPP                  3
#define FB_CAN_KICK             0
#else
#error "Select a supported board profile (-DPIXEL1 or -DMARLIN)"
#endif

#define FB_STRIDE               (FB_W * FB_BPP)
