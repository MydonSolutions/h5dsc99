#include "hdf5/serial/hdf5.h"
#include <string.h>
#include <stdlib.h>

#include "h5ds/h5_dataspace.h"

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
	int_array_dsid.name = "integer_3d_array";
	H5DSset(
		3,
		dims3_int_array_limit,
		dims3_int_array_chunk,
		&int_array_dsid
	);
	H5DSopenInt(chunked_gid, &int_array_dsid);

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

	H5Gclose(chunked_gid);
	// ^^ Write a chunked dataset ^^

	H5Fclose(file_id);
	return 0;
}