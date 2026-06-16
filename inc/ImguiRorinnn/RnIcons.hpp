#pragma once

// RnIcons.hpp — ImguiRorinnn 图标枚举

#include <cstdint>

namespace RorinnnTools::ImguiRorinnn
{

enum class Icon : std::uint32_t
{
    None            = 0,
    Check           = 0xF00C,
    Xmark           = 0xF00D,
    CaretDown       = 0xF0D7,
    CaretRight      = 0xF0DA,
    Gear            = 0xF013,
    Wrench          = 0xF0AD,
    Play            = 0xF04B,
    Stop            = 0xF04D,
    Minus           = 0xF068,
    RotateRight     = 0xF2F9,
    WindowRestore   = 0xF2D2,
    CircleQuestion  = 0xF059,
    CircleInfo      = 0xF05A,
    Copy            = 0xF0C5,
    Download        = 0xF019,
    Upload          = 0xF093,
    FolderOpen      = 0xF07C,
    FloppyDisk      = 0xF0C7,
    Trash           = 0xF1F8,
    MagnifyingGlass = 0xF002,
    Link            = 0xF0C1,
    Discord         = 0xF392,
    Github          = 0xF09B,
};

const char* ToIconString(Icon Value);

} // namespace RorinnnTools::ImguiRorinnn
