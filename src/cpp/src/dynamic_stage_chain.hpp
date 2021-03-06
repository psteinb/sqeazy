#ifndef DYNAMIC_STAGE_CHAIN_H
#define DYNAMIC_STAGE_CHAIN_H

#include <vector>

#include "dynamic_stage.hpp"
#include "sqeazy_common.hpp"

namespace sqeazy {


    template <typename filter_t>
    struct stage_chain {


        typedef typename filter_t::in_type incoming_t;
        typedef typename filter_t::out_type outgoing_t;

        typedef filter_t			filter_base_t;
        typedef std::shared_ptr< filter_t > filter_ptr_t;
        typedef std::vector<filter_ptr_t>   filter_holder_t;

        filter_holder_t chain_;

        friend void swap(stage_chain & _lhs, stage_chain & _rhs){

            std::swap(_lhs.chain_,_rhs.chain_);
        }

        stage_chain():
            chain_(){}

        stage_chain(std::initializer_list<filter_ptr_t> _chain):
            chain_(){

            for(const filter_ptr_t& step : _chain)
            {
                chain_.push_back(step);
            }

        }


        stage_chain(const stage_chain& _rhs):
            chain_(_rhs.chain_.begin(), _rhs.chain_.end())
            {}


        stage_chain& operator=(stage_chain _rhs){

            swap(*this, _rhs);

            return *this;
        }

        const std::size_t size() const { return chain_.size(); }
        const bool empty() const { return chain_.empty(); }
        void clear() { chain_.clear(); };

        typename filter_holder_t::const_iterator begin() const {
            return chain_.begin();
        }

        typename filter_holder_t::const_iterator end() const {
            return chain_.end();
        }

        typename filter_holder_t::const_iterator cbegin() const {
            return chain_.cbegin();
        }

        typename filter_holder_t::const_iterator cend() const {
            return chain_.cend();
        }


        typename filter_holder_t::iterator begin()  {
            return chain_.begin();
        }

        typename filter_holder_t::iterator end()  {
            return chain_.end();
        }

        template <typename pointee_t>
        void push_back(const std::shared_ptr<pointee_t>& _new){

            if(std::is_base_of<filter_t,pointee_t>::value){
                auto casted = std::dynamic_pointer_cast<filter_t>(_new);
                chain_.push_back(casted);
            }

        }

        void add(const filter_ptr_t& _new){

            this->push_back(_new);

        }

        bool is_compressor() const {

            return filter_t::is_compressor ;

        }

        bool valid() const {

            bool value = true;
            if(chain_.empty())
                return !value;

            if(chain_.size()==1 || std::is_same<filter_t,filter<incoming_t> >::value)
                return true;

            //TODO: this loop is obsolete for sqy::filter types
            for(unsigned i = 1; i < chain_.size(); ++i)
            {
                auto this_filter =  const_stage_view(chain_[i]);
                auto last_filter =  const_stage_view(chain_[i-1]);

                if(!this_filter || !last_filter){
                    value = false ;
                    break;
                }
                value = value && (this_filter->input_type() == last_filter->output_type());
            }

            return value;

        }

        filter_ptr_t& front() {

            return chain_.front();

        }

        const filter_ptr_t& front() const {

            return chain_.front();

        }

        filter_ptr_t& back() {

            return chain_.back();

        }

        const filter_ptr_t& back() const {

            return chain_.back();

        }

        std::string name() const
            {

                std::ostringstream value;

                //TODO: REFACTOR THIS!
                for(const auto& step : chain_)
                {
                    auto cview = const_stage_view(step);

                    if(!cview){
                        std::cerr << "[src/cpp/src/dynamic_stage.hpp] \t unable to parse chain step\n";
                        continue;
                    }

                    value << cview->name();
                    std::string cfg = cview->config();

                    if(!cfg.empty())
                        value << "(" << cfg << ")";

                    if(step != chain_.back())
                        value << "->";
                }

                return value.str();
            }

        std::string config() const
            {
                return name();
            }

        /**
           \brief encode one-dimensional array _in and write results to _out

           \param[in]

           \return
           \retval

        */

        outgoing_t * encode(const incoming_t *_in, outgoing_t *_out, std::size_t _len) /*override final*/ {

            std::vector<std::size_t> shape(1,_len);
            return encode(_in,_out,shape);

        }


        /**
           \brief encode one-dimensional array _in and write results to _out

           \param[in] _in input buffer
           \param[out] _out output buffer must be at least of size max_encoded_size
           \param[in] _shape shape in input buffer in nDim

           \return
           \retval

        */

