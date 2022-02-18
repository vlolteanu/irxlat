#include <Arduino.h>

#define DECODE_NEC
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

#define VERSION "0.1"

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

	void print()
	{
		Serial.print("Protocol=NEC Address=0x");
		Serial.print(address, HEX);
		Serial.print(" Command=0x");
		Serial.print(command, HEX);
	}
};

struct NECXlatEntry
{
	NECCode from;
	NECCode to;
};

constexpr NECXlatEntry NEC_XLAT_TABLE[] = {
#include "table.h"
};

IRrecv irRecv(IR_RECEIVE_PIN);
IRsend irSend;

void setup()
{
	Serial.begin(115200);

	irRecv.begin(IR_RECEIVE_PIN, false, 0);

	Serial.println();
	Serial.print("IRXLAT firmware version ");
	Serial.println(VERSION);

	Serial.print("RX pin: ");
	Serial.println(IR_RECEIVE_PIN);
	Serial.print("TX pin: ");
	Serial.println(IR_SEND_PIN);

	Serial.print("Supported protocols: ");
	printActiveIRProtocols(&Serial);
	Serial.println();

	Serial.println("NEC translation table:");
	for (auto entry : NEC_XLAT_TABLE)
	{
		entry.from.print();
		Serial.print(" -> ");
		entry.to.print();
		Serial.println();
	}
}

void loop()
{
	if (!irRecv.decode())
		return;

	NECCode code { irRecv.decodedIRData.address, irRecv.decodedIRData.command };

	Serial.print("Received: ");
	irRecv.printIRResultShort(&Serial);
	
	for (auto entry : NEC_XLAT_TABLE)
	{
		if ( code != entry.from)
			continue;

		Serial.print("Translating: ");
		entry.to.print();
		Serial.println();

		irSend.sendNEC(entry.to.address, entry.to.command, 0);
		break;
	}
	irRecv.resume();
}
