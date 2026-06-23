module;

export module RorinnnTools:Memory;
import std;

export namespace RorinnnTools::Memory
{

bool ReadBytes(std::uintptr_t Ptr, void* PBuffer, std::size_t Size);
bool IsReadablePtr(std::uintptr_t Ptr);
bool IsReadableRange(std::uintptr_t Ptr, std::size_t Size);
bool ReadPtr(std::uintptr_t Ptr, std::uintptr_t& Value);

template <typename T>
bool ReadValue(std::uintptr_t Ptr, T& Value)
{
    if (!ReadBytes(Ptr, &Value, sizeof(T)))
    {
        return false;
    }
    return true;
}

} // namespace RorinnnTools::Memory
