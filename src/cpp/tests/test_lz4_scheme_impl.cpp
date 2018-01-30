#define BOOST_TEST_MODULE TEST_LZ4_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include "array_fixtures.hpp"
#include "encoders/lz4.hpp"

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;
typedef sqeazy::array_fixture<unsigned char> uint8_cube_of_8;

BOOST_FIXTURE_TEST_SUITE( sixteen_bit, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( encodes )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local;
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());
  BOOST_CHECK_LT(std::distance(encoded.data(),res),expected_encoded_bytes);

}

BOOST_AUTO_TEST_CASE( decodes )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);

}


BOOST_AUTO_TEST_CASE( roundtrip )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( eight_bit, uint8_cube_of_8 )

BOOST_AUTO_TEST_CASE( encodes )
{
  std::vector<std::size_t> shape(dims.begin(), dims.end());

  sqeazy::lz4_scheme<value_type> local;
  auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
  std::vector<char> encoded(expected_encoded_bytes);

  auto res = local.encode(&incrementing_cube[0],
                          encoded.data(),
                          shape);

  BOOST_REQUIRE_NE(res,(char*)nullptr);
  BOOST_CHECK_NE(res,encoded.data());

}

BOOST_AUTO_TEST_CASE( decodes )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);

}


BOOST_AUTO_TEST_CASE( roundtrip )
{
    std::vector<std::size_t> shape(dims.begin(), dims.end());

    sqeazy::lz4_scheme<value_type> local;
    auto expected_encoded_bytes = local.max_encoded_size(size_in_byte);
    std::vector<char> encoded(expected_encoded_bytes);

    auto res = local.encode(&incrementing_cube[0],
                            encoded.data(),
                            shape);

    BOOST_REQUIRE_NE(res,(char*)nullptr);

    auto rcode = local.decode(encoded.data(),
                              to_play_with.data(),
                              encoded.size(),
                              size_in_byte);

    BOOST_REQUIRE_NE(rcode,1);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        incrementing_cube.begin(),
        incrementing_cube.end(),
        to_play_with.begin(),
        to_play_with.end()
        );
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( blocksizes, uint16_cube_of_8 )

BOOST_AUTO_TEST_CASE( less_than )
{
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(32),sqeazy::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(0),sqeazy::closest_blocksize::vals[0] );
}

BOOST_AUTO_TEST_CASE( greater_than )
{
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(16 << 10),sqeazy::closest_blocksize::vals.back() );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(~0),sqeazy::closest_blocksize::vals.back() );
}

BOOST_AUTO_TEST_CASE( equal )
{

  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(64),sqeazy::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(256),sqeazy::closest_blocksize::vals[1] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(1024),sqeazy::closest_blocksize::vals[2] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(4096),sqeazy::closest_blocksize::vals[3] );

}

BOOST_AUTO_TEST_CASE( inbetween )
{

  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(65),sqeazy::closest_blocksize::vals[0] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(255),sqeazy::closest_blocksize::vals[1] );
  BOOST_CHECK_EQUAL( sqeazy::closest_blocksize::of(257),sqeazy::closest_blocksize::vals[1] );

}

BOOST_AUTO_TEST_SUITE_END()
