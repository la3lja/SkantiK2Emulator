#include "radio.h"
#include "k3commands.cpp"

Radio::Radio()
{
    initState = 0;
    skantiExpectAck = false;
    skantiLinkActive = false;
    skantiReqStatus = false;
    radioOff = false;
    resetCtr = 0;
    signalState = 0;
    cmdTimeout = 500;
    timeoutCtr = 0;

    ai = 0;
    k2 = 0;
    k3 = 0;

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

bool Radio::openK2Port()
{
    int flags;
    serHandleK2 = open(serialportK2.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serHandleK2 == -1)
    {
        perror("Can't open port: ");
        return false;
    }

    flags = fcntl(serHandleK2, F_GETFL);
    flags |= O_NONBLOCK;

    if (fcntl(serHandleK2, F_SETFL, flags) < 0)
    {
        perror("fcntl failed: ");
        return false;
    }
    struct termios options1;

    speed_t br;

    if (serSpeedK2 == 2400) br = B2400;
    else if (serSpeedK2 == 38400) br = B38400;
    else if (serSpeedK2 == 9600) br = B9600;
    else if (serSpeedK2 == 19200) br = B19200;
    else br = B4800;

    tcgetattr(serHandleK2, &options1);
    cfsetispeed(&options1, br);
    cfsetospeed(&options1, br);
    options1.c_cflag |= (CLOCAL | CREAD);
    options1.c_cflag &= ~PARENB;
    options1.c_cflag &= ~CSTOPB;
    options1.c_cflag &= ~CSIZE;
    options1.c_cflag |= CS8;
    options1.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options1.c_iflag &= ~(ICRNL | IUCLC | INLCR | IGNCR);
    options1.c_oflag &= ~OPOST;

    tcsetattr(serHandleK2, TCSANOW, &options1);
    return true;
}


bool Radio::openSkantiPort()
{
    int flags;
    serHandleSkanti = open(serialportSkanti.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serHandleSkanti == -1)
    {
        perror("Can't open port: ");
        return false;
    }
    flags = fcntl(serHandleSkanti, F_GETFL);
    flags |= O_NONBLOCK;
    if (fcntl(serHandleSkanti, F_SETFL, flags) < 0)
    {
        perror ("fcntl failed: ");
        return false;
    }
    struct termios options;

    speed_t br;

    if (serSpeedSkanti == 300) br = B300;
    else br = B2400;

    tcgetattr(serHandleSkanti, &options);
    cfsetispeed(&options, br);
    cfsetospeed(&options, br);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag |= PARENB;
    options.c_cflag |= PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(ICRNL | IUCLC | INLCR | IGNCR);
    options.c_oflag &= ~OPOST;

    tcsetattr(serHandleSkanti, TCSANOW, &options);
    skantiTXbuffer += "\7\7\7";
    //updSkantiStatus();
    return true;
}

void Radio::updK2Comm()
{
    checkSerialK2();
    if (k2RXbuffer.length() > 1) workCmdK2();
    checkSerialK2();
}

void Radio::updSkantiComm()
{
    checkSerialSkanti();
}

void Radio::checkSerialK2()
{
    char byte = '\0';
    int bytesRead;
    do
    {
        if ((bytesRead = read(serHandleK2, &byte, 1)) > 0) k2RXbuffer.push_back(byte);
    }
    while (bytesRead > 0);

    if (!k2TXbuffer.empty())
    {
        //std::cerr << " -> " << k2TXbuffer << std::endl;
        if ((size_t)write(serHandleK2, k2TXbuffer.c_str(), k2TXbuffer.length()) == k2TXbuffer.length()) k2TXbuffer.clear();
    }
}

void Radio::checkSerialSkanti()
{
    char byte = '\0';
    int bytesRead;
    do
    {
        if ((bytesRead = read(serHandleSkanti, &byte, 1)) > 0) skantiRXbuffer.push_back(byte);
    }
    while (bytesRead > 0);

    if (skantiExpectAck)
    {
        struct timeval timer2;
        gettimeofday(&timer2, NULL);
        auto diff = (double)(timer2.tv_usec - ackTimer.tv_usec) + (double)(timer2.tv_sec - ackTimer.tv_sec) * 1000000;

        if (diff > 80000)
        {
            ++ackTimeoutCtr;
            std::cerr << "ACK timeout..." << std::endl;
            skantiExpectAck = false;
        }
    }

    if (ackTimeoutCtr > 50) // looks like radio is unresponsive, probably swithced off. Let's slow things down and wait for it to come back
    {
        radioOff = true;
        ackTimeoutCtr = 0;
    }

    if (radioOff)
    {
        if (skantiRXbuffer.length() > 0)
        {
            radioOff = false;
        }
    }

    if (skantiReqStatus)
    {
        struct timeval timer2;
        gettimeofday(&timer2, NULL);
        auto diff = (double)(timer2.tv_usec - statusTimer.tv_usec) + (double)(timer2.tv_sec - statusTimer.tv_sec) * 1000000;

        if (diff > 2000000)
        {
            std::cerr << "CMD 29 timeout" << std::endl;
            skantiReqStatus = false;
            ++timeoutCtr;
        }
    }

    if (tx)
    {
        if (difftime(time(NULL), txTimer) > txTimeout)      // TX timeout reached
        {
            tx = false;
            skantiCmdBuffer += '#';
            std::cerr << "TX timeout (" << txTimeout << "), switching to RX" << std::endl;
        }
    }

    if (signalState != 0)
    {
        struct timeval timer2;
        gettimeofday(&timer2, NULL);
        auto diff = (double)(timer2.tv_usec - sigTimer.tv_usec) + (double)(timer2.tv_sec - sigTimer.tv_sec) * 1000000;

        if (diff > 250000)
        {
            //std::cerr << "Signal strength timeout" << std::endl;
            signalState = 0;
            if (!tx)
            {
                skantiTXbuffer.push_back(CAN);
                skantiTXbuffer.push_back(EOT);
                ++timeoutCtr;
            }
        }
    }

    if (timeoutCtr == 10)
    {
        initState = 0;
        timeoutCtr = 0;
    }

    if (!skantiRXbuffer.empty())
    {
        /*std::cerr << std::showbase // show the 0x prefix
             << std::internal // fill between the prefix and the number
             << std::setfill('0'); // fill with 0s

            std::cerr << "RX: " << std::setw(3) << std::hex << (int)skantiRXbuffer[0] << " ";*/
    }

    if (!skantiRXbuffer.empty() && initSkantiLink() && signalState < 2)
    {
        if (skantiRXbuffer.length() > 1)
        {
            skantiRXbuffer.clear();
            initState = 0;
            skantiCmdBuffer.clear();
        }

        if (skantiRXbuffer[0] == ACK && skantiExpectAck)
        {
            ackTimeoutCtr = 0;
            if (signalState == 1) ++signalState;    // CU acked our request for signal update, proceed

            cmdTimeout = 500;
            //std::cout << "ack received and expected" << std::endl;
            skantiRXbuffer.clear();
            if (skantiTXbuffer.length() > 1)
            {
                for (size_t i=0; i<skantiTXbuffer.length(); ++i) skantiTXbuffer[i] = skantiTXbuffer[i+1];
                skantiTXbuffer.pop_back();
            }
            else
            {
                skantiTXbuffer.clear();
            }
            skantiExpectAck = false;
        }

        else if (skantiRXbuffer[0] == NAK && skantiExpectAck)
        {
            //std::cerr << "NAK received, ack expected. resending last TX" << std::endl;
            skantiRXbuffer.clear();
            skantiExpectAck = false;
            cmdTimeout *= 2;
        }
        else if (skantiRXbuffer[0] == ACK && !skantiExpectAck)
        {
            //std::cerr << "received ACK, expected nothing, better play it cool" << std::endl;
            skantiRXbuffer.clear();
        }
        else if (signalState == 0 && !skantiReqStatus && skantiRXbuffer[0] != NAK && skantiRXbuffer[0] != ACK && skantiRXbuffer[0] != 0)
        {
            //std::cerr << "received sth else: " << std::showbase << std::internal << std::setfill('0') << std::setw(3) << std::hex << (int)skantiRXbuffer[0] << " , sending ACK back, I am sure CUR will be happy!" << std::endl;
            skantiTXbuffer.push_back(ACK);
            skantiRXbuffer.clear();
        }
        else if (skantiReqStatus)
        {
            updSkantiStatus();
        }
        else skantiRXbuffer.clear();
    }

    if (!radioOff)
    {
        if (!skantiExpectAck && !skantiTXbuffer.empty())
        {
            if (!initSkantiLink()) initSkantiLink();
            else sendSkantiData();
        }
        else if (!skantiExpectAck && !skantiCmdBuffer.empty() && skantiTXbuffer.empty() && signalState == 0 && !skantiReqStatus)
        {
            skantiTXbuffer += skantiCmdBuffer;

            if (!tx)
            {
                skantiTXbuffer.push_back('\r');
                skantiTXbuffer.push_back(EOT);
            }
            skantiCmdBuffer.clear();
        }
    }
    else
    {
        if (difftime(time(NULL), radioOffTimer) > 3)
        {
            time(&radioOffTimer);
            initSkantiLink();
        }
        else
        {
            initState = 0;
            skantiTXbuffer.clear();
            skantiCmdBuffer.clear();
        }
    }

    if (signalState > 0) updSignalStatus();
}


bool Radio::initSkantiLink()
{

    struct timeval timer2;
    gettimeofday(&timer2, NULL);
    auto diff = (double)(timer2.tv_usec - initTimer.tv_usec) + (double)(timer2.tv_sec - initTimer.tv_sec) * 1000000;
    //if (diff < 500000) cout << "debug diff " << diff  << " initstate " << initState << " rxbuf len " << skantiRXbuffer.length() << " txbuf len " << skantiTXbuffer.length() << " exp.ack: " << skantiExpectAck << std::endl;

    if (initState > 0 && initState < 5 && diff > 78000) initState = -1;        // Timeout, restarting

    if (initState == 0 && timeoutCtr < 10)
    {
        skantiLinkActive = true;
        gettimeofday(&initTimer, NULL);
        writeToSkanti(SOH);
        ++initState;
    }
    else if (initState == 0 && timeoutCtr >= 50)
    {
        initState = -1;
        timeoutCtr = 0;
    }
    else if (initState == 1 && diff > 12000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK)
        {
            if (writeToSkanti(STX)) ++initState;
            else initState = -1;
        }
        else if (skantiRXbuffer[0] == NAK) { initState = -1; writeToSkanti(DLE); }
        else initState = -1;

        gettimeofday(&initTimer, NULL);
        skantiRXbuffer.clear();
    }

    else if (initState == 2 && diff > 12000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK && writeToSkanti(CAN))
        {
            ++initState;        // CU ready for receiving commands at this stage
        }
        else initState = 0;
        gettimeofday(&initTimer, NULL);
        skantiRXbuffer.clear();
    }


    else if (initState == 3 && diff > 12000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK && writeToSkanti('\r')) ++initState;
        else initState = 0;
        gettimeofday(&initTimer, NULL);
        skantiRXbuffer.clear();
    }

    else if (initState == 4 && diff > 2000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK && writeToSkanti('\r')) ++initState;
        else initState = 0;
        gettimeofday(&initTimer, NULL);
        skantiRXbuffer.clear();
    }

    else if (initState == 5 && diff > 2000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK && writeToSkanti('\r')) ++initState;
        else initState = 0;
        gettimeofday(&initTimer, NULL);
        skantiRXbuffer.clear();
    }

    else if (initState == 6 && diff > 2000 && !skantiRXbuffer.empty())
    {
        if (skantiRXbuffer[0] == ACK)
        {
            ++initState;
            std::cerr << "CUR initialized, continuing..." << std::endl;
        }
        else initState = 0;
        gettimeofday(&initTimer, NULL);
        usleep(4000);
        skantiRXbuffer.clear();
    }


    else if (initState == -1)
    {
        if (resetCtr < 5)
        {
            std::cerr << "Reset of Skanti TRP8000 in progress" << std::endl;
            writeToSkanti(SOH);
            usleep(5000);
            writeToSkanti(STX);
            usleep(5000);
            writeToSkanti('!');
            sleep(3);
            initState = 0;
            gettimeofday(&initTimer, NULL);
            skantiRXbuffer.clear();
            //skantiTXbuffer.clear();
            ++resetCtr;
        }

    }


    if (initState == 7)
    {
        resetCtr = 0;
        return true;
    }
    else return false;
}

