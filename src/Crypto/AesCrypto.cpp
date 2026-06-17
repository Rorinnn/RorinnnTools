// AesCrypto.cpp — Windows CryptoAPI AES-CBC 加密解密工具

module;

#include <Windows.h>
#include <wincrypt.h>

#include <algorithm>
#include <cstring>

module RorinnnTools;

namespace RorinnnTools::Crypto
{
static constexpr size_t Aes256KeySize = 32;
static constexpr DWORD  AesBlockSize  = 16;

struct CryptoKeyBlob
{
    BLOBHEADER Header;
    DWORD      KeySize;
    BYTE       KeyData[Aes256KeySize];
};

static void DestroyAesKey(HCRYPTPROV Provider, HCRYPTKEY Key)
{
    if (Key)
    {
        CryptDestroyKey(Key);
    }

    if (Provider)
    {
        CryptReleaseContext(Provider, 0);
    }
}

static bool CreateAesKey(const uint8_t* PKey, size_t KeySize, HCRYPTPROV& Provider, HCRYPTKEY& Key)
{
    if (!PKey || KeySize != Aes256KeySize)
    {
        return false;
    }

    if (!CryptAcquireContextW(&Provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        return false;
    }

    CryptoKeyBlob KeyBlob   = {};
    KeyBlob.Header.bType    = PLAINTEXTKEYBLOB;
    KeyBlob.Header.bVersion = CUR_BLOB_VERSION;
    KeyBlob.Header.aiKeyAlg = CALG_AES_256;
    KeyBlob.KeySize         = static_cast<DWORD>(KeySize);
    memcpy(KeyBlob.KeyData, PKey, KeySize);

    if (!CryptImportKey(Provider, reinterpret_cast<const BYTE*>(&KeyBlob), sizeof(KeyBlob), 0, 0, &Key))
    {
        CryptReleaseContext(Provider, 0);
        Provider = 0;
        return false;
    }

    DWORD Mode = CRYPT_MODE_CBC;
    if (!CryptSetKeyParam(Key, KP_MODE, reinterpret_cast<const BYTE*>(&Mode), 0))
    {
        DestroyAesKey(Provider, Key);
        Key      = 0;
        Provider = 0;
        return false;
    }

    return true;
}

bool EncryptAes256Cbc(
    const uint8_t* PData, size_t DataSize, const uint8_t* PKey, size_t KeySize, std::vector<uint8_t>& Output)
{
    Output.clear();
    if (!PData || DataSize == 0)
    {
        return false;
    }

    HCRYPTPROV Provider = 0;
    HCRYPTKEY  Key      = 0;
    if (!CreateAesKey(PKey, KeySize, Provider, Key))
    {
        return false;
    }

    size_t PaddedSize = ((DataSize / AesBlockSize) + 1) * AesBlockSize;
    if (PaddedSize > static_cast<size_t>(MAXDWORD))
    {
        DestroyAesKey(Provider, Key);
        return false;
    }

    Output.resize(PaddedSize);
    memcpy(Output.data(), PData, DataSize);

    DWORD FinalSize = static_cast<DWORD>(DataSize);
    if (!CryptEncrypt(Key, 0, TRUE, 0, Output.data(), &FinalSize, static_cast<DWORD>(Output.size())))
    {
        DestroyAesKey(Provider, Key);
        Output.clear();
        return false;
    }

    Output.resize(FinalSize);
    DestroyAesKey(Provider, Key);
    return true;
}

bool DecryptAes256Cbc(const uint8_t*        PData,
                      size_t                DataSize,
                      const uint8_t*        PKey,
                      size_t                KeySize,
                      std::vector<uint8_t>& Output,
                      size_t                OriginalSize)
{
    Output.clear();
    if (!PData || DataSize == 0 || OriginalSize > DataSize)
    {
        return false;
    }

    HCRYPTPROV Provider = 0;
    HCRYPTKEY  Key      = 0;
    if (!CreateAesKey(PKey, KeySize, Provider, Key))
    {
        return false;
    }

    if (DataSize > static_cast<size_t>(MAXDWORD))
    {
        DestroyAesKey(Provider, Key);
        return false;
    }

    Output.resize(DataSize);
    memcpy(Output.data(), PData, DataSize);

    DWORD FinalSize = static_cast<DWORD>(DataSize);
    if (!CryptDecrypt(Key, 0, TRUE, 0, Output.data(), &FinalSize))
    {
        DestroyAesKey(Provider, Key);
        Output.clear();
        return false;
    }

    if (OriginalSize > FinalSize)
    {
        DestroyAesKey(Provider, Key);
        Output.clear();
        return false;
    }

    Output.resize(OriginalSize);
    DestroyAesKey(Provider, Key);
    return true;
}
} // namespace RorinnnTools::Crypto
