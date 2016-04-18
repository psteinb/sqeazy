#define __SQY_CPP__
#include <iostream>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <vector>
#include <cmath>
//#include <fstream>
#include <unordered_map>
#include <regex>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "app/verbs.hpp"
#include "sqeazy_pipelines.hpp"


namespace po = boost::program_options;
namespace sqy = sqeazy;


template <typename modes_map_t, typename map_type >
int print_help(
	       const modes_map_t& _modes_args,
	       const map_type& _verb_aliases 
	       ) {

  std::string me = "sqy";
  std::cout << "usage: " << me << " <-h|optional> <verb> <files|..>\n"
	    << "\n"
	    << "available verbs:\n"
	    << "<help>" << "\n";


  if(_modes_args.size()){
      
    for( auto args : _modes_args ){

      auto fitr = _verb_aliases.find(args.first);
      if(fitr!=_verb_aliases.end())
	std::cout << "<" << fitr->second << ">\n" << args.second << "\n";
      else
	std::cout << "<" << args.first << ">\n" << args.second << "\n";
    }
  }

  std::cout << "pipeline builder\n"
	    << "	- pipelines may consist of any number of filters\n"
	    << "          (i.e. a function that ingests data of type T and emits it of type T again)\n"
	    << "	- each pipeline may have no or at least 1 sink\n"
    	    << "	  (i.e. a function that ingests data of type T and emits it of a type with smaller width than T,\n"
    	    << "	  for example quantiser or lz4)\n\n";

  std::cout << "available filters (before sink):\n";
  sqy::dypeline<std::uint16_t>::head_filter_factory_t::print_names("\t");
  
  std::cout << "\navailable sinks:\n";
  sqy::dypeline<std::uint16_t>::sink_factory_t::print_names("\t");
  
  std::cout << "\navailable filters (after sink):\n";
  sqy::dypeline<std::uint16_t>::tail_filter_factory_t::print_names("\t");
  
  std::cout << "\n";

  return 1;
}


int main(int argc, char *argv[])
{


  typedef std::function<void(const std::vector<std::string>&,const po::variables_map&) > func_t;
    
  // static std::vector<std::string> modes{sqeazy::bswap1_lz4_pipe::static_name(),
  //     sqeazy::rmbkg_bswap1_lz4_pipe::static_name()};
  const static std::string default_compression = "bitswap1->lz4";

  static std::unordered_map<std::string,po::options_description> descriptions(4);

  descriptions["compress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("pipeline,p", po::value<std::string>()->default_value(default_compression), "compression pipeline to be used")
    ;

  descriptions["decompress"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ;

  descriptions["scan"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ;

  descriptions["convert"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("chroma_sampling,c", po::value<std::string>()->default_value("C420"),  "from .tif to .y4m (possible values: C420, C444)")
    //("method,m", po::value<std::string>()->default_value("internal"), "method for conversion (possible: sws_scale, internal)")
    ("frame_shape,s", po::value<std::string>()->default_value(""), "shape of each frame to expect (only needed for .yuv sources), e.g. 1920x1020")
    ("dump_quantiser,d", "dump LUT for quantiser")
    ;

  descriptions["compare"].add_options()
    ("help", "produce help message")
    ("verbose,v", "enable verbose output")
    ("metrics,m", po::value<std::string>()->default_value("nrmse"), "comma-separated list of metrics (possible values: mse, psnr)")
    ;

  static     std::unordered_map<std::string, std::string> verb_aliases;
  verb_aliases["compress"] = std::string("compress|enc|encode|comp");
  verb_aliases["decompress"] = std::string("decompress|dec|decode|rec");
  verb_aliases["scan"] = std::string      ("scan|info");                
  verb_aliases["convert"] = std::string   ("convert|transform|trf");
  verb_aliases["compare"] = std::string("compare|cmp");

  
  static     std::unordered_map<std::string, std::regex> verb_aliases_rex(verb_aliases.size());
  for ( auto& pair : verb_aliases ){
    std::ostringstream rex;
    rex << "(" << pair.second << ")";
    verb_aliases_rex[pair.first] = std::regex(rex.str());
  }
    
  // std::vector<std::string> args(argv+1, argv+argc);
  std::string joint_args;
  for( int i = 0;i<argc;++i ){
    joint_args.append(argv[i]);
  }
    
  int retcode = 0;
  if(argc<2 || joint_args.find("-h")!=std::string::npos || joint_args.find("help")!=std::string::npos) {
    retcode = print_help(descriptions,verb_aliases
			 );
  }
  else{
        
    po::options_description options_to_parse;
      
    func_t prog_flow;

    std::string target(argv[1]);

    std::vector<char*> new_argv(argv,argv+argc);
    new_argv.erase(new_argv.begin()+1);
      
    if(descriptions.find(target)!=descriptions.end()){
      options_to_parse.add(descriptions[target]);
    }


    ///////////////////////////////////////////
    //REFACTOR THIS!
    if(std::regex_match(target,verb_aliases_rex["compress"]))
      prog_flow = compress_files;

    if(std::regex_match(target,verb_aliases_rex["decompress"]))
      prog_flow = decompress_files;

    if(std::regex_match(target,verb_aliases_rex["scan"]))
      prog_flow = scan_files;

    if(std::regex_match(target,verb_aliases_rex["convert"]))
      prog_flow = convert_files;

    if(std::regex_match(target,verb_aliases_rex["compare"]))
      prog_flow = compare_files;

    
    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(new_argv.size(), &new_argv[0]// argc, argv
							).options(options_to_parse).allow_unregistered().run();
    po::store(parsed, vm); 
    po::notify(vm);
	
    std::vector<std::string> inputFiles = po::collect_unrecognized(parsed.options, po::include_positional);

    if(inputFiles.empty()){
      std::cout << "[sqy] no input files given, exiting ...\n";
    } else {
      prog_flow(inputFiles, vm);
    }
  }

  return retcode;
}
