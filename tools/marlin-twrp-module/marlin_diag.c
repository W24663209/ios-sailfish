/*
 * Phase 2: read-only live device-tree inspection for Pixel XL recovery.
 *
 * This module only traverses Linux's already-parsed OF tree and prints
 * properties. It does not map MMIO, touch devices, stop CPUs, stage a payload,
 * or access a block device.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>

static void marlin_diag_u32s(const char *label, struct device_node *np,
	const char *property, unsigned want)
{
	u32 values[8] = {0};
	if (!np || want > 8 || of_property_read_u32_array(np, property, values, want)) {
		pr_info("MARLIN DIAG: fdt %s %s absent\n", label, property);
		return;
	}
	pr_info("MARLIN DIAG: fdt %s %s=%#x %#x %#x %#x %#x %#x %#x %#x\n",
		label, property, values[0], values[1], values[2], values[3],
		values[4], values[5], values[6], values[7]);
}

static void marlin_diag_node(const char *label, struct device_node *np)
{
	const __be32 *reg;
	const char *compatible = NULL;
	int len = 0;

	if (!np) {
		pr_info("MARLIN DIAG: fdt %s absent\n", label);
		return;
	}
	(void)of_property_read_string(np, "compatible", &compatible);
	reg = of_get_property(np, "reg", &len);
	pr_info("MARLIN DIAG: fdt %s path=%s compat=%s reg-cells=%d\n",
		label, np->full_name ? np->full_name : "-",
		compatible ? compatible : "-", len / (int)sizeof(*reg));
	if (reg && len >= 2 * (int)sizeof(*reg)) {
		pr_info("MARLIN DIAG: fdt %s reg0=%#x reg1=%#x reg2=%#x reg3=%#x\n",
			label, be32_to_cpu(reg[0]), be32_to_cpu(reg[1]),
			len >= 3 * (int)sizeof(*reg) ? be32_to_cpu(reg[2]) : 0,
			len >= 4 * (int)sizeof(*reg) ? be32_to_cpu(reg[3]) : 0);
	}
}

static int __init marlin_diag_init(void)
{
	struct device_node *touch, *hub, *ramoops, *memory;
	pr_info("MARLIN DIAG: phase2 live-FDT inspection begin\n");
	marlin_diag_node("chosen", of_find_node_by_name(NULL, "chosen"));
	memory = of_find_node_by_type(NULL, "memory");
	marlin_diag_node("dram", memory);
	marlin_diag_node("gic", of_find_compatible_node(NULL, NULL, "arm,gic-v3"));
	marlin_diag_node("ufs", of_find_compatible_node(NULL, NULL, "jedec,ufs-1.1"));
	hub = of_find_compatible_node(NULL, NULL, "nanohub");
	marlin_diag_node("nanohub", hub);
	marlin_diag_u32s("nanohub", hub, "sensorhub,nreset-gpio", 3);
	marlin_diag_u32s("nanohub", hub, "sensorhub,boot0-gpio", 3);
	marlin_diag_u32s("nanohub", hub, "sensorhub,wakeup-gpio", 3);
	touch = of_find_compatible_node(NULL, NULL, "synaptics,dsx-i2c");
	marlin_diag_node("touch", touch);
	marlin_diag_u32s("touch", touch, "synaptics,display-coords", 4);
	ramoops = of_find_compatible_node(NULL, NULL, "ramoops");
	marlin_diag_node("ramoops", ramoops);
	marlin_diag_u32s("ramoops", ramoops, "android,ramoops-buffer-start", 1);
	marlin_diag_u32s("ramoops", ramoops, "android,ramoops-buffer-size", 1);
	pr_info("MARLIN DIAG: phase2 end, no hardware state changed\n");
	return 0;
}

static void __exit marlin_diag_exit(void)
{
	pr_info("MARLIN DIAG: phase2 unloaded\n");
}

module_init(marlin_diag_init);
module_exit(marlin_diag_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read-only Pixel XL TWRP live FDT inspection");
MODULE_AUTHOR("ios-sailfish");
