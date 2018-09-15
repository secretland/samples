#include <QString>
#include <QSysInfo>
#include <iostream>

int main(int, char**)
{
    QString text("SystemInfo:");
    std::cout << text.toStdString() << std::endl;
    std::cout << "\tBuild Abi: " << QSysInfo::buildAbi().toStdString() << std::endl;
    std::cout << "\tBuild Cpu Architecture: " << QSysInfo::buildCpuArchitecture().toStdString() << std::endl;
    std::cout << "\tCurrent Cpu Architecture: " << QSysInfo::currentCpuArchitecture().toStdString() << std::endl;
    std::cout << "\tKernel Type: " << QSysInfo::kernelType().toStdString() << std::endl;
    std::cout << "\tKernel Version: " << QSysInfo::kernelVersion().toStdString() << std::endl;
    std::cout << "\tProduct Type: " << QSysInfo::productType().toStdString() << std::endl;
    std::cout << "\tProduct Version: " << QSysInfo::productVersion().toStdString() << std::endl;
    return 0;
}
