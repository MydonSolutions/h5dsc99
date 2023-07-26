#ifndef H5_DATASPACE_H
#define H5_DATASPACE_H

#include "hdf5/serial/hdf5.h"
#include <string.h>
#include <stdlib.h>

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
  hid_t S_id;
  hid_t C_id; // Chunk_id: the space-ID for the memory
  hid_t D_id;
  hid_t P_id;
  hid_t Tmem_id;
  hid_t Tsto_id;
  char filter_flag; 
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

#endif