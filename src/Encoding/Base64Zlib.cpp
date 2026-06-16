// Base64Zlib.cpp — Base64 与 zlib 解码工具

#include "Encoding/Base64Zlib.hpp"

#include <Windows.h>
#include <zlib.h>

#include <cstdint>
#include <string>
#include <vector>

namespace RorinnnTools::Encoding
{
namespace
{
static constexpr DWORD  CryptStringBase64 = 0x00000001;
static constexpr size_t InflateChunkSize  = 16 * 1024;

static std::string FormatZlibError(int Status, z_stream& Stream)
{
    std::string Error  = "zlib 解压失败: ";
    Error             += std::to_string(Status);
    if (Stream.msg && Stream.msg[0])
    {
        Error += ", ";
        Error += Stream.msg;
    }
    return Error;
}

static bool DecodeBase64(std::string_view Text, std::vector<uint8_t>& Bytes, std::string& ErrorText)
{
    Bytes.clear();
    ErrorText.clear();
    if (Text.empty())
    {
        ErrorText = "base64 为空";
        return false;
    }

    std::string  Padded(Text);
    const size_t Remainder = Padded.size() % 4;
    if (Remainder == 1)
    {
        ErrorText = "base64 长度无效";
        return false;
    }
    if (Remainder != 0)
    {
        Padded.append(4 - Remainder, '=');
    }

    DWORD Size = 0;
    if (!CryptStringToBinaryA(
            Padded.c_str(), static_cast<DWORD>(Padded.size()), CryptStringBase64, nullptr, &Size, nullptr, nullptr) ||
        Size == 0)
    {
        ErrorText = "base64 解码失败";
        return false;
    }

    Bytes.resize(Size);
    if (!CryptStringToBinaryA(Padded.c_str(),
                              static_cast<DWORD>(Padded.size()),
                              CryptStringBase64,
                              Bytes.data(),
                              &Size,
                              nullptr,
                              nullptr))
    {
        Bytes.clear();
        ErrorText = "base64 解码失败";
        return false;
    }

    Bytes.resize(Size);
    return true;
}

static bool InflateZlib(const std::vector<uint8_t>& Compressed,
                        std::string&                Plain,
                        std::string&                ErrorText,
                        size_t                      MaxOutputSize)
{
    Plain.clear();
    ErrorText.clear();
    if (Compressed.empty())
    {
        ErrorText = "zlib 数据为空";
        return false;
    }

    z_stream Stream = {};
    int      Status = inflateInit(&Stream);
    if (Status != Z_OK)
    {
        ErrorText = FormatZlibError(Status, Stream);
        return false;
    }

    Stream.next_in  = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(Compressed.data()));
    Stream.avail_in = static_cast<uInt>(Compressed.size());

    std::vector<uint8_t> Buffer(InflateChunkSize);
    while (true)
    {
        Stream.next_out  = Buffer.data();
        Stream.avail_out = static_cast<uInt>(Buffer.size());

        Status                = inflate(&Stream, Z_NO_FLUSH);
        const size_t Produced = Buffer.size() - Stream.avail_out;
        if (Produced > 0)
        {
            if (Plain.size() + Produced > MaxOutputSize)
            {
                inflateEnd(&Stream);
                Plain.clear();
                ErrorText = "zlib 解压结果太大";
                return false;
            }
            Plain.append(reinterpret_cast<const char*>(Buffer.data()), Produced);
        }

        if (Status == Z_STREAM_END)
        {
            inflateEnd(&Stream);
            return true;
        }
        if (Status != Z_OK)
        {
            ErrorText = FormatZlibError(Status, Stream);
            inflateEnd(&Stream);
            Plain.clear();
            return false;
        }
        if (Stream.avail_in == 0 && Produced == 0)
        {
            inflateEnd(&Stream);
            Plain.clear();
            ErrorText = "zlib 数据不完整";
            return false;
        }
    }
}
} // namespace

bool DecodeBase64Zlib(std::string_view Text, std::string& Plain, std::string& ErrorText, size_t MaxOutputSize)
{
    Plain.clear();
    ErrorText.clear();
    if (MaxOutputSize == 0)
    {
        ErrorText = "输出限制无效";
        return false;
    }

    std::vector<uint8_t> Compressed;
    if (!DecodeBase64(Text, Compressed, ErrorText))
    {
        return false;
    }

    return InflateZlib(Compressed, Plain, ErrorText, MaxOutputSize);
}
} // namespace RorinnnTools::Encoding
