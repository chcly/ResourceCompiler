#include <cstdio>
#include "ThisDir.h"
#include "Utils/Console.h"
#include "Utils/String.h"
#include "gtest/gtest.h"

using namespace Rt2;

GTEST_TEST(ResourceCompiler, RC_001)
{
    const String out = Console::launch(
        TargetFile("ResourceCompiler"),
        " --help");
    StringStream ss;
#ifdef _WIN32
    const String exe = "ResourceCompiler.exe";
#else
    const String exe = "ResourceCompiler";
#endif

    ss << "Usage: "
       << exe
       << " <options> <arg[0] .. arg[n]>\n\n"
          "-h, --help  Display this help message\n"
          "    -o          Specify the output file name\n"
          "    -n          Specify a root namespace\n\n\n";

    size_t       st  = 0;
    const String exp = ss.str();
    if (out.substr(0, 3) == "[0m")
        st = 3;

    const String act = out.substr(st, exp.size());
    Console::writeLine(act);
    EXPECT_EQ(act, exp);
}

void ExpectContainsString(const String& name, const String& string)
{
    InputFileStream ifs(name);
    EXPECT_TRUE(ifs.is_open());

    String temp;
    temp.reserve(string.size());
    while (!ifs.eof())
    {
        if (temp.size() < string.size())
            temp.push_back((char)ifs.get());
        else
        {
            if (temp == string)
                return;

            const std::ios::off_type off = (std::ios::off_type)string.size();
            ifs.seekg(-(off - 1), std::ios::cur);
            temp.resize(0);
            temp.push_back((char)ifs.get());
        }
    }
    FAIL();
}

GTEST_TEST(ResourceCompiler, RC_002)
{
    const String out = Console::launch(
        TargetFile("ResourceCompiler"),
        " -o ",
        OutputFile("out_001"),
        " ",
        TestFile("gen_001.txt"));

    ExpectContainsString(OutputFile("out_001.h"),
                         "static void getGen_001(ByteArray &dest);");
    ExpectContainsString(OutputFile("out_001.cpp"),
                         "// was unable to read any data from the supplied file, so");
}

GTEST_TEST(ResourceCompiler, RC_003)
{
    const String out = Console::launch(
        TargetFile("ResourceCompiler"),
        " -o ",
        OutputFile("out_002"),
        " ",
        TestFile("gen_002.txt"));

    ExpectContainsString(OutputFile("out_002.h"),
                         "static void getGen_002(ByteArray &dest);");
    ExpectContainsString(OutputFile("out_002.cpp"), "gen_002[108]");
    ExpectContainsString(OutputFile("out_002.cpp"),
                         "0x54,0x68,0x69,0x73,0x20,0x69,0x73,0x20,0x6d,0x65,0x61,0x6e,0x74,");
    ExpectContainsString(OutputFile("out_002.cpp"),
                         "0x69,0x6c,0x65,0x72,0x20,0x74,0x65,0x73,0x74,0x20,0x52,0x43,0x5f,");
    ExpectContainsString(OutputFile("out_002.cpp"),
                         "0x74,0x65,0x64,0x20,0x66,0x6f,0x72,0x20,0x20,0x0d,0x0a,0x65,0x71,");
}
