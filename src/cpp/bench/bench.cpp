#define __SQY_BENCH_CPP__
#include <iostream>
#include <functional>
#include <iomanip>
#include <numeric>
#include <vector>
#include <set>
#include <cmath>
#include <fstream>

#include <unordered_map>
#include "bench_fixtures.hpp"
#include "bench_utils.hpp"
#include "bench_common.hpp"
#include "boost/filesystem.hpp"

template <typename MapT>
int print_help(const MapT& _av_targets) {

    auto mbegin = _av_targets.cbegin();
    auto mend = _av_targets.cend();
    auto max_el_itr = std::max_element(mbegin,mend,
                                       [&](const typename MapT::value_type& a,
    const typename MapT::value_type& b) {
        return a.first.size() < b.first.size();

    }
                                      );
    const unsigned field_width = max_el_itr->first.size()+1;

    std::string me = "bench";
    std::cout << "usage: " << me << " <flags> <target>\n"
              << "flags:\n\t -v \t\t print benchmark stats for every file\n"
              << "\t -r <path>\t perform round trip and save output to <path>/<basefilename>-<targetname>.tif\n"
              << "\t -e <path>\t save result of encoding <path>/<basefilename>-<targetname>.tif\n"
              << "\n"
              << "availble targets:\n"
              << std::setw(field_width) << "help" << "\n";


    std::set<std::string> target_names;
    for( auto it : _av_targets )
        target_names.insert(it.first);

    for(const std::string& target : target_names ) {
        std::cout << std::setw(field_width) << target <<"\t<files_to_encode>\n";
    }

    std::cout << "\n";

    return 1;
}

struct fill_suite_config {

    bool verbose = false;
    std::string roundtrip = "";
    std::string save_encoded = "";

    fill_suite_config(std::vector<std::string>& _args) {

        verbose = sqeazy_bench::pop_if_contained(_args, std::string("-v"));
        boost::filesystem::path roundtrip_p = sqeazy_bench::pop_next_if_contained(_args, std::string("-r"));
        boost::filesystem::path save_encoded_p = sqeazy_bench::pop_next_if_contained(_args, std::string("-e"));

        auto morphing = [&](const boost::filesystem::path& _p) {
            std::string value;
            if(boost::filesystem::exists(_p))
            {
                if(boost::filesystem::is_directory(_p) || boost::filesystem::is_regular_file(_p))
                {
                    value = _p.string();
                }
                else {
                    value = roundtrip_p.parent_path().string();
                }
            }
            return value;
        };

        roundtrip = morphing(roundtrip_p);
        save_encoded = morphing(save_encoded_p);

        //TODO: the following is not platform independent
        if(roundtrip[roundtrip.size()-1]!='/')
            roundtrip += "/";

        if(save_encoded[save_encoded.size()-1]!='/')
            save_encoded += "/";
    }

};

template <typename T, typename PipeType>
void fill_suite(const std::vector<std::string>& _args,
                sqeazy_bench::bsuite<T>& _suite,
                const fill_suite_config& _config) {

    typedef T value_type;
    typedef PipeType current_pipe;

    if(_config.verbose)
        std::cerr << "fill_suite :: " << PipeType::name() << "\n";

    unsigned num_files = _args.size();

    if(_suite.size() != num_files)
        _suite.cases.resize(num_files);

    std::vector<value_type> output_data;
    std::array<std::string,2> head_tail;
    boost::filesystem::path current_file;
    unsigned long written_bytes = 0;

    for(unsigned i = 0; i < num_files; ++i) {

        current_file = _args[i];

	//load image located under current_file
	sqeazy_bench::tiff_fixture<value_type, false> reference(current_file.string());

	//skip the rest if nothing was loaded
        if(reference.empty())
            continue;

	written_bytes = 0;

	//compute the maximum size of the output buffer
        unsigned long expected_size = std::ceil(current_pipe::max_bytes_encoded(reference.size_in_byte())/float(sizeof(value_type)));
        if(expected_size>output_data.size()) {
            output_data.resize(expected_size);
        }
	
	//create clean output buffer
	std::fill(output_data.begin(), output_data.end(),0);

        //make a cast so API of pipeline is happy
        char* dest = reinterpret_cast<char*>(output_data.data());
	
	//setup benchmark case for saving time, ratio ... AND START TIMER
        sqeazy_bench::bcase<value_type> temp_case(current_file.string(), reference.data(), reference.axis_lengths);
	
	//perform encoding and write output to output_data
	
        temp_case.return_code = current_pipe::compress(reference.data(),
						       dest,
						       reference.axis_lengths,
						       written_bytes);
	//STOP TIMER
        temp_case.stop(written_bytes);
	
	//save benchmark case to suite for later reuse
        _suite.at(i,temp_case);

	//-e was set
        if(_config.save_encoded.size()>1) {

            std::string name = _config.save_encoded;
            name += current_file.stem().string();
            name += "_";
            name += PipeType::name();
            name += "-enc.sqy";

            std::ofstream encoded(name.c_str(), std::ios::binary | std::ios::out);
            std::cout << "Writing to " << name << "\n";
            encoded.write(dest,current_pipe::last_num_encoded_bytes);
            encoded.close();
        }

        //-r was set
        if(_config.roundtrip.size()>1) {
            //decompress what was just compressed
            int dec_ret = current_pipe::decompress(dest,reference.data(),
                                                   written_bytes);
            if(dec_ret && _config.verbose) {
                std::cerr << "decompression failed! Nothing to write to disk...\n";
                continue;
            }

            std::string name = _config.roundtrip;
            name += current_file.stem().string();
            name += "_";
            name += PipeType::name();
            name += "-enc-dec";
            name += current_file.extension().string();
            sqeazy_bench::write_tiff_from_vector(reference.tiff_data, 
						 reference.axis_lengths,
						 name);
        }


    }



}

