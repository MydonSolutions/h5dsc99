#ifndef H5_DATASPACE_H
#define H5_DATASPACE_H

#include <string.h>
#include <stdlib.h>
#include "hdf5.h"

#include "h5_bool.h"

enum H5_FILTER_FLAG {
  H5_FILTER_FLAG_NONE = -1,
  H5_FILTER_FLAG_DEFLATE_0 = 0,
  H5_FILTER_FLAG_DEFLATE_1 = 1,
  H5_FILTER_FLAG_DEFLATE_2 = 2,
  H5_FILTER_FLAG_DEFLATE_3 = 3,
  H5_FILTER_FLAG_DEFLATE_4 = 4,
  H5_FILTER_FLAG_DEFLATE_5 = 5,
  H5_FILTER_FLAG_DEFLATE_6 = 6,
  H5_FILTER_FLAG_DEFLATE_7 = 7,
  H5_FILTER_FLAG_DEFLATE_8 = 8,
  H5_FILTER_FLAG_DEFLATE_9 = 9,
  H5_FILTER_FLAG_SHUFFLE = 10
};

typedef struct {
  int rank;
  hsize_t* dims;
  hsize_t* dimlims;
  hsize_t* dimchunks;
  char* name;
  hid_t S_id; // Space ID
  hid_t C_id; // Chunk ID: memory
  hid_t D_id; // Dataset ID
  hid_t P_id; // Access Properties ID
  hid_t Tmem_id;
  hid_t Tsto_id;
  char filter_flag;
  hsize_t* hyperslab_start;
} H5_open_dataspace_t;

void H5DSopen(
  hid_t dest_id, hid_t Tmem_id, hid_t Tsto_id, H5_open_dataspace_t* dataspace
);

herr_t H5DSchunk_update(H5_open_dataspace_t* dataspace);

herr_t H5DSclose(H5_open_dataspace_t* dataspace);

herr_t H5DSfree(H5_open_dataspace_t* dataspace);

size_t H5DSsize(H5_open_dataspace_t* dataspace);

void* H5DSmalloc(H5_open_dataspace_t* dataspace);

void H5DSset(
  int rank,
  const hsize_t* dimlims,
  const hsize_t* chunks,
  H5_open_dataspace_t* dataspace
);

herr_t H5DSwrite(H5_open_dataspace_t* dataspace, const void* data);

herr_t H5DSextend(H5_open_dataspace_t* dataspace);

static inline herr_t H5DSextend_write(H5_open_dataspace_t* dataspace, const void* data) {
  herr_t status = H5DSextend(dataspace);
  if (status == 0) {
    status += H5DSwrite(dataspace, data);
  }
  return status;
}

void H5DSopenBool(
  hid_t dest_id, H5_open_dataspace_t* dataspace
);

void H5DSopenDouble(
  hid_t dest_id, H5_open_dataspace_t* dataspace
);

void H5DSopenFloat(
  hid_t dest_id, H5_open_dataspace_t* dataspace
);

void H5DSopenInt(
  hid_t dest_id, H5_open_dataspace_t* dataspace
);

size_t H5DSnelem(H5_open_dataspace_t* dataspace);

size_t H5DSnelem_chunks(H5_open_dataspace_t* dataspace);

size_t H5DSnelem_lims(H5_open_dataspace_t* dataspace);


herr_t H5DSboolWrite(
  hid_t dest_id,
  char *d_name,
  const int rank,
  const hsize_t *dims,
  const H5_bool_t *data
);

herr_t H5DSdoubleWrite(
  hid_t dest_id,
  char *d_name,
  const int rank,
  const hsize_t *dims,
  const double *data
);

herr_t H5DSfloatWrite(
  hid_t dest_id,
  char *d_name,
  const int rank,
  const hsize_t *dims,
  const float *data
);

herr_t H5DSintWrite(
  hid_t dest_id,
  char *d_name,
  const int rank,
  const hsize_t *dims,
  const int *data
);

herr_t H5DSstringWrite(
  hid_t dest_id,
  char *d_name,
  const int rank,
  const hsize_t *dims,
  const char *data
);

void H5DSaccess(
  hid_t src_id,
  hid_t P_id,
  H5_open_dataspace_t *dataspace
);

void H5DSaccess_set_chunks(H5_open_dataspace_t *dataspace);

herr_t H5DSread(
  H5_open_dataspace_t *dataspace,
  void *data
);

void* H5DSread_all(
  hid_t src_id,
  char *d_name
);

#endif