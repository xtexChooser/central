#include "persistent_ram.h"

#ifdef CONFIG_ANDROID_RAM_CONSOLE_SAMSUNG_KLTE
//#define PERSISTENT_RAM_BASE 0x7FA00000
//#define PERSISTENT_RAM_SIZE SZ_1M
#define RAM_CONSOLE_SIZE (124 * SZ_1K * 2)

#define PERSISTENT_RAM_BASE 0x3e8e0000
#define PERSISTENT_RAM_SIZE SZ_2M
#else
#error No CONFIG_ANDROID_RAM_CONSOLE_XXX is selected
#endif

static struct persistent_ram_descriptor per_ram_descs[] __initdata = { {
	.name = "ram_console",
	.size = RAM_CONSOLE_SIZE,
} };

static struct persistent_ram per_ram __initdata = {
	.descs = per_ram_descs,
	.num_descs = ARRAY_SIZE(per_ram_descs),
	.start = PERSISTENT_RAM_BASE,
	.size = PERSISTENT_RAM_SIZE
};

int __init ram_console_early_init(void)
{
	int ret;
	ret = persistent_ram_early_init(&per_ram);
	if (ret != 0)
		return ret;
	return 0;
}
