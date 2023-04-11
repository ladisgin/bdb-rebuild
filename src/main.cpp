#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <exception>
#include <filesystem>
#include "json.hpp"

using json = nlohmann::json;

class MakeFileTarget {
public:
    std::string target;
    std::string directory;
    std::string command;
    std::vector<std::string> dependencies;

    MakeFileTarget() = default;

    MakeFileTarget(std::string target, std::vector<std::string> dependencies,
                   std::string command, std::string directory) :
            target(std::move(target)), dependencies(std::move(dependencies)),
            command(std::move(command)), directory(std::move(directory)) {
    }

    MakeFileTarget(const json &entry) : target(), dependencies(), command(), directory() {
        directory = entry.at("directory");

        auto arguments_it = entry.find("arguments");
        if (arguments_it != entry.end()) {
            for (const std::string &arg: *arguments_it) {
                command += arg + " ";
            }
        } else {
            command = entry.at("command");
        }

        auto target_it = entry.find("output");
        if (target_it != entry.end()) {
            target = *target_it;
        } else {
            size_t output_pos = command.find("-o ") + 3;
            size_t end_pos = command.find(' ', output_pos);
            if (output_pos == end_pos) {
                throw std::exception();
            }
            target = output_pos == std::string::npos ?
                     command.substr(output_pos) : command.substr(output_pos, end_pos - output_pos);
        }

        auto file_it = entry.find("file");
        auto files_it = entry.find("files");
        if (file_it == entry.end() && files_it == entry.end()) {
            throw std::exception();
        }
        if (file_it != entry.end()) {
            dependencies.push_back(*file_it);
        }
        if (files_it != entry.end()) {
            for (const std::string &file: *files_it) {
                dependencies.push_back(file);
            }
        }
    }

    std::string toStr() {
        std::stringstream ss;
        ss << target << ": ";
        for (const auto &i: dependencies) {
            ss << i << " ";
        }
        ss << "\n";
        ss << "\t" << "cd " << directory << "\n";
        std::filesystem::path res_path = std::filesystem::path(target).parent_path();
        if (!res_path.empty()) {
            ss << "\t" << "mkdir -p " << res_path << "\n";
        }
        ss << "\t" << command << "\n";
        ss << "\n";
        return ss.str();
    }
};


int main() {
    std::ifstream cc("compile_commands.json");
    json cc_data = json::parse(cc);
    std::ifstream lc("link_commands.json");
    json lc_data = json::parse(lc);
    std::ofstream os("Makefile");

    std::vector<std::string> all;

    for (const auto &i: cc_data) {
        MakeFileTarget target(i);
        all.push_back(target.target);
        os << target.toStr();
    }

    for (const auto &i: lc_data) {
        MakeFileTarget target(i);
        all.push_back(target.target);
        os << target.toStr();
    }

    os << "all : ";
    for (const auto &i: all) {
        os << i << " ";
    }
    os << "\n\n";

    os << ".PHONY : all\n\n"
       << ".DEFAULT_GOAL := all";

    return 0;
}
