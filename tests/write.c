#include "hdf5/serial/hdf5.h"
#include <string.h>
#include <stdlib.h>

#include "h5dsc99/h5_dataspace.h"

int main() {

	hid_t file_id = H5Fcreate("test.hdf5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	// vv Write some primitives vv
	hid_t primitives_gid = H5Gcreate(file_id, "/1_primitives", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	H5_bool_t scalar_bool = H5_TRUE;
	H5DSboolWrite(
		primitives_gid,
		"bool_scalar",
		0,
		NULL,
		&scalar_bool
	);

	int scalar_int = 42;
	H5DSintWrite(
		primitives_gid,
		"int_scalar",
		0,
		NULL,
		&scalar_int
	);

	double scalar_double = 42.0;
	H5DSdoubleWrite(
		primitives_gid,
		"double_scalar",
		0,
		NULL,
		&scalar_double
	);

	float scalar_float = 42.0;
	H5DSfloatWrite(
		primitives_gid,
		"float_scalar",
		0,
		NULL,
		&scalar_float
	);

	H5Gclose(primitives_gid);
	// ^^ Write some primitives ^^
	
	// vv Write some strings vv
	hid_t string_gid = H5Gcreate(file_id, "/2_strings", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	H5DSstringWrite(
		string_gid,
		"literal",
		0,
		NULL,
		"literal"
	);

	char str[] = "char array";
	str[4] = '_';

	H5DSstringWrite(
		string_gid,
		"char_array",
		0,
		NULL,
		str
	);

	H5Gclose(string_gid);
	// ^^ Write some strings ^^

	// vv Write a chunked dataset vv
	hid_t chunked_gid = H5Gcreate(file_id, "/3_chunked", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	// the first most dimension (slowest) is extended each time,
	// so to get the matrix writes to build a cube, the
	// matrix chunk is described as in 3D
	const hsize_t dims3_int_array_chunk[] = {1, 3, 2};
	const hsize_t dims3_int_array_limit[] = {H5S_UNLIMITED, 3, 2};

	H5_open_dataspace_t int_array_dsid;
	int array_dimension_count = 3;
	int_array_dsid.name = "integer_3d_array";
	H5DSset(
		array_dimension_count,
		dims3_int_array_limit,
		dims3_int_array_chunk,
		&int_array_dsid
	);
	H5DSopenInt(chunked_gid, &int_array_dsid);

  /*
  * array attribute.
  */
 	const hsize_t adim[] = {3};
	const int chunk_dims3[] = {1, 3, 2};
	hid_t aid1 = H5Screate(H5S_SIMPLE);
  H5Sset_extent_simple(aid1, 1, adim, NULL);
  hid_t attr1 = H5Acreate2(
		int_array_dsid.D_id,
		"Chunk Dimension Lengths",
		H5T_NATIVE_INT,
		aid1,
		H5P_DEFAULT,
		H5P_DEFAULT
	);
  H5Awrite(attr1, H5T_NATIVE_INT, chunk_dims3);

	const char* labels_dims3[] = {
		"Matrix",
		"Row",
		"Column"
	};
	hid_t T_id = H5Tcopy(H5T_C_S1);
	H5Tset_size(T_id, H5S_UNLIMITED);
  hid_t attr1_1 = H5Acreate2(
		int_array_dsid.D_id,
		"Dimension Labels",
		T_id,
		aid1,
		H5P_DEFAULT,
		H5P_DEFAULT
	);
  H5Awrite(attr1_1, T_id, labels_dims3);
  
  /*
  * Create scalar attribute.
  */
  hid_t attr2 = H5Acreate2(
		int_array_dsid.D_id,
		"Number of Dimensions",
		H5T_NATIVE_INT,
		H5Screate(H5S_SCALAR),
		H5P_DEFAULT,
		H5P_DEFAULT
	);
	H5Awrite(attr2, H5T_NATIVE_INT, &array_dimension_count);

	int int_matrix_3x2[] = {
		111, 112,
		121, 122,
		131, 132,
	};

	for (int chunk = 0; chunk < 4; chunk += 1) {
		H5DSextend(&int_array_dsid);
		H5DSwrite(&int_array_dsid, int_matrix_3x2);

		for (int i = 0; i < 6; i += 1) {
			int_matrix_3x2[i] += 100;
		}
	}

	// update chunk size and write more
	int_array_dsid.dimchunks[1] = 1;
	H5DSchunk_update(&int_array_dsid);

	for (int chunk = 0; chunk < 3; chunk += 1) {
		H5DSextend(&int_array_dsid);
		H5DSwrite(&int_array_dsid, int_matrix_3x2);

		for (int i = 0; i < 2; i += 1) {
			int_matrix_3x2[i] += 100;
		}
	}

	H5Gclose(chunked_gid);
	// ^^ Write a chunked dataset ^^

	H5Fclose(file_id);
	return 0;
}