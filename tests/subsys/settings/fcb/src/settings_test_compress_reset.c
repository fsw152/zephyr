/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "settings_test.h"
#include "settings/settings_fcb.h"

void test_config_compress_reset(void)
{
	int rc;
	struct settings_fcb cf;
	struct flash_sector *fa;
	int elems[4];
	int i;

	config_wipe_srcs();
	config_wipe_fcb(fcb_sectors, ARRAY_SIZE(fcb_sectors));

	cf.cf_fcb.f_magic = CONFIG_SETTINGS_FCB_MAGIC;
	cf.cf_fcb.f_sectors = fcb_sectors;
	cf.cf_fcb.f_sector_cnt = ARRAY_SIZE(fcb_sectors);

	rc = settings_fcb_src(&cf);
	zassert_true(rc == 0, "can't register FCB as configuration source\n");

	rc = settings_fcb_dst(&cf);
	zassert_true(rc == 0,
		     "can't register FCB as configuration destination\n");

	c2_var_count = 1;
	memset(elems, 0, sizeof(elems));

	for (i = 0; ; i++) {
		test_config_fill_area(test_ref_value, i);
		memcpy(val_string, test_ref_value, sizeof(val_string));

		rc = settings_save();
		zassert_true(rc == 0, "fcb write error\n");

		if (cf.cf_fcb.f_active.fe_sector == &fcb_sectors[2]) {
			/*
			 * Started using space just before scratch.
			 */
			break;
		}
		memset(val_string, 0, sizeof(val_string));

		rc = settings_load();
		zassert_true(rc == 0, "fcb read error\n");
		zassert_true(!memcmp(val_string, test_ref_value,
				     SETTINGS_MAX_VAL_LEN),
			     "bad value read\n");
	}

	fa = cf.cf_fcb.f_active.fe_sector;
	rc = fcb_append_to_scratch(&cf.cf_fcb);
	zassert_true(rc == 0, "fcb_append_to_scratch call failure\n");
	zassert_true(fcb_free_sector_cnt(&cf.cf_fcb) == 0,
		     "expected non of free sectors\n");
	zassert_true(fa != cf.cf_fcb.f_active.fe_sector,
		     "active page should change\n");

	config_wipe_srcs();

	memset(&cf, 0, sizeof(cf));

	cf.cf_fcb.f_magic = CONFIG_SETTINGS_FCB_MAGIC;
	cf.cf_fcb.f_sectors = fcb_sectors;
	cf.cf_fcb.f_sector_cnt = ARRAY_SIZE(fcb_sectors);

	rc = settings_fcb_src(&cf);
	zassert_true(rc == 0, "can't register FCB as configuration source\n");

	rc = settings_fcb_dst(&cf);
	zassert_true(rc == 0,
		     "can't register FCB as configuration destination\n");


	zassert_true(fcb_free_sector_cnt(&cf.cf_fcb) == 1,
		     "expected one free sector\n");
	zassert_true(fa == cf.cf_fcb.f_active.fe_sector,
		   "active sector should become free after garbage collection");

	c2_var_count = 0;
}