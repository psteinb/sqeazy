#ifndef _DYNAMIC_PIPELINE_H_
#define _DYNAMIC_PIPELINE_H_

#include "string_parsers.hpp"
#include "sqeazy_utils.hpp"
#include "traits.hpp"
#include "dynamic_stage.hpp"
#include "dynamic_stage_chain.hpp"
#include "dynamic_stage_factory.hpp"

#include "sqeazy_header.hpp"
#include "sqeazy_algorithms.hpp"

#include <utility>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <future>


namespace sqeazy
{

  template <typename T>
  using default_filter_factory = stage_factory<blank_filter<T> >;

  template <typename T>
  using default_sink_factory = stage_factory<blank_sink<T> >;



  template <
    typename raw_t,
    template<typename = raw_t> class filter_factory_t = default_filter_factory,
    typename inbound_sink_factory_t = default_sink_factory<raw_t>,
    typename optional_tail_factory_t = void
    >
  struct dynamic_pipeline : public sink<raw_t>
  {

    typedef sink<raw_t> base_t;
    typedef raw_t incoming_t;
    typedef typename base_t::out_type outgoing_t;

    static_assert(std::is_arithmetic<incoming_t>::value==true, "[dynamic_pipeline.hpp:56] received non-arithmetic type for input");
    static_assert(std::is_arithmetic<outgoing_t>::value==true, "[dynamic_pipeline.hpp:57] received non-arithmetic type for output");


    typedef sink<incoming_t> sink_t;
    typedef std::shared_ptr< sink<incoming_t> > sink_ptr_t;

    typedef filter<incoming_t> head_filter_t;
    typedef filter<outgoing_t> tail_filter_t;

    typedef sqeazy::stage_chain<head_filter_t> head_chain_t;
    typedef sqeazy::stage_chain<tail_filter_t> tail_chain_t;

    using head_filter_factory_t = filter_factory_t<incoming_t>;
    using tail_filter_factory_t = typename binary_select_type<
      filter_factory_t<outgoing_t>,//true
      optional_tail_factory_t,//false
      std::is_same<void,optional_tail_factory_t>::value
      >::type;
    using sink_factory_t = inbound_sink_factory_t;

    //FIXME: check that incoming type of tail_filter_factory is equal to
    // static_assert(std::is_same<tail_filter_factory_t>::value==true, "[dynamic_pipeline.hpp:57] received non-arithmetic type for output");

    head_chain_t head_filters_;
    tail_chain_t tail_filters_;
    sink_ptr_t sink_;

    std::uint32_t n_threads_;

    /**
       \brief given a buffer that contains a valid header, this static method can fill the filter_holder and set the sink

       \param[in] _buffer string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return
       \retval

    */
    static dynamic_pipeline bootstrap(const std::string &_config)
    {

      if(_config.empty())
        return dynamic_pipeline();
      else
        return bootstrap(_config.begin(), _config.end());
    }

    /**
       \brief given an iterator range that contains a valid header, this static method can fill the filter_holder and set the sink

       \param[in] _buffer string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return
       \retval

    */
    template <
      typename iterator_t
      >
    static dynamic_pipeline bootstrap(iterator_t _begin, iterator_t _end)
    {

      const auto len = std::distance(_begin,_end);
      if(!len)
        return dynamic_pipeline();

      sqeazy::header hdr(_begin,_end);
      auto pipe = hdr.pipeline();
      return from_string(pipe.begin(),pipe.end());
    }

    /**
       \brief given a string, this static method can fill the filter_holder and set the sink

       \param[in] _config string to parse
       \param[in] head_f filter factory that yields possible filters that filter incoming_type to incoming_type arrays
       \param[in] s sink factory that yields possible sinks to go from incoming_type to outgoing_type
       \param[in] tail_f filter factory that yields possible filters that filter outgoing_type to outgoing_type arrays
       \return
       \retval

    */
    template <typename iter_t>
    static dynamic_pipeline from_string(iter_t _config_begin, iter_t _config_end)
    {

      pipeline_parser parser;
      sqeazy::vec_of_pairs_t parsed = parser.to_pairs(_config_begin,_config_end);

      dynamic_pipeline value;

      for(const auto &pair : parsed)
        {
          if(!value.sink_){
            if(head_filter_factory_t::has(pair.first)){
              value.add((head_filter_factory_t::template create<head_filter_t>(pair.first, pair.second)));
              continue;
            }

            if(sink_factory_t::has(pair.first)){
              //auto temp = sink_factory_t::template create< sink_t >(pair.first, pair.second);
              //value.sink_ = temp;
              value.add(sink_factory_t::template create< sink_t >(pair.first, pair.second));
            }
          } else {
            if(tail_filter_factory_t::has(pair.first)){
              value.add((tail_filter_factory_t::template create<tail_filter_t>(pair.first, pair.second)));

            }
          }
          //what if sink is present in _config, but not in factory
        }


      return value;
    }

