#ifndef _LZ4_HPP_
#define _LZ4_HPP_
#include <cstdint>
#include <string>
#include <typeinfo>
#include <future>

#include "sqeazy_common.hpp"
#include "traits.hpp"

#include "string_parsers.hpp"
#include "dynamic_stage.hpp"

//TODO: what if lz4 is not available??
#ifndef LZ4_VERSION_MAJOR
//#include "lz4.h"
#include <lz4frame.h>

#endif

namespace sqeazy {

  //FIXME: lz4 can also be a filter for char->char conversions
  template <typename in_type>
  using lz4_scheme_base_type = typename binary_select_type<filter<in_type>,//true
                                                           sink<in_type>,//false
                                                           std::is_same<in_type,char>::value
                                                           >::type;

  template < typename T , typename S = std::size_t>
  struct lz4_scheme :  public lz4_scheme_base_type<T> {

    typedef lz4_scheme_base_type<T> sink_type;
    typedef T raw_type;
    typedef typename sink_type::out_type compressed_type;

    static_assert(std::is_arithmetic<raw_type>::value==true,"[lz4_scheme] input type is non-arithmetic");
    static const std::string description() { return std::string("compress input with lz4, <accel|default = 1> improves compression speed at the price of compression ratio, <blocksize_kb|default = 256> ... the lz4 blocksize (possible values: 64/265/1024/4048), <encodestep_kb|default = 32 > ... the atomic amount of input data to submit to lz4 compression "); };

    std::string lz4_config;
    int acceleration;
    std::uint32_t blocksize_kb;
    std::uint32_t encodestep_kb;

    LZ4F_preferences_t lz4_prefs;

    //TODO: check syntax of lz4 configuration at runtime
    lz4_scheme(const std::string& _payload=""):
      lz4_config(_payload),
      acceleration(1),
      blocksize_kb(256),
      encodestep_kb(16),
      lz4_prefs(){

      pipeline_parser p;
      auto config_map = p.minors(_payload.begin(),_payload.end());

      if(config_map.size()){
        auto f_itr = config_map.find("accel");
        if(f_itr!=config_map.end())
          acceleration = std::stof(f_itr->second);

        f_itr = config_map.find("blocksize_kb");
        if(f_itr!=config_map.end())
          blocksize_kb = std::stof(f_itr->second);

        f_itr = config_map.find("encodestep_kb");
        if(f_itr!=config_map.end())
          encodestep_kb = std::stof(f_itr->second);

      }

      lz4_prefs = {
        { LZ4F_max256KB, //commonly L2 size on Intel platforms
          LZ4F_blockLinked,
          LZ4F_noContentChecksum,
          LZ4F_frame,
          0 /* content size unknown */,
          0 /* no dictID */ ,
          LZ4F_noBlockChecksum },
        acceleration,   /* compression level */
        0,   /* autoflush */
        { 0, 0, 0, 0 },  /* reserved, must be set to 0 */
      };

    }




