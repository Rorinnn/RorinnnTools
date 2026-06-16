#pragma once

// Base64Zlib.hpp — Base64 与 zlib 解码工具

#include <cstddef>
#include <string>
#include <string_view>

namespace RorinnnTools::Encoding
{
bool DecodeBase64Zlib(std::string_view Text, std::string& Plain, std::string& ErrorText, size_t MaxOutputSize);
} // namespace RorinnnTools::Encoding
