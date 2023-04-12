//
// Created by kWX1061625 on 11.04.2023.
//

#include "MakeFileWriter.h"
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <iostream>

MakeFileTarget::MakeFileTarget(std::string target, std::vector<std::string> dependencies,
                               std::string command, std::string directory,
                               std::filesystem::path jsonPath) :
        target(std::move(target)), dependencies(std::move(dependencies)),
        command(std::move(command)), directory(std::move(directory)),
        jsonPath(std::move(jsonPath)) {
}

MakeFileTarget::MakeFileTarget(const nlohmann::json &entry, std::filesystem::path jsonPath) :
        target(), dependencies(), command(), directory(), jsonPath(std::move(jsonPath)) {
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
            while (start != std::string::npos) {
                start += 1;
                end = command.find(' ', start);
                if (end != std::string::npos && command.substr(end - 2, 2) == ".a") {
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

const std::string &MakeFileTarget::getTarget() const {
    return target;
}

const std::string &MakeFileTarget::getDirectory() const {
    return directory;
}

const std::string &MakeFileTarget::getCommand() const {
    return command;
}

const std::vector<std::string> &MakeFileTarget::getDependencies() const {
    return dependencies;
}

std::string getAbsPath(const std::string &modPath, const std::filesystem::path &jsonPath) {
    if (jsonPath.empty()) {
        return modPath;
    }
    std::filesystem::path p(modPath);
    if (!p.is_absolute()) {
        p = jsonPath / p;
    }
    return std::filesystem::weakly_canonical(p).string();
}

std::string MakeFileTarget::getDirectoryWithAbsPath() const {
    return getAbsPath(directory, jsonPath);
}

std::string MakeFileTarget::getTargetWithAbsPath() const {
    return getAbsPath(target, getDirectoryWithAbsPath());
}

//std::string MakeFileTarget::getCommandWithAbsPath() const {
//    return getAbsPath(command, getDirectoryWithAbsPath());
//}

std::vector<std::string> MakeFileTarget::getDependenciesWithAbsPath() const {
    std::vector<std::string> dependenciesCopy(dependencies.size());
    std::transform(dependencies.begin(), dependencies.end(),
                   dependenciesCopy.begin(),
                   [&](const std::string &s) { return getAbsPath(s, getDirectoryWithAbsPath()); });
    return dependenciesCopy;
}


MakeFileWriter::MakeFileWriter(const std::string &outPath,
                               std::filesystem::path jsonPath)
        : jsonPath(std::move(jsonPath)) {
    os = std::ofstream(outPath);
}

void MakeFileWriter::addTargetLine(const std::string &target, const std::vector<std::string> &dependencies) {
    os << target << ": ";
    for (const auto &i: dependencies) {
        os << i << " ";
    }
    os << "\n";
}

void MakeFileWriter::addStepLine(const std::string &command, const std::string &dir) {
    if (!dir.empty()) {
        os << "\t" << "cd " << dir << " && \\\n\t";
    }
    os << "\t" << command << "\n";
}

void MakeFileWriter::addTarget(const MakeFileTarget &target) {
    addTargetLine(target.getTargetWithAbsPath(), target.getDependenciesWithAbsPath());
    std::filesystem::path res_path = std::filesystem::path(target.getTargetWithAbsPath()).parent_path();
    if (!res_path.empty()) {
        addStepLine("mkdir -p " + res_path.string());
    }
    addStepLine(target.getCommand(), target.getDirectoryWithAbsPath());
    os << "\n";
    all.push_back(target.getTargetWithAbsPath());
}

void MakeFileWriter::addTarget(const nlohmann::json &entry) {
    addTarget(MakeFileTarget(entry, jsonPath));
}

void MakeFileWriter::addAll() {
    addTargetLine("all", all);
    os << "\n";
    addTargetLine(".PHONY", {});
    os << "\n"
       << ".DEFAULT_GOAL := all\n";
}
