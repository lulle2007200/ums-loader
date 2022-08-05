#include <gfx.h>
#include <mem/mc.h>
#include <soc/hw_init.h>
#include <storage/nx_sd.h>
#include <mem/heap.h>
#include <memory_map.h>
#include <storage/sdmmc.h>
#include <string.h>
#include <usb/usbd.h>
#include <utils/btn.h>
#include <utils/types.h>
#include <utils/util.h>

#include <display/di.h>
#include <gfx_utils.h>
#include <tui.h>

// Offset: 0xa4
// +-----+---------+------------------------------------+
// | Bit | Default | Function                           |
// +-----+---------+------------------------------------+
// | 0   | 0       | 0: Show the interactive menu       |
// |     |         | 1: Start UMS automatically and use |
// |     |         |    the configuration in 0xa5       |
// +-----+---------+------------------------------------+
// | 1:2 | 0       | 0: Return to menu after UMS has    |
// |     |         |    stopped                         |
// |     |         | 1: Power off after UMS has stopped |
// |     |         | 2: Reboot to RCM after UMS has     |
// |     |         |    stopped                         |
// +-----+---------+------------------------------------+

// Offset: 0xa5
// +-----+---------+------------------------------------+
// | Bit | Default | Function                           |
// +-----+---------+------------------------------------+
// | 0:1 | 2       | 0: Don't mount SD                  |
// |     |         | 1: Mount SD as read-only           |
// |     |         | 2: Mount SD as read/write          |
// +-----+---------+------------------------------------+
// | 2:3 | 0       | 0: Don't mount EMMC-GPP            |
// |     |         | 1: Mount EMMC-GPP as read-only     |
// |     |         | 2: Mount EMMC-GPP as read/write    |
// +-----+---------+------------------------------------+
// | 4:5 | 0       | 0: Don't mount EMMC-BOOT0          |
// |     |         | 1: Mount EMMC-BOOT0 as read-only   |
// |     |         | 2: Mount EMMC-BOOT0 as read/write  |
// +-----+---------+------------------------------------+
// | 6:7 | 0       | 0: Don't mount EMMC-BOOT1          |
// |     |         | 1: Mount SD EMMC-BOOT1 read-only   |
// |     |         | 2: Mount SD EMMC-BOOT1 read/write  |
// +-----+---------+------------------------------------+

#define MEMLOADER_NO_MOUNT              0
#define MEMLOADER_RO                    1
#define MEMLOADER_RW                    2

#define MEMLOADER_SD                    0
#define MEMLOADER_EMMC_GPP              1
#define MEMLOADER_EMMC_BOOT0            2
#define MEMLOADER_EMMC_BOOT1            3

#define MEMLOADER_AUTOSTART_MASK        0x01
#define MEMLOADER_AUTOSTART_YES         0x01
#define MEMLOADER_AUTOSTART_NO          0x00

#define MEMLOADER_STOP_ACTION_MASK      0x06
#define MEMLOADER_STOP_ACTION_MENU      0x00  
#define MEMLOADER_STOP_ACTION_OFF       0x02
#define MEMLOADER_STOP_ACTION_RCM       0x04 

#define MEMLOADER_ERROR_SD              0x01
#define MEMLOADER_ERROR_EMMC            0x02

typedef struct __attribute__((__packed__)) ums_loader_boot_cfg_t{
	u8 magic;
	u8 config;
}ums_loader_boot_cfg_t;

typedef struct ums_loader_ums_cfg_t{
	union{
		u8 mount_modes[4];
		struct{
			u8 mount_mode_sd;
			u8 mount_mode_emmc_gpp;
			u8 mount_mode_emmc_boot0;
			u8 mount_mode_emmc_boot1;
		};
	};
	u32 storage_state;
	u32 stop_action;
	bool autostart;
}ums_loader_ums_cfg_t;

ums_loader_boot_cfg_t ums_loader_boot_cfg __attribute__((__section__("._ums_loader_cfg"))) = {
	.magic = 0x0,
	.config = 0x2,
};

typedef struct ums_toggle_cb_data_t{
	tui_entry_t *entry;
	ums_loader_ums_cfg_t *config;
	u8 volume;
}ums_toggle_cb_data_t;

