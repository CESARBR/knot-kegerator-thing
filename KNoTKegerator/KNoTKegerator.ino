/*
 * Copyright (c) 2018, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <KNoTThing.h>
#include "HX711.h"

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

#define BOUNCE_RANGE		200

#define K1			8410743
#define K2			9622553
#define REF_W			62.6

#define NAME_LENGTH		20

KNoTThing thing;
HX711 scale(A3, A2);

struct myBeer {
	bool setup_request;
	float total_vol;
	float total_weight;
	uint8_t beer_type[NAME_LENGTH];
};
static struct myBeer beer = {.setup_request = false, .total_vol = 0, .total_weight = 0};

static float remaining_vol = 0;

static int32_t previous_value = 0;

static int32_t remove_noise(int32_t value)
{
	if (value > (previous_value + BOUNCE_RANGE) || value < (previous_value - BOUNCE_RANGE))
		previous_value = value;

	return previous_value;
}

static float get_weight(byte times)
{
	float raw_kg, mes, a, b;

	mes = scale.get_value(times);
	a = REF_W/(K2 - K1);
	b = (-1) * REF_W * K1 / (K2 - K1);
	raw_kg = a * mes + b;

	return raw_kg;
}

static int remaining_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}

static int remaining_vol_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}

static int setup_read(uint8_t *val)
{
	*val = beer.setup_request;
	return 0;
}

static int setup_write(uint8_t *val)
{
	beer.setup_request = *val;
	return 0;
}

static int total_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	*val_int = beer.total_vol;
	*val_dec = 0;
	*multiplier = 1;
	return 0;
}

static int total_vol_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	beer.total_vol = *val_int;
	return 0;
}

static int beer_type_read(uint8_t *val, uint8_t *len)
{
	return 0;
}

static int beer_type_write(uint8_t *val, uint8_t *len)
{
	int length;

	length = *len > NAME_LENGTH ? NAME_LENGTH : *len;
	memcpy(beer.beer_type, val, length);

	return 0;
}


void setup(void)
{
	Serial.begin(9600);
	scale.power_up();

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
		remaining_vol_read, remaining_vol_write);

	thing.registerDefaultConfig(REMAINING_VOL_ID, KNOT_EVT_FLAG_CHANGE |
		KNOT_EVT_FLAG_TIME, 60, 0, 0, 0, 0);

	/* Register total volume data and the default config */
	thing.registerFloatData(TOTAL_VOL_NAME, TOTAL_VOL_ID,
		KNOT_TYPE_ID_VOLUME, KNOT_UNIT_VOLUME_ML,
		total_vol_read, total_vol_write);

	thing.registerDefaultConfig(TOTAL_VOL_ID, KNOT_EVT_FLAG_CHANGE,
		0, 0, 0, 0, 0);

	/* Register beer type data and the default config */
	thing.registerRawData(BEER_TYPE_NAME, beer.beer_type, sizeof(beer.beer_type),
		BEER_TYPE_ID, KNOT_TYPE_ID_NONE, KNOT_UNIT_NOT_APPLICABLE,
		beer_type_read, beer_type_write);

	thing.registerDefaultConfig(BEER_TYPE_ID, KNOT_EVT_FLAG_CHANGE,
		0, 0, 0, 0, 0);

	Serial.println(F("KNoT Kegerator"));
}

void loop(void)
{
	thing.run();
}
