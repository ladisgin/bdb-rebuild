#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

class MakeFileTarget {
private:
    std::string target;
    std::string dependency;
    std::string command;
    std::string directory
public:
    MakeFileWriter() {

    }
};

int main() {
    std::ifstream cc("compile_commands.json");
    std::ifstream lc("compile_commands.json");
    json cc_data = json::parse(cc);
    json lc_data = json::parse(lc);



    return 0;
}
