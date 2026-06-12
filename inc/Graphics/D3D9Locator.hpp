#pragma once

// D3D9Locator — D3D9 方法地址定位接口

#include "Graphics/GraphicsLocator.hpp"

#include <vector>

namespace RorinnnTools::Graphics
{
struct D3D9Methods
{
    std::vector<VTableEntry> DeviceMethods;
};

LocateStatus LocateD3D9(D3D9Methods& Out);

} // namespace RorinnnTools::Graphics