bool Radio::sendSkantiData()
{
    struct timeval timer2;
    gettimeofday(&timer2, NULL);
    auto diff = (double)(timer2.tv_usec - timer.tv_usec) + (double)(timer2.tv_sec - timer.tv_sec) * 1000000;

    if (!skantiTXbuffer.empty() && diff > cmdTimeout)
    {
        /*std::cerr << std::showbase // show the 0x prefix
             << std::internal // fill between the prefix and the number
             << std::setfill('0'); // fill with 0s

        for (unsigned i=0; i<skantiTXbuffer.length(); ++i) std::cerr << "TX: " << std::setw(3) << std::hex << (int)skantiTXbuffer[0] << " ";*/

        gettimeofday(&timer, NULL);
        if (write (serHandleSkanti, &skantiTXbuffer[0], 1) == 1)
        {
            if (skantiTXbuffer[0] != ACK)
            {
                skantiExpectAck = true;
                gettimeofday(&ackTimer, NULL);
            }
            else
            {
                if (skantiTXbuffer.length() == 1) skantiTXbuffer.clear();
                else
                {
                    for (size_t i=0; i<skantiTXbuffer.length(); ++i) skantiTXbuffer[i] = skantiTXbuffer[i+1];
                    skantiTXbuffer.pop_back();
                }
            }
            return true;
        }
    }
    return false;
}

