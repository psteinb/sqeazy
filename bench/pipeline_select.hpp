#ifndef _PIPELINE_SELECT_HPP_
#define _PIPELINE_SELECT_HPP_
#include <map>
#include <functional>
#include <utility>
#include <sstream>
#include <stdexcept>
#include "bench_common.hpp"

#include "boost/variant.hpp"


namespace sqeazy_bench {

  //
  //FIXME: this selection can be done through boost::variant 
  //
  

  struct compress_select {

    typedef std::pair<int, std::string> spec_t;
    
    typedef std::function<int(const unsigned char*, char*, std::vector<unsigned>&, unsigned long&)> compress8_ftype;
    typedef std::function<int(const unsigned short*, char*, std::vector<unsigned>&, unsigned long&)> compress16_ftype;
    typedef std::map<std::string, compress8_ftype> compress8_map_t;
    typedef std::map<std::string, compress16_ftype> compress16_map_t;

    static compress8_map_t compress8_map_;
    static compress16_map_t compress16_map_;

    spec_t current_;

  

    static compress8_map_t fill_compress8(){
      
      compress8_map_t value;
      value[char_rmbkg_bswap1_lz4_pipe::name()] = compress_select::compress8_ftype(char_rmbkg_bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      value[char_bswap1_lz4_pipe::name()] = compress_select::compress8_ftype(char_bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      return value;

    }

    static compress16_map_t fill_compress16(){
      
      compress16_map_t value;
      value[rmbkg_bswap1_lz4_pipe::name()] = compress_select::compress16_ftype(rmbkg_bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      value[bswap1_lz4_pipe::name()] = compress_select::compress16_ftype(bswap1_lz4_pipe::compress<std::vector<unsigned>,unsigned long>);
      return value;

    }


    compress_select(int _n_bits, std::string _pipe_name = ""):
      current_(std::make_pair(_n_bits, _pipe_name)){
            
    }


    compress_select(spec_t _spec = std::make_pair(0,"") ):
      current_(_spec){
            
    }
    	           
    bool empty() const {
      
      return current_.first < 1 && current_.second.empty();
      
    }

    void set(spec_t _spec = std::make_pair(0,"")){
      current_ = _spec;
    }

    void set(int _n_bits, std::string _pipe_name = ""){
      current_ = std::make_pair(_n_bits, _pipe_name);
    }

    int compress(const char* _input, char* _output, std::vector<unsigned>& _dims, unsigned long& _num_encoded){
      
      std::ostringstream msg;
      int value = 0;
      switch(current_.first){

      case 8:
	if(compress8_map_.find(current_.second)!=compress8_map_.end()){
	  value = compress8_map_[current_.second](reinterpret_cast<const unsigned char*>(_input), _output, _dims, _num_encoded);
	} else {
	  msg << "[compress_select]\t unable to perform compress in for pipeline "<< current_.second <<"\n";
	  throw std::runtime_error(msg.str().c_str());
	}
	break;

      case 16:
	if(compress16_map_.find(current_.second)!=compress16_map_.end()){
	  value = compress16_map_[current_.second](reinterpret_cast<const unsigned short*>(_input), _output, _dims, _num_encoded);
	} else {
	  msg << "[compress_select]\t unable to perform compress in for pipeline "<< current_.second <<"\n";
	  throw std::runtime_error(msg.str().c_str());
	}
	break;
      default:
	msg << "[compress_select]\t unable to perform compress in " << current_.first << "-bit mode\n";
	throw std::runtime_error(msg.str().c_str());
      }

      return value;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //REFACTORING

    //typedef boost::variant<char, unsigned char, short, unsigned short> supported_types_t;
    typedef boost::variant<char_rmbkg_bswap1_lz4_pipe, char_bswap1_lz4_pipe, rmbkg_bswap1_lz4_pipe, bswap1_lz4_pipe> supported_pipes_t;
    supported_pipes_t pipeholder_;


    struct  give_max_compressed_size : public boost::static_visitor<unsigned long> {
    
      unsigned long len_in_byte;
      
      give_max_compressed_size(unsigned long _in):
	len_in_byte(_in){}

      template <typename T>
      unsigned long operator()(T){
	return T::template max_encoded_size<unsigned long>(len_in_byte);
      }
      
    };

    unsigned long max_compressed_size(unsigned long _in_byte){
      
      give_max_compressed_size visitor(_in_byte);

      if(current_.second == char_rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = char_rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == char_bswap1_lz4_pipe::name())
	pipeholder_ = char_bswap1_lz4_pipe();

      if(current_.second == rmbkg_bswap1_lz4_pipe::name())
	pipeholder_ = rmbkg_bswap1_lz4_pipe();
      
      if(current_.second == bswap1_lz4_pipe::name())
	pipeholder_ = bswap1_lz4_pipe();

      unsigned long value = boost::apply_visitor(visitor, pipeholder_);
      return value;
      
    }
    

  };


  compress_select::compress8_map_t compress_select::compress8_map_ = compress_select::fill_compress8();
  compress_select::compress16_map_t compress_select::compress16_map_ = compress_select::fill_compress16();
  // 


};
#endif /* _PIPELINE_SELECT_HPP_ */
