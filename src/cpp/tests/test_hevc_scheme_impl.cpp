#define BOOST_TEST_MODULE TEST_HEVC_SCHEME_IMPL
#define BOOST_TEST_MAIN
#include "boost/test/included/unit_test.hpp"
#include <climits>
#include <cstdint>
#include <vector>
#include <iostream>
#include <bitset>
#include <map>
#include <cstdint>

#define DEBUG_HEVC

#include "encoders/external_encoders.hpp"
#include "volume_fixtures.hpp"
#include "tiff_utils.hpp"
#include "yuv_utils.hpp"
#include "sqeazy_algorithms.hpp"
#include "traits.hpp"
#include "test_algorithms.hpp"

namespace sqy = sqeazy;

BOOST_FIXTURE_TEST_SUITE( avcodec_8bit, sqeazy::volume_fixture<std::uint8_t> )

BOOST_AUTO_TEST_CASE( encode ){

  av_register_all();

  std::vector<std::uint8_t> results(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};
  std::size_t bytes_written = 0;
  sqeazy::hevc_scheme<std::uint8_t> scheme;
  auto encoded_end = scheme.encode(embryo_.data(),
                    results.data(),
                    shape);

  BOOST_REQUIRE(encoded_end!=nullptr);
  bytes_written = encoded_end - results.data();

  results.resize(bytes_written);


  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  float sum = std::accumulate(results.begin(), results.end(),0);
  BOOST_CHECK_NE(sum,0);

}

BOOST_AUTO_TEST_CASE( roundtrip ){

  std::vector<std::uint8_t> encoded(embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                 static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};
  std::size_t bytes_written = 0;

  sqeazy::hevc_scheme<std::uint8_t> scheme;
  auto encoded_end = scheme.encode(embryo_.data(),
                    encoded.data(),
                    shape);

  BOOST_REQUIRE(encoded_end!=nullptr);
  bytes_written = encoded_end - encoded.data();
  std::vector<std::size_t> encoded_shape = {bytes_written};

  encoded.resize(bytes_written);


  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());


  std::vector<std::uint8_t> retrieved(embryo_.num_elements(),0);
  int err = scheme.decode(encoded.data(),
              retrieved.data(),
              encoded_shape,
              shape
              );

  BOOST_CHECK_EQUAL(err,0);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm);

  try{
    BOOST_REQUIRE_LT(l2norm,1e-2);
    BOOST_WARN_MESSAGE(std::equal(retrieved.begin(),
                         retrieved.begin()+retrieved.size(),
                         embryo_.data()),
               "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"embryo.tiff");
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }

}

BOOST_AUTO_TEST_CASE( noisy_roundtrip ){


  av_register_all();

  std::vector<std::uint8_t> encoded(noisy_embryo_.num_elements(),0);
  std::vector<std::size_t> shape = {static_cast<uint32_t>(noisy_embryo_.shape()[0]),
                 static_cast<uint32_t>(noisy_embryo_.shape()[1]),
                 static_cast<uint32_t>(noisy_embryo_.shape()[2])};

  std::size_t bytes_written = 0;
  sqeazy::hevc_scheme<std::uint8_t> scheme;
  auto encoded_end = scheme.encode(noisy_embryo_.data(),encoded.data(),shape);
  BOOST_REQUIRE(encoded_end!=nullptr);
  bytes_written = encoded_end - encoded.data();
  std::vector<std::size_t> encoded_shape = {bytes_written};

  encoded.resize(bytes_written);

  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements());

  std::vector<std::uint8_t> retrieved(noisy_embryo_.num_elements(),0);
  int err = scheme.decode(encoded.data(),
              retrieved.data(),
              encoded_shape,
              shape);

  BOOST_CHECK_EQUAL(err,0u);

  sqy::uint8_image_stack_cref retrieved_cref(retrieved.data(),shape);
  sqy::uint8_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(retrieved_cref,embryo_cref);
  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm);
  try{

    BOOST_REQUIRE_LT(l2norm,1e-2);

    //TODO: looks like h265 is not bit exaxt for lossless encoding/decoding
    //TODO: check https://bitbucket.org/multicoreware/x265/issues/173/lossless-mode-is-not-bit-exact-and-the
    BOOST_WARN_MESSAGE(std::equal(retrieved.begin(),
                  retrieved.begin()+retrieved.size(),
                  noisy_embryo_.data()),
               "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(noisy_embryo_,"noisy_embryo.tiff");
    sqeazy::write_stack_as_y4m(noisy_embryo_,"noisy_embryo",true);
    sqeazy::write_stack_as_yuv(noisy_embryo_,"noisy_embryo",true);

    stack_t::extent_gen extent;
    retrieved_.resize(extent[noisy_embryo_.shape()[0]][noisy_embryo_.shape()[1]][noisy_embryo_.shape()[2]]);
    std::copy(retrieved.begin(), retrieved.end(),retrieved_.data());
    sqeazy::write_image_stack(retrieved_,"retrieved.tiff");
    throw;
  }


}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( avcodec_16bit, sqeazy::volume_fixture<std::uint16_t> )

