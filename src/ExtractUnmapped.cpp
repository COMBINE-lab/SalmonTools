#include <iostream>
#include <fstream>
#include <functional>

#include "FastxParser.hpp"
#include "sparsepp/spp.h"
#include "args.hpp"

//enum class MappedState : uint8_t {UNMAPPED, LEFT_MAPPED, RIGHT_MAPPED, BOTH_MAPPED};
//inline std::ostream& operator<<(std::ostream & os, MappedState & ms) {
//  if (ms == MappedState::UNMAPPED)
//}

void ExtractUnmapped(const std::string &progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs) {
    args::ArgumentParser parser("");
    parser.Prog(progname + " extract-unmapped");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> unmappedFile(parser, "unmapped-file", "salmon file of unmapped names", {'u', "unmapped"});
    args::ValueFlag<std::string> leftReads(parser, "left-reads", "left read files", {'1', "left"});
    args::ValueFlag<std::string> rightReads(parser, "right-reads", "right read files", {'2', "right"});
    args::ValueFlag<std::string> singleReads(parser, "single-reads", "single read files", {'r', "single"});
    args::ValueFlag<std::string> outFile(parser, "output-base", "the base name of the output file", {'o', "outbase"});
    args::Flag quiet(parser, "quiet", "be quiet", {'q', "quiet"});

    spp::sparse_hash_map<std::string, std::string> targetReads;

    //args::Positional<std::string> directory(parser, "directory", "The directory to create in", ".");
    try
    {
        parser.ParseArgs(beginargs, endargs);
        std::cerr << std::boolalpha;
        if (!unmappedFile) {
          std::cerr << "you must provide an argument for [-u/--unmapped]\n";
          std::exit(1);
        }

        if (!(singleReads or (leftReads and rightReads))) {
          std::cerr << bool{singleReads} << ", " << bool{leftReads} << ", " << bool{rightReads} << '\n';
          std::cerr << "you must provide an argument for either [-r/--single], or both [-1/--left] and [-2/--right]\n";
          std::exit(1);
        }
        if (singleReads and leftReads and rightReads) {
          std::cerr << "you can not provide unpaired [-r/--single] and paired-end [-1/--left], [-2/--right] reads at the same time\n";
          std::exit(1);
        }
        if (!outFile) {
          std::cerr << "you must provide an output basename for where to write the unmapped reads\n";
          std::exit(1);
        }

        auto fname = args::get(unmappedFile);
        std::ifstream uf(fname);
        if (!uf.good()) {
          uf.close();
          std::cerr << "could not open the file [" << fname << "]\n";
          std::exit(1);
        }
        std::string readName, readState;
        while (uf >> readName >> readState) {
          /*
          MappedState ms{MappedState::UNMAPPED};
          if (readState == "u") {
            ms = MappedState::UNMAPPED;
          } else if (readState == "m1") {
            ms = MappedState::LEFT_MAPPED;
          } else if (readState == "m2") {
            ms = MappedState::RIGHT_MAPPED;
          } else if (readState == "m12") {
            ms = MappedState::BOTH_MAPPED;
          } else {
            std::cerr << "should not happen\n";
          }
          targetReads[readName] = ms;
          */
          targetReads[readName] = readState;
        }
        uf.close();
        std::cerr << "There were " << targetReads.size() << " unmapped reads\n";
        if (singleReads) {
          auto outName = args::get(outFile);
          std::ofstream out(outName + ".fa");
          std::vector<std::string> files{args::get(singleReads)};
          fastx_parser::FastxParser<fastx_parser::ReadSeq> parser(files, 1, 1);
          parser.start();
          // Get the read group by which this thread will
          auto rg = parser.getReadGroup();

          while (parser.refill(rg)) {
            // Here, rg will contain a chunk of read pairs we can process.
            for (auto& r : rg) {
              auto targetIt = targetReads.find(r.name);
              if (targetIt != targetReads.end()) {
                out << '>'  << r.name << ' ' << targetIt->second << '\n' << r.seq << '\n';
              }
            }
          }
          out.close();
        } else {
          auto outName = args::get(outFile);
          std::ofstream outLeft(outName + "_1.fa");
          std::ofstream outRight(outName + "_2.fa");
          std::vector<std::string> files1{args::get(leftReads)};
          std::vector<std::string> files2{args::get(rightReads)};
          fastx_parser::FastxParser<fastx_parser::ReadPair> parser(files1, files2, 1, 1);
          parser.start();

          // Get the read group by which this thread will
          auto rg = parser.getReadGroup();

          while (parser.refill(rg)) {
            // Here, rg will contain a chunk of read pairs we can process.
            for (auto& rp : rg) {
              auto& r1 = rp.first;
              auto& r2 = rp.second;
              auto targetIt = targetReads.find(r1.name);
              if (targetIt != targetReads.end()) {
                outLeft << '>' << rp.first.name << ' ' << targetIt->second << '\n' << rp.first.seq << '\n';
                outRight << '>' << rp.second.name << ' ' << targetIt->second << '\n' << rp.second.seq << '\n';
              }
            }
          }
          outLeft.close();
          outRight.close();
        }
    }
    catch (args::Help)
    {
        std::cout << parser;
        return;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return;
    }
}
