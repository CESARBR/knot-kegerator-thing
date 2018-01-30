/*
 * Copyright (c) 2018, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <KNoTThing.h>

KNoTThing thing;

void setup(void)
{
	Serial.begin(9600);

	thing.init("KNoTKegerator");

	Serial.println(F("KNoT Kegerator"));
}

void loop(void)
{
	thing.run();
}
