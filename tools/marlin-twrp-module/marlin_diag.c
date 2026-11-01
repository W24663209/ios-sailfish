/*
 * Phase 0: ABI-only recovery-module diagnostic for Pixel XL / marlin.
 *
 * This module intentionally performs no MMIO, no CPU control, no memory
 * staging and no storage I/O. Its sole purpose is verifying that a module
 * built against the recovered exact TWRP ABI can be loaded and unloaded.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init marlin_diag_init(void)
{
	pr_info("MARLIN DIAG: exact-TWRP module ABI probe loaded\n");
	return 0;
}

static void __exit marlin_diag_exit(void)
{
	pr_info("MARLIN DIAG: exact-TWRP module ABI probe unloaded\n");
}

module_init(marlin_diag_init);
module_exit(marlin_diag_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Non-invasive Pixel XL TWRP ABI probe");
MODULE_AUTHOR("ios-sailfish");
