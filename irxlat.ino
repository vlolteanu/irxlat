#include <Arduino.h>

#define DECODE_NEC
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

struct NECCode
{
	uint16_t address;
	uint16_t command;

	friend bool operator ==(const NECCode &a, const NECCode &b)
	{
		return a.address == b.address && a.command == b.command;
	}

	friend bool operator !=(const NECCode &a, const NECCode &b)
	{
		return !(a == b);
	}
};

struct XlatEntry
{
	NECCode from;
	NECCode to;
};

constexpr XlatEntry XLAT_TABLE[] = {
	{ { 0xE918, 0x5E }, { 0xE78, 0x12 } }, /* vol up */
	{ { 0xE918, 0x5F }, { 0xE78, 0x11 } }, /* vol down */
};

IRrecv irRecv(IR_RECEIVE_PIN);
IRsend irSend;

void setup()
{
	Serial.begin(115200);

	irRecv.begin(IR_RECEIVE_PIN, false, 0);

	Serial.print("Supported protocols:");
	printActiveIRProtocols(&Serial);
	Serial.println();
}

void loop()
{
	if (!irRecv.decode())
		return;

	NECCode code { irRecv.decodedIRData.address, irRecv.decodedIRData.command };

	irRecv.printIRResultShort(&Serial);
	irRecv.resume();

	for (auto entry : XLAT_TABLE)
	{
		if ( code != entry.from)
			continue;

		Serial.print("Translating to NEC address ");
		Serial.print(entry.to.address);
		Serial.print(" command ");
		Serial.print(entry.to.command);
		Serial.println();

		irSend.sendNEC(entry.to.address, entry.to.command, 0);
		break;
	}
}