int main(int argc, char *argv[])
{


    typedef std::function<void(const std::vector<std::string>&,
                               sqeazy_bench::bsuite<unsigned short>&,
                               const fill_suite_config&) > func_t;
    std::unordered_map<std::string, func_t> prog_flow;

    prog_flow["compress_rmbkg_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    prog_flow["compress_rmbkg_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_bswap1_lz4_pipe>);
    prog_flow["compress_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    prog_flow["compress_diffonrow_bswap1_lz4"] = func_t(fill_suite<unsigned short, diffonrow_bswap1_lz4_pipe>);
    prog_flow["compress_bswap1_lz4"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    prog_flow["compress_diff_lz4"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    prog_flow["compress_lz4"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    prog_flow["experim_rmbkg_lz4"] = func_t(fill_suite<unsigned short, rmbkg_lz4_pipe>);
        prog_flow["experim_diffonrow_lz4"] = func_t(fill_suite<unsigned short, diffonrow_lz4_pipe>);
    prog_flow["experim_huff_lz4"] = func_t(fill_suite<unsigned short, huff_lz4_pipe>);
    prog_flow["experim_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, huff_bswap1_lz4_pipe>);
    prog_flow["experim_rmbkg_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_huff_bswap1_lz4_pipe>);
    prog_flow["experim_rmbkg_bswap1_huff_lz4"] = func_t(fill_suite<unsigned short, rmbkg_bswap1_huff_lz4_pipe>);

    prog_flow["speed_rmbkg_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_diff_bswap1_lz4_pipe>);
    prog_flow["speed_rmbkg_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_huff_bswap1_lz4_pipe>);
    prog_flow["speed_rmbkg_bswap1_lz4"] = func_t(fill_suite<unsigned short, rmbkg_bswap1_lz4_pipe>);
    prog_flow["speed_diff_bswap1_lz4"] = func_t(fill_suite<unsigned short, diff_bswap1_lz4_pipe>);
    prog_flow["speed_huff_bswap1_lz4"] = func_t(fill_suite<unsigned short, huff_bswap1_lz4_pipe>);
    prog_flow["speed_bswap1_lz4"] = func_t(fill_suite<unsigned short, bswap1_lz4_pipe>);
    prog_flow["speed_diff_lz4"] = func_t(fill_suite<unsigned short, diff_lz4_pipe>);
    prog_flow["speed_lz4"] = func_t(fill_suite<unsigned short, lz4_pipe>);

    int retcode = 0;
    if(argc>1) {
        unsigned int f_index = 1;

        std::unordered_map<std::string, func_t>::const_iterator f_func = prog_flow.end();

        std::vector<std::string> args(argv+1, argv+argc);
        fill_suite_config config(args);

        for(f_index = 0; f_index<args.size(); ++f_index) {
            if((f_func = prog_flow.find(args[f_index])) != prog_flow.end())
                break;
        }

        std::vector<std::string> filenames(args.begin()+1, args.end());


        sqeazy_bench::bsuite<unsigned short> suite(filenames.size());
        if(f_func!=prog_flow.end()) {
	  //call fill_suite
            (f_func->second)(filenames, suite,config);
	    
            int prec = std::cout.precision();

	    std::cout << filenames.size() << "\t";

            if(f_func->first.find("speed")!=std::string::npos) {

                std::cout.precision(5);
                suite.speed_summary();
                std::cout << std::setw(12) << "speed (MB/s) \t";
		
            } else {

                std::cout.precision(5);
                suite.compression_summary();
                std::cout << std::setw(12) << "ratio (in/out)\t";

            }

            std::cout << "\t" << f_func->first << "\n";
            std::cout.precision(prec);
	    std::cout << "number_of_files\tmin\tmean\tmax\tmedian\tobservable\ttarget name\n";

            if(config.verbose) {
                std::cout << "\n";
                suite.print_cases();
            }

        } else {

            std::cerr << "unable to detect known target! \nreceived:\tbench ";
            for( const std::string& word : args ) {
                std::cerr << word << "\n";
            }
            std::cerr << "\n";
            print_help(prog_flow);
        }
    }
    else {
        retcode = print_help(prog_flow);
    }

    return retcode;
}