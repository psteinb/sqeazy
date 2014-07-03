#define SQEAZY_CPP_
#include "sqeazy_impl.hpp"

/*
*	Sqeazy - Fast and flexible volume compression library
*
* 	Header file
*
*	Note: endianess is little by default.
*/

int SQY_RasterDiffEncode_3D_UI16(int width, int height, int depth, const char* src, char* dst){
  
  typedef unsigned short value_type;

  return sqeazy::diff_scheme<value_type>::encode(width, height, depth, 
						 reinterpret_cast<const value_type*>(src),
						 reinterpret_cast<value_type*>(dst)
						 );
    
}

int SQY_RasterDiffDecode_3D_UI16(int width, int height, int depth, const char* src, char* dst){return 42;}


int SQY_BitSwap4Encode_UI16(const char* src, char* dst, long length){return 42;}

int SQY_BitSwap4Decode_UI16(const char* src, char* dst, long length){return 42;}


int SQY_BitSwap8Encode_UI16(const char* src, char* dst, long length){return 42;}


int SQY_BitSwap8Decode_UI16(const char* src, char* dst, long length){return 42;}


int SQY_RLEEncode_UI8(const char* src, char* dst, long length, long minrunlength){return 42;}


int SQY_RLEDecode_UI8(const char* src, char* dst, long length){return 42;}

int SQY_LZ4Encode(const char* src, long srclength, char* dst, long* dstlength){return 42;}


int SQY_LZ4Length(const char* buffer, long* length){return 42;}


int SQY_LZ4Decode(const char* src, long srclength, char* dst){return 42;}

