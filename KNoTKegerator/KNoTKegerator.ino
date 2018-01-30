/*
 * Copyright (c) 2018, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <KNoTThing.h>

/* Setup request defines */
#define SETUP_REQUEST_ID	1
#define SETUP_REQUEST_NAME	"Setup request"

/* Remaning volume defines */
#define REMAINING_VOL_ID	2
#define REMAINING_VOL_NAME	"Remaning volume"

/* Total volume defines */
#define TOTAL_VOL_ID		3
#define TOTAL_VOL_NAME		"Total volume"

/* Beer type defines */
#define BEER_TYPE_ID		4
#define BEER_TYPE_NAME		"Beer type"

#define NAME_LENGTH		20

static KNoTThing thing;

struct myTap {
	bool setup_request;
	int32_t total_vol;
	int32_t max_weight;
	uint8_t beer_type[NAME_LENGTH];
};
static struct myTap tap = {.setup_request = false, .total_vol = 0, .max_weight = 0};

static int32_t remaining_vol = 0;

static int remaining_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}

static int setup_read(uint8_t *val)
{
	return 0;
}

static int setup_write(uint8_t *val)
{
	return 0;
}

static int total_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}

static int total_vol_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}


void setup(void)
{
	Serial.begin(115200);

	thing.init("KNoTKegerator");

	/* Register setup request data and the default config */
	thing.registerBoolData(SETUP_REQUEST_NAME, SETUP_REQUEST_ID,
		KNOT_TYPE_ID_SWITCH, KNOT_UNIT_NOT_APPLICABLE,
		setup_read, setup_write);

	thing.registerDefaultConfig(SETUP_REQUEST_ID, KNOT_EVT_FLAG_CHANGE,
		0, 0, 0, 0, 0);

	/* Register remaining volume data and the default config */
	thing.registerFloatData(REMAINING_VOL_NAME, REMAINING_VOL_ID,
		KNOT_TYPE_ID_VOLUME, KNOT_UNIT_VOLUME_ML,
		remaining_vol_read, NULL);

	thing.registerDefaultConfig(REMAINING_VOL_ID, KNOT_EVT_FLAG_CHANGE |
		KNOT_EVT_FLAG_TIME, 300, 0, 0, 0, 0);

	/* Register total volume data and the default config */
	thing.registerFloatData(TOTAL_VOL_NAME, TOTAL_VOL_ID,
		KNOT_TYPE_ID_VOLUME, KNOT_UNIT_VOLUME_ML,
		total_vol_read, total_vol_write);

	thing.registerDefaultConfig(TOTAL_VOL_ID, KNOT_EVT_FLAG_CHANGE,
		0, 0, 0, 0, 0);

	/* Register beer type data and the default config */
	thing.registerRawData(BEER_TYPE_NAME, tap.beer_type, sizeof(tap.beer_type),
		BEER_TYPE_ID, KNOT_TYPE_ID_NONE, KNOT_UNIT_NOT_APPLICABLE,
		NULL, NULL);

	thing.registerDefaultConfig(BEER_TYPE_ID, KNOT_EVT_FLAG_CHANGE,
		0, 0, 0, 0, 0);

	Serial.println(F("KNoT Kegerator"));
}

void loop(void)
{
	thing.run();
}
