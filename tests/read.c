#include <stdlib.h>
#include "hdf5.h"

#include "h5dsc99/h5_dataspace.h"

int main() {
    hid_t file_id = H5Fopen("test.hdf5", H5F_ACC_RDONLY, H5P_DEFAULT);

	// Primitives
    hid_t gprim_id = H5Gopen(file_id, "/1_primitives", H5P_DEFAULT);
	double data_double = H5DSread_double(gprim_id, "double_scalar");
    printf("double_scalar: %f\n", data_double);

	float data_float = H5DSread_float(gprim_id, "float_scalar");
    printf("float_scalar: %f\n", data_float);

	int data_int = H5DSread_int(gprim_id, "int_scalar");
    printf("int_scalar: %d\n", data_int);

	bool data_bool = H5DSread_bool(gprim_id, "bool_scalar");
    printf("bool_scalar: %d\n", data_bool);

    printf("Close: gprim_id\n");
    H5Gclose(gprim_id);

	// Strings
	printf("\n");
    hid_t gstr_id = H5Gopen(file_id, "/2_strings", H5P_DEFAULT);
	char *data_char = H5DSread_all(gstr_id, "literal");
    printf("literal: '%s'\n", data_char);
	free(data_char);

	data_char = H5DSread_all(gstr_id, "char_array");
    printf("char_array: '%s'\n", data_char);
	free(data_char);
    printf("Close: gstr_id\n");
    H5Gclose(gstr_id);

	// Multi-dimensional data
    printf("\n");
    H5_open_dataspace_t h5ds_int3d = {0};
    h5ds_int3d.name = "/3_chunked/integer_3d_array";
    H5DSaccess(file_id, H5P_DEFAULT, &h5ds_int3d);
    printf("%s\n", h5ds_int3d.name);
	h5ds_int3d.dimchunks = calloc(h5ds_int3d.rank, sizeof(hsize_t));
	h5ds_int3d.dimchunks[0] = 1;
	H5DSaccess_set_chunks(&h5ds_int3d);

	//// Attributes
	char *attribute_names[] = {
		"Chunk Dimension Lengths",
		"Dimension Labels"
	};
	for (int a = 0; a < 2; a++) {
		H5_open_dataspace_t h5ds_int3d_attr = {0};
		h5ds_int3d_attr.name = attribute_names[a];
		H5DSaccess(h5ds_int3d.D_id, H5P_DEFAULT, &h5ds_int3d_attr);
	
		if (h5ds_int3d_attr.D_id != H5I_INVALID_HID) {
			printf("Attribute '%s' rank: %d\n", h5ds_int3d_attr.name, h5ds_int3d_attr.rank);
			for (int i = 0; i < h5ds_int3d_attr.rank; i++) {
				printf("\tD%d: %ld\n", i, h5ds_int3d_attr.dims[i]);
			}
			size_t attr_n_elem = H5DSnelem(&h5ds_int3d_attr);
			printf("Nelem: %ld\n",attr_n_elem );
			H5T_class_t cls = H5Tget_class(h5ds_int3d_attr.Tmem_id);

			void *attr_data = H5DSmalloc(&h5ds_int3d_attr);
			H5DSread(&h5ds_int3d_attr, attr_data);
			for (int i = 0; i < attr_n_elem; i++) {
				switch (cls) {
					case H5T_INTEGER:   printf("@%d: %d\n", i, ((int*)attr_data)[i]);		break;
					case H5T_FLOAT:     printf("@%d: %f\n", i, ((float*)attr_data)[i]);		break;
					case H5T_STRING:    printf("@%d: '%s'\n", i, ((char**)attr_data)[i]);	break;
					case H5T_COMPOUND:  printf("@%d: *** (compound)", i);   break;
					case H5T_ARRAY:     printf("@%d: *** (array)", i);      break;
					case H5T_ENUM:      printf("@%d: *** (enum)", i);       break;
					case H5T_VLEN:      printf("@%d: *** (vlen)", i);       break;
					case H5T_BITFIELD:  printf("@%d: *** (bitfield)", i);   break;
					case H5T_OPAQUE:    printf("@%d: *** (opaque)", i);     break;
					case H5T_REFERENCE: printf("@%d: *** (reference)", i);  break;
					default:            printf("@%d: *** (other)", i);      break;
				}
			}
			free(attr_data);
			H5DSclose(&h5ds_int3d_attr);
		}
	}
	printf("\n");

    printf("Integer array rank: %d\n", h5ds_int3d.rank);
    for (int i = 0; i < h5ds_int3d.rank; i++) {
        printf("\tD%d: %ld (chunk len: %ld)\n", i, h5ds_int3d.dims[i], h5ds_int3d.dimchunks[i]);
    }
	size_t n_elem = H5DSnelem(&h5ds_int3d);
    printf("Nelem: %ld\n", n_elem);
	n_elem = H5DSnelem_chunks(&h5ds_int3d);

    int *data = H5DSmalloc(&h5ds_int3d);
	herr_t status = 0;
	int chunk = 0;
	for (
		;
		status == 0 && chunk < h5ds_int3d.dims[0]/h5ds_int3d.dimchunks[0];
		chunk++
	) {
		for (int r = 0; r < h5ds_int3d.rank; r ++) {
			printf("\thyperslab_start[%d]: %ld\n", r, h5ds_int3d.hyperslab_start[r]);
		}
		status = H5DSread(&h5ds_int3d, data);
		printf("\tread status returned: %d\n", status);
		for (int i = 0; i < n_elem; i++) {
			printf("\tC%d, @%d: %d\n", chunk, i, data[i]);
		}
	}
	for (int r = 0; r < h5ds_int3d.rank; r ++) {
		printf("final hyperslab_start[%d]: %ld\n", r, h5ds_int3d.hyperslab_start[r]);
	}

	int ret = 0;
	if (status != 1) {
		printf("H5DSread did not flag hyperslab wrap: returned value %d\n", status);
		ret = 1;
	}
	if (chunk != h5ds_int3d.dims[0]/h5ds_int3d.dimchunks[0]) {
		printf("H5DSread read more chunks than expected: %d != %ld\n", chunk, h5ds_int3d.dims[0]/h5ds_int3d.dimchunks[0]);
		ret = 1;
	}
    
    printf("Close:\n");
    H5DSclose(&h5ds_int3d);
    printf("Free: data\n");
    free(data);
    
    // 4. Close identifiers
    printf("Close: file_id\n");
    H5Fclose(file_id);

    return ret;
}