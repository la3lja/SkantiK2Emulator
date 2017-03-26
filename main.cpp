/*
 *
 * A Skanti TRP8000 series radio serial interface program. It reads/writes Skanti serial commands on one
 * serial port, and read/writes a simplified set of Elecraft K2 or K3 commands on another serial port.
 *
 * Developed because current amateur software has limited or no support for Skanti radios, however
 * most of them support the Elecraft radios.
 *
 * This version polls data from the CU (control unit). For this to work CU software have to be
 * version 92 or newer. Software was developed and tested on version 92. Version 80 will NOT work
 * with polling, however other commands should work.
 *
 * Feel free to use or modify this code as to your liking.
 *
 * The inireader library is copied from https://github.com/benhoyt/inih
 *
 */

#include <iostream>
#include <ctime>

#include "config.h"
#include "radio.h"

using namespace std;

#define CONF "skanti.ini"


void delay(long sleep); // usec sleep
void printData(Radio *r);

int main()
{
    bool forceStatus = false;

    time_t print = time(NULL), now = time(NULL), sigUpdate = time(NULL)+5, statusUpdate = time(NULL), txTimer = time(NULL);

    Config *c = new Config();
    Radio *radio = new Radio();
    if (!c->init(CONF, radio)) return 0;
    statusUpdate -= c->updStatus+4;

    if (!radio->openK2Port())
    {
        cerr << "Error opening K2/K3 serial port " << radio->serialportK2 << endl;
        return 0;
    }

    if (!radio->openSkantiPort())
    {
        cerr << "Error opening Skanti serial port " << radio->serialportSkanti << endl;
        return 0;
    }

    while (true)
    {
        delay(500);
        time(&now);

        if (c->updSignal > 0 && difftime(now, sigUpdate) > c->updSignal)
        {
            time(&sigUpdate);
            radio->updSignalStatus();
        }

        else if (c->updStatus == 0 && !forceStatus && difftime(now, statusUpdate) > 9)
        {
            cout << "Status readout disabled, forcing single read" << endl;
            radio->updSkantiStatus();
            forceStatus = true;
        }

        else if (!radio->tx && c->updStatus > 0 && difftime(now, statusUpdate) > c->updStatus)
        {
            time(&statusUpdate);
            radio->updSkantiStatus();
        }

        else if (difftime(now, print) > 9)
        {
            printData(radio);
            time(&print);
        }

        else if (radio->tx && difftime(now, txTimer) > 3) // send CR every 4 sec to keep TX on as long as needed
        {
            radio->skantiCmdBuffer += '\r';
            time(&txTimer);
        }

        radio->updK2Comm();
        radio->updSkantiComm();

        if (radio->resetCtr > 4)
        {
            cout << "Skanti has become unresponsive. Try to switch it off and on again, then restart this program" << endl;
            return -1;
        }
    }

    return 1;
} // main


void delay(long sleep)
{
    struct timespec timeOut,remains;

    timeOut.tv_sec = 0;
    timeOut.tv_nsec = sleep * 1000;

    nanosleep(&timeOut, &remains);
}

void printData(Radio* r)
{
    cout << "================= Status readout ====================" << endl;
    cout << "RX freq\t" << r->freqRX << "Hz\tTX freq\t" << r->freqTX << " Hz" << endl;
    cout << "mode\t";
    switch(r->mode)
    {
    case(USB): cout << "USB"; break;
    case(LSB): cout << "LSB"; break;
    case(R3E): cout << "R3E"; break;
    case(AM): cout << "AM"; break;
    case(TELEX): cout << "TELEX"; break;
    case(CW): cout << "CW"; break;
    case(MCW): cout << "MCW"; break;
    }

    cout << "\t\tRX tune\t";
    switch(r->tunerate)
    {
    case(HZ10): cout << "10 Hz"; break;
    case(HZ100): cout << "100 Hz"; break;
    case(HZ1000): cout << "1000 Hz"; break;
    }
    cout << endl;

    cout << "filter\t";
    switch(r->filter)
    {
    case(VWIDE): cout << "very wide"; break;
    case(WIDE): cout << "wide"; break;
    case(NARROW): cout << "narrow"; break;
    case(VNARROW): cout << "very narrow"; break;
    }
    cout << "\t\tAGC\t" << (r->agc?"on":"off") << (r->agcSpeed?", fast":", slow") << endl;
    cout << "att\t" << (r->att?"on":"off") << "\t\tRF amp\t" << (r->rfamp? "on":"off") << endl;
    cout << "power\t";
    switch(r->pwr)
    {
    case(FULL): cout << "full"; break;
    case(MEDIUM): cout << "medium"; break;
    case(LOW): cout << "low"; break;
    }
    cout << "\t\tsquelch\t" << (r->squelch?"on":"off") << endl;
    cout << "duplex\t" << (r->duplex?"on":"off") << "\t\tvolume\t" << r->vol << endl;
    cout << "RX\t" << r->signalRX << "\t\tTX\t" << r->signalTX << endl;
}
