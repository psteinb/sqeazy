#ifndef _ZCURVE_REORDER_SCHEME_IMPL_H_
#define _ZCURVE_REORDER_SCHEME_IMPL_H_

#include <sstream>
#include <string>
#include <functional>

#include "sqeazy_common.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "string_parsers.hpp"

#include "zcurve_reorder_utils.hpp"

namespace sqeazy {


  /**
* @brief this implements a shot noise type removal scheme in static member function encode, i.e. inside the neighborhood of a pixel
* in _input, the number of pixels are counted that fall under a certain threshold, if this count exceeds
* the limit "percentage_below" (or the one given at runtime), the central pixel
* (around which the neighborhood is located) is set to 0 as well.
*
* this scheme cannot be run inplace.
* this scheme is not reversable.
*
*/
  template <typename in_type>
  struct zcurve_reorder_scheme : public filter<in_type> {

    typedef filter<in_type> base_type;
    typedef in_type raw_type;
    typedef in_type compressed_type;
    typedef decltype(detail::morton_at_ct<>::from)	from_func_t;
    typedef decltype(detail::morton_at_ct<>::to)	to_func_t;

    static const std::string description() { return std::string("reorder the memory layout of the incoming buffer using space filling z curves"); };


    std::size_t tile_size;
    detail::zcurve encoder;

    zcurve_reorder_scheme(const std::string& _payload=""):
      tile_size(2),
      encoder(2)
      {

        pipeline_parser p;
        auto config_map = p.minors(_payload.begin(),_payload.end());

        if(config_map.size()){

          auto f_itr = config_map.find("tile_size");
          if(f_itr!=config_map.end()){
            tile_size = std::stoi(f_itr->second);
            encoder = detail::zcurve(tile_size);

          }

        }
      }

    std::string name() const override final {

      std::ostringstream msg;
      msg << "zcurve_reorder";

      return msg.str();

    }

    std::string config() const override final {

      std::ostringstream msg;
      msg << "tile_size=" << std::to_string(tile_size);
      return msg.str();

    }

    std::intmax_t max_encoded_size(std::intmax_t _size_bytes) const override final {

      return _size_bytes;
    }

    compressed_type* encode( const raw_type* _input,
                             compressed_type* _output,
                             const std::vector<std::size_t>& _shape) override final {

      const std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      auto value = encoder.encode(_input,
                                  _input + len,
                                  _output,
                                  _shape,
                                  this->n_threads());
      return value;
    }

    int decode( const compressed_type* _input,
                raw_type* _output,
                const std::vector<std::size_t>& _ishape,
                std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const override final {

      std::size_t len = std::accumulate(_ishape.begin(), _ishape.end(), 1, std::multiplies<std::size_t>());

      auto value = encoder.decode(_input,
                                  _input + len,
                                  _output,
                                  _ishape,
                                  this->n_threads());
      if(value==(_output+len))
        return SUCCESS;
      else
        return FAILURE;

    }


    ~zcurve_reorder_scheme(){};

    std::string output_type() const final override {

      return sqeazy::header_utils::represent<compressed_type>::as_string();

    }

    bool is_compressor() const final override {

      return base_type::is_compressor;

    }




  };

}

#endif /* _ZCURVE_REORDER_SCHEME_IMPL_H_ */
