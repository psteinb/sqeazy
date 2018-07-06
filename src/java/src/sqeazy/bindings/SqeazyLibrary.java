package sqeazy.bindings;
import org.bridj.BridJ;
import org.bridj.CRuntime;
import org.bridj.Pointer;
import org.bridj.ann.Library;
import org.bridj.ann.Ptr;
import org.bridj.ann.Runtime;
/**
 * Wrapper for library <b>sqeazy</b><br>
 * This file was autogenerated by <a href="http://jnaerator.googlecode.com/">JNAerator</a>,<br>
 * a tool written by <a href="http://ochafik.com/">Olivier Chafik</a> that <a href="http://code.google.com/p/jnaerator/wiki/CreditsAndLicense">uses a few opensource projects.</a>.<br>
 * For help, please visit <a href="http://nativelibs4java.googlecode.com/">NativeLibs4Java</a> or <a href="http://bridj.googlecode.com/">BridJ</a> .
 */
@Library("sqeazy") 
@Runtime(CRuntime.class) 
public class SqeazyLibrary {
	static {
		BridJ.register();
	}
	/**
	 * SQY_Header_Size - Detect length of header stored in src.<br>
	 * Search for the sqy header in src and save its length in Byte in lenght<br>
	 * src 					: LZ4 compressed buffer (externally allocated & filled)<br>
	 * length					: length in bytes of header in compressed buffer<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Header_Size(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:22</i>
	 */
	public static int SQY_Header_Size(Pointer<Byte > src, Pointer<org.bridj.CLong > length) {
		return SQY_Header_Size(Pointer.getPeer(src), Pointer.getPeer(length));
	}
	protected native static int SQY_Header_Size(@Ptr long src, @Ptr long length);
	/**
	 * SQY_Decompressed_NDims - Detect the number of dimensions contained in blob<br>
	 * Search for the sqy header in src and save its length in Byte in lenght<br>
	 * src 					: LZ4 compressed buffer (externally allocated & filled)<br>
	 * num					: (in) size of src in bytes<br>
	 * (out) scalar that tells the number of<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Decompressed_NDims(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:37</i>
	 */
	public static int SQY_Decompressed_NDims(Pointer<Byte > src, Pointer<org.bridj.CLong > num) {
		return SQY_Decompressed_NDims(Pointer.getPeer(src), Pointer.getPeer(num));
	}
	protected native static int SQY_Decompressed_NDims(@Ptr long src, @Ptr long num);
	/**
	 * SQY_Decompressed_Shape - Detect length of header stored in src.<br>
	 * Search for the sqy header in src and save its length in Byte in lenght<br>
	 * src 					: LZ4 compressed buffer (externally allocated & filled)<br>
	 * shape					: (in) array of longs where first item describes the size of src<br>
	 * (array must be allocated to number of dimensions prior to this call)<br>
	 * (out) array holding the shape of the decoded volume in units of pixel/voxel<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Decompressed_Shape(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:52</i>
	 */
	public static int SQY_Decompressed_Shape(Pointer<Byte > src, Pointer<org.bridj.CLong > shape) {
		return SQY_Decompressed_Shape(Pointer.getPeer(src), Pointer.getPeer(shape));
	}
	protected native static int SQY_Decompressed_Shape(@Ptr long src, @Ptr long shape);
	/**
	 * SQY_Decompressed_Sizeof - Pixel size in bytes<br>
	 * Search for the sqy header in src and save its length in Byte in lenght<br>
	 * src 					: LZ4 compressed buffer (externally allocated & filled)<br>
	 * shape					: (in) pointer to long which describes the length of src<br>
	 * (out) array holding the shape of the decoded volume in units of pixel/voxel<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Decompressed_Sizeof(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:66</i>
	 */
	public static int SQY_Decompressed_Sizeof(Pointer<Byte > src, Pointer<org.bridj.CLong > shape) {
		return SQY_Decompressed_Sizeof(Pointer.getPeer(src), Pointer.getPeer(shape));
	}
	protected native static int SQY_Decompressed_Sizeof(@Ptr long src, @Ptr long shape);
	/**
	 * SQY_Version_Triple - store version.<br>
	 * Just store the version of sqeazy used. (NB: gives dummy values right now)<br>
	 * version					: 3 element int array that holds the sqeazy version<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Version_Triple(int*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:79</i>
	 */
	public static int SQY_Version_Triple(Pointer<Integer > version) {
		return SQY_Version_Triple(Pointer.getPeer(version));
	}
	protected native static int SQY_Version_Triple(@Ptr long version);
	/**
	 * SQY_PipelineEncode - Compress using Pipeline.<br>
	 * Compresses the source buffer into the destination buffer using Pipeline.<br>
	 * The destination buffer should be a bit larger than the source buffer<br>
	 * in case the compressed buffer requires more space. <br>
	 * ATTENTION: The output buffer contains a sqy only header!<br>
	 * pipeline				: pipeline name<br>
	 * src 					: contiguous array of voxels (externally allocated)<br>
	 * shape     				: shape of the nD construct given as src (in units of unsigned int 16-bit)<br>
	 * shape_size     				: number of items in shape<br>
	 * dst 					: Pipeline compressed buffer (already allocated)<br>
	 * dstlength 				: length in bytes of externally allocated destination buffer (needs to give the length of dst in Bytes),<br>
	 * modified by function call to reflect the effective <br>
	 * compressed buffer length after the call.<br>
	 * nthreads                : set the number of threads allowed for the entire pipeline.<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * error 1 -  destination buffer is not large enough<br>
	 * Original signature : <code>int SQY_PipelineEncode_UI16(const char*, const char*, long*, unsigned, char*, long*, int)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:107</i>
	 */
	public static int SQY_PipelineEncode_UI16(Pointer<Byte > pipeline, Pointer<Byte > src, Pointer<org.bridj.CLong > shape, int shape_size, Pointer<Byte > dst, Pointer<org.bridj.CLong > dstlength, int nthreads) {
		return SQY_PipelineEncode_UI16(Pointer.getPeer(pipeline), Pointer.getPeer(src), Pointer.getPeer(shape), shape_size, Pointer.getPeer(dst), Pointer.getPeer(dstlength), nthreads);
	}
	protected native static int SQY_PipelineEncode_UI16(@Ptr long pipeline, @Ptr long src, @Ptr long shape, int shape_size, @Ptr long dst, @Ptr long dstlength, int nthreads);
	/**
	 * SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression<br>
	 * pipeline				: pipeline name<br>
	 * length 					: (in) length in bytes of decompressed buffer<br>
	 * (out) maximum length in bytes of compressed buffer<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Pipeline_Max_Compressed_Length_UI16(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:126</i>
	 */
	public static int SQY_Pipeline_Max_Compressed_Length_UI16(Pointer<Byte > pipeline, Pointer<org.bridj.CLong > length) {
		return SQY_Pipeline_Max_Compressed_Length_UI16(Pointer.getPeer(pipeline), Pointer.getPeer(length));
	}
	protected native static int SQY_Pipeline_Max_Compressed_Length_UI16(@Ptr long pipeline, @Ptr long length);
	/**
	 * SQY_Pipeline_Max_Compressed_Length - Calculates the maximum size of the output buffer from Pipeline compression<br>
	 * pipeline				: pipeline name<br>
	 * shape					: (in) shape of the incoming nD dataset<br>
	 * shape_size				: (in) number of items in shape<br>
	 * length 					: (out) maximum length in bytes of compressed buffer<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Pipeline_Max_Compressed_Length_3D_UI16(const char*, long*, unsigned, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:138</i>
	 */
	public static int SQY_Pipeline_Max_Compressed_Length_3D_UI16(Pointer<Byte > pipeline, Pointer<org.bridj.CLong > shape, int shape_size, Pointer<org.bridj.CLong > length) {
		return SQY_Pipeline_Max_Compressed_Length_3D_UI16(Pointer.getPeer(pipeline), Pointer.getPeer(shape), shape_size, Pointer.getPeer(length));
	}
	protected native static int SQY_Pipeline_Max_Compressed_Length_3D_UI16(@Ptr long pipeline, @Ptr long shape, int shape_size, @Ptr long length);
	/**
	 * SQY_Decompressed_Length - Extracts the size of the decompressed buffer from the first 8 Byte of data<br>
	 * data					: buffer that contains the compressed data as output by SQY_PipelineEncode<br>
	 * length					: (in)  length in bytes of incoming data buffer<br>
	 * (out) extracted length in bytes of decompressed buffer to be output by SQY_PipelineDecode called on data<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_Decompressed_Length(const char*, long*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:150</i>
	 */
	public static int SQY_Decompressed_Length(Pointer<Byte > data, Pointer<org.bridj.CLong > length) {
		return SQY_Decompressed_Length(Pointer.getPeer(data), Pointer.getPeer(length));
	}
	protected native static int SQY_Decompressed_Length(@Ptr long data, @Ptr long length);
	/**
	 * SQY_PipelineDecode_UI16 - Decompress using Pipeline.<br>
	 * Decompresses the source buffer into the destination buffer using Pipeline.<br>
	 * The destination buffer should be allocated externally. The first 8 bytes of <br>
	 * the source buffer are not part of the Pipeline compressed buffer but indicate the<br>
	 * length of the original uncompressed buffer.<br>
	 * The necessary buffer length can be obtained by calling SQY_PipelineLength. <br>
	 * pipeline				: pipeline name<br>
	 * src 					: Pipeline compressed buffer (externally allocated)<br>
	 * srclength 				: length in bytes of compressed buffer<br>
	 * dst 					: contiguous array of voxels <br>
	 * (externally allocated, length from SQY_Pipeline_Decompressed_Length)<br>
	 * nthreads                : set the number of threads allowed for the entire pipeline.<br>
	 * Returns 0 if success, another code if there was an error (error codes provided below)<br>
	 * Original signature : <code>int SQY_PipelineDecode_UI16(const char*, long, char*, int)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:171</i>
	 */
	public static int SQY_PipelineDecode_UI16(Pointer<Byte > src, @org.bridj.ann.CLong long srclength, Pointer<Byte > dst, int nthreads) {
		return SQY_PipelineDecode_UI16(Pointer.getPeer(src), srclength, Pointer.getPeer(dst), nthreads);
	}
	protected native static int SQY_PipelineDecode_UI16(@Ptr long src, @org.bridj.ann.CLong long srclength, @Ptr long dst, int nthreads);
	/**
	 * SQY_Pipeline_Possible - check if pipeline string can be used to build pipeline from<br>
	 * By default 16-bit input data is assumed.<br>
	 * pipeline_string				: string that describes the pipeline ('->' delimited)<br>
	 * Returns true if success, false if not!<br>
	 * Original signature : <code>bool SQY_Pipeline_Possible(const char*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:186</i>
	 */
	public static boolean SQY_Pipeline_Possible(Pointer<Byte > pipeline_string) {
		return SQY_Pipeline_Possible(Pointer.getPeer(pipeline_string));
	}
	protected native static boolean SQY_Pipeline_Possible(@Ptr long pipeline_string);
	/**
	 * SQY_h5_query_sizeof - query the size of the datatype stored in an hdf5 file (in byte)<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * _sizeof					: number of bytes the stored data type is wide <br>
	 * (filled with 0 if dataset is not found)<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_query_sizeof(const char*, const char*, unsigned*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:216</i>
	 */
	public static int SQY_h5_query_sizeof(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Integer > _sizeof) {
		return SQY_h5_query_sizeof(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(_sizeof));
	}
	protected native static int SQY_h5_query_sizeof(@Ptr long fname, @Ptr long dname, @Ptr long _sizeof);
	/**
	 * SQY_h5_query_dtype - query the type of the data stored in an hdf5 file (floating point, signed integer or unsigned integer)<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * dtype					: constant that notes if datatype found is floating point, signed integer or unsigned integer<br>
	 * dtype = 0			: floating point<br>
	 * dtype = 1			: signed integer<br>
	 * dtype = 2			: unsigned integer<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_query_dtype(const char*, const char*, unsigned*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:233</i>
	 */
	public static int SQY_h5_query_dtype(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Integer > dtype) {
		return SQY_h5_query_dtype(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(dtype));
	}
	protected native static int SQY_h5_query_dtype(@Ptr long fname, @Ptr long dname, @Ptr long dtype);
	/**
	 * SQY_h5_query_ndims - query the rank of the data stored in an hdf5 file (1D, 2D, 3D, ...)<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * dtype					: rank of the stored data<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_query_ndims(const char*, const char*, unsigned*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:247</i>
	 */
	public static int SQY_h5_query_ndims(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Integer > rank) {
		return SQY_h5_query_ndims(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(rank));
	}
	protected native static int SQY_h5_query_ndims(@Ptr long fname, @Ptr long dname, @Ptr long rank);
	/**
	 * SQY_h5_query_shape - query the shape of the data stored in an hdf5 file<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * shape					: shape of the stored data (in row-wise ordering a la C), externally allocated<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_query_shape(const char*, const char*, unsigned*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:261</i>
	 */
	public static int SQY_h5_query_shape(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Integer > shape) {
		return SQY_h5_query_shape(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(shape));
	}
	protected native static int SQY_h5_query_shape(@Ptr long fname, @Ptr long dname, @Ptr long shape);
	/**
	 * SQY_h5_read_UI16 - load contents of hdf5 file into data (if the dataset on disk is found to be compressed by sqeazy, it is uncompressed)<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * data					: data buffer (externally allocated)<br>
	 * TODO: add multi-threading support for the pipeline only<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_read_UI16(const char*, const char*, unsigned short*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:276</i>
	 */
	public static int SQY_h5_read_UI16(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Short > data) {
		return SQY_h5_read_UI16(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(data));
	}
	protected native static int SQY_h5_read_UI16(@Ptr long fname, @Ptr long dname, @Ptr long data);
	/**
	 * SQY_h5_write_UI16 - store unsigned 16-bit int buffer in hdf5 file (no compression is applied).<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * data					: unsigned 16-bit data to compress and store<br>
	 * shape_size				: number of dimensions in data<br>
	 * shape					: dimension of data<br>
	 * filter					: filter to use<br>
	 * TODO: add multi-threading support for the pipeline only<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_write_UI16(const char*, const char*, const unsigned short*, unsigned, const unsigned*, const char*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:294</i>
	 */
	public static int SQY_h5_write_UI16(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Short > data, int shape_size, Pointer<Integer > shape, Pointer<Byte > filter) {
		return SQY_h5_write_UI16(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(data), shape_size, Pointer.getPeer(shape), Pointer.getPeer(filter));
	}
	protected native static int SQY_h5_write_UI16(@Ptr long fname, @Ptr long dname, @Ptr long data, int shape_size, @Ptr long shape, @Ptr long filter);
	/**
	 * SQY_h5_write - store compressed data into file.<br>
	 * fname 					: hdf5 file to store data in<br>
	 * dname 					: dataset name inside hdf5 file <br>
	 * data					: compressed data<br>
	 * data_size				: size of data in byte<br>
	 * TODO: add multi-threading support for the pipeline only<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_write(const char*, const char*, const char*, unsigned long)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:314</i>
	 */
	public static int SQY_h5_write(Pointer<Byte > fname, Pointer<Byte > dname, Pointer<Byte > data, @org.bridj.ann.CLong long data_size) {
		return SQY_h5_write(Pointer.getPeer(fname), Pointer.getPeer(dname), Pointer.getPeer(data), data_size);
	}
	protected native static int SQY_h5_write(@Ptr long fname, @Ptr long dname, @Ptr long data, @org.bridj.ann.CLong long data_size);
	/**
	 * SQY_h5_link - establish hdf5 link between src.h5:/path/to/object/by_name and dest.h5:/path/to/object/by_name<br>
	 * pSrcFileName				: path to file the link is stored in<br>
	 * pSrcLinkPath				: path/group inside pSrcFileName <br>
	 * pSrcLinkName				: name of link inside pSrcFileName <br>
	 * pDestFileName				: path to file the dataset the link points to<br>
	 * pDestDatasetPath			: path/group inside pDestFileName where dataset is located <br>
	 * pDestDatasetName			: name of dataset inside pDestFileName<br>
	 * Returns 0 if success, another code if there was an error<br>
	 * Original signature : <code>int SQY_h5_link(const char*, const char*, const char*, const char*, const char*, const char*)</code><br>
	 * <i>native declaration : src/cpp/inc/sqeazy.h:333</i>
	 */
	public static int SQY_h5_link(Pointer<Byte > pSrcFileName, Pointer<Byte > pSrcLinkPath, Pointer<Byte > pSrcLinkName, Pointer<Byte > pTargetFile, Pointer<Byte > pTargetDatasetPath, Pointer<Byte > pTargetDatasetName) {
		return SQY_h5_link(Pointer.getPeer(pSrcFileName), Pointer.getPeer(pSrcLinkPath), Pointer.getPeer(pSrcLinkName), Pointer.getPeer(pTargetFile), Pointer.getPeer(pTargetDatasetPath), Pointer.getPeer(pTargetDatasetName));
	}
	protected native static int SQY_h5_link(@Ptr long pSrcFileName, @Ptr long pSrcLinkPath, @Ptr long pSrcLinkName, @Ptr long pTargetFile, @Ptr long pTargetDatasetPath, @Ptr long pTargetDatasetName);
}
