#ifndef H5_BOOL_H
#define H5_BOOL_H

#include "hdf5/serial/hdf5.h"

typedef enum {
	H5_FALSE,
	H5_TRUE
} H5_bool_t;

#define CPTR(VAR,CONST) ((VAR)=(CONST),&(VAR))
hid_t H5TcreateBool();

#endif