    std::string name() const override final {

      return std::string("lz4");

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      return lz4_config;

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      std::intmax_t lz4_bound = LZ4F_compressBound(_size_bytes, &lz4_prefs)
        + 4 /*the mysterious lz4 frame footer, see https://github.com/lz4/lz4/issues/445 */
        + LZ4F_HEADER_SIZE_MAX ;

      return lz4_bound;
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length number of char items in _in
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, std::size_t _length) override final {
      std::vector<std::size_t> shape = {_length};
      return encode(_in,_out,shape);
    }

    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _shape mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return pointer to end of payload
     */
    compressed_type* encode( const raw_type* _in, compressed_type* _out, const std::vector<std::size_t>& _shape) override final {

      typedef typename sqeazy::twice_as_wide<size_t>::type local_size_type;

      const local_size_type total_length = std::accumulate(_shape.begin(), _shape.end(), 1, std::multiplies<size_t>());
      const local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      const compressed_type* input = reinterpret_cast<const compressed_type*>(_in);

      const local_size_type max_payload_length_in_byte = max_encoded_size(total_length_in_byte);
      local_size_type encodestep_byte = encodestep_kb << 10;
      if(encodestep_byte >= total_length_in_byte){
        encodestep_byte = total_length_in_byte;
      }
      const local_size_type max_encodestep_in_byte = max_encoded_size(encodestep_byte);

      const int nthreads = this->n_threads();
      size_type num_written_bytes = 0;
      compressed_type* value = nullptr;

      if(nthreads==1){

        lz4_prefs.frameInfo.contentSize = total_length_in_byte;

        LZ4F_compressionContext_t ctx;
        auto rcode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
        if (LZ4F_isError(rcode)) {
          std::cerr << "[sqy::lz4] Failed to create context: error " << rcode << "\n";
          return value;
        } else {
          rcode = 1;//don't ask me why, taken from https://github.com/lz4/lz4/blob/v1.8.0/examples/frameCompress.c#L34
        }

        rcode = num_written_bytes = LZ4F_compressBegin(ctx,
                                                       _out, max_encodestep_in_byte ,
                                                       &lz4_prefs);
        if (LZ4F_isError(rcode)) {
          std::cerr << "[sqy::lz4] Failed to start compression: error " << rcode << "\n";
          return value;
        }

        std::size_t n_steps = (total_length_in_byte + encodestep_byte - 1) / encodestep_byte;

        auto dst = _out + num_written_bytes;
        auto src = input;

        for( std::size_t s = 0;s<n_steps;++s){

          auto n = LZ4F_compressUpdate(ctx,
                                       dst,
                                       max_payload_length_in_byte - std::distance(_out,dst),
                                       src,
                                       encodestep_byte,
                                       NULL);

          if (LZ4F_isError(n)) {
            std::cerr << "[sqy::lz4] Failed to compress: error " << n << "\n";
            break;
          }

          src += encodestep_byte;
          dst += n;

        }

        rcode = LZ4F_compressEnd(ctx, dst, max_payload_length_in_byte - std::distance(_out,dst), NULL);
        if (LZ4F_isError(rcode)) {
          std::cerr << "[sqy::lz4] Failed to end compression: error " << rcode << "\n";
          return value;
        }

        dst += rcode;


        if (ctx)
          LZ4F_freeCompressionContext(ctx);

        value = dst;
        lz4_prefs.frameInfo.contentSize = 0;
      }
      // else{

      //   std::vector<std::future<size_type>> tasks;
      //   tasks.reserve(nthreads);

      //   const local_size_type blocksize_byte = (total_length_in_byte + nthreads - 1) / nthreads;//TODO: this chunk size could also be volume limited (think cache sizes from compass)
      //   const local_size_type blocksize = blocksize_byte/sizeof(raw_type);
      //   const local_size_type max_encoded_blocksize_in_byte = LZ4_compressBound(blocksize_byte);
      //   const local_size_type max_payload_length_in_byte = nthreads*max_encoded_blocksize_in_byte;

      //   for(int t = 0;t < nthreads;t++){
      //     tasks.push_back(
      //       std::async(
      //         LZ4_compress_fast,
      //         (const char*)input+t*blocksize,
      //         (char*)_out + t*max_encoded_blocksize_in_byte,
      //         max_encoded_blocksize_in_byte,
      //         acceleration
      //         )
      //       );
      //   }

      //   //TODO: would love to have a uniform barrier here
      //   std::size_t last_offset = 0;
      //   for(int t = 0;t < nthreads;t++){
      //     tasks[t].wait();
      //     if(t>0){
      //       std::copy(_out + t*max_encoded_blocksize_in_byte,
      //                 _out + (t+1)*max_encoded_blocksize_in_byte,
      //                 _out + last_offset
      //         );
      //     }
      //     last_offset += tasks[t].get();
      //   }


      // }
      return value;
    }



    int decode( const compressed_type* _in, raw_type* _out,
                const std::vector<std::size_t>& _inshape,
                std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
        _outshape = _inshape;

      size_type _len_in = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<size_type>());
      size_type _len_out = std::accumulate(_outshape.begin(), _outshape.end(),1,std::multiplies<size_type>());

      return decode(_in,_out,_len_in,_len_out);
    }

    int decode( const compressed_type* _in, raw_type* _out,
                std::size_t _inlen,
                std::size_t _outlen=0)  const override final {

      int value = 1;//failing

      if(!_outlen)
        _outlen = _inlen;

      compressed_type* dst = reinterpret_cast<compressed_type*>(_out);
      compressed_type* dstEnd = reinterpret_cast<compressed_type*>(_out+_outlen);

      const compressed_type* src = _in;
      const compressed_type* srcEnd = _in + _inlen;

      const std::size_t expected_bytes_decoded = _outlen*sizeof(raw_type);

      if(this->n_threads()==1){
        LZ4F_dctx *dctx = nullptr;
        auto ret = LZ4F_createDecompressionContext(&dctx, 100);
        if (LZ4F_isError(ret)) {
          std::cerr << "[sqy::lz4] LZ4F_dctx creation error: " << LZ4F_getErrorName(ret) << "\n";
          return value;
        }

        std::size_t srcSize = _inlen;
        ret = 1;
        while (src != srcEnd && ret != 0) {
          /* INVARIANT: Any data left in dst has already been written */
          std::size_t dstSize = std::distance(dst,dstEnd);
          ret = LZ4F_decompress(dctx, dst, &dstSize,
                                src, &srcSize,
                                /* LZ4F_decompressOptions_t */ NULL);
          if (LZ4F_isError(ret)) {
            std::cerr << "[sqy::lz4] Decompression error: " << LZ4F_getErrorName(ret) << "\n";
            break;
          }

          src += _inlen;
          srcSize = srcEnd - src;
          dst += dstSize;
        }

        // int num_bytes_decoded = LZ4_decompress_safe(_in,
        //                                             output,
        //                                             _inlen*sizeof(compressed_type),
        //                                             expected_bytes_decoded);

      }

      std::size_t num_bytes_decoded = std::distance(reinterpret_cast<compressed_type*>(_out),dst)*sizeof(raw_type);
      if(num_bytes_decoded > 0 && num_bytes_decoded<=expected_bytes_decoded)
        return 0;
      else
        return 1;

    }



