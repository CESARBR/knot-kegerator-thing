/*
 * Copyright (c) 2018, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <KNoTThing.h>
#include <math.h>
#include "HX711.h"
#include <EEPROM.h>

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

/* Solenoid pin */
#define SOLENOID_PIN		4

/* LED bicolor pins */
#define GREEN_LED_PIN		3
#define RED_LED_PIN		5

#define BOUNCE_RANGE		200
#define TIMES_READING		20
#define READ_INTERVAL		1000

#define LOWER_THRESHOLD_VOL	1500
#define UPPER_THRESHOLD_VOL	20000

/* Constants defined to get a valid weight */
#define K1			8410743
#define K2			9622553
#define REF_W			62.6
#define A			REF_W/(K2 - K1)
#define B			(-1) * REF_W * K1 / (K2 - K1);

#define OFFSET_ADDR		0

#define NAME_LENGTH		20

KNoTThing thing;
HX711 scale(A3, A2);

struct myTap {
	bool setup_request;
	float total_vol;
	float total_weight;
	uint8_t beer_type[NAME_LENGTH];
};
static struct myTap tap = {.setup_request = false, .total_vol = 0, .total_weight = 0};

static float remaining_vol = 0;

static float keg_weight;
static float tare_offset = 0;
static unsigned long previousMillis = 0;
static int32_t previous_value = 0;

enum States {
	INITIAL,
	RUNNING,
	SETUP_REQ,
	SETUP_RDY
};
static States state = INITIAL;

void set_led_green(int state)
{
	digitalWrite(GREEN_LED_PIN, state);
}

void set_led_red(int state)
{
	digitalWrite(RED_LED_PIN, state);
}

void set_solenoid(int state)
{
	digitalWrite(SOLENOID_PIN, state);
}

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
	raw_kg = A * mes + B;

	return raw_kg;
}

static int remaining_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	unsigned long currentMillis;
	static float last_value = 0, read_value = 0;

	/* Tares de scale when tare_offset is zero */
	if (tare_offset == 0){
		tare_offset = get_weight(TIMES_READING);

		/* Save tare_offset on EEPROM */
		EEPROM.put(OFFSET_ADDR, tare_offset);
	}

	/*
	* Read only on interval
	*/
	currentMillis = millis();
	if (currentMillis - previousMillis >= READ_INTERVAL) {
		previousMillis = currentMillis;
		keg_weight = get_weight(TIMES_READING) - tare_offset;

		read_value = remove_noise(keg_weight * 1000);

		if (state == RUNNING){
			if ((tap.total_weight - read_value) <= tap.total_vol)
				remaining_vol = tap.total_vol - (tap.total_weight - read_value);

			if (remaining_vol < last_value){
				last_value = remaining_vol;
				*val_int = remaining_vol;
				*multiplier = 1;
				*val_dec = 0;
			} else {
				*val_int = last_value;
				*multiplier = 1;
				*val_dec = 0;
			}
		} else {
			tap.total_weight = read_value;
			last_value = tap.total_weight;
			*val_int = remaining_vol;
			*multiplier = 1;
			*val_dec = 0;
		}
	} else {
		*val_int = remaining_vol;
		*multiplier = 1;
		*val_dec = 0;
	}

	return remaining_vol;
}

static int remaining_vol_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	return 0;
}

static int setup_read(uint8_t *val)
{
	*val = tap.setup_request;
	return 0;
}

static int setup_write(uint8_t *val)
{
	tap.setup_request = *val;
	return 0;
}

static int total_vol_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	*val_int = tap.total_vol;
	*val_dec = 0;
	*multiplier = 1;
	return 0;
}

static int total_vol_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
	tap.total_vol = *val_int;
	return 0;
}


void setup(void)
{
	Serial.begin(115200);
	scale.power_up();

	pinMode(SOLENOID_PIN, OUTPUT);
	pinMode(GREEN_LED_PIN, OUTPUT);
	pinMode(RED_LED_PIN, OUTPUT);

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

	/* Read tare_offset from EEPROM */
	EEPROM.get(OFFSET_ADDR, tare_offset);

	Serial.println(F("KNoT Kegerator"));
}

void loop(void)
{
	thing.run();

	switch (state) {

	case INITIAL:
		if (isnan(tare_offset))
			tare_offset = 0;

		state = RUNNING;
		break;

	case RUNNING:
		set_led_red(LOW);
		set_led_green(HIGH);
		if (remaining_vol <= LOWER_THRESHOLD_VOL){
			set_solenoid(HIGH);
			tap.total_vol = 0;
			tap.total_weight = 0;
			remaining_vol = 0;
			state = SETUP_REQ;
		}
		break;

	case SETUP_REQ:
		set_led_red(HIGH);
		set_led_green(LOW);
		if (tap.total_weight >= UPPER_THRESHOLD_VOL){
			tap.setup_request = true;
			state = SETUP_RDY;
		}
		break;

	case SETUP_RDY:
		if (tap.setup_request == false){
			set_solenoid(LOW);
			remaining_vol = tap.total_weight;
			state = RUNNING;
		}
		break;

	}
}
