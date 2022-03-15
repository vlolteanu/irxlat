#include <Arduino.h>

#define DECODE_NEC
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

#define VERSION "0.2"

struct Code
{
	decode_type_t proto;
	uint16_t address;
	uint16_t command;

	friend bool operator ==(const Code &a, const Code &b)
	{
		return a.proto == b.proto && a.address == b.address && a.command == b.command;
	}

	friend bool operator !=(const Code &a, const Code &b)
	{
		return !(a == b);
	}

	void send(IRsend *sender)
	{
		switch (proto)
		{
		case NEC:
			sender->sendNEC(address, command, 0);
			break;

		default:
			Serial.println("Unsupported protocol");
		}
	}

	void print()
	{
		Serial.print("Protocol=");
		Serial.print(getProtocolString(proto));
		Serial.print(" Address=0x");
		Serial.print(address, HEX);
		Serial.print(" Command=0x");
		Serial.print(command, HEX);
	}
};

struct XlatEntry
{
	Code from;
	Code to;
};

constexpr XlatEntry XLAT_TABLE[] = {
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
	for (auto entry : XLAT_TABLE)
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

	const IRData &data = irRecv.decodedIRData;
	Code code { data.protocol, data.address, data.command };

	Serial.print("Received: ");
	irRecv.printIRResultShort(&Serial);
	
	for (auto entry : XLAT_TABLE)
	{
		if (code != entry.from)
			continue;

		Serial.print("Translating: ");
		entry.to.print();
		Serial.println();

		entry.to.send(&irSend);
		break;
	}

	irRecv.resume();
}