static const char *toggle_menu_strings[][3] = {
		[MEMLOADER_EMMC_BOOT1] = {" BOOT1 - ", " BOOT1 RO", " BOOT1 RW"},
		[MEMLOADER_EMMC_BOOT0] = {" BOOT0 - ", " BOOT0 RO", " BOOT0 RW"},
		[MEMLOADER_EMMC_GPP]   = {"   GPP - ", "   GPP RO", "   GPP RW"},
		[MEMLOADER_SD]         = {"    SD - ", "    SD RO", "    SD RW"}
	};
	
extern void pivot_stack(u32 stack_top);

void system_maintenance(bool refresh){

}

void set_text(void *label, const char *text){
	u32 pos_x;
	u32 pos_y;
	gfx_con_getpos(&pos_x, &pos_y);
	gfx_clear_partial_grey(0x0, pos_y, 8);
	gfx_printf("%s", text);
	gfx_con_setpos(pos_x, pos_y);
}


void ums_cfg_menu_toggle_cb(void *data){
	ums_toggle_cb_data_t *ums_toggle_cb_data = (ums_toggle_cb_data_t*)data;

	switch(ums_toggle_cb_data->config->mount_modes[ums_toggle_cb_data->volume]){
		case MEMLOADER_NO_MOUNT:
			ums_toggle_cb_data->config->mount_modes[ums_toggle_cb_data->volume] = MEMLOADER_RO;
			break;
		case MEMLOADER_RO:
			ums_toggle_cb_data->config->mount_modes[ums_toggle_cb_data->volume] = MEMLOADER_RW;
			break;
		case MEMLOADER_RW:
			ums_toggle_cb_data->config->mount_modes[ums_toggle_cb_data->volume] = MEMLOADER_NO_MOUNT;
			break;
	}

	ums_toggle_cb_data->entry->title.text = toggle_menu_strings[ums_toggle_cb_data->volume][ums_toggle_cb_data->config->mount_modes[ums_toggle_cb_data->volume]];
}



void ums_start(ums_loader_ums_cfg_t *config){
	gfx_clear_color(0x0);
	gfx_con_setpos(0, 0);
	gfx_con_setcol(TUI_COL_FG, 1, TUI_COL_BG);

	gfx_printf("Running UMS\n\n");
	gfx_con_setcol(TUI_COL_DISABLED_FG, true, TUI_COL_DISABLED_BG);
	gfx_printf("Volume Mount\n");

	for(u32 i = MEMLOADER_SD; i <= MEMLOADER_EMMC_BOOT1; i++){
		gfx_printf("%s\n", toggle_menu_strings[i][config->mount_modes[i]]);
	}

	gfx_con_setcol(TUI_COL_FG, 1, TUI_COL_BG);

	gfx_printf("\nTo stop, hold\n VOL+ and VOL-, or\n eject all volumes\n safely.\n\nStatus:\n");

	usb_ctxt_vol_t volumes[4];
	u32 volumes_cnt = 0;

	if(config->mount_mode_sd != MEMLOADER_NO_MOUNT){
		if(!(config->storage_state & MEMLOADER_ERROR_SD)){
			volumes[volumes_cnt].offset = 0;
			volumes[volumes_cnt].partition = 0;
			volumes[volumes_cnt].sectors = 0;
			volumes[volumes_cnt].type = MMC_SD;
			volumes[volumes_cnt].ro = config->mount_mode_sd == MEMLOADER_RO;

			volumes_cnt++;

		}
	}

	if(config->mount_mode_emmc_gpp != MEMLOADER_NO_MOUNT){
		if(!(config->storage_state & MEMLOADER_ERROR_EMMC)){
			volumes[volumes_cnt].offset = 0;
			volumes[volumes_cnt].partition = EMMC_GPP + 1;
			volumes[volumes_cnt].sectors = 0;
			volumes[volumes_cnt].type = MMC_EMMC;
			volumes[volumes_cnt].ro = config->mount_mode_emmc_gpp == MEMLOADER_RO;

			volumes_cnt++;
		}
	}

	if(config->mount_mode_emmc_boot0 != MEMLOADER_NO_MOUNT){
		if(!(config->storage_state & MEMLOADER_ERROR_EMMC)){
			volumes[volumes_cnt].offset = 0;
			volumes[volumes_cnt].partition = EMMC_BOOT0 + 1;
			volumes[volumes_cnt].sectors = 0x2000;
			volumes[volumes_cnt].type = MMC_EMMC;
			volumes[volumes_cnt].ro = config->mount_mode_emmc_boot0 == MEMLOADER_RO;

			volumes_cnt++;
		}
	}

	if(config->mount_mode_emmc_boot1 != MEMLOADER_NO_MOUNT){
		if(!(config->storage_state & MEMLOADER_ERROR_EMMC)){
			volumes[volumes_cnt].offset = 0;
			volumes[volumes_cnt].partition = EMMC_BOOT1 + 1;
			volumes[volumes_cnt].sectors = 0x2000;
			volumes[volumes_cnt].type = MMC_EMMC;
			volumes[volumes_cnt].ro = config->mount_mode_emmc_boot1 == MEMLOADER_RO;

			volumes_cnt++;
		}
	}

	usb_ctxt_t usbs;

	usbs.label = NULL;
	usbs.set_text = &set_text;
	usbs.system_maintenance = &system_maintenance;
	usbs.volumes_cnt = volumes_cnt;
	usbs.volumes = volumes;


	usb_device_gadget_ums(&usbs);

	msleep(1000);

	switch(config->stop_action){
		case MEMLOADER_STOP_ACTION_OFF:
			power_set_state(POWER_OFF);
			break;
		case MEMLOADER_STOP_ACTION_RCM:
			power_set_state(REBOOT_RCM);
			break;
		case MEMLOADER_STOP_ACTION_MENU:
		default:
			break;
	}
}

