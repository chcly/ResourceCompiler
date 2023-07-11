/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include <cstdint>
#include <iomanip>
#include "Utils/CommandLine/Parser.h"
#include "Utils/Console.h"
#include "Utils/Exception.h"
#include "Utils/Path.h"
#include "Utils/TextStreamWriter.h"
#include "Utils/Win32/CrtUtils.h"

using namespace std;

namespace Rt2::ResourceCompiler
{
    enum Option
    {
        OptOutputFileName,
        OptNameSpace,
        OptAsciiOnly,
        OptStripNewlines,
        OptionsMax,
    };

    constexpr CommandLine::Switch Switches[OptionsMax] = {
        {OptOutputFileName, 'o', nullptr,      "Specify the output file name", true, 1},
        {     OptNameSpace, 'n', nullptr,          "Specify a root namespace", true, 1},
        {     OptAsciiOnly, 'a', nullptr,      "Filter only ascii characters", true, 0},
        { OptStripNewlines, 's', nullptr, R"(Strip all \n and \r characters)", true, 0},
    };

    struct Resource
    {
        String data;
        size_t size{};
    };

    using ResourceMap = std::unordered_map<String, Resource>;

    class Application
    {
    private:
        String      _namespace;
        String      _output;
        StringArray _input;
        ResourceMap _resources;
        bool        _asciiOnly{false};
        bool        _strip{false};

    public:
        Application() = default;

        bool parse(const int argc, char** argv)
        {
            CommandLine::Parser p;
            if (p.parse(argc, argv, Switches, OptionsMax) < 0)
                return false;

            _output    = p.string(OptOutputFileName, 0, "Resources");
            _namespace = p.string(OptNameSpace, 0, "");
            _input     = p.arguments();
            _asciiOnly = p.isPresent(OptAsciiOnly);
            _strip     = p.isPresent(OptStripNewlines);

            if (_input.empty())
                throw Exception("no input files");
            return true;
        }

        void loadInput(const String& input)
        {
            InputFileStream in(input, std::ios_base::binary);
            if (!in.is_open())
                throw Exception("failed to open the supplied file '", input, '\'');

            String name = Su::toUpperFirst(PathUtil(input).stem());

            OutputStringStream bufferImpl;

            size_t len = 0;
            bufferImpl << std::setfill(' ') << std::setw(0x0B) << ' ';

            char ch;
            while (in.read(&ch, 1))
            {
                const int v = ch;
                if (_asciiOnly && !isPrintableAscii(v))
                    continue;
                if (_strip && isNewLine(v))
                    continue;
                if (_strip && isWhiteSpace(v) && isWhiteSpace(in.peek()))
                    continue;

                bufferImpl << "0x"
                           << std::setfill('0')
                           << std::setw(2)
                           << std::hex << (uint32_t)(uint8_t)v
                           << ",";

                if (len % 13 == 12)
                {
                    bufferImpl << std::endl;
                    bufferImpl << std::setfill(' ') << std::setw(0x0B) << ' ';
                }
                ++len;
            }

            if (len > 0)
            {
                OutputStringStream srcImpl;
                srcImpl << std::setw(0x03) << ' ' << "constexpr uint8_t "
                        << name
                        << "["
                        << len << "]={"
                        << std::endl
                        << bufferImpl.str()
                        << std::endl
                        << std::setw(0x03) << ' ' << "};" << std::endl;

                _resources[name] = {srcImpl.str(), len};
            }
            else
                _resources[name] = {"", 0};
        }

        void writeHeader(OStream& out)
        {
            Ts::write(out, 0x00, "#pragma once");
            Ts::write(out, 0x00, "#include \"Utils/Array.h\"");
            Ts::write(out, 0x00, "#include \"Utils/String.h\"");
            Ts::nl(out, 0x00);

            String namespaceName;
            if (!_namespace.empty())
                namespaceName = Su::join(namespaceName, _namespace);
            else
                namespaceName = "Resources";

            Ts::write(out, 0x00, "namespace ", namespaceName);
            Ts::write(out, 0x00, '{');
            Ts::write(out, 0x04, "using ByteArray = SimpleArray<uint8_t>;");
            Ts::write(out, 0x00, '\n');
            Ts::write(out, 0x04, "class Resource");
            Ts::write(out, 0x04, '{');
            Ts::write(out, 0x04, "public:");

            bool first = true;

            for (auto& [name, source] : _resources)
            {
                if (!first)
                    Ts::nl(out, 1);
                first = false;
                Ts::write(out,
                          0x08,
                          "static void get",
                          StringUtils::toUpperFirst(name),
                          "(ByteArray &dest);\n");
                Ts::write(out,
                          0x08,
                          "static void get",
                          StringUtils::toUpperFirst(name),
                          "(String &dest);");
            }

            Ts::write(out, 0x04, "};");
            Ts::write(out, 0x00, "} // namespace ", namespaceName);
        }

        void writeSource(OStream& out)
        {
            Ts::writeLine(out, 0x00, 2, "#include \"", PathUtil(_output).stem(), ".h\"");
            String namespaceName;

            if (!_namespace.empty())
                namespaceName = Su::join(namespaceName, _namespace);
            else
                namespaceName = "Resources";

            Ts::write(out, 0x00, "namespace ", namespaceName);
            Ts::write(out, 0x00, '{');

            bool first = true;

            for (auto& [name, source] : _resources)
            {
                if (!first)
                    Ts::nl(out, 1);
                first = false;

                String methodName = Su::join("get", StringUtils::toUpperFirst(name));
                ///////////////////////////////////
                if (source.size > 0)
                {
                    Ts::write(out, 0x00, source.data);
                }

                Ts::write(out,
                          0x04,
                          "void Resource::",
                          methodName,
                          "(ByteArray &dest)");
                Ts::write(out, 0x04, '{');
                if (source.size > 0)
                    Ts::write(out, 0x08, "dest.write(", name, ", ", source.size, ");");
                else
                {
                    Ts::write(out, 0x08, "// was unable to read any data from the supplied file, so");
                    Ts::write(out, 0x08, "dest.clear();");
                }
                Ts::write(out, 0x04, "}\n");

                ///////////////////////////////////
                Ts::write(out,
                          0x04,
                          "void Resource::",
                          methodName,
                          "(String &dest)");
                Ts::write(out, 0x04, '{');
                if (source.size > 0)
                    Ts::write(out, 0x08, "dest.assign((const char *)", name, ", ", source.size, ");");
                else
                {
                    Ts::write(out, 0x08, "// was unable to read any data from the supplied file, so");
                    Ts::write(out, 0x08, "dest.clear();");
                }
                Ts::write(out, 0x04, '}');
            }
            Ts::write(out, 0x00, "} // namespace ", namespaceName);
        }

        int go()
        {
            for (const String& input : _input)
                loadInput(input);

            OutputFileStream src(Su::join(_output, ".h"));
            if (src.is_open())
                writeHeader(src);

            src.close();
            src.open(Su::join(_output, ".cpp"));

            if (src.is_open())
                writeSource(src);

            return 0;
        }
    };

}  // namespace Rt2::ResourceCompiler

int main(const int argc, char** argv)
{
    Rt2::CrtTestMemory();
    int rc = 0;
    try
    {
        if (Rt2::ResourceCompiler::Application app;
            app.parse(argc, argv))
            rc = app.go();
    }
    catch (std::exception& ex)
    {
        Rt2::Console::writeError(ex.what());
    }

    Rt2::CrtDump();
    return rc;
}
