// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Tomasz Duszynski <tduszyns@gmail.com>
 *
 * Sample module that allows userspace to apply DT overlay.
 */

#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sizes.h>

static struct dentry *overlay;
static struct dentry *dtb;
static int ovcs_id = -1;
static u8 data[SZ_1M];

static int overlay_dtb_write(struct file *filep, const char __user *buf,
			     size_t len, loff_t *off)
{
	int ret;

	/* partial writes not allowed */
	if (*off)
		return -EINVAL;

	/* remove overlay if already applied */
	if (ovcs_id != -1) {
		ret = of_overlay_remove(&ovcs_id);
		if (!ret)
			ovcs_id = -1;

		return len;
	}

	ret = simple_write_to_buffer(data, sizeof(data), off, buf, len);
	if (ret < 0)
		return ret;

	ret = of_overlay_fdt_apply(data, len, &ovcs_id);
	if (ret)
		ovcs_id = -1;

	return len;
}

static const struct file_operations dtb_fops = {
	.open = simple_open,
	.write = overlay_dtb_write,
	.owner = THIS_MODULE,
};

static int __init overlay_init(void)
{
	overlay = debugfs_create_dir("overlay", NULL);
	if (!overlay)
		return -ENOMEM;

	dtb = debugfs_create_file("dtb", 0222, overlay, NULL, &dtb_fops);
	if (!dtb) {
		debugfs_remove(overlay);
		return -ENOMEM;
	}

	return 0;
}

static void __exit overlay_exit(void)
{
	debugfs_remove_recursive(overlay);

	if (ovcs_id != -1)
		of_overlay_remove(&ovcs_id);
}

module_init(overlay_init);
module_exit(overlay_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Tomasz Duszynski <tduszyns@gmail.com>");