char Radio::readSkantiData()
{
    return 0;
}

bool Radio::skantiEndLink()
{
    std::cerr << "End skanti link" << std::endl;
    skantiTXbuffer.push_back(EOT);
    skantiLinkActive = false;
    return true;
}

bool Radio::writeToSkanti(char c)
{
    /*std::cerr << std::showbase // show the 0x prefix
         << std::internal // fill between the prefix and the number
         << std::setfill('0'); // fill with 0s
    std::cerr << "TX: " << std::setw(3) << std::hex << (int)c << " " <<std::endl;*/

    if (write(serHandleSkanti, &c, 1)) return true; else return false;
}

bool Radio::updSkantiStatus()
{
    if (!skantiReqStatus && signalState == 0 && skantiTXbuffer.empty())
    {
        skantiTXbuffer.push_back(')');
        skantiReqStatus = true;
        gettimeofday(&statusTimer, NULL);
    }
    else if (skantiReqStatus && !skantiRXbuffer.empty() && skantiRXbuffer[0] != '>')
    {
        skantiStatusBuffer.push_back(skantiRXbuffer[0]);
        skantiTXbuffer.push_back(ACK);
        skantiRXbuffer.clear();
    }
    else if (skantiReqStatus && skantiRXbuffer[0] == '>')
    {
        std::string rx, stx;

        for (int i=2, j=8; i<8; ++i, ++j)
        {
            rx.push_back(skantiStatusBuffer[i]);
            stx.push_back(skantiStatusBuffer[j]);
        }

        int rxF = 100 * atoi(rx.c_str()), txF = 100 * atoi(stx.c_str());

        if (rxF <= 30000000 && rxF >= 10000 && txF <= 30000000 && txF >= 10000)
        {
            if (freqRX != rxF || freqTX != txF)
            {
                freqRX = rxF;
                freqTX = txF;
                if (ai == 1) IF();
                else if (ai > 1) { FA(); IF(); }
            }
        }
        MODE md;
        if (skantiStatusBuffer[14] == '0') md = USB;
        else if (skantiStatusBuffer[14] == '1') md = R3E;
        else if (skantiStatusBuffer[14] == '2') md = AM;
        else if (skantiStatusBuffer[14] == '3') md = CW;
        else if (skantiStatusBuffer[14] == '4') md = MCW;
        else if (skantiStatusBuffer[14] == '5') md = TELEX;
        else md = LSB;
        if (md != mode)
        {
            mode = md;
            if (ai >1) IF();
        }
        if (skantiStatusBuffer[17] == '0') tunerate = HZ10;
        else if (skantiStatusBuffer[17] == '1') tunerate = HZ100;
        else tunerate = HZ1000;

        FILTER f;
        if (skantiStatusBuffer[21] == '0') f = WIDE;
        else if (skantiStatusBuffer[21] == '1') f= VWIDE;
        else if (skantiStatusBuffer[21] == '2') f = NARROW;
        else f = VNARROW;
        if (f != filter)
        {
            filter = f;
            if (ai == 1) IF();
            if (ai > 1) { FW(); IF(); }
        }
        std::string tmp;
        std::stringstream cmd;
        tmp.push_back(skantiStatusBuffer[15]);
        tmp.push_back(skantiStatusBuffer[16]);
        cmd << std::hex << tmp;
        int v;
        cmd >> v;
        v = 186 - v;
        v /= 2;
        if (v != vol)
        {
            vol = v;
            if (ai > 1) AG();
        }

        bool a, s;
        if (skantiStatusBuffer[22] == '0')
        {
            a = true;
            s = false;
        }
        else if (skantiStatusBuffer[22] == '1')
        {
            a = true;
            s = true;
        }
        else if (skantiStatusBuffer[22] == '2')
        {
            a = false;
            s = false;
        }
        else
        {
            a = false;
            s = true;
        }
        if (a != agc || s != agcSpeed)
        {
            agc = a;
            agcSpeed = s;
            if (ai > 1) GT();
        }

        bool r, at;
        if (skantiStatusBuffer[23] == '0')
        {
            r = false;
            at = false;
        }
        else if (skantiStatusBuffer[23] == '1')
        {
            r = true;
            at = false;
        }
        else if (skantiStatusBuffer[23] == '2')
        {
            r = false;
            at = true;
        }
        else
        {
            r = true;
            at = true;
        }
        if (r != rfamp || at != att)
        {
            rfamp = r;
            att = at;
            if (ai > 1) { PA(); RA(); }
        }

        if(skantiStatusBuffer[24] == '0') pwr = FULL;
        else if(skantiStatusBuffer[24] == '1') pwr = MEDIUM;
        else pwr = LOW;

        if (skantiStatusBuffer[25] == '0')
        {
            squelch = false;
            duplex = false;
        }
        else if (skantiStatusBuffer[25] == '1')
        {
            squelch = false;
            duplex = true;
        }
        else if (skantiStatusBuffer[25] == '2')
        {
            squelch = true;
            duplex = false;
        }
        else
        {
            squelch = true;
            duplex = true;
        }

        skantiStatusBuffer.clear();
        skantiRXbuffer.clear();
        if (!tx)
        {
            skantiTXbuffer.push_back(ACK);
            skantiTXbuffer.push_back(EOT);
        }
        skantiReqStatus = false;
        timeoutCtr = 0;
    }
    return true;
}

