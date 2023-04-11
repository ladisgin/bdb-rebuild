//
// Created by kWX1061625 on 11.04.2023.
//

#ifndef BDB_REBUILD_MAKEFILEWRITER_H
#define BDB_REBUILD_MAKEFILEWRITER_H

#include "json.hpp"
#include <fstream>

class MakeFileTarget {
public:
    std::string target;
    std::string directory;
    std::string command;
    std::vector<std::string> dependencies;

    MakeFileTarget() = default;

    MakeFileTarget(const MakeFileTarget &makeFileTarget) = default;

    MakeFileTarget(std::string target, std::vector<std::string> dependencies,
                   std::string command, std::string directory);

    MakeFileTarget(const nlohmann::json &entry);
};

class MakeFileWriter {
    std::ofstream os;
    std::vector<std::string> all;

    void addTargetLine(const std::string &target, const std::vector<std::string> &dependencies);

    void addStepLine(const std::string &command);

public:
    MakeFileWriter(std::string outPath);

    void addTarget(const MakeFileTarget &target);

    void addTarget(const nlohmann::json &entry);

    void addAll();
};


#endif //BDB_REBUILD_MAKEFILEWRITER_H