void ums_menu_cb(void *data){
	ums_start((ums_loader_ums_cfg_t*)data);
}

void menu_power_off_cb(void *data){
	power_set_state(POWER_OFF);
}

void menu_reboot_rcm_cb(void *data){
	power_set_state(REBOOT_RCM);
}

extern void _reset(void);

void menu_reload_cb(void *data){
	_reset();
}

void main(){
	ums_loader_ums_cfg_t ums_cfg = {0};

	for(u32 i = MEMLOADER_SD; i <= MEMLOADER_EMMC_BOOT1; i++){
		ums_cfg.mount_modes[i] = (ums_loader_boot_cfg.config >> 2 * i) & 0x3;
	}

	ums_cfg.autostart = (ums_loader_boot_cfg.magic & MEMLOADER_AUTOSTART_MASK) == MEMLOADER_AUTOSTART_YES;
	ums_cfg.stop_action = (ums_loader_boot_cfg.magic & MEMLOADER_STOP_ACTION_MASK);

	if(!sd_initialize(false)){
		ums_cfg.storage_state |= MEMLOADER_ERROR_SD;
	}
	sd_end();

	sdmmc_storage_t storage_emmc;
	sdmmc_t sdmmc_emmc;

	if(!sdmmc_storage_init_mmc(&storage_emmc, &sdmmc_emmc, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_HS400)){
		ums_cfg.storage_state |= MEMLOADER_ERROR_EMMC;
	}
	sdmmc_storage_end(&storage_emmc);

	if(ums_cfg.storage_state & MEMLOADER_ERROR_SD){
		ums_cfg.mount_mode_sd = MEMLOADER_NO_MOUNT;
	}
	if(ums_cfg.storage_state & MEMLOADER_ERROR_EMMC){
		for(u32 i = MEMLOADER_EMMC_GPP; i <= MEMLOADER_EMMC_BOOT1; i++){
			ums_cfg.mount_modes[i] = MEMLOADER_NO_MOUNT;
		}
	}

	if(ums_cfg.autostart){
		ums_cfg.autostart = false;
		ums_start(&ums_cfg);
	}

	ums_toggle_cb_data_t ums_toggle_cb_data[4] = {
		[MEMLOADER_SD] = {
			.config = &ums_cfg,
			.volume = MEMLOADER_SD
		},
		[MEMLOADER_EMMC_GPP] = {
			.config = &ums_cfg,
			.volume = MEMLOADER_EMMC_GPP
		},
		[MEMLOADER_EMMC_BOOT0] = {
			.config = &ums_cfg,
			.volume = MEMLOADER_EMMC_BOOT0
		},
		[MEMLOADER_EMMC_BOOT1] = {
			.config = &ums_cfg,
			.volume = MEMLOADER_EMMC_BOOT1
		},
	};


	tui_entry_t ums_menu_entries[] = {
		[0] = TUI_ENTRY_TEXT("Volume Mount", &ums_menu_entries[1]),
		[1] = TUI_ENTRY_ACTION_NO_BLANK(toggle_menu_strings[MEMLOADER_SD][ums_cfg.mount_modes[MEMLOADER_SD]], ums_cfg_menu_toggle_cb, &ums_toggle_cb_data[MEMLOADER_SD], ums_cfg.storage_state & MEMLOADER_ERROR_SD ? true : false, &ums_menu_entries[2]),
		[2] = TUI_ENTRY_ACTION_NO_BLANK(toggle_menu_strings[MEMLOADER_EMMC_GPP][ums_cfg.mount_modes[MEMLOADER_EMMC_GPP]], ums_cfg_menu_toggle_cb, &ums_toggle_cb_data[MEMLOADER_EMMC_GPP], ums_cfg.storage_state & MEMLOADER_ERROR_EMMC ? true : false, &ums_menu_entries[3]),
		[3] = TUI_ENTRY_ACTION_NO_BLANK(toggle_menu_strings[MEMLOADER_EMMC_BOOT0][ums_cfg.mount_modes[MEMLOADER_EMMC_BOOT0]], ums_cfg_menu_toggle_cb, &ums_toggle_cb_data[MEMLOADER_EMMC_BOOT0], ums_cfg.storage_state & MEMLOADER_ERROR_EMMC ? true : false, &ums_menu_entries[4]),
		[4] = TUI_ENTRY_ACTION_NO_BLANK(toggle_menu_strings[MEMLOADER_EMMC_BOOT1][ums_cfg.mount_modes[MEMLOADER_EMMC_BOOT1]], ums_cfg_menu_toggle_cb, &ums_toggle_cb_data[MEMLOADER_EMMC_BOOT1], ums_cfg.storage_state & MEMLOADER_ERROR_EMMC ? true : false, &ums_menu_entries[5]),
		[5] = TUI_ENTRY_TEXT("\n", &ums_menu_entries[6]),
		[6] = TUI_ENTRY_ACTION("Start UMS", ums_menu_cb, &ums_cfg, false, &ums_menu_entries[7]),
		[7] = TUI_ENTRY_TEXT("\n", &ums_menu_entries[8]),
		[8] = TUI_ENTRY_ACTION("Reload", menu_reload_cb, NULL, false, &ums_menu_entries[9]),
		[9] = TUI_ENTRY_ACTION("Reboot RCM", menu_reboot_rcm_cb, NULL, false, &ums_menu_entries[10]),
		[10] = TUI_ENTRY_ACTION("Power Off", menu_power_off_cb, NULL, false, NULL),
	};


	ums_toggle_cb_data[MEMLOADER_SD].entry = &ums_menu_entries[1];
	ums_toggle_cb_data[MEMLOADER_EMMC_GPP].entry = &ums_menu_entries[2];
	ums_toggle_cb_data[MEMLOADER_EMMC_BOOT0].entry = &ums_menu_entries[3];
	ums_toggle_cb_data[MEMLOADER_EMMC_BOOT1].entry = &ums_menu_entries[4];

	tui_entry_menu_t ums_menu;
	ums_menu.title.text = "UMS";
	ums_menu.entries = ums_menu_entries;

	tui_menu_start(&ums_menu);

	power_set_state(POWER_OFF);
}


void ipl_main(){
	hw_init();
	pivot_stack(IPL_STACK_TOP);
	heap_init(IPL_HEAP_START);

	mc_enable_ahb_redirect(true);

	display_init();
	u8 *fb = display_init_small_framebuffer_pitch1();

	gfx_init_ctxt(fb, 180, 320, 192);
	gfx_con_init();
	display_backlight_pwm_init();

	display_backlight_brightness(255, 1000);
	main();
}