    static bool can_be_built_from(const std::string& _config_str, std::string _sep = "->")
    {
      return can_be_built_from(_config_str.begin(), _config_str.end(),_sep);
    }

    template <typename iter_t>
    static bool can_be_built_from(iter_t _begin, iter_t _end,
                                  std::string _sep = "->")
    {

      sqeazy::pipeline_parser p;
      auto steps_n_args = p.to_pairs(_begin,_end);

      auto major_steps = sqeazy::split_char_range(_begin,_end,_sep);
      bool value = false;

      if(major_steps.size()!=steps_n_args.size())
        return value;


      std::uint32_t found = 0;
      bool sink_matched = false;

      for(const auto &pair : steps_n_args)
        {

          if(!sink_matched && head_filter_factory_t::has(pair.first)){
            found++;
            continue;
          }

          if(sink_factory_t::has(pair.first)){
            found++;
            sink_matched = true;
            continue;
          }

          if(tail_filter_factory_t::has(pair.first)){
            found++;
          }

        }

      value = found == steps_n_args.size();
      size_t rebuild_size = _sep.size()*(steps_n_args.size()-1);
      for( const auto& item : steps_n_args ){
        rebuild_size += item.first.size();
        if(!item.second.empty())
          rebuild_size += 2 + item.second.size();
      }

      const std::size_t config_len = std::distance(_begin, _end);
      value = value && rebuild_size == config_len;
      return value;
    }

    /**
       \brief given a char buffer, this static method can fill the filter_holder and set the sink

       \param[in] _config string to parse
       \param[in] f filter factory that is aware of all possible filters to choose from
       \param[in] s sink factory that is aware of all possible sinks to choose from

       \return
       \retval

    */
    static dynamic_pipeline from_string(const std::string& _config)
    {
      return from_string(_config.begin(),_config.end());
    }



    friend void swap(dynamic_pipeline &_lhs, dynamic_pipeline &_rhs)
    {
      std::swap(_lhs.head_filters_, _rhs.head_filters_);
      std::swap(_lhs.tail_filters_, _rhs.tail_filters_);
      std::swap(_lhs.sink_, _rhs.sink_);
      //_lhs.set_n_threads(_rhs.n_threads());

    }

    /**
       \brief construct an empty pipeline


       \return
       \retval

    */
    dynamic_pipeline()
      :
      head_filters_(),
      tail_filters_(),
      sink_(nullptr),
      n_threads_(1)
    {};

    /**
     //DEPRECATE THIS?//
     \brief construct an pipeline from given stages (mainly for debugging)

     \param[in] _size how many stages will the pipeline have

     \return
     \retval

    */
    dynamic_pipeline(std::initializer_list<typename head_chain_t::filter_ptr_t> _stages)
      :
      head_filters_(_stages),
      tail_filters_(),
      sink_(nullptr),
      n_threads_(1)
    {


    };

    /**
       \brief copy-constructor

       \param[in]

       \return
       \retval

    */
    template <template<typename> class T, typename U>
    dynamic_pipeline(const dynamic_pipeline<incoming_t, T, U>& _rhs):
      head_filters_(_rhs.head_filters_),
      tail_filters_(_rhs.tail_filters_),
      sink_        (_rhs.sink_        ),
      n_threads_   (_rhs.n_threads_   )
    {
      set_n_threads(_rhs.n_threads_);
    }

    /**
       \brief copy-by-assignment using copy&swap idiom

       \param[in]

       \return
       \retval

    */
    template <template<typename> class T, typename U>
    dynamic_pipeline &operator=(dynamic_pipeline<incoming_t, T, U> _rhs)
    {

      swap(*this, _rhs);
      this->set_n_threads(_rhs.n_threads());
      return *this;
    }

