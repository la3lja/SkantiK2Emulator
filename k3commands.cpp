#include "radio.h"

void Radio::workCmdK2()
{
    if (k2RXbuffer.length() > 2)  // Min cmd is 2 characters, + semicolon
    {
        std::string cmd;

        size_t len = k2RXbuffer.find(';');
        if (len != k2RXbuffer.npos)  // cmd sent on serial
        {
            for (size_t t = 0; t<len; ++t) cmd.push_back(k2RXbuffer[t]);

            std::cout << cmd;

            k2RXbuffer.erase(0, len+1);

            if (cmd.find("AG") == 0)
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
                    std::stringstream buf;
                    buf << "AG" << std::setw(3) << std::setfill('0') << (int)(vol * 2.73) << ";";
                    k2TXbuffer += buf.str();
                }
            }

            else if (cmd.find("AN") == 0)
            {
                k2TXbuffer += "AN1;";
            }

            else if (cmd.find("AI") == 0)
            {
                if (cmd.length() == 3)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp < 4 && tmp >= 0) ai = tmp;
                }
                std::stringstream buf;
                buf << "AI"  << ai << ";";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("AP") == 0)
            {
                k2TXbuffer += "AP0;";
            }

            else if (cmd.find("BG") == 0)
            {
                std::stringstream buf;
                if (!tx) buf << "BG"  << std::setw(2) << std::setfill('0') << signalRX << "R;";
                else buf << "BG"  << std::setw(2) << std::setfill('0') << signalTX/2 << "T;";

                k2TXbuffer += buf.str();
            }

            else if (cmd.find("BN") == 0)
            {
                std::stringstream buf;
                if (!tx && cmd[2] != '$') buf << "BG"  << std::setw(2) << std::setfill('0') << signalRX << "R;";
                else if (tx && cmd[2] != '$') buf << "BG"  << std::setw(2) << std::setfill('0') << signalTX/2 << "T;";
                else if (!tx && cmd[2] == '$') buf << "BG$"  << std::setw(2) << std::setfill('0') << signalRX << "R;";
                else buf << "BG$"  << std::setw(2) << std::setfill('0') << signalRX << "R;";

                k2TXbuffer += buf.str();
            }

            else if (cmd.find("BW") == 0)
            {
                if (cmd.length() == 6)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp < 9999 && tmp >= 0)
                    {
                        if (tmp < 300) { filter = VNARROW; skantiCmdBuffer += 'E'; }
                        else if (tmp >= 300 && tmp < 1200) { filter = NARROW; skantiCmdBuffer += 'D'; }
                        else if (tmp >= 1200 && tmp < 2800) { filter = WIDE; skantiCmdBuffer += 'C'; }
                        else { filter = VWIDE; skantiCmdBuffer += 'B'; }
                    }
                }
                std::stringstream buf;
                if (cmd[2] != '$') buf << "BW";
                else buf << "BW$";
                if (filter == VNARROW) buf << "0250;";
                else if (filter == NARROW) buf << "0600;";
                else if (filter == WIDE) buf << "2700;";
                else if (filter == VWIDE) buf << "4000;";

                k2TXbuffer += buf.str();
            }

            else if (cmd.find("CP") == 0) k2TXbuffer += "CP000;";

            else if (cmd.find("CW") == 0) k2TXbuffer += "CW70;";

            else if (cmd.find("DT") == 0) k2TXbuffer += "DT1;";

            else if (cmd.find("DV") == 0) k2TXbuffer += "DV0;";

            if (cmd.find("FA") == 0)
            {
                if (cmd.length() == 13)   // new freq sent on K2 port
                {
                    cmd.erase(0, 5);      // ignore FA and 3 first digits
                    cmd.pop_back();
                    cmd.push_back('0');     // insert 0 for last digit

                    int tmp = atoi(cmd.c_str());
                    if (tmp < 30000000 && tmp >= 50000)
                    {
                        if (tmp != freqRX && tmp != freqTX)
                        {
                            std::stringstream buf;
                            if (delayedTXFreqUpdate) buf << ":";
                            else buf << ":;";
                            buf << tmp / 100 << "\r";
                            skantiCmdBuffer += buf.str();
                        }
                        freqRX = tmp;
                        if (!delayedTXFreqUpdate) freqTX = tmp;
                    }
                    else std::cerr << "Frequency out of range" << std::endl;
                }
                std::stringstream buf;
                buf << "FA" << std::setw(11) << std::setfill('0') << freqRX << ";";
                k2TXbuffer += buf.str();

            }

            else if (cmd.find("FB") == 0)        // FB treated same as frequency A for now...
            {
                if (cmd.length() == 13)   // new freq sent on K2 port
                {
                    cmd.erase(0, 5);      // ignore FA and 3 first digits
                    cmd.pop_back();
                    cmd.push_back('0');     // insert 0 for last digit

                    int tmp = atoi(cmd.c_str());
                    if (tmp < 30000000 && tmp >= 50000) freqRX = freqTX = tmp;
                    else std::cerr << "Frequency out of range" << std::endl;

                    /*std::stringstream buf;
                    buf << ":;" << freqRX / 100 << "\r";
                    skantiCmdBuffer += buf.str();*/

                }
                std::stringstream buf;
                buf << "FB" << std::setw(11) << std::setfill('0') << freqRX << ";";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("FI") == 0) k2TXbuffer += "FI0000;";

            else if (cmd.find("FR") == 0)
            {
                k2TXbuffer += "FR0;";
            }

            else if (cmd.find("FT") == 0)
            {
                k2TXbuffer += "FT0;";
            }

            else if (cmd.find("FW") == 0)
            {
                if (cmd.length() == 6)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp < 9999 && tmp >= 0)
                    {
                        if (tmp < 300) { filter = VNARROW; skantiCmdBuffer += 'E'; }
                        else if (tmp >= 300 && tmp < 1200) { filter = NARROW; skantiCmdBuffer += 'D'; }
                        else if (tmp >= 1200 && tmp < 2800) { filter = WIDE; skantiCmdBuffer += 'C'; }
                        else { filter = VWIDE; skantiCmdBuffer += 'B'; }
                    }
                }
                std::stringstream buf;
                if (cmd[2] != '$') buf << "FW";
                else buf << "FW$";
                if (filter == VNARROW) buf << "0250;";
                else if (filter == NARROW) buf << "0600;";
                else if (filter == WIDE) buf << "2700;";
                else if (filter == VWIDE) buf << "4000;";

                k2TXbuffer += buf.str();
            }

            else if (cmd.find("GT") == 0)
            {
                if (cmd.length() == 5)
                {
                    if (cmd[5] == 2) { agcSpeed = true; skantiCmdBuffer += 'K'; }
                    else if (cmd[5] == 4) { agcSpeed = false; skantiCmdBuffer += 'L'; }
                }
                std::stringstream buf;
                if (!agcSpeed) buf << "GT004;";
                else buf << "GT002;";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("ID") == 0)
            {
                k2TXbuffer += "ID017;";
            }

            else if (cmd.find("IF") == 0)
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

            else if (cmd.find("IS") == 0)
            {
                k2TXbuffer += "IS 0000;";
            }

            else if (cmd.find("K2") == 0)
            {
                if (cmd.length() == 3)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp < 4 && tmp >= 0) k2 = tmp;
                }
                std::stringstream buf;
                buf << "K2"  << k2 << ";";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("K3") == 0)
            {
                if (cmd.length() == 3)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp < 4 && tmp >= 0) k3 = tmp;
                }
                std::stringstream buf;
                buf << "K3"  << k3 << ";";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("KS") == 0)
            {
                k2TXbuffer += "KS020;";
            }

            else if (cmd.find("LK") == 0)
            {
                if (cmd[2] != '$') k2TXbuffer += "LK0;";
                else k2TXbuffer += "LK$0;";
            }

            else if (cmd.find("LN") == 0) k2TXbuffer += "LN0;";

            else if (cmd.find("MD") == 0)
            {
                if (cmd.length() == 3)
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
                        }

                    }
                }
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
                case(TELEX): buf << "6;"; break;
                }
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("MG") == 0) k2TXbuffer += "MG030;";

            else if (cmd.find("ML") == 0) k2TXbuffer += "ML030;";

            else if (cmd.find("NB") == 0)
            {
                if (cmd[2] != '$') k2TXbuffer += "NB0;";
                else k2TXbuffer += "NB$0;";
            }

            else if (cmd.find("OM") == 0)
            {
                k2TXbuffer += "K3 OM APXSDFf-----;";
            }

            else if (cmd.find("PA") == 0)
            {
                if (cmd.length() > 2)
                {
                    if (cmd[2] == '0') { rfamp = 0; skantiCmdBuffer += 'n'; }
                    else if (cmd[2] == '1') { rfamp = 1; skantiCmdBuffer += 'm'; }
                }
                std::stringstream buf;
                if (cmd[2] != '$') buf << "PA"; else buf << "PA$";
                if (rfamp) buf << "1;";
                else buf << "0;";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("PC") == 0)
            {
                if (cmd.length() > 2)
                {
                    cmd.erase(0, 2);
                    if (k2 == 2 && cmd[3] == 0) { pwr = LOW; skantiCmdBuffer += 'S'; }  // Power is max 12W, setting to LOW always
                    else if (k2 == 2 && cmd[3] == 1)                                    // 0-120W range
                    {
                        cmd.erase(3,1);
                        int tmp = atoi(cmd.c_str());
                        if (tmp < 12) { pwr = LOW; skantiCmdBuffer += 'S'; }
                        else if (tmp >= 12 && tmp < 50) { pwr = MEDIUM; skantiCmdBuffer += 'U'; }
                        else { pwr = FULL; skantiCmdBuffer += 'W'; }
                    }
                }
                if (k2 != 2)
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

            else if (cmd.find("PO") == 0)
            {
                if (pwr == LOW) k2TXbuffer += "PO012;";
                else if (pwr == MEDIUM) k2TXbuffer += "PO050;";
                else k2TXbuffer += "PO120;";
            }

            else if (cmd.find("PS") == 0)
            {
                if (cmd.length() > 2)
                {
                if (cmd[2] == 1) { txOn = true; skantiCmdBuffer += 'u'; }
                else if (cmd[2] == 0) { txOn = false; skantiCmdBuffer += 'v'; }
                }
                if (txOn) k2TXbuffer += "PS1;";
                else k2TXbuffer += "PS0;";
            }

            else if (cmd.find("RA") == 0)
            {
                if (cmd[2] != '$') k2TXbuffer += "RA"; else k2TXbuffer += "RA$";
                if (cmd.length() > 2)
                {
                    if (cmd[3] == '1') { att = true; skantiCmdBuffer += 'o'; }
                    else if (cmd[3] == '0') { att = false; skantiCmdBuffer += 'p'; }
                }
                if (att) k2TXbuffer += "01;";
                else k2TXbuffer += "00;";
            }

            else if (cmd.find("RG") == 0)
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

            else if (cmd.find("RV") == 0)
            {
                if (cmd[2] == 'M') k2TXbuffer += "RVM99.99;";
                else if (cmd[2] == 'D') k2TXbuffer += "RVD99.99;";
                else if (cmd[2] == 'A') k2TXbuffer += "RVA99.99;";
                else if (cmd[2] == 'R') k2TXbuffer += "RVR99.99;";
                else k2TXbuffer += "RVF99.99;";
            }

            else if (cmd.find("RX") == 0)
            {
                tx = false;
                skantiCmdBuffer += '#';
                k2TXbuffer += "RX;";
            }

            else if (cmd.find("SB") == 0)
            {
                k2TXbuffer += "SB0;";
            }

            else if (cmd.find("SM") == 0)
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

            else if (cmd.find("RT") == 0)
            {
                k2TXbuffer += "RT0;";
            }

            else if (cmd.find("SQ") == 0)
            {
                if (cmd.length() == 5)
                {
                    cmd.erase(0, 2);
                    int tmp = atoi(cmd.c_str());
                    if (tmp > 0) { squelch = true; skantiCmdBuffer += 'q'; }
                    else { squelch = false; skantiCmdBuffer += 'r'; }
                }
                std::stringstream buf;
                if (cmd[2] != '$') buf << "SQ"; else buf << "SQ$";
                if (squelch) buf << "010;"; else buf << "000;";
                k2TXbuffer += buf.str();
            }

            else if (cmd.find("TQ") == 0)
            {
                if (tx) k2TXbuffer += "TQ1;";
                else k2TXbuffer += "TQ0;";
            }

            else if (cmd.find("TX") == 0)
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
                k2TXbuffer += "TX;";
            }

            else if (cmd.find("VX") == 0) k2TXbuffer += "VX1;";

            else if (cmd.find("XF") == 0)
            {
                if (cmd[2] != '$') k2TXbuffer += "XF1;";
                else k2TXbuffer += "XF$1;";
            }

            else if (cmd.find("XT") == 0) k2TXbuffer += "XT0;";

            //else std::cout << "Unrecognised K2/K3 command: " << cmd << std::endl;

            cmd.erase();
        }
    }
}

