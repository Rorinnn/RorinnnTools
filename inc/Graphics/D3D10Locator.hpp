#pragma once

// D3D10Locator — D3D10 方法地址定位接口

#include "Graphics/GraphicsLocator.hpp"

#include <vector>

namespace RorinnnTools::Graphics
{
struct D3D10Methods
{
    std::vector<VTableEntry> SwapChainMethods;
    std::vector<VTableEntry> DeviceMethods;
};

LocateStatus LocateD3D10(D3D10Methods& Out);

} // namespace RorinnnTools::Graphics
