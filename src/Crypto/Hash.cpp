// Hash.cpp — 哈希工具

module;

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <Windows.h>
#include <bcrypt.h>

module RorinnnTools;

namespace RorinnnTools::Crypto
{

bool Sha256Bytes(std::string_view Text, std::vector<uint8_t>& Hash)
{
    Hash.clear();

    BCRYPT_ALG_HANDLE  AlgHandle  = nullptr;
    BCRYPT_HASH_HANDLE HashHandle = nullptr;

    NTSTATUS Status = BCryptOpenAlgorithmProvider(&AlgHandle, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (Status < 0)
    {
        return false;
    }

    Status = BCryptCreateHash(AlgHandle, &HashHandle, nullptr, 0, nullptr, 0, 0);
    if (Status < 0)
    {
        BCryptCloseAlgorithmProvider(AlgHandle, 0);
        return false;
    }

    Status = BCryptHashData(
        HashHandle, reinterpret_cast<PUCHAR>(const_cast<char*>(Text.data())), static_cast<ULONG>(Text.size()), 0);
    if (Status < 0)
    {
        BCryptDestroyHash(HashHandle);
        BCryptCloseAlgorithmProvider(AlgHandle, 0);
        return false;
    }

    Hash.resize(32);
    Status = BCryptFinishHash(HashHandle, Hash.data(), static_cast<ULONG>(Hash.size()), 0);

    BCryptDestroyHash(HashHandle);
    BCryptCloseAlgorithmProvider(AlgHandle, 0);

    if (Status < 0)
    {
        Hash.clear();
        return false;
    }

    return true;
}

bool Sha256Hex(std::string_view Text, std::string& HashHex)
{
    static constexpr char Hex[] = "0123456789abcdef";

    HashHex.clear();

    std::vector<uint8_t> Hash;
    if (!Sha256Bytes(Text, Hash))
    {
        return false;
    }

    HashHex.reserve(Hash.size() * 2);
    for (uint8_t Byte : Hash)
    {
        HashHex.push_back(Hex[(Byte >> 4) & 0x0F]);
        HashHex.push_back(Hex[Byte & 0x0F]);
    }

    return true;
}

} // namespace RorinnnTools::Crypto
