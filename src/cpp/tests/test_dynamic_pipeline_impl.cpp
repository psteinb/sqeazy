#define BOOST_TEST_MODULE TEST_DYNAMIC_PIPELINE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <cstdint>
#include <sstream>
#include "../src/dynamic_pipeline.hpp"
#include "array_fixtures.hpp"
#include "../src/sqeazy_impl.hpp"

namespace sqy = sqeazy;

template <typename T>
struct add_one : public sqy::stage {

  typedef T raw_type;
  typedef T compressed_type;
  
  bool is_compressor() const {
    
    return false;
    
  }
  
  std::string name() const {

    return std::string("add_one");

  }


  T operator()( const T& _in) {
    return _in + 1;
  }

  static int encode( const raw_type* _in, compressed_type* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;
   
    add_one<T> adder;
    std::transform(begin, end, _out, adder);

    return 0;
  }

  static const unsigned long max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

  ~add_one(){};


  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }
  
};

template <typename T >
struct square : public sqy::stage {

  typedef T raw_type;
  typedef T compressed_type;

  bool is_compressor() const {
    
    return false;
    
  }

  std::string name() const {

    return std::string("square");

  }

  T operator()( const T& _in) {
    return _in * _in;
  }

  static int encode( const raw_type* _in, compressed_type* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;

    square<T> operation;
    std::transform(begin, end, _out, operation);

    return 0;
  }

  static const unsigned long max_encoded_size(unsigned long _size_bytes){
    return _size_bytes;
  }

  ~square(){};

  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }
  
};

template <typename T >
struct sum_up : public sqy::stage {

  typedef T raw_type;
  typedef uint32_t compressed_type;
  
  bool is_compressor() const {
    
    return true;
    
  }
  
  std::string name() const {

    return std::string("sum_up");

  }

  static int encode( const raw_type* _in, compressed_type* _out, const unsigned long& _size) {
    const T* begin = _in;
    const T* end = begin + _size;

    *_out = std::accumulate(begin, end, 0);

    return 0;
  }

  static const unsigned long max_encoded_size(unsigned long _size_bytes){
    return sizeof(compressed_type);
  }

  ~sum_up(){};
  

  std::string input_type() const {

    return typeid(raw_type).name();
    
  }

  std::string output_type() const {

    return typeid(compressed_type).name();
    
  }
  
};

typedef add_one<int> add_one_to_ints;
using int_factory_with_one_entry = sqy::stage_factory<add_one_to_ints> ;
using int_factory = sqy::stage_factory<add_one_to_ints, square<int>, sum_up<int> > ;

template <typename T>
using filter_factory = sqy::stage_factory<add_one<T>, square<T> >;

template <typename T>
using sink_factory = sqy::stage_factory<sum_up<T> >;


BOOST_AUTO_TEST_SUITE( factory_test_suite )


  
BOOST_AUTO_TEST_CASE( factory_finds_valid )
{

  std::shared_ptr<sqy::stage> result = int_factory_with_one_entry::create("add_one");
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
    
}

BOOST_AUTO_TEST_CASE( factory_finds_valid_from_list )
{

  std::shared_ptr<sqy::stage> result = int_factory::create("add_one");
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
  BOOST_CHECK_EQUAL(int_factory::create("sum_up")->name(),"sum_up");
  
}


BOOST_AUTO_TEST_CASE( templated_factory_finds_valid )
{

  std::shared_ptr<sqy::stage> result = filter_factory<int>::create("square");
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"square");
    
}


BOOST_AUTO_TEST_CASE( factory_finds_nothing )
{

  BOOST_CHECK(int_factory::create("dope")==nullptr);
    
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( access_test_suite )


BOOST_AUTO_TEST_CASE( without_pipeline )
{
    std::vector<int> test_in(42,1);
    std::vector<int> test_out(42,0);

    std::transform(test_in.begin(), test_in.end(), test_out.begin(), add_one<int>() );

    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*2);

    std::copy(test_out.begin(), test_out.end(), test_in.begin());
    std::transform(test_in.begin(), test_in.end(), test_out.begin(), square<int>() );
    BOOST_CHECK_EQUAL(std::accumulate(test_out.begin(), test_out.end(),0),42*4);
}

BOOST_AUTO_TEST_CASE (construct) {

  sqy::dynamic_pipeline empty_pipe;
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.size(), 0);
  BOOST_CHECK_EQUAL(empty_pipe.empty(), true);

  
}

