#pragma once

// MachineId.hpp — Windows 机器码生成工具

#include <string>

namespace RorinnnTools
{

struct MachineCodeResult
{
    bool        Success = false;
    std::string MachineCode;
    std::string Message;
};

MachineCodeResult BuildMachineCode();

} // namespace RorinnnTools
