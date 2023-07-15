#include "h5dsc99/h5_bool.h"

hid_t H5TcreateBool() {
	hid_t H5_bool = H5Tcreate(H5T_ENUM, sizeof(H5_bool_t));
	H5_bool_t val;
	H5Tenum_insert(H5_bool, "FALSE", CPTR(val, H5_FALSE));
	H5Tenum_insert(H5_bool, "TRUE" , CPTR(val, H5_TRUE));
	return H5_bool;
}
