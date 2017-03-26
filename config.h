#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <iostream>
#include "radio.h"
#include "inih/INIReader.h"


class Config
{
    INIReader *cfg;
    void printConfig(Radio *r);

public:
    int updStatus, updSignal, txTimeout;
    bool debugSkantiRTX, debugK2RTX;
    Config();
    bool init(std::string filename, Radio *r);
};

#endif // CONFIG_H
