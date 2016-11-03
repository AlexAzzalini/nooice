/*
 * nooice - ...
 * Copyright (C) 2016 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "common.hpp"

namespace GenericJoystick {

// --------------------------------------------------------------------------------------------------------------------

enum Bytes {
    kBytesAxes1    = 0,
    kBytesAxes2,
    kBytesAxes3,
    kBytesAxes4,
    kBytesAxes5,
    kBytesAxes6,
    kBytesAxes7,
    kBytesAxes8,
    kBytesButtons,
};

static const Bytes kListCCs[] = {
    kBytesAxes1,
    kBytesAxes2,
    kBytesAxes3,
    kBytesAxes4,
    kBytesAxes5,
    kBytesAxes6,
    kBytesAxes7,
    kBytesAxes8,

};

static inline
void process(JackData* const jackdata, void* const midibuf, unsigned char tmpbuf[JackData::kBufSize])
{
    jack_midi_data_t mididata[3];

    // first time, send everything
    unsigned int maxAxes = (sizeof(kListCCs)/sizeof(Bytes) < jackdata->naxes) ? sizeof(kListCCs)/sizeof(Bytes) : jackdata->naxes;

    // send CCs
    mididata[0] = 0xB0;
    for (size_t i=0, k; i < maxAxes; ++i)
    {
        k = kListCCs[i];
        tmpbuf[k] /= 2;
        if (tmpbuf[k] == jackdata->oldbuf[k])
            continue;

        mididata[1] = i+1;
        mididata[2] = jackdata->oldbuf[k] = tmpbuf[k];
        jack_midi_event_write(midibuf, 0, mididata, 3);
    }

    // handle button presses
    unsigned int nbytes = jackdata->nbuttons/8 + 1;
    for (unsigned int j=0; j<nbytes; j++)
    {
        if (tmpbuf[kBytesButtons+j] != jackdata->oldbuf[kBytesButtons+j])
        {
            unsigned char newbyte, oldbyte;
            int mask;

            // 8 byte masks
            for (unsigned int i=0; i<8; ++i)
            {
                mask    = 1<<i;
                newbyte = tmpbuf[kBytesButtons+j] & mask;
                oldbyte = jackdata->oldbuf[kBytesButtons+j] & mask;

                if (newbyte == oldbyte)
                    continue;

                // we only care about button presses here
                if (newbyte > 0)
		{ // Note on
		  mididata[0] = 0xB0;
		  mididata[1] = 64+j*8+i;
		  mididata[2] = 100;
		}
		else
		{
		  mididata[0] = 0xB0;
		  mididata[1] = 64+j*8+i;
		  mididata[2] = 0;
		}
		jack_midi_event_write(midibuf, 0, mididata, 3);
                
            }

            jackdata->oldbuf[kBytesButtons] = tmpbuf[kBytesButtons];

        }
    }

}
// --------------------------------------------------------------------------------------------------------------------

}
