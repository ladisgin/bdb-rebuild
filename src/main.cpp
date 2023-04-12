#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <exception>
#include <filesystem>
#include "json.hpp"
#include "MakeFileWriter.h"

int main() {
    std::ifstream cc("compile_commands.json");
    nlohmann::json cc_data = nlohmann::json::parse(cc);
    std::ifstream lc("link_commands.json");
    nlohmann::json lc_data = nlohmann::json::parse(lc);
    std::ofstream os("Makefile");

    MakeFileWriter mfw("Makefile", std::filesystem::current_path());

    for (const auto &i: cc_data) {
        mfw.addTarget(i);
    }

    for (const auto &i: lc_data) {
        mfw.addTarget(i);
    }
    mfw.addAll();

    return 0;
}
