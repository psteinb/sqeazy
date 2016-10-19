#ifndef _COMPASS_PREPROCESSOR_IMPL_H_
#define _COMPASS_PREPROCESSOR_IMPL_H_

#if defined __SSE2__ && defined __SSE2_MATH__
#define COMPASS_HAS_SSE2
#endif

#if defined __SSE3__ && defined __SSSE3__
#define COMPASS_HAS_SSE3
#endif

#if defined __SSE4_2__ && defined __SSE4_1__
#define COMPASS_HAS_SSE4
#endif


#include "../compass_tags.hpp"


namespace compass {

  namespace compiletime {

    template<typename feature_t>
    struct has{
      static const bool enabled = false;
    };

    template<>
    struct has<feature::sse2>{
      static const bool enabled= 
#ifdef COMPASS_HAS_SSE2
	 true;
#else
	 false;
#endif
      
    };

    template<>
    struct has<feature::sse3>{
      static const bool enabled= 
#ifdef COMPASS_HAS_SSE3
	 true;
#else
	 false;
#endif
      
    };

    template<>
    struct has<feature::sse4>{
      static const bool enabled= 
#ifdef COMPASS_HAS_SSE4
	 true;
#else
	 false;
#endif
      
    };

    // template<typename feature_t>
    // static const bool has(){
    //   //unknown feature
    //   return false;
    // }
    
    // template<>
    // static const bool has<feature::sse2>(){
    //   #ifdef COMPASS_HAS_SSE2
    //   return true;
    //   #else
    //   return false;
    //   #endif
    // }

    // template<>
    // static const bool has<feature::sse3>(){
    //   #ifdef COMPASS_HAS_SSE3
    //   return true;
    //   #else
    //   return false;
    //   #endif
    // }

    // template<>
    // static const bool has<feature::sse4>(){
    //   #ifdef COMPASS_HAS_SSE4
    //   return true;
    //   #else
    //   return false;
    //   #endif
    // }

  };
};
  
#endif /* _COMPASS_PREPROCESSOR_IMPL_H_ */