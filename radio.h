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
    time_t txTimer, radioOffTimer;
    unsigned char ACK, NAK, SOH, STX, CAN, ETX, DLE, EOT;
    std::string skantiStatusBuffer, cmd;

    struct timeval timer, initTimer, ackTimer, statusTimer, sigTimer;

    int ackTimeoutCtr;

    int an, nb;
    int rxFineOffsetA, rxFineOffsetB;

    bool skantiExpectAck, skantiLinkActive, skantiReqStatus, radioOff, fr, ft, split;
    int serHandleSkanti, serHandleK2, initState;

    void checkSerialSkanti();
    void checkSerialK2();
    void workCmdK2();

    bool initSkantiLink();
    bool skantiEndLink();
    bool sendSkantiData();
    char readSkantiData();
    bool writeToSkanti(char c);
    void AG();
    void AN();
    void AI();
    void BG();
    void BN();
    void BW();
    void DN();
    void DS();
    void FA();
    void FB();
    void FR();
    void FT();
    void FW();
    void GT();
    void IF();
    void K2();
    void K3();
    void LK();
    void MD();
    void NB();
    void PA();
    void PC();
    void PO();
    void PS();
    void RA();
    void RC();
    void RD();
    void RT();
    void RU();
    void RG();
    void RV();
    void RX();
    void SM();
    void SQ();
    void SW();
    void TQ();
    void UP();
    void TX();
    void XF();
    void XT();

    int cmdTimeout, timeoutCtr;
    int ai, k2, k3;

public:
    bool debugSkantiRTX, debugK2RTX;
    std::string k2RXbuffer, k2TXbuffer, skantiRXbuffer, skantiTXbuffer, skantiCmdBuffer;

    Radio();
    std::string serialportK2, serialportSkanti;
    int freqRX, freqTX, vfoA, vfoB;
    int serSpeedK2, serSpeedSkanti;

    int signalRX, signalTX, signalState, resetCtr;

    MODE mode;
    FILTER filter;
    PWR pwr;
    TUNERATE tunerate;
    int txfreqDiff, ritFreq, txTimeout;
    int cts, rxRate, txRate, vol;
    bool txOn, tx, agc, agcSpeed, rfamp, att, squelch, duplex, delayedTXFreqUpdate, rit, xit;

    bool openSkantiPort();
    bool openK2Port();
    void updSkantiComm();   // Call every x ms to send/rec. data
    void updK2Comm();       // ---  " -----
    bool updSkantiStatus();
    bool printSkantiStatus(); // TODO/taken care of in main
    bool updSignalStatus();
};

#endif // RADIO_H
