#define BOOST_TEST_MODULE TEST_DYNAMIC_STAGE_FACTORY
#include "boost/test/unit_test.hpp"

#include "test_dynamic_pipeline_impl.hpp"
#include "dynamic_stage_factory.hpp"


BOOST_AUTO_TEST_SUITE( factory_test_suite )

BOOST_AUTO_TEST_CASE( factory_finds_valid )
{

  auto local = int_factory_with_one_entry::create<int_filter>("add_one");
  auto result = sqy::const_stage_view(local);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
    
}

BOOST_AUTO_TEST_CASE( factory_finds_valid_from_list )
{

  auto stage = int_factory::create<int_filter>("add_one");
  // std::shared_ptr<int_stage> result = ;
  auto result = sqy::const_stage_view(stage);
  BOOST_CHECK_NE(result->name(),"");
  BOOST_CHECK_EQUAL(result->name(),"add_one");
  BOOST_CHECK(int_factory::has("square"));
  
}


BOOST_AUTO_TEST_CASE( templated_factory_finds_valid )
{

  auto result = filter_factory<int>::create<int_filter>("square");
  BOOST_REQUIRE(result!=nullptr);
  BOOST_CHECK_NE(sqy::const_stage_view(result)->name(),"");
  BOOST_CHECK_EQUAL(sqy::const_stage_view(result)->name(),"square");
    
}


BOOST_AUTO_TEST_CASE( factory_finds_nothing )
{

  BOOST_CHECK(int_factory::create<int_filter>("dope")==nullptr);
    
}

BOOST_AUTO_TEST_SUITE_END()