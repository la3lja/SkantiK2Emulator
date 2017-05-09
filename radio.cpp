#include "radio.h"
#include "k3commands.cpp"
#include "skanticommands.cpp"

Radio::Radio()
{
    initState = 0;
    skantiExpectAck = false;
    skantiLinkActive = false;
    skantiReqStatus = false;
    radioOff = false;
    ritFreq = 0; rit = false, xit = false, fr = false, ft = false, split = false, tx = false;
    rxFineOffsetA = rxFineOffsetB = 0;
    nb = 0;
    an = 1;
    resetCtr = 0;
    signalState = 0;
    cmdTimeout = 750;
    timeoutCtr = 0;

    ai = 0;
    k2 = 0;
    k3 = 0;
    vfoA = vfoB = 0;

    gettimeofday(&timer, NULL);
    gettimeofday(&ackTimer, NULL);
    gettimeofday(&sigTimer, NULL);
    gettimeofday(&statusTimer, NULL);
    gettimeofday(&initTimer, NULL);

    ACK = 0x06;
    NAK = 0x15;
    SOH = 0x01;
    CAN = 0x18;
    ETX = 0x03;
    DLE = 0x10;
    EOT = 0x04;
    STX = 0x02;
}