    /**
       \brief how many stages are setup in the pipeline

       \return
       \retval

    */
    const std::size_t size() const { return head_filters_.size() + (sink_ ? 1 : 0) + tail_filters_.size(); }

    const bool empty() const { return (head_filters_.empty() && !sink_ && tail_filters_.empty()); }

    void clear() {
      head_filters_.clear();
      sink_.reset();
      tail_filters_.clear();
    }

    template <typename pointee_t>
    typename std::enable_if<!std::is_base_of<sink_t,pointee_t>::value>::type
    add(const std::shared_ptr<pointee_t>& _new_filter)
    {

      if(_new_filter.get() == nullptr)
        return;

      auto view = const_stage_view(_new_filter);

      if(std::is_base_of<typename head_chain_t::filter_base_t,pointee_t>::value){
        head_filters_.push_back(_new_filter);
        set_n_threads(n_threads());
        return;
      }

      if(std::is_base_of<typename tail_chain_t::filter_base_t,pointee_t>::value){
        tail_filters_.push_back(_new_filter);
        set_n_threads(n_threads());
        return;
      }

      std::ostringstream msg;
      msg << "sqeazy::dynamic_pipeline::add failed to push_back filter called " << _new_filter.get()->name() << "\n";
      throw std::runtime_error(msg.str());
    }



    // void add_sink(const sink_ptr_t& _new_sink)
    void add(const sink_ptr_t& _new_sink)
    {
      if(_new_sink.get() == nullptr)
        return;

      if(sink_)
        sink_.reset();

      sink_ = _new_sink;

      set_n_threads(n_threads());

    }


    bool valid_filters() const
    {

      bool value = true;
      if(head_filters_.empty() && tail_filters_.empty())
        return !value;

      if(!head_filters_.empty()){
        value = value && head_filters_.valid();

        if(sink_){
          auto back_view = const_stage_view(head_filters_.back());
          auto sink_view = const_stage_view(sink_);
          value = value && back_view->output_type() == sink_view->input_type();
        }
      }

      if(!tail_filters_.empty()){
        auto front_view = const_stage_view(tail_filters_.front());

        if(sink_){
          auto sink_view = const_stage_view(sink_);
          value = value && front_view->input_type() == sink_view->output_type();
        }

        value = value && tail_filters_.valid();
      }
      return value;
    }

    std::string input_type() const override
    {

      if(head_filters_.size())
        return const_stage_view(head_filters_.front())->input_type();

      if(sink_)
        return const_stage_view(sink_)->input_type();

      return std::string("");
    };

    std::string output_type() const override
    {

      if(sink_)
        {
          return const_stage_view(sink_)->output_type();
        }
      else
        {
          if(tail_filters_.size())
            {
              return const_stage_view(tail_filters_.front())->output_type();
            }

          if(sink_)
            {
              return const_stage_view(sink_)->output_type();
            }

          if(head_filters_.size())
            {
              return const_stage_view(head_filters_.front())->output_type();
            }

          return std::string("");
        }
    };

    bool is_compressor() const override {

      return sink_ ? true : false ;

    }

    /**
       \brief produce string of pipeline, separated by ->

       \param[in]

       \return
       \retval

    */
    std::string name() const override
    {

      std::ostringstream value;

      if(head_filters_.size())
        value << head_filters_.name();

      if(is_compressor())
        {
          if(head_filters_.size())
            value << "->";

          value << const_stage_view(sink_)->name();
          std::string cfg = const_stage_view(sink_)->config();

          if(!cfg.empty())
            value << "(" << cfg << ")";

          if(tail_filters_.size())
            value << "->";
        }

      if(tail_filters_.size())
        value << tail_filters_.name();

      return value.str();
    };

    std::string config() const override
    {
      return name();
    }

    std::string header(const std::vector<std::size_t>& _in_shape) const
    {


      std::size_t len = std::accumulate(_in_shape.begin(), _in_shape.end(),1,std::multiplies<std::size_t>());

      sqeazy::header hdr(incoming_t(),
                               _in_shape,
                               name(),
                               len*sizeof(incoming_t));

      std::string value(hdr.begin(),hdr.end());

      return value;
    }