BOOST_AUTO_TEST_CASE (copy_construct) {

  sqy::dynamic_pipeline filled_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(filled_pipe.size(), 2);
  
  sqy::dynamic_pipeline assigned_pipe = filled_pipe;
  BOOST_CHECK_EQUAL(assigned_pipe.size(), 2);
  BOOST_CHECK_NE(assigned_pipe.empty(), true);

  sqy::dynamic_pipeline copied_pipe(filled_pipe);
  BOOST_CHECK_EQUAL(copied_pipe.size(), 2);
  BOOST_CHECK_NE(copied_pipe.empty(), true);

}


BOOST_AUTO_TEST_CASE (types_match) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_NE(empty_step.valid_filters(),true);
  
  sqy::dynamic_pipeline single_step = {std::make_shared<add_one<int> >()};
  BOOST_CHECK(single_step.valid_filters());
  
  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK(working_pipe.valid_filters());
  
  sqy::dynamic_pipeline failing_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<char> >()};
  BOOST_CHECK_NE(failing_pipe.valid_filters(),true);

  sqy::dynamic_pipeline working_pipe_with_sink = {std::make_shared<add_one<int> >(),
						  std::make_shared<square<int> >(),
						  std::make_shared<sum_up<int> >()};
  BOOST_CHECK(working_pipe_with_sink.valid_filters());
  
}

BOOST_AUTO_TEST_CASE (name_given) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.name(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.name(),"add_one->square");

  sqy::dynamic_pipeline compressing_pipe = {std::make_shared<add_one<int> >(),std::make_shared<sum_up<int> >()};
  BOOST_CHECK_EQUAL(compressing_pipe.name(),"add_one->sum_up");

}

BOOST_AUTO_TEST_CASE (input_type) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.input_type(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.input_type(),typeid(int).name());

}

BOOST_AUTO_TEST_CASE (output_type) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline working_pipe = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  BOOST_CHECK_EQUAL(working_pipe.output_type(),typeid(int).name());

}

BOOST_AUTO_TEST_CASE (add) {

  sqy::dynamic_pipeline empty_step;
  BOOST_CHECK_EQUAL(empty_step.output_type(),"");

  sqy::dynamic_pipeline::filter_holder_t input = {std::make_shared<add_one<int> >(),std::make_shared<square<int> >()};
  for(auto step : input)
    empty_step.add(step);
  
  BOOST_CHECK_EQUAL(empty_step.empty(),false);
  BOOST_CHECK_EQUAL(empty_step.size(),2);

}


BOOST_AUTO_TEST_CASE (bootstrap) {

  sqy::dynamic_pipeline filter_pipe;
  BOOST_CHECK_EQUAL(filter_pipe.output_type(),"");

  filter_pipe = sqy::dynamic_pipeline::load("add_one->square",
					    filter_factory<int>(),
					    sink_factory<int>());
  BOOST_CHECK_NE(filter_pipe.empty(),true);
  BOOST_CHECK_EQUAL(filter_pipe.size(),2);

  sqy::dynamic_pipeline sink_pipe;
  BOOST_CHECK_EQUAL(sink_pipe.output_type(),"");

  sink_pipe = sqy::dynamic_pipeline::load("add_one->sum_up",filter_factory<int>(), sink_factory<int>());
  BOOST_CHECK_NE(sink_pipe.empty(),true);
  BOOST_CHECK_EQUAL(sink_pipe.size(),2);
  BOOST_CHECK_EQUAL(sink_pipe.is_compressor(),true);
  BOOST_CHECK_EQUAL(sink_pipe.name(),"add_one->sum_up");

  sqy::dynamic_pipeline empty_pipe;
  empty_pipe = sqy::dynamic_pipeline::load("add_one->square");
  BOOST_CHECK_EQUAL(empty_pipe.empty(),true);
}

BOOST_AUTO_TEST_CASE (encode) {

  std::vector<int> input(10);
  std::iota(input.begin(), input.end(),0);
  BOOST_CHECK_EQUAL(input.front(),0);
  BOOST_CHECK_EQUAL(input.back(),9);
  
  std::vector<int> output(input);

  sqy::dynamic_pipeline sink_pipe = sqy::dynamic_pipeline::load("square",filter_factory<int>(), sink_factory<int>());
  sink_pipe.encode(&input[0],&output[0],input.size());

  BOOST_CHECK_NE(output.front(),0);
  BOOST_CHECK_NE(output.back(),9);
  
}
  
BOOST_AUTO_TEST_SUITE_END()


// BOOST_AUTO_TEST_SUITE( stage_suite )


