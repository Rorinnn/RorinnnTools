// RnIcons.cpp — ImguiRorinnn 图标枚举

module;

module RorinnnTools;

namespace RorinnnTools::ImguiRorinnn
{
namespace
{
static const char* IconLiteral(Icon Value)
{
    switch (Value)
    {
        case Icon::Check:
            return "\xEF\x80\x8C";
        case Icon::Xmark:
            return "\xEF\x80\x8D";
        case Icon::CaretDown:
            return "\xEF\x83\x97";
        case Icon::CaretRight:
            return "\xEF\x83\x9A";
        case Icon::Gear:
            return "\xEF\x80\x93";
        case Icon::Wrench:
            return "\xEF\x82\xAD";
        case Icon::Play:
            return "\xEF\x81\x8B";
        case Icon::Stop:
            return "\xEF\x81\x8D";
        case Icon::Minus:
            return "\xEF\x81\xA8";
        case Icon::RotateRight:
            return "\xEF\x8B\xB9";
        case Icon::WindowRestore:
            return "\xEF\x8B\x92";
        case Icon::CircleQuestion:
            return "\xEF\x81\x99";
        case Icon::CircleInfo:
            return "\xEF\x81\x9A";
        case Icon::Copy:
            return "\xEF\x83\x85";
        case Icon::Download:
            return "\xEF\x80\x99";
        case Icon::Upload:
            return "\xEF\x82\x93";
        case Icon::FolderOpen:
            return "\xEF\x81\xBC";
        case Icon::FloppyDisk:
            return "\xEF\x83\x87";
        case Icon::Trash:
            return "\xEF\x87\xB8";
        case Icon::MagnifyingGlass:
            return "\xEF\x80\x82";
        case Icon::Link:
            return "\xEF\x83\x81";
        case Icon::Discord:
            return "\xEF\x8E\x92";
        case Icon::Github:
            return "\xEF\x82\x9B";
        case Icon::None:
        default:
            return "";
    }
}
} // namespace

const char* ToIconString(Icon Value)
{
    return IconLiteral(Value);
}

} // namespace RorinnnTools::ImguiRorinnn
