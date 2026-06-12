#pragma once

// D3D12Locator — D3D12 方法地址定位接口

#include "Graphics/GraphicsLocator.hpp"

#include <vector>

namespace RorinnnTools::Graphics
{
struct D3D12Methods
{
    std::vector<VTableEntry> DeviceMethods;
    std::vector<VTableEntry> CommandQueueMethods;
    std::vector<VTableEntry> CommandAllocatorMethods;
    std::vector<VTableEntry> CommandListMethods;
    std::vector<VTableEntry> SwapChainMethods;
};

LocateStatus LocateD3D12(D3D12Methods& Out);

} // namespace RorinnnTools::Graphics
