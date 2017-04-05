#include "radio.h"


void Radio::workCmdK2()
{
    if (k2RXbuffer.length() > 2)  // Min cmd is 2 characters, + semicolon
    {
        size_t len = k2RXbuffer.find(';');
        if (len != k2RXbuffer.npos)  // cmd received on serial
        {
            for (size_t t = 0; t<len; ++t) cmd.push_back(k2RXbuffer[t]);
            //std::cout << cmd << std::endl;
            k2RXbuffer.erase(0, len+1);

            if      (cmd.find("AG") == 0) AG();                 // TODO: commands without functions are not implemented (yet)...
            else if (cmd.find("AN") == 0) k2TXbuffer += "AN1;";
            else if (cmd.find("AI") == 0) AI();
            else if (cmd.find("AP") == 0) k2TXbuffer += "AP0;";
            else if (cmd.find("BG") == 0) BG();
            else if (cmd.find("BN") == 0) BN();
            else if (cmd.find("BW") == 0) BW();
            else if (cmd.find("CP") == 0) k2TXbuffer += "CP000;";
            else if (cmd.find("CW") == 0) k2TXbuffer += "CW70;";
            else if (cmd.find("DS") == 0) DS();
            else if (cmd.find("DT") == 0) k2TXbuffer += "DT1;";
            else if (cmd.find("DV") == 0) k2TXbuffer += "DV0;";
            else if (cmd.find("FA") == 0) FA();
            else if (cmd.find("FB") == 0) FB();
            else if (cmd.find("FI") == 0) k2TXbuffer += "FI0000;";
            else if (cmd.find("FR") == 0) k2TXbuffer += "FR0;";
            else if (cmd.find("FT") == 0) k2TXbuffer += "FT0;";
            else if (cmd.find("FW") == 0) FW();
            else if (cmd.find("GT") == 0) GT();
            else if (cmd.find("ID") == 0) k2TXbuffer += "ID017;";
            else if (cmd.find("IF") == 0) IF();
            else if (cmd.find("IS") == 0) k2TXbuffer += "IS 0000;";
            else if (cmd.find("K2") == 0) K2();
            else if (cmd.find("K3") == 0) K3();
            else if (cmd.find("KS") == 0) k2TXbuffer += "KS020;";
            else if (cmd.find("LK") == 0) LK();
            else if (cmd.find("LN") == 0) k2TXbuffer += "LN0;";
            else if (cmd.find("MD") == 0) MD();
            else if (cmd.find("MG") == 0) k2TXbuffer += "MG030;";
            else if (cmd.find("ML") == 0) k2TXbuffer += "ML030;";
            else if (cmd.find("NB") == 0) NB();
            else if (cmd.find("OM") == 0) k2TXbuffer += "OM AP---F------;";
            else if (cmd.find("PA") == 0) PA();
            else if (cmd.find("PC") == 0) PC();
            else if (cmd.find("PO") == 0) PO();
            else if (cmd.find("PS") == 0) PS();
            else if (cmd.find("RA") == 0) RA();
            else if (cmd.find("RG") == 0) RG();
            else if (cmd.find("RV") == 0) RV();
            else if (cmd.find("RX") == 0) RX();
            else if (cmd.find("SB") == 0) k2TXbuffer += "SB0;";
            else if (cmd.find("SM") == 0) SM();
            else if (cmd.find("RT") == 0) k2TXbuffer += "RT0;";
            else if (cmd.find("SQ") == 0) SQ();
            else if (cmd.find("TQ") == 0) TQ();
            else if (cmd.find("TX") == 0) TX();
            else if (cmd.find("VX") == 0) k2TXbuffer += "VX1;";
            else if (cmd.find("XF") == 0) XF();
            else if (cmd.find("XT") == 0) k2TXbuffer += "XT0;";

            else std::cerr << "Unrecognised K2/K3 command: " << cmd << std::endl;

            cmd.erase();
        }
    }
}

