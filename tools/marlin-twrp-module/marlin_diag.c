/*
 * Phase 1: read-only recovery-kernel context inspection for Pixel XL.
 *
 * This module does not map or touch device registers, does not stop CPUs,
 * does not stage a payload and does not access block devices. It only reports
 * scheduler/CPU and iomem topology through the exact ABI validated in phase 0.
 */
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/resource.h>
#include <linux/smp.h>
#include <asm/cputype.h>

static void marlin_diag_resource(const struct resource *res, unsigned depth)
{
	const struct resource *child;
	if (!res || depth > 2)
		return;
	pr_info("MARLIN DIAG: iomem depth=%u %#llx-%#llx flags=%#lx name=%s\n",
		depth, (unsigned long long)res->start,
		(unsigned long long)res->end, res->flags,
		res->name ? res->name : "-");
	for (child = res->child; child; child = child->sibling)
		marlin_diag_resource(child, depth + 1);
}

static int __init marlin_diag_init(void)
{
	unsigned cpu;
	pr_info("MARLIN DIAG: phase1 begin cpu=%u mpidr=%#llx pages=%lu\n",
		smp_processor_id(), (unsigned long long)read_cpuid_mpidr(),
		totalram_pages);
	for_each_online_cpu(cpu)
		pr_info("MARLIN DIAG: online cpu=%u\n", cpu);
	marlin_diag_resource(&iomem_resource, 0);
	pr_info("MARLIN DIAG: phase1 end, no hardware state changed\n");
	return 0;
}

static void __exit marlin_diag_exit(void)
{
	pr_info("MARLIN DIAG: phase1 unloaded\n");
}

module_init(marlin_diag_init);
module_exit(marlin_diag_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read-only Pixel XL TWRP trampoline inspection");
MODULE_AUTHOR("ios-sailfish");
