#define BOOST_TEST_MODULE TEST_YUV_UTILS
#include "boost/test/unit_test.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "yuv_utils.hpp"
#include "image_stack.hpp"
#include "volume_fixtures.hpp"


struct yuv_fixture {
  
  const std::string     example_yuv = "mini_example_w92_h128.yuv";
  const std::string     example_y4m = "mini_example.y4m";
  const std::vector<uint32_t> shape = {51,128,92};
  
};

BOOST_FIXTURE_TEST_SUITE( file_inspection , yuv_fixture )

BOOST_AUTO_TEST_CASE( is_y4m )
{
  std::string wrong_path = example_y4m;
  wrong_path += "_foo";

  BOOST_CHECK(sqeazy::is_y4m_file(example_y4m));
  BOOST_CHECK(sqeazy::is_y4m_file(wrong_path)==false);
  
}

BOOST_AUTO_TEST_CASE( is_yuv )
{
  std::string wrong_path = example_yuv;
  wrong_path += "_foo";

  BOOST_CHECK(sqeazy::is_yuv_file(example_yuv));
  BOOST_CHECK(sqeazy::is_yuv_file(wrong_path)==false);
  
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( file_io , yuv_fixture )

BOOST_AUTO_TEST_CASE( y4m_right_shape )
{

  BOOST_CHECK(sqeazy::is_y4m_file(example_y4m));

  std::vector<uint8_t> payload;
  std::vector<uint32_t> dims = sqeazy::read_y4m_to_gray(payload, example_y4m);

  BOOST_CHECK_EQUAL_COLLECTIONS(shape.begin(), shape.end(),dims.begin(), dims.end());
  
}

BOOST_AUTO_TEST_CASE( yuv_right_shape )
{

  BOOST_CHECK(sqeazy::is_yuv_file(example_yuv));

  std::vector<uint8_t> payload;
  std::vector<uint32_t> dims = sqeazy::read_yuv_to_gray8(payload, example_yuv);

  BOOST_CHECK_EQUAL_COLLECTIONS(shape.begin(), shape.end(),dims.begin(), dims.end());
  
}

BOOST_AUTO_TEST_CASE( y4m_roundtrip )
{

  const std::string path_stem = "ramp_roundtrip";
  const std::string path = "ramp_roundtrip.y4m";
  
  const size_t n_elemens = std::accumulate(shape.begin(), shape.end(),1,std::multiplies<size_t>());
  
  std::vector<uint8_t> payload(n_elemens,0);
  for(size_t i = 0;i<n_elemens;++i)
    payload[i] = i % 256;

  std::random_device rd;
  std::mt19937 g(rd());
    
  std::shuffle(payload.begin(), payload.end(), g);
  
  sqeazy::uint8_image_stack_cref stack(&payload[0],shape);
  sqeazy::write_stack_as_y4m(stack, path_stem);

  BOOST_REQUIRE(bfs::exists(path));
  BOOST_REQUIRE(bfs::file_size(path)>0);
  
  std::vector<uint8_t> reread;  
  std::vector<uint32_t> dims = sqeazy::read_y4m_to_gray(reread, path);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(shape.begin(), shape.end(),dims.begin(), dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(payload.begin(), payload.end(),reread.begin(), reread.end());

  bfs::remove(path);
}

BOOST_AUTO_TEST_CASE( yuv_roundtrip )
{

  const std::string path_stem = "ramp_roundtrip";
  const std::string path = "ramp_roundtrip.yuv";
  const std::string expected_path = "ramp_roundtrip_w92_h128_f51_yuv420p.yuv";
  
  const size_t n_elemens = std::accumulate(shape.begin(), shape.end(),1,std::multiplies<size_t>());
  
  std::vector<uint8_t> payload(n_elemens,0);
  for(size_t i = 0;i<n_elemens;++i)
    payload[i] = i % 256;

  std::random_device rd;
  std::mt19937 g(rd());
    
  std::shuffle(payload.begin(), payload.end(), g);

  
  sqeazy::uint8_image_stack_cref stack(&payload[0],shape);
  sqeazy::write_stack_as_yuv(stack, path_stem);

  BOOST_REQUIRE(bfs::exists(expected_path));
  BOOST_REQUIRE(bfs::file_size(expected_path)>0);
  
  std::vector<uint8_t> reread;  
  std::vector<uint32_t> dims = sqeazy::read_yuv_to_gray8(reread, expected_path);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(shape.begin(), shape.end(),dims.begin(), dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(payload.begin(), payload.end(),reread.begin(), reread.end());

  bfs::remove(expected_path);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE( yuv4xx_on_16_bit , sqeazy::volume_fixture<uint16_t> )

BOOST_AUTO_TEST_CASE( y4m_roundtrip )
{

  const std::string path_stem = "noisy_embry_roundtrip";
  std::string path = path_stem;path+=".y4m";
  
  const size_t n_elemens = std::accumulate(shape_.begin(), shape_.end(),1,std::multiplies<size_t>());
  

  sqeazy::write_stack_as_y4m(noisy_embryo_, path_stem,false,sqeazy::yuv444formatter());

  BOOST_REQUIRE(bfs::exists(path));
  BOOST_REQUIRE(bfs::file_size(path)>0);
  
  std::vector<uint16_t> reread;  
  std::vector<uint32_t> dims = sqeazy::read_y4m_to_gray(reread, path);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(shape_.begin(), shape_.end(),dims.begin(), dims.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS(noisy_embryo_.data(), noisy_embryo_.data()+128,reread.begin(), reread.begin()+128);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(noisy_embryo_.data(), noisy_embryo_.data()+n_elemens,reread.begin(), reread.end());

  bfs::remove(path);
}

BOOST_AUTO_TEST_SUITE_END()