        outgoing_t* encode(const incoming_t *_in,
                           outgoing_t *_out,
                           const std::vector<std::size_t>& _shape) /*override final*/ {
            outgoing_t* value = nullptr;
            const std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
            const std::size_t bytes = len*sizeof(incoming_t);

            const std::intmax_t max_output_bytes = this->max_encoded_size(bytes);

            //TODO: Ouch, this can be a quite large allocation
            //(may cost a lot of cycles, more than 50% of the cycles consumed by this method with 400MB of input)
            sqeazy::vec_32algn_t<incoming_t> temp_in(std::ceil(float(max_output_bytes)/sizeof(incoming_t)));
            std::copy(_in, _in+len,temp_in.data());

            outgoing_t * out_ptr = _out;
            incoming_t * in_ptr = temp_in.data();

            std::size_t compressed_items = 0;

            for( std::size_t fidx = 0;fidx<chain_.size();++fidx )
            {

                value = chain_[fidx]->encode( in_ptr,
                                              out_ptr,
                                              _shape);

                if(value)
                    compressed_items = std::distance(out_ptr,value);
                else
                    compressed_items = 0;

                if(compressed_items>temp_in.size() || compressed_items == 0){
                    std::ostringstream msg;
                    msg << __FILE__ << ":" << __LINE__ << "\t encode wrote past the end of temporary buffers\n";
                    throw std::runtime_error(msg.str());
                }

                // std::copy(temp_out.data(),value,temp_in.begin());
                std::swap(in_ptr,out_ptr);
            }

            if(chain_.size() % 2 == 0){
                value = std::copy(reinterpret_cast<outgoing_t*>(temp_in.data()),
                                  value,
                                  _out);
            }

            return value;

        }


        /**
           \brief encode one-dimensional array _in and write results to _out

           \param[in] _in input buffer
           \param[in] _shape shape in input buffer in nDim
           \param[out] _out output buffer must be at least of size max_encoded_size
           \param[in] _scratchpad temporary memory used for iterating the chain (can be nullptr)


           \return
           \retval

        */
        outgoing_t* encode(const incoming_t *_in,
                           outgoing_t *_out,
                           const std::vector<std::size_t>& _shape,
                           outgoing_t *_scratchpad
            ){

            outgoing_t* value = nullptr;
            const std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());

            const std::size_t max_output_bytes = this->max_encoded_size(len*sizeof(incoming_t));

            outgoing_t * out_ptr = _out;
            incoming_t * in_ptr = nullptr;

            std::size_t compressed_items = 0;

            for( std::size_t fidx = 0;fidx<chain_.size();++fidx )
            {

                value = chain_[fidx]->encode( in_ptr ?  in_ptr : _in,
                                              out_ptr,
                                              _shape);

                if(value)
                    compressed_items = std::distance(out_ptr,value);
                else
                    compressed_items = 0;

                if(compressed_items>max_output_bytes || compressed_items == 0){
                    std::ostringstream msg;
                    msg << __FILE__ << ":" << __LINE__ << "\t encode wrote past the end of temporary buffers\n";
                    throw std::runtime_error(msg.str());
                }

                if(!in_ptr && _scratchpad)
                    in_ptr = _scratchpad;

                std::swap(in_ptr,out_ptr);
            }

            if(chain_.size() % 2 == 0){
                value = std::copy(reinterpret_cast<outgoing_t*>(in_ptr),
                                  value,
                                  _out);
            }

            return value;

        }


        int decode(const outgoing_t *_in, incoming_t *_out, std::size_t _len) const /*override final*/ {

            std::vector<std::size_t> shape(1,_len);
            return decode(_in,_out,shape);

        }
        /**
           \brief decode one-dimensional array _in and write results to _out

           \param[in] _in input buffer
           \param[out] _out output buffer //NOTE: might be larger than _in for sink type pipelines
           \param[in] _shape of input buffer size in units of its type, aka outgoing_t

           \return
           \retval

        */

        int decode(const outgoing_t *_in,
                   incoming_t *_out,
                   const std::vector<std::size_t>& _ishape,
                   std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const /*override final*/ {

            int value = 0;
            int err_code = 0;
            if(_oshape.empty())
                _oshape = _ishape;

            std::size_t len = std::accumulate(_ishape.begin(), _ishape.end(),1,std::multiplies<std::size_t>());

            //TODO: Ouch, this can be a quite large allocation
            sqeazy::vec_32algn_t<incoming_t> temp(_in,_in+len);

            auto rev_begin = chain_.rbegin();
            auto rev_end   = chain_.rend();
            int fidx = 0;

            for(;rev_begin!=rev_end;++rev_begin,++fidx)
            {

                err_code = (*rev_begin)->decode(temp.data(),
                                                _out,
                                                _ishape,
                                                _oshape);
                value += err_code ? (10*(fidx+1))+err_code : 0;
                std::copy(_out,
                          _out+len,
                          temp.data());
            }

            return value;

        }

        std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const {

            std::vector<std::intmax_t> values;
            for( auto & f : chain_ )
                values.push_back(f->max_encoded_size(_incoming_size_byte));

            if(values.empty())
                return 0;
            else
                return *std::max_element(values.begin(), values.end());


        }
    };


}  // Namespace

#endif /* DYNAMIC_STAGE_CHAIN_H */
