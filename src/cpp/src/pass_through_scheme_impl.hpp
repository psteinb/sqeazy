#ifndef _PASS_THROUGH_SCHEME_UTILS_H_
#define _PASS_THROUGH_SCHEME_UTILS_H_

#include <functional>
#include <algorithm>
#include <sstream>
#include <climits>
#include <numeric>
#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"

namespace sqeazy {

  template <typename in_type>
  using pass_through_scheme_base_type = typename binary_select_type<filter<in_type>,//true
                                    sink<in_type>,//false
                                    std::is_same<in_type,char>::value
                                    >::type;

  template <typename in_type, typename out_type = char >
  //  struct pass_through : public sink<in_type,out_type>  {
  struct pass_through : public pass_through_scheme_base_type<in_type>  {

    // typedef sink<in_type,out_type> base_type;
    typedef pass_through_scheme_base_type<in_type> base_type;
    typedef in_type raw_type;
    typedef typename base_type::out_type compressed_type;


    static_assert(std::is_arithmetic<raw_type>::value==true,"[pass_through] input type is non-arithmetic");

    static const std::string description() { return std::string("pass/copy content to next stage"); };

    //TODO: check syntax of lz4 configuration at runtime
    pass_through(const std::string& _payload="")
    {

    }




    std::string name() const override final {

      return std::string("pass_through");

    }


    /**
       \brief serialize the parameters of this filter

       \return
       \retval string .. that encodes the configuration paramters

    */
    std::string config() const override {

      return "";

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }


    /**
     * @brief encode input raw_type buffer and write to output (not owned, not allocated)
     *
     * @param _input input raw_type buffer
     * @param _output output char buffer (not owned, not allocated)
     * @param _length mutable std::vector<size_type>, contains the length of _input at [0] and the number of written bytes at [1]
     * @return sqeazy::error_code
     */
    compressed_type* encode( const raw_type* _in,
                 compressed_type* _out,
                 const std::vector<std::size_t>& _shape) override final {


      std::size_t in_elements = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      raw_type* out_begin = reinterpret_cast<raw_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);

      std::size_t num_written_bytes = in_elements*sizeof(raw_type)/sizeof(compressed_type);

      return _out+num_written_bytes;
    }



    int decode( const compressed_type* _in, raw_type* _out,
        const std::vector<std::size_t>& _inshape,
        std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {

      if(_outshape.empty())
    _outshape = _inshape;

      std::size_t in_elements = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<std::size_t>());
      compressed_type* out_begin = reinterpret_cast<compressed_type*>(_out);
      std::copy(_in,_in+in_elements,out_begin);

      return 0;

    }



    ~pass_through(){};

    std::string output_type() const final override {

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return sink<in_type>::is_compressor;

    }

  };

} //sqeazy
#endif /* _PASS_THROUGH_SCHEME_UTILS_H_ */
