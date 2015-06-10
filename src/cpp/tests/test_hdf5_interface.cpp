#define BOOST_TEST_MODULE TEST_HDF5_INTERFACE
#include "boost/test/unit_test.hpp"
#include <numeric>
#include <vector>
#include <iostream>
#include <string>

#include "boost/filesystem.hpp"
#include "array_fixtures.hpp"

extern "C" {
#include "sqeazy.h"
}

//taken from 
//http://www.hdfgroup.org/ftp/HDF5/current/src/unpacked/c++/examples/h5tutr_cmprss.cpp
#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

//#include "hdf5_test_utils.hpp"
#include "hdf5_fixtures.hpp"

static const std::string default_filter_name = "bswap1_lz4";

// static sqeazy::loaded_hdf5_plugin always_load;
/*
  as the loaded_hdf5_plugin class is instantiated as a static object, 
  the sqy h5 filter will be registered at the start of any program that 
  includes this (either by code or by linking against the library).
  The filter will be unregistered once the app/lib finishes running
  or being called. 
*/

typedef sqeazy::array_fixture<unsigned short> uint16_cube_of_8;



BOOST_FIXTURE_TEST_SUITE( hdf5_inference_queries, helpers_fixture )

BOOST_AUTO_TEST_CASE( sizeof_dataset ){

  unsigned size_in_byte = 0;
  int rvalue = SQY_h5_query_sizeof(tfile.c_str(), dname.c_str(), &size_in_byte);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(size_in_byte,4);
  
}

BOOST_AUTO_TEST_CASE( sizeof_dataset_wrong_input ){

  
  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned size_in_byte = 0;
  
  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0);

  rvalue = SQY_h5_query_sizeof(tfile.c_str(), wrong_dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0);

  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), wrong_dname.c_str(), &size_in_byte);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(size_in_byte,0);

}

BOOST_AUTO_TEST_CASE( dtype_dataset ){

  unsigned dtype = 0;
  int rvalue = SQY_h5_query_dtype(tfile.c_str(), dname.c_str(), &dtype);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(dtype,2);
  
}

BOOST_AUTO_TEST_CASE( dtype_dataset_wrong_input ){

  
  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned dtype = 0;
  
  rvalue = SQY_h5_query_sizeof(wrong_tfile.c_str(), dname.c_str(), &dtype);
  BOOST_CHECK_EQUAL(rvalue, 1);
  BOOST_CHECK_EQUAL(dtype,0);


}

BOOST_AUTO_TEST_CASE( rank_dataset ){

  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue, 0);
  BOOST_CHECK_EQUAL(ndims,2);
  
}

BOOST_AUTO_TEST_CASE( rank_dataset_wrong_input ){

  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  int rvalue = 0;
  unsigned ndims = 0;
  
  rvalue = SQY_h5_query_ndims(wrong_tfile.c_str(), dname.c_str(), &ndims);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0);

  rvalue = SQY_h5_query_ndims(tfile.c_str(), wrong_dname.c_str(), &ndims);
    
  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0);

  rvalue = SQY_h5_query_ndims(wrong_tfile.c_str(), wrong_dname.c_str(), &ndims);
    
  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(ndims,0);
  
}

BOOST_AUTO_TEST_CASE( shape_dataset ){

  
  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);
  std::vector<unsigned> dims(ndims);
  
  rvalue = SQY_h5_query_shape(tfile.c_str(), dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,0);
  BOOST_CHECK_EQUAL(dims[0],5);
  BOOST_CHECK_EQUAL(dims[1],6);
  
}