    /**
       \brief encode one-dimensional array _in and write results to _out

       \param[in] _in input buffer
       \param[out] _out output buffer must be at least of size max_encoded_size
       \param[in] _in_len number of elements in _in

       \return
       \retval

    */
    outgoing_t * encode(const incoming_t *_in, outgoing_t *_out, std::size_t _in_len) override final {

      std::vector<std::size_t> shape(1,_in_len);
      return encode(_in,_out,shape);

    }




    /**
       \brief encode one-dimensional array _in and write results to _out

       \param[in] _in input buffer
       \param[out] _out output buffer must be at least of size max_encoded_size
       \param[in] _in_shape shape in input buffer in nDim

       \return
       \retval

    */

    outgoing_t* encode(const incoming_t *_in,
                       outgoing_t *_out,
                       const std::vector<std::size_t>& _in_shape) override final {

      outgoing_t* value = nullptr;
      std::size_t len = std::accumulate(_in_shape.begin(), _in_shape.end(),1,std::multiplies<std::size_t>());
      const std::size_t max_available_output_size = max_encoded_size(len*sizeof(incoming_t));
      const size_t scratchpad_bytes = (std::max)(max_available_output_size,len*sizeof(incoming_t));

      //create scratchpad here asynchronously as the allocation it consumes quite a long time the larger the data gets
      std::future<unique_array<incoming_t>> scratchpad = std::async(make_aligned<incoming_t>,
                                                                    std::size_t(32),
                                                                    scratchpad_bytes);

      ////////////////////// HEADER RELATED //////////////////
      //insert header
      sqeazy::header hdr(incoming_t(),
                               _in_shape,
                               name(),
                               len*sizeof(incoming_t));

      const std::intmax_t hdr_shift = hdr.size();
      char* output_buffer = reinterpret_cast<char*>(_out);
      std::copy(hdr.begin(), hdr.end(), output_buffer);
      outgoing_t* first_output = reinterpret_cast<outgoing_t*>(output_buffer+hdr_shift);

//TODO:check for alignment of first_output here!

      std::size_t available_bytes_out_buffer = max_available_output_size - hdr_shift;

      ////////////////////// ENCODING //////////////////
      value = detail_encode(_in,
                            first_output,
                            _in_shape,
                            available_bytes_out_buffer,
                            scratchpad);

      ////////////////////// HEADER RELATED //////////////////
      //update header as the encoding could have changed it
      std::size_t compressed_bytes = std::distance(first_output,value);
      hdr.set_compressed_size_byte<incoming_t>(compressed_bytes*sizeof(outgoing_t));
      hdr.set_pipeline<incoming_t>(name());

      if(hdr.size()!=(std::size_t)hdr_shift){
        std::copy(first_output, first_output + compressed_bytes,
                  output_buffer+hdr.size()
                  );
        first_output = reinterpret_cast<outgoing_t*>(output_buffer+hdr.size());
      }

      std::copy(hdr.begin(), hdr.end(), output_buffer);

      value = first_output+(compressed_bytes*sizeof(outgoing_t));

      return value;

    }

    template <typename future_t>
    outgoing_t* detail_encode(const incoming_t *_in,
                              outgoing_t *_out,
                              std::vector<std::size_t> _shape,
                              std::size_t available_output_bytes,
                              std::future<future_t>& scratchpad)  {

      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

      auto temp = scratchpad.get();
      incoming_t* head_filters_end = nullptr;
      incoming_t* head_results = nullptr;

      if(head_filters_.size()){
        head_filters_end = head_filters_.encode(_in,
                                                temp.get(),
                                                _shape,
                                                reinterpret_cast<incoming_t*>(_out)
                                                );

        if(!head_filters_end){
          std::cerr << "[dynamic_pipeline::detail_encode] unable to process data with head_filters\n";
          return nullptr;
        }

        head_results = temp.get();
      }

      outgoing_t* encoded_end = head_filters_end ? reinterpret_cast<outgoing_t*>(head_filters_end) : nullptr;
      if(sink_){
        encoded_end = sink_->encode( head_results ? head_results : _in,
                                    _out,
                                    _shape);

        if(!encoded_end){
          std::cerr << "[dynamic_pipeline::detail_encode] unable to process data with sink\n";
          return nullptr;
        }

        std::size_t compressed_size = std::distance(_out,encoded_end);
        if(tail_filters_.size()){

          outgoing_t* casted_temp = reinterpret_cast<outgoing_t*>(temp.get());


          std::vector<std::size_t> sinked_shape(_shape);

          if(compressed_size!=(len*sizeof(outgoing_t))){
            std::fill(sinked_shape.begin(), sinked_shape.end(),1);
            sinked_shape[row_major::x] = compressed_size;
          }

          encoded_end = tail_filters_.encode(_out,
                                             casted_temp,
                                             sinked_shape);
          if(!encoded_end){
            std::cerr << "[dynamic_pipeline::detail_encode] unable to process data with tail_filters\n";
            return nullptr;
          }
          encoded_end = std::copy(casted_temp,encoded_end,_out);

        }
      } else {

        if(head_results)
          encoded_end = std::copy(reinterpret_cast<outgoing_t*>(head_results),
                                  reinterpret_cast<outgoing_t*>(head_filters_end),
                                  _out);

      }

      return encoded_end;
    }








