#include "config.h"

Config::Config()
{

}

bool Config::init(std::string filename, Radio *r)
{
    INIReader *cfg = new INIReader(filename);

    if (cfg->ParseError() < 0)
    {
        std::cerr << "Can't load " << filename << ", creating defaults" << std::endl;
    }

    r->serialportK2 = cfg->Get("Serial config", "K2/K3 serial port", "/dev/ttyUSB0");
    r->serialportSkanti = cfg->Get("Serial config", "Skanti serial port", "/dev/ttyUSB1");
    r->serSpeedK2 = cfg->GetInteger("Serial config", "K2/K3 baud rate", 4800);
    r->serSpeedSkanti = cfg->GetInteger("Serial config", "Skanti baud rate", 2400);
    updSignal = cfg->GetInteger("Intervals", "Signal strength update rate (seconds)", 0);
    updStatus = cfg->GetInteger("Intervals", "CU status update rate (seconds)", 0);
    debugK2RTX = cfg->GetBoolean("Debug", "show K2/K3 serial RX/TX", false);
    debugSkantiRTX = cfg->GetBoolean("Debug", "show Skanti serial RX/TX", false);
    r->txTimeout = cfg->GetInteger("Intervals", "TX timeout", 180);
    r->delayedTXFreqUpdate = cfg->GetBoolean("Other", "delay TX frequency update", false);

    delete cfg;
    printConfig(r);
    return true;
}

void Config::printConfig(Radio *r)
{
    std::cout << "Serial config:" << std::endl;
    std::cout << "K2/K3 serial port\t" << r->serialportK2 << std::endl;
    std::cout << "K2/K3 baud rate\t\t" << r->serSpeedK2 << std::endl;
    std::cout << "Skanti serial port\t" << r->serialportSkanti << std::endl;
    std::cout << "K2/K3 baud rate\t\t" << r->serSpeedSkanti << std::endl << std::endl;

    std::cout << "Intervals:" << std::endl;
    std::cout << "Signal strength update rate (seconds)\t" << updSignal << std::endl;
    std::cout << "CU status update rate (seconds)\t\t" << updStatus << std::endl;
    std::cout << "TX timeout (seconds)\t\t\t" << r->txTimeout << std::endl;
    std::cout << "Delay TX frequency update:\t\t" << r->delayedTXFreqUpdate << std::endl;
}

