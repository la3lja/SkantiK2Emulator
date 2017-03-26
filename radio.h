#ifndef RADIO_H
#define RADIO_H

#include <string>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

enum MODE       { USB, R3E, AM, CW, MCW, TELEX, LSB };
enum FILTER     { VWIDE, WIDE, NARROW, VNARROW };
enum PWR        { FULL, MEDIUM, LOW };
enum TUNERATE   { HZ10, HZ100, HZ1000 };
enum SWITCH     { OFF, ON };

class Radio
{
    friend class Config;
    time_t txTimer;
    unsigned char ACK, NAK, SOH, STX, CAN, ETX, DLE, EOT;
    std::string skantiStatusBuffer;

    struct timeval timer, initTimer, ackTimer, statusTimer, sigTimer;

    bool skantiExpectAck, skantiLinkActive, skantiReqStatus;
    int serHandleSkanti, serHandleK2, initState;

    void checkSerialSkanti();
    void checkSerialK2();
    void workCmdK2();

    bool initSkantiLink();
    bool skantiEndLink();
    bool sendSkantiData();
    char readSkantiData();
    bool writeToSkanti(char c);

    int cmdTimeout, timeoutCtr;
    int ai, k2, k3;

public:
    std::string k2RXbuffer, k2TXbuffer, skantiRXbuffer, skantiTXbuffer, skantiCmdBuffer;

    Radio();
    std::string serialportK2, serialportSkanti;
    int freqRX, freqTX;
    int serSpeedK2, serSpeedSkanti;

    int signalRX, signalTX, signalState, resetCtr;

    MODE mode;
    FILTER filter;
    PWR pwr;
    TUNERATE tunerate;
    int txfreqDiff, rit, ritFreq, txTimeout;
    int cts, rxRate, txRate, vol;
    bool txOn, tx, agc, agcSpeed, rfamp, att, squelch, duplex, delayedTXFreqUpdate;

    bool openSkantiPort();
    bool openK2Port();
    void updSkantiComm();   // Call every x ms to send/rec. data
    void updK2Comm();       // ---  " -----
    bool updSkantiStatus();
    bool printSkantiStatus(); // TODO/taken care of in main
    bool updSignalStatus();
};

#endif // RADIO_H