    /**
       \brief decode one-dimensional array _in and write results to _out

       \param[in]

       \return
       \retval

    */

    int decode(const outgoing_t *_in,
               incoming_t *_out,
               std::size_t _ilen,
               std::size_t _olen = 0
               ) const override final {

      if(!_olen)
        _olen = _ilen;

      std::vector<std::size_t> ishape = {_ilen};
      std::vector<std::size_t> oshape = {_olen};

      return decode(_in,_out,ishape,oshape);

    }

    /**
       \brief decode one-dimensional array _in and write results to _out

       \param[in] _in input buffer
       \param[out] _out output buffer //NOTE: might be larger than _in for sink type pipelines
       \param[in] _shape of input buffer size in units of its type, aka outgoing_t

       \return error code encoded as 3-digit decimal number
       0   .. filter error code before sink
       10  .. sink error code
       100 .. filter error code after sink
       \retval

    */

    int decode(const outgoing_t *_in,
               incoming_t *_out,
               const std::vector<std::size_t>& _inshape,
               std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final {


      std::size_t len = std::accumulate(_inshape.begin(), _inshape.end(),1,std::multiplies<std::size_t>());

      ////////////////////// HEADER RELATED //////////////////
      //load header
      const char* _in_char_begin = (const char*)_in;

      //FIXME: works only if len is greater than hdr.size()
      const char* _in_char_end = _in_char_begin + (len*sizeof(outgoing_t));

      sqeazy::header hdr(_in_char_begin,_in_char_end);
      std::vector<std::size_t> output_shape(hdr.shape()->begin(),
                                            hdr.shape()->end());

      if(_outshape.empty())
        _outshape = output_shape;

      const outgoing_t* payload_begin = reinterpret_cast<const outgoing_t*>(_in_char_begin + hdr.size());
      size_t in_size_bytes = (len*sizeof(outgoing_t)) - hdr.size();

      int value = detail_decode(payload_begin, _out,
                                in_size_bytes,
                                output_shape);

      return value;
    }

    int detail_decode(const outgoing_t *_in, incoming_t *_out,
                      std::size_t in_size_bytes,
                      std::vector<std::size_t> out_shape) const {
      int value = 0;
      int err_code = 0;

      ////////////////////// DECODING //////////////////
      std::size_t input_len = in_size_bytes/sizeof(*_in);
      std::size_t output_len = std::accumulate(out_shape.begin(), out_shape.end(),
                                               1,
                                               std::multiplies<std::size_t>());


      std::vector<incoming_t> temp(output_len,0);
      std::vector<std::size_t> in_shape(out_shape.size(),1);
      if(!in_shape.empty())
        in_shape.back() = input_len;

      if(is_compressor()){
        typedef typename sink_t::out_type sink_out_t;

        const sink_out_t* compressor_begin = reinterpret_cast<const sink_out_t*>(_in);
        std::vector<outgoing_t> sink_in;

        if(tail_filters_.size()){
          const outgoing_t* tail_in = reinterpret_cast<const outgoing_t*>(_in);

          //FIXME: filters may change the size of the buffer!
          sink_in.resize(output_len*sizeof(*_out)/*/sizeof(*_in)*/);
          std::fill(sink_in.begin(), sink_in.end(),0);

          outgoing_t* tail_out = reinterpret_cast<outgoing_t*>(sink_in.data());

          err_code = tail_filters_.decode(tail_in,
                                          tail_out,
                                          in_shape,
                                          out_shape);
          value += err_code ;

          //preparing for the sink
          compressor_begin = reinterpret_cast<const outgoing_t*>(sink_in.data());
          input_len    = sink_in.size();
          in_shape     = out_shape;
        }

        //FIXME: provide shape vectors?
        err_code = sink_->decode(compressor_begin,
                                 temp.data(),
                                 input_len,
                                 temp.size());
        value += err_code ? err_code+10 : 0 ;
        if(!err_code){
          std::fill(in_shape.begin(), in_shape.end(),1);
          in_shape.back() = temp.size();
        }
      }
      else{
        std::copy(_in,
                  _in+input_len,
                  reinterpret_cast<outgoing_t*>(temp.data()));
      }

      if(head_filters_.empty()){
        std::copy(temp.begin(),temp.begin()+output_len,_out);
      }
      else{

        err_code = head_filters_.decode(reinterpret_cast<const incoming_t*>(temp.data()),
                                        _out,out_shape);
        value += err_code ? err_code+100 : 0 ;

      }
      return value;

    }

    /**
       \brief function to estimate output buffer size for an input of _incoming_size_byte
       this value is calculated by taking twice the header size as currently available and adding the maximum encoded_size_byte of any registered stages

       for example: assuming the integer based pipeline "square_all_values->store_only_sum" yields a header of size 100
       * header_size = 100
       * given an input buffer of 100 integers, i.e. 400 Bytes, the filters provide
       square_all_values->max_encoded_size(400) = 400
       store_only_sum->max_encoded_size(400) = 8
       * the result would be: 2*100 + max(400,8) = 600 Bytes!


       \param[in]

       \return
       \retval

    */
    std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const override final {

      sqeazy::header hdr(incoming_t(),
                       _incoming_size_byte,
                       name()
                       );

      std::intmax_t value = hdr.size()*2;

      std::vector<std::intmax_t> max_enc_size;
      if(head_filters_.size())
        max_enc_size.push_back(head_filters_.max_encoded_size(_incoming_size_byte));

      if(sink_)
        max_enc_size.push_back(sink_->max_encoded_size(_incoming_size_byte));

      if(tail_filters_.size())
        max_enc_size.push_back(tail_filters_.max_encoded_size(_incoming_size_byte));

      if(!max_enc_size.empty())
        return value + *std::max_element(max_enc_size.begin(), max_enc_size.end());
      else
        return value;

    }

    template <typename T>
    std::intmax_t decoded_size(const T* _begin, const T* _end){

      static_assert(sizeof(T)==1, "[dynamic_pipeline.hpp::decoded_size] recived non-byte-size iterator");

      sqeazy::header found_header(_begin, _end);
      std::intmax_t value = found_header.raw_size_byte_as<incoming_t>();

      return value;
    }

    /**
       \brief obtain shape of decoded buffer that contains header from _begin to _end

       \param[in]

       \return
       \retval

    */
    template <typename T>
    std::vector<std::size_t> decoded_shape(const T* _begin, const T* _end){

      static_assert(sizeof(T)==1, "[dynamic_pipeline.hpp::decoded_shape] recived non-byte-size iterator");

      sqeazy::header found_header(_begin, _end);
      std::vector<std::size_t> value(found_header.shape()->begin(),
                                     found_header.shape()->end());

      return value;
    }

    virtual std::uint32_t n_threads() const override {
      return n_threads_;
    }

    virtual void set_n_threads(const std::uint32_t& _number) override {

      n_threads_ = clean_number_of_threads(_number);

      if(sink_)
        sink_->set_n_threads(n_threads_);

      auto head_begin = head_filters_.begin();
      auto head_end = head_filters_.end();
      for(;head_begin != head_end;++head_begin){
        if((*head_begin)!=nullptr)
          (*head_begin)->set_n_threads(n_threads_);
      }

      auto tail_begin = tail_filters_.begin();
      auto tail_end = tail_filters_.end();
      for(;tail_begin != tail_end;++tail_begin){
        if((*tail_begin)!=nullptr)
          (*tail_begin)->set_n_threads(n_threads_);
      }

    }


  };
}

#endif /* _DYNAMIC_PIPELINE_H_ */
