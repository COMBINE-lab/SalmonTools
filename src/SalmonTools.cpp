#include <iostream>
#include <fstream>
#include <unordered_map>
#include <functional>
#include "args.hpp"

void ExtractUnmapped(const std::string &progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);

using commandtype = std::function<decltype(ExtractUnmapped)>;

int main(int argc, char **argv) {
    std::unordered_map<std::string, commandtype> map{
        {"extract-unmapped", ExtractUnmapped}
    };

    const std::vector<std::string> args(argv + 1, argv + argc);
    args::ArgumentParser parser("A tool for helping to process Salmon output",
                                "valid commands are: \n"
                                "extract-unmapped");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    parser.Prog(argv[0]);
    parser.ProglinePostfix("{command options}");
    args::Flag version(parser, "version", "Show the version of this program", {"version"});
    args::MapPositional<std::string, commandtype> command(parser, "command", "Command to execute", map);
    command.KickOut(true);
    try {
        auto next = parser.ParseArgs(args);
        if (version) {
          std::cerr << "Salmon Tools 0.1.0\n";
          return 0;
        }

        if (command) {
            args::get(command)(argv[0], next, std::end(args));
        } else {
            std::cout << parser;
        }
    }
    catch (args::Help) {
        std::cout << parser;
        return 0;
    }
    catch (args::Error e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    return 0;
}
