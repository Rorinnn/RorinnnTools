module;

#include <cstdint>

export module RnTools:Algorithms;
import std;

export namespace RnTools
{
template <class Container>
void SortUnique(Container& Values)
{
    std::sort(Values.begin(), Values.end());
    Values.erase(std::unique(Values.begin(), Values.end()), Values.end());
}

template <class Container, class Value>
bool Contains(const Container& Values, const Value& Target)
{
    return std::find(Values.begin(), Values.end(), Target) != Values.end();
}

template <class Container, class Value>
bool EraseValue(Container& Values, const Value& Target)
{
    auto It = std::find(Values.begin(), Values.end(), Target);
    if (It == Values.end())
        return false;

    Values.erase(It);
    return true;
}
} // namespace RnTools
