# 二进制资源转 C++ 头文件 / 源文件。

from pathlib import Path
import sys


def BuildIdentifier(Value):
    Result = []
    for Char in Value:
        if Char.isalnum():
            Result.append(Char)
        else:
            Result.append("_")

    Text = "".join(Result).strip("_")
    if not Text:
        Text = "Resource"
    if Text[0].isdigit():
        Text = "_" + Text
    return Text


def BuildHeader(DisplayPath, OutputFileName, Namespace, Name):
    Lines = [
        "#pragma once",
        "",
        f"// {OutputFileName} — 嵌入式二进制资源",
        f"// 自动生成: {DisplayPath}",
        "",
        "#include <cstddef>",
        "#include <cstdint>",
        "",
        f"namespace {Namespace}",
        "{",
        f"extern const std::uint8_t {Name}[];",
        f"extern const std::size_t {Name}Size;",
        f"}} // namespace {Namespace}",
        "",
    ]
    return "\n".join(Lines)


def BuildSource(InputPath, DisplayPath, HeaderFileName, OutputFileName, Namespace, Name):
    Data = InputPath.read_bytes()
    Lines = [
        f"// {OutputFileName} — 嵌入式二进制资源",
        f"// 自动生成: {DisplayPath}",
        "",
        f'#include "{HeaderFileName}"',
        "",
        f"namespace {Namespace}",
        "{",
        f"extern const std::uint8_t {Name}[] = {{",
    ]

    for Offset in range(0, len(Data), 16):
        Chunk = Data[Offset:Offset + 16]
        Values = ", ".join(f"0x{Value:02X}" for Value in Chunk)
        Lines.append(f"    {Values},")

    Lines.extend([
        "};",
        f"extern const std::size_t {Name}Size = sizeof({Name});",
        f"}} // namespace {Namespace}",
        "",
    ])
    return "\n".join(Lines)


def WriteIfChanged(PathValue, Content):
    OldContent = PathValue.read_text(encoding="utf-8") if PathValue.exists() else ""
    if Content != OldContent:
        PathValue.write_text(Content, encoding="utf-8", newline="\n")


def Main():
    if len(sys.argv) < 3:
        print("usage: GenerateBinaryResource.py <input> <output.hpp> [namespace] [name]")
        return 1

    InputPath = Path(sys.argv[1]).resolve()
    OutputPath = Path(sys.argv[2]).resolve()
    SourcePath = OutputPath.with_suffix(".cpp")
    Namespace = sys.argv[3] if len(sys.argv) >= 4 else "RorinnnTools::Resources"
    Name = BuildIdentifier(sys.argv[4] if len(sys.argv) >= 5 else InputPath.stem)
    try:
        DisplayPath = InputPath.relative_to(Path.cwd()).as_posix()
    except ValueError:
        DisplayPath = InputPath.as_posix()

    OutputPath.parent.mkdir(parents=True, exist_ok=True)
    WriteIfChanged(OutputPath, BuildHeader(DisplayPath, OutputPath.name, Namespace, Name))
    WriteIfChanged(
        SourcePath,
        BuildSource(InputPath, DisplayPath, OutputPath.name, SourcePath.name, Namespace, Name),
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(Main())