bool Radio::updSignalStatus()
{
    if (signalState == 0 && !skantiReqStatus && skantiTXbuffer.empty())
    {
        gettimeofday(&sigTimer, NULL);
        skantiTXbuffer.push_back('*');
        ++signalState;
    }

    else
    {
        struct timeval timer2;
        gettimeofday(&timer2, NULL);
        auto diff = (double)(timer2.tv_usec - sigTimer.tv_usec) + (double)(timer2.tv_sec - sigTimer.tv_sec) * 1000000;

        if (signalState == 2 && diff > 80000 && !skantiRXbuffer.empty()) // waited long enough for this shit
        {
            signalRX = (int)skantiRXbuffer[0] - 96;
            skantiRXbuffer.clear();
            skantiTXbuffer.push_back(ACK);
            ++signalState;
        }
        else if (signalState == 3 && diff > 80000 && !skantiRXbuffer.empty())
        {
            signalTX = (int)skantiRXbuffer[0] - 96;
            skantiRXbuffer.clear();
            skantiTXbuffer.push_back(CAN);
            if (!tx)
            {
                skantiTXbuffer.push_back(EOT);
            }
            signalState = 0;
            skantiRXbuffer.clear();
            return true;
            timeoutCtr = 0;
        }
    }
    return false;
}