BOOST_AUTO_TEST_CASE( shape_dataset_wrong_input ){

  std::string wrong_dname = dname;wrong_dname += "_foo";
  std::string wrong_tfile = tfile;wrong_tfile += "_foo";

  unsigned ndims = 0;
  int rvalue = SQY_h5_query_ndims(tfile.c_str(), dname.c_str(), &ndims);
  std::vector<unsigned> dims(ndims,0);
  
  
  rvalue = SQY_h5_query_shape(wrong_tfile.c_str(), dname.c_str(), &dims[0]);

  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0);

  rvalue = SQY_h5_query_shape(tfile.c_str(), wrong_dname.c_str(), &dims[0]);
    
  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0);

  rvalue = SQY_h5_query_shape(wrong_tfile.c_str(), wrong_dname.c_str(), &dims[0]);
    
  BOOST_CHECK_EQUAL(rvalue,1);
  BOOST_CHECK_EQUAL(dims[0],0);
  
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE( hdf5_file_io, helpers_fixture )

BOOST_AUTO_TEST_CASE( write_no_filter ){

  uint16_cube_of_8 data;
  
  int rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 "");

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0);
  BOOST_REQUIRE_GT(float(bfs::file_size(test_output_path)),float(data.size_in_byte));
    
  bfs::remove(test_output_name);
}

BOOST_AUTO_TEST_CASE( roundtrip_no_filter ){

  uint16_cube_of_8 data;
  
  int rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 "");

  BOOST_REQUIRE_EQUAL(rvalue,0);

  data.to_play_with.clear();
  data.to_play_with.resize(data.constant_cube.size());
  
  rvalue = SQY_h5_read_UI16(test_output_name.c_str(),
			    dname.c_str(),
			    &data.to_play_with[0]);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(), data.to_play_with.end(),
				  data.constant_cube.begin(), data.constant_cube.end());
  bfs::remove(test_output_name);
}

BOOST_AUTO_TEST_CASE( write_filter ){

  uint16_cube_of_8 data;

  bfs::path no_filter_path = "no_filter.h5";
  
  int rvalue = SQY_h5_write_UI16(no_filter_path.string().c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 "");

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(no_filter_path));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path),0);
  
  rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
			     dname.c_str(),
			     &data.constant_cube[0],
			     data.dims.size(),
			     &data.dims[0],
			     default_filter_name.c_str());

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0);
  BOOST_REQUIRE_LT(bfs::file_size(test_output_path),bfs::file_size(no_filter_path));

    
  bfs::remove(test_output_name);
  bfs::remove(no_filter_path);
}

BOOST_AUTO_TEST_CASE( roundtrip_filter ){

  uint16_cube_of_8 data;
  
  int rvalue = SQY_h5_write_UI16(test_output_name.c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 default_filter_name.c_str());

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(test_output_path));
  BOOST_REQUIRE_GT(bfs::file_size(test_output_path),0);

  data.to_play_with.clear();
  data.to_play_with.resize(data.constant_cube.size());
  
  rvalue = SQY_h5_read_UI16(test_output_name.c_str(),
			    dname.c_str(),
			    &data.to_play_with[0]);

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(data.to_play_with.begin(), data.to_play_with.end(),
				  data.constant_cube.begin(), data.constant_cube.end());
  
  
  bfs::remove(test_output_name);

}

BOOST_AUTO_TEST_CASE( write_compressed_data ){

  uint16_cube_of_8 data;

  bfs::path no_filter_path = "no_filter.h5";
  
  int rvalue = SQY_h5_write_UI16(no_filter_path.string().c_str(),
				 dname.c_str(),
				 &data.constant_cube[0],
				 data.dims.size(),
				 &data.dims[0],
				 "ushort_lz4");

  BOOST_REQUIRE_EQUAL(rvalue,0);
  BOOST_REQUIRE(bfs::exists(no_filter_path));
  BOOST_REQUIRE_GT(bfs::file_size(no_filter_path),0);

  std::vector<char> compressed(data.size_in_byte);
  long size = compressed.size();
  
  rvalue = SQY_LZ4Encode((const char*)&data.constant_cube[0],size,&compressed[0],&size);

  
  rvalue = SQY_h5_write(test_output_name.c_str(),
			dname.c_str(),
			&compressed[0],
			size);
  

  BOOST_REQUIRE_EQUAL(rvalue,0);
  
    
  bfs::remove(test_output_name);
  bfs::remove(no_filter_path);
}


BOOST_AUTO_TEST_SUITE_END()
