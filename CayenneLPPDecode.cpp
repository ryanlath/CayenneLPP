// Copyright � Luiz Henrique Cassettari. All rights reserved.
// Licensed under the MIT license.

#include "CayenneLPPDecode.h"

CayenneLPPDecode::CayenneLPPDecode(uint8_t size)
{
    if (size > 0) payload_sizemax = size;

    payload_buffer = (uint8_t *)malloc(payload_sizemax);
}

CayenneLPPDecode::~CayenneLPPDecode(void)
{
    free(payload_buffer);
}

size_t CayenneLPPDecode::write(uint8_t c)
{
    if (payload_sizemax > payload_length) {
        payload_buffer[payload_length++] = c;
        return 1;
    }

    return 0;
};

size_t CayenneLPPDecode::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    for (size_t i = 0; i < size; i++)
    {
        n += write(*(buffer + i));
    }
    return n;
}

int CayenneLPPDecode::available()
{
    return payload_length - payload_position;
}

int CayenneLPPDecode::read()
{
    return payload_position < payload_length ? payload_buffer[payload_position++] : -1;
}

int CayenneLPPDecode::peek()
{
    return payload_position < payload_length ? payload_buffer[payload_position] : -1;
}

void CayenneLPPDecode::flush()
{
}
/*
uint8_t *CayenneLPPDecode::buffer()
{
    return ((uint8_t *)&payload_buffer) + payload_position;
}
*/
int16_t CayenneLPPDecode::read16()
{
    if (available() < 2)
        return 0;
    return (read() << 8 | read());
}

int32_t CayenneLPPDecode::read24()
{
    if (available() < 3)
        return 0;
    return (int32_t) ((int32_t) read() << 24 | (int32_t) read() << 16 | (int32_t) read() << 8) >> 8;
}

int32_t CayenneLPPDecode::read32()
{
    if (available() < 4)
        return 0;
    return (int32_t) ((int32_t) read() << 24 | (int32_t) read() << 16 | (int32_t) read() << 8 | (int32_t) read());
}

void CayenneLPPDecode::dumpBuffer(Stream &out)
{
    if (payload_length > 0)
        CAYENNE_LPP_DECODE_PRINTLN(out, payload_buffer, payload_length);
}

void CayenneLPPDecode::reset(void)
{
    payload_length = 0;
    payload_position = 0;
}

void CayenneLPPDecode::decode(JsonObject &_root)
{
    JsonObject &root = (JsonObject &)_root;
	//

	// skip channel
    payload_position = 1;

	// TODO: > vs >=  -> > cuts off my last reading...
    while (available() >= 2)
    {
        //int channel = read();
        int type = read();
        //String s = channalField(channel, type);
        String s = channelField(type);

        switch (type)
        {
        case LPP_DIGITAL_INPUT:
        case LPP_DIGITAL_OUTPUT:
            root[s] = read();
            break;
        case LPP_ANALOG_INPUT:
        case LPP_ANALOG_OUTPUT:
            root[s] = read16() / 100.0f;
            break;
        case LPP_LUMINOSITY:
            root[s] = read16();
            break;
        case LPP_PRESENCE:
            root[s] = read();
            break;
        case LPP_TEMPERATURE:
            root[s] = read16() / 10.0f;
            break;
        case LPP_RELATIVE_HUMIDITY:
            root[s] = read() / 2.0f;
            break;
        case LPP_ACCELEROMETER:
        {
            JsonObject &json = root.createNestedObject(s);
            json[F("x")] = read16() / 1000.0f;
            json[F("y")] = read16() / 1000.0f;
            json[F("z")] = read16() / 1000.0f;
            break;
        }
        case LPP_BAROMETRIC_PRESSURE:
            root[s] = read16() / 10.0f;
            break;
		case LPP_VOLTAGE:
            root[s] = read16() / 1000.0f;
            break;
        case LPP_GYROMETER:
        {
            JsonObject &json = root.createNestedObject(s);
            json[F("x")] = read16() / 100.0f;
            json[F("y")] = read16() / 100.0f;
            json[F("z")] = read16() / 100.0f;
            break;
        }
        case LPP_GPS:
        {
            JsonObject &json = root.createNestedObject(s);
            json[F("latitude")] = read24() / 10000.0f;
            json[F("longitude")] = read24() / 10000.0f;
            json[F("altitude")] = read24() / 100.0f;
            break;
        }
        }
    }
    reset();
}

String CayenneLPPDecode::channelField(byte type)
{
    switch (type) {

    case LPP_DIGITAL_INPUT:
        return String(F("digital_in"));
    case LPP_DIGITAL_OUTPUT:
        return String(F("digital_out"));
    case LPP_ANALOG_INPUT:
        return String(F("analog_in"));
    case LPP_ANALOG_OUTPUT:
        return String(F("analog_out"));
    case LPP_LUMINOSITY:
        return String(F("luminosity"));
    case LPP_PRESENCE:
        return String(F("presence"));
    case LPP_TEMPERATURE:
        return String(F("temperature"));
    case LPP_RELATIVE_HUMIDITY:
        return String(F("humidity"));
    case LPP_ACCELEROMETER:
        return String(F("accelerometer"));
    case LPP_BAROMETRIC_PRESSURE:
        return String(F("pressure"));
    case LPP_VOLTAGE:
        return String(F("voltage"));
    case LPP_GYROMETER:
        return String(F("gyrometer"));
    case LPP_GPS:
        return String(F("gps"));
    }

    return String(F("unknown"));
}
