/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2019-2022 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EMMC_H_
#define _EMMC_H_

#include <storage/sdmmc.h>
#include <utils/types.h>
#include <utils/list.h>

#define GPT_FIRST_LBA  1
#define GPT_NUM_BLOCKS 33
#define EMMC_BLOCKSIZE SDMMC_DAT_BLOCKSIZE

enum
{
	EMMC_INIT_FAIL = 0,
	EMMC_1BIT_HS52 = 1,
	EMMC_8BIT_HS52 = 2,
	EMMC_MMC_HS200 = 3,
	EMMC_MMC_HS400 = 4,
};

enum
{
	EMMC_ERROR_INIT_FAIL = 0,
	EMMC_ERROR_RW_FAIL   = 1,
	EMMC_ERROR_RW_RETRY  = 2
};

typedef struct _emmc_part_t
{
	u32 index;
	u32 lba_start;
	u32 lba_end;
	u64 attrs;
	char name[37];
	link_t link;
} emmc_part_t;

extern sdmmc_t emmc_sdmmc;
extern sdmmc_storage_t emmc_storage;

u32  emmc_get_mode();
int  emmc_init_retry(bool power_cycle);
bool emmc_initialize(bool power_cycle);
int  emmc_set_partition(u32 partition);
void emmc_end();

#endif