BOOST_AUTO_TEST_CASE( lossless_roundtrip_step_ramp ){

  av_register_all();

  std::vector<std::size_t> shape = {static_cast<uint32_t>(embryo_.shape()[sqy::row_major::z]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::y]),
                    static_cast<uint32_t>(embryo_.shape()[sqy::row_major::x])};

  const std::size_t frame_size = shape[sqy::row_major::y]*shape[sqy::row_major::x];

  std::vector<char> encoded(embryo_.num_elements()*sizeof(std::uint16_t),0);

  sqeazy::hevc_scheme<std::uint16_t> scheme;

  for(std::size_t i = 0;i<embryo_.num_elements();++i){
    embryo_.data()[i] = i % frame_size;

  }

  auto encoded_end = scheme.encode(embryo_.data(),
                   encoded.data(),
                   shape);
  std::size_t bytes_written = encoded_end - encoded.data();
  encoded.resize(bytes_written);

  BOOST_CHECK(encoded_end!=nullptr);
  BOOST_CHECK_NE(bytes_written,0u);
  BOOST_CHECK_LT(bytes_written,embryo_.num_elements()*2);


  sqy::uint16_image_stack roundtrip = embryo_;
  int err = scheme.decode(encoded.data(),
              roundtrip.data(),
              shape);

  BOOST_REQUIRE_EQUAL(err,0u);

  sqy::uint16_image_stack_cref roundtripcref(roundtrip.data(),shape);
  sqy::uint16_image_stack_cref embryo_cref(embryo_.data(),shape);
  double l2norm = sqeazy::l2norm(roundtripcref,embryo_cref);
  double l1norm = sqeazy::l1norm(roundtripcref,embryo_cref);

  BOOST_TEST_MESSAGE(boost::unit_test::framework::current_test_case().p_name << "\t l2norm = " << l2norm << ", l1norm = " << l1norm );

  try{
    BOOST_REQUIRE_LT(l2norm,1);
    BOOST_REQUIRE_MESSAGE(std::equal(roundtrip.data(),
                     roundtrip.data()+roundtrip.num_elements(),
                     embryo_.data()),
            "raw noisy volume and encoded/decoded volume do not match exactly, l2norm " << l2norm);


  }
  catch(...){
    sqeazy::write_image_stack(embryo_,"step_ramp_16.tiff");
    // sqeazy::write_stack_as_y4m(embryo_,"step_ramp.y4m");
    sqeazy::write_image_stack(roundtrip,"step_ramp_rt_16.tiff");
    // sqeazy::write_stack_as_y4m(roundtrip,"step_ramp_rt.y4m");


    std::size_t printed = 0;
    for(std::size_t i = 0;i < (10*frame_size) && printed < 64;++i){
      if(embryo_.data()[i]!=roundtrip.data()[i]){
        std::cout << "frame:0 pixel:" << i
          << " ["<< embryo_.shape()[sqy::row_major::x] << "x" << embryo_.shape()[sqy::row_major::y] <<"]"
          << " differ, " << (int)embryo_.data()[i] << " != " << (int)roundtrip.data()[i] << "\n";
    ++printed;
      }
    }
    throw;
  }
  const size_t len = embryo_.num_elements();

  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+10,
                  roundtrip.data(), roundtrip.data()+10);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data()+len-10, embryo_.data()+len,
                  roundtrip.data()+len-10, roundtrip.data()+len);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(embryo_.data(), embryo_.data()+len,
                  roundtrip.data(), roundtrip.data()+len);

}
BOOST_AUTO_TEST_SUITE_END()
