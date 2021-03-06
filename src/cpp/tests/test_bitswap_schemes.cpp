#define BOOST_TEST_MODULE TEST_BITSWAP_SCHEMES
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <climits>
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;

/**
   \brief produce the bit pattern (as T) that would appear in an
   encoded bit stream where only values of _const appear.
   If the input only contains the value 1 as 16bit integer, the exepected output element
   can be constructed by hand as in:
   value_type condensed = ((constant_cube[0] << 1) << 4) + (constant_cube[0] << 1);
   condensed += condensed << 8;

   \param[in] _const single value that appears in encoding input buffer

   \return type T
   \retval

*/
template <unsigned plane_width,typename T>
T condense_constant(const T& _const){

  if(_const > (1<<plane_width))
    return 0;

  T condensed = 0;

  for(unsigned i = 0;i<(CHAR_BIT/plane_width);++i){
    T temp = _const ;//<< 1;//emulates rotate_left<1>

    condensed += (temp << i*plane_width);
  }

  if(sizeof(T)>1){
    for(unsigned i = 1;i<=sizeof(T);++i)
      condensed += condensed << (8*i);
  }

  return condensed;

}


BOOST_FIXTURE_TEST_SUITE( bitswap4x4_scheme_encode_out_of_place, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( encode_success )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap4Encode_UI16(input,
                    output,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(constant_cube[0],to_play_with[0]);
}

BOOST_AUTO_TEST_CASE( encode_constant_correct )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap4Encode_UI16(input,
                    output,
                    uint16_cube_of_8::size_in_byte);

  //constant cube has all elements set to 1
  //BitSwap4Encode splits each element into 4 pieces: Ai, Bi, Ci, Di
  //the output is filled as [Ai,..,Bi,..,Ci,..,Di,..]
  //if the input only contains the value 1 as 16bit integer, the exepected output element
  //can be constructed by hand
  // value_type condensed = ((constant_cube[0] << 1) << 4) + (constant_cube[0] << 1);
  // condensed += condensed << 8;
  value_type condensed = condense_constant<4>(constant_cube[0]);

  const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
  const unsigned expected_sum = (uint16_cube_of_8::size-end_three_quarters)*condensed;

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],0);
  BOOST_CHECK_EQUAL(std::accumulate(&to_play_with[0],&to_play_with[0]+ uint16_cube_of_8::size,0u),expected_sum);
  BOOST_CHECK_EQUAL(to_play_with[uint16_cube_of_8::size-1],condensed);

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( bitswap4x4_scheme_encode_out_of_place, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( decode_success )
{

  value_type condensed = condense_constant<4>(constant_cube[0]);

  const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
  std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

  const char* input = reinterpret_cast<char*>(&to_play_with[0]);
  value_type  output[uint16_cube_of_8::size];
  char* output_casted = reinterpret_cast<char*>(&output[0]);

  int retcode = SQY_BitSwap4Decode_UI16(input,
                    output_casted,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(to_play_with[0],output[0]);

}

BOOST_AUTO_TEST_CASE( is_synthetic_input_valid )
{
  value_type condensed = condense_constant<4>(constant_cube[0]);

  const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
  std::fill(&to_play_with[0],&to_play_with[0]+uint16_cube_of_8::size,0);
  std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  value_type output[uint16_cube_of_8::size];
  std::fill(&output[0],&output[0] + uint16_cube_of_8::size,0);
  char* output_casted = reinterpret_cast<char*>(&output[0]);

  int retcode = SQY_BitSwap4Encode_UI16(input,
                    output_casted,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],output[0]);
  try{
    BOOST_REQUIRE_EQUAL_COLLECTIONS(&output[0], &output[0] + uint16_cube_of_8::size,
                    &to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);
  }catch(...){
    BOOST_TEST_MESSAGE("is_synthetic_input_valid threw exception");
    throw;
  }

}

BOOST_AUTO_TEST_CASE( decode_exact )
{
  value_type condensed = condense_constant<4>(constant_cube[0]);

  const unsigned end_three_quarters = .75*uint16_cube_of_8::size;
  std::fill(&to_play_with[0],&to_play_with[0]+uint16_cube_of_8::size,0);
  std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

  const char* input = reinterpret_cast<char*>(&to_play_with[0]);
  value_type  output[uint16_cube_of_8::size];
  std::fill(output,output+uint16_cube_of_8::size,0);
  char* output_casted = reinterpret_cast<char*>(&output[0]);

  int retcode = SQY_BitSwap4Decode_UI16(input,
                    output_casted,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(output[0],constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&output[0], &output[0] + uint16_cube_of_8::size,
                &constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size);

}

BOOST_AUTO_TEST_CASE( decode_encoded )
{

  value_type  intermediate[uint16_cube_of_8::size];
  std::fill(intermediate,intermediate+uint16_cube_of_8::size,0);
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  SQY_BitSwap4Encode_UI16(reinterpret_cast<const char*>(&constant_cube[0]),
              intermediate_casted,
              uint16_cube_of_8::size_in_byte);

  const char* input = reinterpret_cast<const char*>(&intermediate[0]);

  int retcode = SQY_BitSwap4Decode_UI16(input,
                    reinterpret_cast<char*>(&to_play_with[0]),
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size,
                &to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);

}

BOOST_AUTO_TEST_CASE( decode_encoded_incrementing )
{

  value_type  intermediate[uint16_cube_of_8::size];
  std::fill(intermediate,intermediate+uint16_cube_of_8::size,1);
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  SQY_BitSwap4Encode_UI16(reinterpret_cast<const char*>(&incrementing_cube[0]),
              intermediate_casted,
              uint16_cube_of_8::size_in_byte);

  const char* input = reinterpret_cast<const char*>(&intermediate[0]);

  int retcode = SQY_BitSwap4Decode_UI16(input,
                    reinterpret_cast<char*>(&to_play_with[0]),
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],incrementing_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&incrementing_cube[0], &incrementing_cube[0] + uint16_cube_of_8::size,
                &to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( bitswap8x2_scheme_encode_out_of_place, uint16_cube_of_8 )


BOOST_AUTO_TEST_CASE( encode_success )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap8Encode_UI16(input,
                    output,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_NE(constant_cube[0],to_play_with[0]);
}

BOOST_AUTO_TEST_CASE( encode_constant_correct )
{

  const char* input = reinterpret_cast<char*>(&constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap8Encode_UI16(input,
                    output,
                    uint16_cube_of_8::size_in_byte);

  //constant cube has all elements set to 1
  //BitSwap4Encode splits each element into 2 pieces: Ai, Bi
  //the output the is filled as [Ai,..,Bi,..]
  //if the input only contains the value 1 as 16bit integer, the exepected output element
  //can be constructed by hand
  value_type condensed = condense_constant<8>(constant_cube[0]);

  const unsigned end_three_quarters = .5*uint16_cube_of_8::size;
  const unsigned expected_sum = (uint16_cube_of_8::size-end_three_quarters)*condensed;

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],0);
  BOOST_CHECK_EQUAL(to_play_with[uint16_cube_of_8::size-1],condensed);

  BOOST_CHECK_EQUAL(std::accumulate(&to_play_with[0],&to_play_with[0]+ uint16_cube_of_8::size,0u),expected_sum);

}

BOOST_AUTO_TEST_CASE( decode_exact )
{

  value_type condensed = condense_constant<8>(constant_cube[0]);

  const unsigned end_three_quarters = .5*uint16_cube_of_8::size;
  std::fill(&to_play_with[0],&to_play_with[0]+uint16_cube_of_8::size,0);
  std::fill(&to_play_with[0]+end_three_quarters,&to_play_with[0]+uint16_cube_of_8::size,condensed);

  const char* input = reinterpret_cast<char*>(&to_play_with[0]);
  value_type  output[uint16_cube_of_8::size];
  std::fill(output,output+uint16_cube_of_8::size,0);
  char* output_casted = reinterpret_cast<char*>(&output[0]);

  int retcode = SQY_BitSwap8Decode_UI16(input,
                    output_casted,
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(output[0],constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&output[0], &output[0] + uint16_cube_of_8::size,
                &constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size);

}

BOOST_AUTO_TEST_CASE( decode_encoded )
{

  value_type  intermediate[uint16_cube_of_8::size];
  std::fill(intermediate,intermediate+uint16_cube_of_8::size,0);
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  SQY_BitSwap8Encode_UI16(reinterpret_cast<const char*>(&constant_cube[0]),
              intermediate_casted,
              uint16_cube_of_8::size_in_byte);

  const char* input = reinterpret_cast<const char*>(&intermediate[0]);

  int retcode = SQY_BitSwap8Decode_UI16(input,
                    reinterpret_cast<char*>(&to_play_with[0]),
                    uint16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&constant_cube[0], &constant_cube[0] + uint16_cube_of_8::size,
                &to_play_with[0], &to_play_with[0] + uint16_cube_of_8::size);

}

BOOST_AUTO_TEST_SUITE_END()

typedef sqeazy::array_fixture<short> int16_cube_of_8;


BOOST_FIXTURE_TEST_SUITE( signed_bitswap4x4_scheme_encode_out_of_place, int16_cube_of_8 )


BOOST_AUTO_TEST_CASE( signed_encode_success )
{

  value_type negative_constant_cube[size];
  std::fill(&negative_constant_cube[0], &negative_constant_cube[0]+size,-1*constant_cube[0]);

  const char* input = reinterpret_cast<char*>(&negative_constant_cube[0]);
  char* output = reinterpret_cast<char*>(&to_play_with[0]);

  int retcode = SQY_BitSwap4Encode_I16(input,
                    output,
                    int16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(negative_constant_cube[0],to_play_with[0]);

}

BOOST_AUTO_TEST_CASE( signed_decode_encoded_of_negative )
{

  value_type negative_constant_cube[size];
  std::fill(&negative_constant_cube[0], &negative_constant_cube[0]+size,-1*constant_cube[0]);

  value_type  intermediate[int16_cube_of_8::size];
  std::fill(intermediate,intermediate+int16_cube_of_8::size,0);
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  SQY_BitSwap4Encode_I16(reinterpret_cast<const char*>(&negative_constant_cube[0]),
              intermediate_casted,
              int16_cube_of_8::size_in_byte);

  const char* input = reinterpret_cast<const char*>(&intermediate[0]);

  int retcode = SQY_BitSwap4Decode_I16(input,
                       reinterpret_cast<char*>(&to_play_with[0]),
                       int16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],negative_constant_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&negative_constant_cube[0], &negative_constant_cube[0] + int16_cube_of_8::size,
                &to_play_with[0], &to_play_with[0] + int16_cube_of_8::size);

}




BOOST_AUTO_TEST_CASE( signed_decode_encoded_of_mixture )
{

  value_type negative_incrementing_cube[size];
  std::copy(&incrementing_cube[0], &incrementing_cube[0]+size,&negative_incrementing_cube[0]);

  for(unsigned i = 0;i<int16_cube_of_8::size;++i)
    negative_incrementing_cube[i] -= .25*std::numeric_limits<value_type>::max();

  value_type  intermediate[int16_cube_of_8::size];
  std::fill(intermediate,intermediate+int16_cube_of_8::size,0);
  char* intermediate_casted = reinterpret_cast<char*>(&intermediate[0]);

  SQY_BitSwap4Encode_I16(reinterpret_cast<const char*>(&negative_incrementing_cube[0]),
              intermediate_casted,
              int16_cube_of_8::size_in_byte);

  const char* input = reinterpret_cast<const char*>(&intermediate[0]);

  int retcode = SQY_BitSwap4Decode_I16(input,
                       reinterpret_cast<char*>(&to_play_with[0]),
                       int16_cube_of_8::size_in_byte);

  BOOST_CHECK_EQUAL(retcode,0);
  BOOST_CHECK_EQUAL(to_play_with[0],negative_incrementing_cube[0]);
  BOOST_CHECK_EQUAL_COLLECTIONS(&negative_incrementing_cube[0], &negative_incrementing_cube[0] + int16_cube_of_8::size,
                &to_play_with[0], &to_play_with[0] + int16_cube_of_8::size);

}
BOOST_AUTO_TEST_SUITE_END()
