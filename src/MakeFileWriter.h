//
// Created by kWX1061625 on 11.04.2023.
//

#ifndef BDB_REBUILD_MAKEFILEWRITER_H
#define BDB_REBUILD_MAKEFILEWRITER_H

#include <fstream>
#include <filesystem>

#include "json.hpp"

class MakeFileTarget {
    std::string target;
    std::string directory;
    std::string command;
    std::vector<std::string> dependencies;
    std::filesystem::path jsonPath;
public:
    MakeFileTarget() = default;

    MakeFileTarget(const MakeFileTarget &makeFileTarget) = default;

    MakeFileTarget(std::string target, std::vector<std::string> dependencies,
                   std::string command, std::string directory,
                   std::filesystem::path jsonPath);

    explicit MakeFileTarget(const nlohmann::json &entry,
                            std::filesystem::path jsonPath);

    [[nodiscard]] const std::string &getTarget() const;

    [[nodiscard]] const std::string &getDirectory() const;

    [[nodiscard]] const std::string &getCommand() const;

    [[nodiscard]] const std::vector<std::string> &getDependencies() const;

    [[nodiscard]] std::string getTargetWithAbsPath() const;

    [[nodiscard]] std::string getDirectoryWithAbsPath() const;

//    [[nodiscard]] std::string getCommandWithAbsPath() const;

    [[nodiscard]] std::vector<std::string> getDependenciesWithAbsPath() const;
};

class MakeFileWriter {
    std::ofstream os;
    std::vector<std::string> all;
    std::filesystem::path jsonPath;

    void addTargetLine(const std::string &target, const std::vector<std::string> &dependencies);

    void addStepLine(const std::string &command, const std::string &dir = "");

public:
    explicit MakeFileWriter(const std::string &outPath,
                            std::filesystem::path jsonPath);

    void addTarget(const MakeFileTarget &target);

    void addTarget(const nlohmann::json &entry);

    void addAll();
};


#endif //BDB_REBUILD_MAKEFILEWRITER_H
