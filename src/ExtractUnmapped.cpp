#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include "FastxParser.hpp"
#include "sparsepp/spp.h"
#include "args.hpp"
#include "zstr.hpp"

//enum class MappedState : uint8_t {UNMAPPED, LEFT_MAPPED, RIGHT_MAPPED, BOTH_MAPPED};
//inline std::ostream& operator<<(std::ostream & os, MappedState & ms) {
//  if (ms == MappedState::UNMAPPED)
//}

// from http://stackoverflow.com/questions/9435385/split-a-string-using-c11
std::vector<std::string> tokenize(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

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
          
	  //std::ofstream out(outName + ".fa.gz");
	std::unique_ptr<std::ostream> out = 
				std::unique_ptr<std::ostream> (new zstr::ofstream(outName+".fa.gz")) ;

	 

          auto readNames = args::get(singleReads);
          std::vector<std::string> files = tokenize(readNames, ',');

          fastx_parser::FastxParser<fastx_parser::ReadSeq> parser(files, 1, 1);
          parser.start();
          // Get the read group by which this thread will
          auto rg = parser.getReadGroup();

          while (parser.refill(rg)) {
            // Here, rg will contain a chunk of read pairs we can process.
            for (auto& r : rg) {
              auto targetIt = targetReads.find(r.name);
	      std::stringstream ss ; 
	      //std::string buff(">"+r.name+" "+targetIt->second+"\n"+r.seq+"\n") ;
              if (targetIt != targetReads.end()) {
                ss << '>'  << r.name << ' ' << targetIt->second << '\n' << r.seq << '\n';
              }
	      std::string buf = ss.str();
	      const char *op = buf.c_str() ;
	      std::streamsize toCopy = buf.size() ;
	      out->write(op,toCopy) ;
            }
          }
          //out.close();
        } else {
          auto outName = args::get(outFile);
          //std::ofstream outLeft(outName + "_1.fa.gz");
          //std::ofstream outRight(outName + "_2.fa.gz");
	std::unique_ptr<std::ostream> outLeft = 
				std::unique_ptr<std::ostream> (new zstr::ofstream(outName+"_1.fa.gz")) ;
	std::unique_ptr<std::ostream> outRight = 
				std::unique_ptr<std::ostream> (new zstr::ofstream(outName+"_2.fa.gz")) ;



          auto readNamesLeft = args::get(leftReads);
          auto readNamesRight= args::get(rightReads);
          std::vector<std::string> files1 = tokenize(readNamesLeft, ',');
          std::vector<std::string> files2 = tokenize(readNamesRight, ',');
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
	      std::stringstream ssLeft ;
	      std::stringstream ssRight ;
              if (targetIt != targetReads.end()) {
                ssLeft << '>' << rp.first.name << ' ' << targetIt->second << '\n' << rp.first.seq << '\n';
                ssRight << '>' << rp.second.name << ' ' << targetIt->second << '\n' << rp.second.seq << '\n';
              }

	      std::string bufLeft = ssLeft.str();
	      std::string bufRight = ssRight.str();
	      const char *opLeft = bufLeft.c_str() ;
	      const char *opRight = bufRight.c_str() ;
	      std::streamsize toCopyLeft = bufLeft.size() ;
	      std::streamsize toCopyRight = bufRight.size() ;
	      outLeft->write(opLeft, toCopyLeft) ;
	      outRight->write(opRight, toCopyRight) ;
	      
            }
          }
          //outLeft.close();
          //outRight.close();
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