    ~lz4_scheme(){};

    std::string output_type() const final override {

      return typeid(compressed_type).name();

    }

    bool is_compressor() const final override {

      return sink<T>::is_compressor;

    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DEPRECATED API
    typedef S size_type;

    static S last_num_encoded_bytes;

    static const bool is_sink = true;

    static const std::string static_name() {

      return std::string("lz4");

    }


    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    template <typename SizeType>
    static const error_code static_encode(const raw_type* _input,
                                          compressed_type* _output,
                                          std::vector<SizeType>& _dims,//size of _input
                                          size_type& _bytes_written = last_num_encoded_bytes
      ) {

      typedef typename sqeazy::twice_as_wide<SizeType>::type local_size_type;
      local_size_type total_length = std::accumulate(_dims.begin(), _dims.end(), 1, std::multiplies<SizeType>());
      local_size_type total_length_in_byte = total_length*sizeof(raw_type);
      local_size_type max_payload_length_in_byte = LZ4_compressBound(total_length_in_byte);

      const compressed_type* input = reinterpret_cast<const compressed_type*>(_input);

#if LZ4_VERSION_NUMBER > 10701
      size_type num_written_bytes = LZ4_compress_default(input,
                                                         _output,
                                                         total_length_in_byte,
                                                         max_payload_length_in_byte);
#else
      size_type num_written_bytes = LZ4_compress_limitedOutput(input,
                                                               _output,
                                                               total_length_in_byte,
                                                               max_payload_length_in_byte);
#endif
      if(num_written_bytes > 0) {
        _bytes_written = num_written_bytes;
        last_num_encoded_bytes = _bytes_written;
        return SUCCESS;
      }
      else {
        _bytes_written = 0;
        last_num_encoded_bytes = 0;
        return FAILURE;
      }

    }


    /**
     * @brief encode _input using the lz4 library, the output contains the number of input elements encoded as 64bit signed integer (long) and the encoded data
     *
     * @param _input array/buffer that contains data of raw_type
     * @param _output the char buffer that contains the compressed data (not truncated, not owned)
     * @param _length size of _input (unmutable)
     * @param _bytes_written number of bytes/elements in _output that actually contain data
     * @return sqeazy::error_code SUCCESS if non-zero number of bytes have been encoded by lz4
     */
    static const error_code static_encode(const raw_type* _input,
                                          compressed_type* _output,
                                          const size_type& _length,//size of _input
                                          size_type& _bytes_written = last_num_encoded_bytes
      ) {

      std::vector<size_type> artificial_dims(1);
      artificial_dims.front() = _length;

      return static_encode(_input, _output, artificial_dims, _bytes_written);

    }

    /**
     * @brief decode the input data stream _input with lz4 (the number of output elements is extracted from the first 64
     * bit of the input stream)
     *
     * @param _input input buffer as char
     * @param _output output buffer (needs to be allocated outside this function)
     * @param _length number of elements in _input
     * @return sqeazy::error_code
     */
    static const error_code static_decode(const compressed_type* _input,
                                          raw_type* _output,
                                          const size_type& _len_in,
                                          const size_type& _len_out
      ) {

      char* output = reinterpret_cast<char*>(_output);
      const char* input = reinterpret_cast<const char*>(_input);

      size_type num_bytes_decoded = LZ4_decompress_safe(input,output,_len_in, _len_out);

      return ( num_bytes_decoded > 0 && num_bytes_decoded == _len_out ) ? SUCCESS : FAILURE;

    }

    //////////////////////////////////////////////////////////////////////////
    //TODO: the following functions are actually not very compressor specific
    //      -> refactor to policy!

    template <typename U>
    static const unsigned long static_max_encoded_size(U _src_length_in_bytes) {

      unsigned long lz4_bound = LZ4_compressBound(_src_length_in_bytes);

      return lz4_bound;

    }


  };

  template < typename T , typename S>
  S lz4_scheme<T,S>::last_num_encoded_bytes = 0;


};//sqy namespace


#endif /* _LZ4_H_ */