void Radio::AG()
{
    if (cmd[2] == '$') k2TXbuffer += "AG$000;";
    else
    {
        if (cmd.length() == 5)
        {
            cmd.erase(0, 2);
            int tmp = atoi(cmd.c_str());
            if (tmp < 256 && tmp >= 0) vol = tmp / 2.73;

            std::stringstream buf;
            buf << "y" << 99 - vol << "\r";
            skantiCmdBuffer += buf.str();
        }
        else
        {
            std::stringstream buf;
            buf << "AG" << std::setw(3) << std::setfill('0') << (int)(vol * 2.73) << ";";
            k2TXbuffer += buf.str();
        }
    }
}

void Radio::AI()
{
    bool resp = false;
    if (cmd.length() == 3)
    {
        cmd.erase(0, 2);
        int tmp = atoi(cmd.c_str());
        if (tmp < 4 && tmp >= 0) ai = tmp;
        if (ai == 1)
        {
            k2TXbuffer += "AI1;";
            IF();
            resp = true;
        }
    }
    else if (!resp)
    {
        std::stringstream buf;
        buf << "AI" << ai << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::BG()    // BG (Bargraph Read; GET only) RSP format: BGnn;
{
    std::stringstream buf;
    if (!tx) buf << "BG"  << std::setw(2) << std::setfill('0') << (int)(0.5+(signalRX/2)) << "R;";
    else buf << "BG" << std::setw(2) << std::setfill('0') << (int)(0.5+(signalTX/2)) << "T;";

    k2TXbuffer += buf.str();
}

void Radio::BN() // K3 cmd, BN $ (Band Number; GET/SET)SET/RSP format: BNnn;
{
    //TODO
    k2TXbuffer += "BN00;";
}

void Radio::BW() // K3 cmd, BW $ (Filter Bandwidth; GET/SET)SET/RSP format: BWxxxx;
{
    if (cmd.length() == 6)
    {
        cmd.erase(0, 2);
        int tmp = atoi(cmd.c_str());
        if (tmp < 9999 && tmp >= 0)
        {
            if (tmp < 300) { filter = VNARROW; skantiCmdBuffer += 'E'; }
            else if (tmp >= 300 && tmp < 1500) { filter = NARROW; skantiCmdBuffer += 'D'; }
            else if (tmp >= 1500 && tmp < 2800) { filter = WIDE; skantiCmdBuffer += 'C'; }
            else { filter = VWIDE; skantiCmdBuffer += 'B'; }
        }
    }
    else
    {
        std::stringstream buf;
        if (cmd[2] != '$') buf << "BW";
        else buf << "BW$";
        if (filter == VNARROW) buf << "0250;";
        else if (filter == NARROW) buf << "1200;";
        else if (filter == WIDE) buf << "2700;";
        else if (filter == VWIDE) buf << "4000;";

        k2TXbuffer += buf.str();
    }
}

void Radio::DS() // DS (Display Read; GET only)GET format: DS;
{
    std::stringstream buf;
    buf << "DS" << std::setw(8) << std::setfill('0') << freqRX;
    char icon = 0x80, flash = 0x80;
    if (rfamp) icon += 0x10;
    if (att) icon += 0x8;
    buf << icon << flash << ";";
    std::string tmp = buf.str();
    tmp[4] += 0x80;
    tmp[7] += 0x80;
    k2TXbuffer += tmp;
}

void Radio::FA()    // FA and FB (VFO A/B Frequency; GET/SET) SET/RSP format: FAxxxxxxxxxxx; or FBxxxxxxxxxxx; where xxxxxxxxxxx is the frequency in Hz
{
    if (cmd.length() == 13)   // new freq sent on K2 port
    {
        cmd.erase(0, 5);      // ignore FA and 3 first digits
        cmd.pop_back();
        cmd.push_back('0');     // insert 0 for last digit

        int tmp = atoi(cmd.c_str());
        if (tmp <= 30000000 && tmp >= 10000)
        {
            if (tmp != freqRX)
                /*{
                int diff = tmp - freqRX;

                if (diff < 1000 && diff > -1000)
                {
                    if (tunerate != HZ100) { skantiCmdBuffer += "w1\r"; tunerate = HZ100; }
                    if (diff > 0) for (int i=0; i<=diff; i += 100) skantiCmdBuffer += '>';
                    else for (int i=0; i>=diff; i -= 100) skantiCmdBuffer += '=';
                }

                else if (diff <= 10000 && diff >= -10000)
                {
                    if (tunerate != HZ1000) { skantiCmdBuffer += "w2\r"; tunerate = HZ1000; }
                    if (diff > 0) for (int i=0; i<=diff; i += 1000) skantiCmdBuffer += '>';
                    else for (int i=0; i>=diff; i -= 1000) skantiCmdBuffer += '=';
                }
                else*/
            {
                std::stringstream buf;
                if (delayedTXFreqUpdate) buf << ":";
                else buf << ":;";
                buf << tmp / 100 << "\r";
                skantiCmdBuffer += buf.str();
                //}
            }
            freqRX = tmp;
            if (!delayedTXFreqUpdate) freqTX = tmp;
            if (ai == 1) IF();
            else if (ai == 2) { cmd = "FA"; FA(); IF(); }
        }
        else std::cerr << "Frequency out of range" << std::endl;
    }
    else
    {
        std::stringstream buf;
        buf << "FA" << std::setw(11) << std::setfill('0') << freqRX << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::FB() // FB treated same as frequency A for now...
{
    if (cmd.length() == 13)   // new freq sent on K2 port
    {
        cmd.erase(0, 5);      // ignore FA and 3 first digits
        cmd.pop_back();
        cmd.push_back('0');     // insert 0 for last digit

        int tmp = atoi(cmd.c_str());
        if (tmp <= 30000000 && tmp >= 10000) freqRX = tmp;
        else std::cerr << "Frequency out of range" << std::endl;

        /*std::stringstream buf;
             buf << ":;" << freqRX / 100 << "\r";
             skantiCmdBuffer += buf.str();*/

    }
    else
    {
        std::stringstream buf;
        buf << "FB" << std::setw(11) << std::setfill('0') << freqRX << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::FW() // FW (Filter Bandwidth and Number; GET/SET) Basic SET format: FWxxxx;Extended SET format: FWxxxxn;
{
    if (cmd.length() >= 6)
    {
        cmd.erase(0, 2);
        if (k2 < 2 || k3 == 1)
        {
            FILTER f;
            int tmp = atoi(cmd.c_str());
            if (tmp < 9999 && tmp >= 0)
            {
                if (tmp < 300) f = VNARROW;
                else if (tmp >= 300 && tmp < 1500) f = NARROW;
                else if (tmp >= 1500 && tmp < 2800) f = WIDE;
                else f = VWIDE;
                if (f != filter)
                {
                    filter = f;
                    if (filter == VNARROW) skantiCmdBuffer += 'E';
                    else if (filter == NARROW) skantiCmdBuffer += 'D';
                    else if (filter == WIDE) skantiCmdBuffer += 'C';
                    else skantiCmdBuffer += 'B';
                }
            }
        }
        else if (k2 > 1 && k3 == 0)
        {
            if (cmd[4] == '4') { filter = VNARROW; skantiCmdBuffer += 'E'; }
            else if (cmd[4] == '3') { filter = NARROW; skantiCmdBuffer += 'D'; }
            else if (cmd[4] == '2') { filter = WIDE; skantiCmdBuffer += 'C'; }
            else { filter = VWIDE; skantiCmdBuffer += 'B'; }
        }
        if (ai > 1)
        {
            k2TXbuffer += "IS 0000;";   //FIXME
            IF();
        }
    }
    else
    {
        std::stringstream buf;
        if (cmd[2] != '$') buf << "FW";
        else buf << "FW$";
        if (filter == VNARROW) buf << "0250";
        else if (filter == NARROW) buf << "1200";
        else if (filter == WIDE) buf << "2700";
        else if (filter == VWIDE) buf << "4000";

        if (k2 > 1) // K22 extended response
        {
            if (filter == VNARROW) buf << "40";
            else if (filter == NARROW) buf << "30";
            else if (filter == WIDE) buf << "20";
            else if (filter == VWIDE) buf << "10";
        }
        buf << ";";

        k2TXbuffer += buf.str();
    }
}

void Radio::GT() // GT (AGC Time Constant; GET/SET)Basic SET/RSP format: GTnnn; Extended SET/RSP format: GTnnnx;
{
    if (cmd.length() > 4)
    {
        if (cmd[4] == '2') { agcSpeed = true; skantiCmdBuffer += 'K'; }
        else if (cmd[4] == '4') { agcSpeed = false; skantiCmdBuffer += 'L'; }
    }

    if (cmd.length() == 6 && (k2 > 1 || k3 == 1)) // Extended set cmd
    {
        if (cmd[5] == '0') { agc = false; skantiCmdBuffer += 'M'; }
        else if (cmd[5] == '1') { agc = true; skantiCmdBuffer += 'J'; }
    }
    else
    {
        std::stringstream buf;
        if (!agcSpeed) buf << "GT004";
        else buf << "GT002";
        if (k2 > 1 || k3 == 1)
        {
            if (agc) buf << "1";
            else buf << "0";
        }
        buf << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::IF() // IF (Transceiver Information; GET only)RSP format: IF[f]*****+yyyyrx*00tmvspb01*;
{
    std::stringstream buf;
    buf << "IF" << std::setw(11) << std::setfill('0') << freqRX;
    buf << "     +0000" << "00 00";
    if (tx) buf << "1"; else buf << "0";
    switch(mode)
    {
    case(R3E): buf << "4"; break;
    case(LSB): buf << "1"; break;
    case(USB): buf << "2"; break;
    case(MCW): buf << "3"; break;
    case(CW): buf << "3"; break;
    case(AM): buf << "4"; break;
    case(TELEX): buf << "6"; break;
    }
    buf << "000";
    if (k2 == 2) buf << "0"; else buf << "0";   // TODO: Repair K2/K3 extended responses
    if (k3 == 1) buf << "0"; else buf << "0";
    buf << "1 ;";
    k2TXbuffer += buf.str();
}

void Radio::K2() // K2 (Command Mode; GET/SET)SET/RSP format: K2n;
{
    if (cmd.length() == 3)
    {
        cmd.erase(0, 2);
        int tmp = atoi(cmd.c_str());
        if (tmp < 4 && tmp >= 0) k2 = tmp;
    }
    else
    {
        std::stringstream buf;
        buf << "K2"  << k2 << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::K3() // K3 * (Command Mode; GET/SET)SET/RSP format: K3n;
{
    if (cmd.length() == 3)
    {
        if (cmd[2] == '1') k3 = 1; else k3 = 0;
    }
    else
    {
        std::stringstream buf;
        buf << "K3"  << k3 << ";";
        k2TXbuffer += buf.str();
    }
}

void Radio::LK() // LK (VFO Lock; GET/SET)SET/RSP format: LKn;
{
    if (cmd[2] != '$') k2TXbuffer += "LK0;";
    else k2TXbuffer += "LK$0;";
}

void Radio::MD() // MD (Operating Mode; GET/SET)SET/RSP format: MDn;
{
    if (cmd.length() == 3 && cmd[2] != '$')
    {
        cmd.erase(0, 2);
        int tmp = atoi(cmd.c_str());
        if (tmp < 11 && tmp >= 0)
        {
            switch(tmp)
            {
            case(1): mode = LSB; skantiCmdBuffer += 'Y'; break;
            case(2): mode = USB; skantiCmdBuffer += 'X'; break;
            case(3): mode = CW; skantiCmdBuffer += ']'; break;
            case(4): mode = AM; skantiCmdBuffer += 'Z'; break; // really FM, but doesn't exist on TRP8000. Pretends like nothing
            case(5): mode = AM; skantiCmdBuffer += 'Z'; break;
            case(6): mode = TELEX; skantiCmdBuffer += '['; break;
            case(7): mode = CW; skantiCmdBuffer += ']'; break;  // added mode CW-REV = CW
            case(9): mode = TELEX; skantiCmdBuffer += '['; break;   // added mode DATA-REV = TELEX
            }
        }
    }
    else
    {
        std::stringstream buf;
        if (cmd[2] != '$') buf << "MD";
        else buf << "MD$";
        switch(mode)
        {
        case(R3E): buf << "4;"; break;
        case(LSB): buf << "1;"; break;
        case(USB): buf << "2;"; break;
        case(MCW): buf << "3;"; break;
        case(CW): buf << "3;"; break;
        case(AM): buf << "4;"; break;
        case(TELEX): if (k2 ==1 || k2 == 3) buf << "1"; else buf << "6;"; break;        // Added K2 mode 1/3 response
        }
        k2TXbuffer += buf.str();
    }
}

void Radio::NB() // NB (Noise Blanker Mode; GET/SET) SET format: NBn;Basic RSP format: NBn; Extended RSP format: NBnm;
{
    if (cmd[2] != '$') k2TXbuffer += "NB0";
    else k2TXbuffer += "NB$0";
    if (k2 > 1 || k3 == 1) k2TXbuffer += "0;";     // Ext. response cmd
    else k2TXbuffer += ";";
}

void Radio::PA() // PA (Receive Preamp Control; GET/SET) SET/RSP format: PAn;
{
    if (cmd.length() > 2)
    {
        if (cmd[2] == '0') { rfamp = 0; skantiCmdBuffer += 'n'; }
        else if (cmd[2] == '1') { rfamp = 1; skantiCmdBuffer += 'm'; }
    }
    else
    {
        std::stringstream buf;
        if (cmd[2] != '$') buf << "PA"; else buf << "PA$";
        if (rfamp) buf << "1;";
        else buf << "0;";
        k2TXbuffer += buf.str();
    }
}

void Radio::PC() // PC (Power Output Level; GET/SET)Basic SET/RSP format: PCnnn; Extended SET format: PCnnnx; Extended RSP format: PCnnnx;
{
    if (cmd.length() > 2)
    {
        cmd.erase(0, 2);
        cmd[3] = 0;
        int tmp = atoi(cmd.c_str());
        if (tmp < 12) { pwr = LOW; skantiCmdBuffer += 'S'; }
        else if (tmp >= 12 && tmp < 50) { pwr = MEDIUM; skantiCmdBuffer += 'U'; }
        else { pwr = FULL; skantiCmdBuffer += 'W'; }
    }

    else
    {
        if (k2 < 2 && k3 == 0)
        {
            if (pwr == LOW) k2TXbuffer += "PC012;";
            else if (pwr == MEDIUM) k2TXbuffer += "PC050;";
            else k2TXbuffer += "PC120;";
        }
        else
        {
            if (pwr == LOW) k2TXbuffer += "PC0121;";
            else if (pwr == MEDIUM) k2TXbuffer += "PC0501;";
            else k2TXbuffer += "PC1201;";
        }
    }
}

void Radio::PO() // PO ** (Actual Power Output Level; GET only; KX3 only) RSP format: PCnnn;    FIXME
{
    if (pwr == LOW) k2TXbuffer += "PO012;";
    else if (pwr == MEDIUM) k2TXbuffer += "PO050;";
    else k2TXbuffer += "PO120;";
}

void Radio::PS() // PS (Transceiver Power Status; GET only)RSP format: PS1;
{
    if (cmd.length() > 2)
    {
        if (cmd[2] == 1) { txOn = true; skantiCmdBuffer += 'u'; }
        else if (cmd[2] == 0) { txOn = false; skantiCmdBuffer += 'v'; }
    }
    else
    {
        if (txOn) k2TXbuffer += "PS1;";
        else k2TXbuffer += "PS0;";
    }
}

void Radio::RA() // RA (Receive Attenuator Control; GET/SET) SET/RSP format: RAnn;
{
    if (cmd.length() > 3)
    {
        if (cmd[3] == '1') { att = true; skantiCmdBuffer += 'o'; }
        else if (cmd[3] == '0') { att = false; skantiCmdBuffer += 'p'; }
    }
    else
    {
        if (cmd[2] != '$') k2TXbuffer += "RA"; else k2TXbuffer += "RA$";
        if (att) k2TXbuffer += "01;";
        else k2TXbuffer += "00;";
    }
}

void Radio::RG() // K3: RG $ (RF Gain; GET/SET) SET/RSP format: RGnnn; or RG$nnn;
{
    if (cmd.length() < 4)
    {
        if (cmd[2] != '$')
        {
            if (rfamp) k2TXbuffer += "RG250;";
            else k2TXbuffer += "RG190;";
        }
        else
        {
            if (rfamp) k2TXbuffer += "RG$250;";
            else k2TXbuffer += "RG$190;";
        }
    }
}

void Radio::RV() // RV (Firmware Revisions; GET only) GET format: RVx; RSP format: RVxNN.NN
{
    if (cmd[2] == 'M') k2TXbuffer += "RVM01.23;";
    else if (cmd[2] == 'D') k2TXbuffer += "RVD01.23;";
    else if (cmd[2] == 'A') k2TXbuffer += "RVA01.23;";
    else if (cmd[2] == 'R') k2TXbuffer += "RVR01.23;";
    else if (cmd[2] == 'F') k2TXbuffer += "RVF01.23;";
}

void Radio::RX() // RX (Receive Mode; SET only) SET format: RX;
{
    tx = false;
    skantiCmdBuffer += '#';
}

void Radio::SM() // SM (S-meter Read; GET only) RSP format: SMnnnn;
{
    std::stringstream buf;
    if (cmd[2] != '$') buf << "SM"; else buf << "SM$";
    if (!tx)
    {
        if (k3 == 1) buf << std::setw(4) << std::setfill('0') << signalRX << ";";
        else buf << std::setw(4) << std::setfill('0') << signalRX * 0.68 << ";";
    }
    else buf << "0000;";
    k2TXbuffer += buf.str();
}

void Radio::SQ() // SQ (Squelch Level; GET/SET) SET/RSP format: SQnnn;
{
    if (cmd.length() == 5)
    {
        cmd.erase(0, 2);
        int tmp = atoi(cmd.c_str());
        if (tmp > 0) { squelch = true; skantiCmdBuffer += 'q'; }
        else { squelch = false; skantiCmdBuffer += 'r'; }
    }
    else
    {
        std::stringstream buf;
        if (cmd[2] != '$') buf << "SQ"; else buf << "SQ$";
        if (squelch) buf << "010;"; else buf << "000;";
        k2TXbuffer += buf.str();
    }
}

void Radio::TQ() // TQ (Transmit Query; GET only) RSP format: TQ0 (receive mode) or TQ1 (transmit mode)
{
    if (tx) k2TXbuffer += "TQ1;";
    else k2TXbuffer += "TQ0;";
}

void Radio::TX() // TX (Transmit Mode; SET only) SET format: TX;
{
    if (delayedTXFreqUpdate && freqRX != freqTX)
    {
        std::stringstream buf;
        buf << ";" << freqRX / 100;
        skantiCmdBuffer += buf.str();
        freqTX = freqRX;
    }
    tx = true;
    time(&txTimer);
    skantiCmdBuffer += '\"';
}

void Radio::XF() // K3: XF $ (XFIL Number; GET only) RSP format: XFn;
{
    if (cmd[2] != '$') k2TXbuffer += "XF1;";
    else k2TXbuffer += "XF$1;";
}
