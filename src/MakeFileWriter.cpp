//
// Created by kWX1061625 on 11.04.2023.
//

#include "MakeFileWriter.h"
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <filesystem>

#include <iostream>

MakeFileTarget::MakeFileTarget(std::string target, std::vector<std::string> dependencies,
                               std::string command, std::string directory) :
        target(std::move(target)), dependencies(std::move(dependencies)),
        command(std::move(command)), directory(std::move(directory)) {
}

MakeFileTarget::MakeFileTarget(const nlohmann::json &entry) : target(), dependencies(), command(), directory() {
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
        size_t output_pos = command.find("-o ");
        std::string bin = command.substr(0, command.find(' '));
        if (output_pos != std::string::npos) {
            output_pos += 3;
            size_t end_pos = command.find(' ', output_pos);
            if (output_pos == end_pos) {
                throw std::exception();
            }
            target = output_pos == std::string::npos ?
                     command.substr(output_pos) : command.substr(output_pos, end_pos - output_pos);
        } else if (bin.find("ar") != std::string::npos) {
            size_t start = command.find(' ');
            size_t end;
            while(start != std::string::npos) {
                start += 1;
                end = command.find(' ', start);
                if(end != std::string::npos && command.substr(end - 2, 2) == ".a" ) {
                    target = command.substr(start, end - start);
                    break;
                }
                start = end;
            }
        } else {
            throw std::exception();
        }
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
        for (const auto &file: *files_it) {
            dependencies.push_back(file);
        }
    }
}

MakeFileWriter::MakeFileWriter(std::string outPath) {
    os = std::ofstream(outPath);
}

void MakeFileWriter::addTargetLine(const std::string &target, const std::vector<std::string> &dependencies) {
    os << target << ": ";
    for (const auto &i: dependencies) {
        os << i << " ";
    }
    os << "\n";
}

void MakeFileWriter::addStepLine(const std::string &command) {
    os << "\t" << command << "\n";
}

void MakeFileWriter::addTarget(const MakeFileTarget &target) {
    addTargetLine(target.target, target.dependencies);
    addStepLine("cd " + target.directory);
    std::filesystem::path res_path = std::filesystem::path(target.target).parent_path();
    if (!res_path.empty()) {
        addStepLine("mkdir -p " + res_path.string());
    }
    addStepLine(target.command);
    os << "\n";
    all.push_back(target.target);
}

void MakeFileWriter::addTarget(const nlohmann::json &entry) {
    addTarget(MakeFileTarget(entry));
}

void MakeFileWriter::addAll() {
    addTargetLine("all", all);
    os << "\n";

    addTargetLine(".PHONY", {});
    addTargetLine(".PHONY", {});
    os << "\n"
       << ".DEFAULT_GOAL := all\n";
}
