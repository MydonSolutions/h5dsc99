#include "h5dsc99/h5_dataspace.h"

#ifndef NDEBUG
#include <errno.h>
#endif

void _H5DSprint_debug(const char *name, const char *msg, ...) {
#ifndef NDEBUG
	fprintf(stderr, "Debug H5DS (%s)", name);
	if(msg) {
		va_list ap;
		va_start(ap, msg);
		fprintf(stderr, ": ");
		vfprintf(stderr, msg, ap);
		va_end(ap);
	}
	if(errno) {
		fprintf(stderr, " [%s]", strerror(errno));
	}
	fprintf(stderr, "\n");
	fflush(stderr);
#endif // DEBUG
}

void H5DSopen(
	hid_t dest_id, hid_t Tmem_id, hid_t Tsto_id, H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);

	dataspace->P_id = 0;
	dataspace->Tmem_id = H5T_NO_CLASS;
	dataspace->Tsto_id = H5T_NO_CLASS;
	dataspace->D_id = 0;
	dataspace->S_id = 0;
	dataspace->C_id = 0;

	if(Tmem_id != H5T_NO_CLASS)
		dataspace->Tmem_id = Tmem_id;
	if(Tsto_id != H5T_NO_CLASS)
		dataspace->Tsto_id = Tsto_id;

	if(!dataspace->P_id) {
		dataspace->P_id = H5Pcreate(H5P_DATASET_CREATE);
	}

	if(dataspace->rank > 0) {
		dataspace->S_id = H5Screate_simple(dataspace->rank, dataspace->dims, dataspace->dimlims);
	}
	else {
		dataspace->S_id = H5Screate(H5S_SCALAR);
		H5Pset_layout(dataspace->P_id, H5D_COMPACT);
	}

	if(dataspace->rank > 0 && dataspace->dimchunks != NULL) {
		H5Pset_chunk(dataspace->P_id, dataspace->rank, dataspace->dimchunks);
		dataspace->C_id = H5Screate_simple(dataspace->rank, dataspace->dimchunks, NULL);
		// filters can only be used with chunked data
		if(dataspace->filter_flag >= H5_FILTER_FLAG_NONE){
			if(dataspace->filter_flag <= H5_FILTER_FLAG_DEFLATE_9){
				H5Pset_deflate(dataspace->P_id, dataspace->filter_flag);
			}
			if(dataspace->filter_flag == H5_FILTER_FLAG_SHUFFLE){
				H5Pset_shuffle(dataspace->P_id);
			}
		}
	}
	dataspace->D_id = H5Dcreate(dest_id, dataspace->name, dataspace->Tmem_id, dataspace->S_id, H5P_DEFAULT, dataspace->P_id, H5P_DEFAULT);
}

herr_t H5DSchunk_update(H5_open_dataspace_t* dataspace) {
	herr_t status = 0;
	status += H5Sclose(dataspace->C_id);
	status += H5Pset_chunk(dataspace->P_id, dataspace->rank, dataspace->dimchunks);
	dataspace->C_id = H5Screate_simple(dataspace->rank, dataspace->dimchunks, NULL);	
	return status;
}

herr_t H5DSclose(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	herr_t status = 0;
	if (dataspace->P_id) {
		_H5DSprint_debug(__FUNCTION__, "\tP_id");
		status += H5Pclose(dataspace->P_id);
	}
	if (dataspace->Tmem_id != H5T_NO_CLASS) {
		_H5DSprint_debug(__FUNCTION__, "\tTmem_id");
		status += H5Tclose(dataspace->Tmem_id);
	}
	if (dataspace->Tsto_id != H5T_NO_CLASS) {
		_H5DSprint_debug(__FUNCTION__, "\tTsto_id");
		status += H5Tclose(dataspace->Tsto_id);
	}
	if (dataspace->C_id) {
		_H5DSprint_debug(__FUNCTION__, "\tC_id");
		status += H5Sclose(dataspace->C_id);
	}
	if (dataspace->S_id) {
		_H5DSprint_debug(__FUNCTION__, "\tS_id");
		status += H5Sclose(dataspace->S_id);
	}
	if (dataspace->D_id) {
		_H5DSprint_debug(__FUNCTION__, "\tD_id");
		
		if (H5Iget_type(dataspace->D_id) == H5I_ATTR) {
			status += H5Aclose(dataspace->D_id);
		}
		else {
			status += H5Dclose(dataspace->D_id);
		}
	}
	return status + H5DSfree(dataspace);
}

herr_t H5DSfree(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if (dataspace->dims != NULL) {
		_H5DSprint_debug(__FUNCTION__, "\tdims");
		free(dataspace->dims);
	}
	if (dataspace->dimlims != NULL) {
		_H5DSprint_debug(__FUNCTION__, "\tdimlims");
		free(dataspace->dimlims);
	}
	if (dataspace->dimchunks != NULL) {
		_H5DSprint_debug(__FUNCTION__, "\tdimchunks");
		free(dataspace->dimchunks);
	}
	if (dataspace->hyperslab_start != NULL) {
		_H5DSprint_debug(__FUNCTION__, "\thyperslab_start");
		free(dataspace->hyperslab_start);
	}
	return 0;
}

size_t H5DSsize(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	size_t element_byte_size = H5Tget_size(dataspace->Tmem_id);
	size_t nelem = H5DSnelem_chunks(dataspace);
	if (nelem == 0) {
		nelem = H5DSnelem_lims(dataspace);
		nelem = nelem > 0 ? nelem : H5DSnelem(dataspace);
	}
	return nelem*element_byte_size;
}

void* H5DSmalloc(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	size_t bytesize = H5DSsize(dataspace);
	_H5DSprint_debug(__FUNCTION__, "'%s' allocated %ld bytes.", dataspace->name, bytesize);
	return malloc(bytesize);
}

void H5DSset(
	int rank,
	const hsize_t* dimlims,
	const hsize_t* chunks,
	H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	
	dataspace->rank = rank;
	dataspace->dims = NULL;
	dataspace->dimlims = NULL;
	dataspace->dimchunks = NULL;
	dataspace->hyperslab_start = NULL;
	if(rank > 0) {
		dataspace->dims = malloc(rank * sizeof(hsize_t));
		dataspace->dimlims = malloc(rank * sizeof(hsize_t));
		dataspace->hyperslab_start = malloc(rank * sizeof(hsize_t));
		
		if(chunks != NULL) {
			dataspace->dimchunks = malloc(rank * sizeof(hsize_t));
		}

		for (size_t i = 0; i < rank; i++)
		{
			dataspace->dimlims[i] = dimlims[i];
			dataspace->dims[i] = dimlims[i] == H5S_UNLIMITED ? chunks[i] : dimlims[i];
			dataspace->hyperslab_start[i] = 0;
			_H5DSprint_debug(__FUNCTION__, "\tdim %ld (%llu/%llu)", i, dataspace->hyperslab_start[i], dataspace->dimlims[i]);
			if(chunks != NULL) {
				// TODO assert chunk[i] < dimlims[i]
				dataspace->dimchunks[i] = chunks[i];
				_H5DSprint_debug(__FUNCTION__, "\t\tchunked %llu", dataspace->dimchunks[i]);
			}
		}
	}
}

herr_t H5DSwrite(H5_open_dataspace_t* dataspace, const void* data) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if(dataspace->dimchunks != NULL){
 		herr_t ret = H5Dwrite(dataspace->D_id, dataspace->Tsto_id, dataspace->C_id, dataspace->S_id, H5P_DEFAULT, data);
		for (size_t i = dataspace->rank; i-- > 0; )
		{
			dataspace->hyperslab_start[i] += dataspace->dimchunks[i];
			if(dataspace->dimlims[i] != H5S_UNLIMITED && dataspace->hyperslab_start[i] < dataspace->dimlims[i]) {
				break;
			}
		}
		return ret;
	}
	else {
 		return H5Dwrite(dataspace->D_id, dataspace->Tsto_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	}
}

herr_t H5DSextend(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	// for selecting extension of space
	for (size_t i = dataspace->rank; i-- > 0; )
	{
		// TODO assert at least one unlimited dimension
		if(dataspace->hyperslab_start[i] == dataspace->dimlims[i]) {
			dataspace->hyperslab_start[i] = 0;
		}
		else if (dataspace->dimlims[i] == H5S_UNLIMITED && dataspace->hyperslab_start[i] == dataspace->dims[i]) {
			// unlimited dimension needs an extension
			dataspace->dims[i] += dataspace->dimchunks[i];
		}
		else {
			// the next hyperslab doesn't require an extension
			break;
		}
	}
	
	herr_t status = H5Dset_extent(dataspace->D_id, dataspace->dims);
	status += H5Sclose(dataspace->S_id); // this
	dataspace->S_id = H5Dget_space(dataspace->D_id); // this

	// select extension of space
	status += H5Sselect_hyperslab(dataspace->S_id, H5S_SELECT_SET, dataspace->hyperslab_start, NULL, dataspace->dimchunks, NULL);

	return status;
}

void H5DSopenBool(
	hid_t dest_id, H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	H5DSopen(dest_id, H5TcreateBool(), H5TcreateBool(), dataspace);
}

void H5DSopenDouble(
	hid_t dest_id, H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	H5DSopen(dest_id, H5Tcopy(H5T_NATIVE_DOUBLE), H5Tcopy(H5T_NATIVE_DOUBLE), dataspace);
}

void H5DSopenFloat(
	hid_t dest_id, H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	H5DSopen(dest_id, H5Tcopy(H5T_NATIVE_FLOAT), H5Tcopy(H5T_NATIVE_FLOAT), dataspace);
}

void H5DSopenInt(
	hid_t dest_id, H5_open_dataspace_t* dataspace
) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	H5DSopen(dest_id, H5Tcopy(H5T_NATIVE_INT), H5Tcopy(H5T_NATIVE_INT), dataspace);
}

size_t H5DSnelem(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if (dataspace->dims == NULL) {
		return 1;
	}
	size_t nelem = 1;
	for (int i = 0; i < dataspace->rank; i++)
	{
		nelem *= dataspace->dims[i];
	}
	return nelem;
}

size_t H5DSnelem_chunks(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if (dataspace->dimchunks == NULL) {
		return 0;
	}
	size_t nelem = 1;
	for (int i = 0; i < dataspace->rank; i++)
	{
		nelem *= dataspace->dimchunks[i];
	}
	return nelem;
}

size_t H5DSnelem_lims(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if (dataspace->dimlims == NULL) {
		return 0;
	}
	size_t nelem = 1;
	for (int i = 0; i < dataspace->rank; i++)
	{
		if(dataspace->dimlims[i] == H5S_UNLIMITED && nelem > 0) {
			nelem *= -1;
		}
		else {
			nelem *= dataspace->dimlims[i];
		}
	}
	return nelem;
}

herr_t H5DSboolWrite(
	hid_t dest_id,
	char *d_name,
	const int rank,
	const hsize_t *dims,
	const H5_bool_t *data
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSset(rank, dims, NULL, &dataspace);
	if (rank > 0)
	{
		dataspace.filter_flag = 3;
	}
	H5DSopenBool(dest_id, &dataspace);

	herr_t status = H5DSwrite(&dataspace, data);
	status += H5DSclose(&dataspace);
	return status;
}

herr_t H5DSdoubleWrite(
	hid_t dest_id,
	char *d_name,
	const int rank,
	const hsize_t *dims,
	const double *data
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSset(rank, dims, NULL, &dataspace);
	if (rank > 0)
	{
		dataspace.filter_flag = 3;
	}
	H5DSopenDouble(dest_id, &dataspace);

	herr_t status = H5DSwrite(&dataspace, data);
	status += H5DSclose(&dataspace);
	return status;
}

herr_t H5DSfloatWrite(
	hid_t dest_id,
	char *d_name,
	const int rank,
	const hsize_t *dims,
	const float *data
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSset(rank, dims, NULL, &dataspace);
	if (rank > 0)
	{
		dataspace.filter_flag = 3;
	}
	H5DSopenFloat(dest_id, &dataspace);

	herr_t status = H5DSwrite(&dataspace, data);
	status += H5DSclose(&dataspace);
	return status;
}

herr_t H5DSintWrite(
	hid_t dest_id,
	char *d_name,
	const int rank,
	const hsize_t *dims,
	const int *data
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSset(rank, dims, NULL, &dataspace);
	if (rank > 0)
	{
		dataspace.filter_flag = 3;
	}
	H5DSopenInt(dest_id, &dataspace);

	herr_t status = H5DSwrite(&dataspace, data);
	status += H5DSclose(&dataspace);
	return status;
}

herr_t H5DSstringWrite(
	hid_t dest_id,
	char *d_name,
	const int rank,
	const hsize_t *dims,
	const char *data
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSset(rank, dims, NULL, &dataspace);

	hid_t T_id = H5Tcopy(H5T_C_S1);
	H5Tset_size(T_id, rank > 0 ? H5S_UNLIMITED : strlen(data));
	H5DSopen(dest_id, T_id, H5Tcopy(T_id), &dataspace);

	herr_t status = H5DSwrite(&dataspace, data);
	status += H5DSclose(&dataspace);
	return status;
}

void H5DSaccess(
    hid_t src_id,
    hid_t P_id,
    H5_open_dataspace_t *dataspace
) {
    // expects `name` to be populated
	if (H5Aexists(src_id, dataspace->name)) {
		_H5DSprint_debug(__FUNCTION__, "Attribute: %s", dataspace->name);
		dataspace->D_id = H5Aopen(src_id, dataspace->name, H5P_DEFAULT);
		if (dataspace->D_id == H5I_INVALID_HID) {
			_H5DSprint_debug(__FUNCTION__, "Could not open: %s", dataspace->name);
			return;
		}
		dataspace->S_id = H5Aget_space(dataspace->D_id);
		dataspace->Tmem_id = H5Aget_type(dataspace->D_id);
		dataspace->Tsto_id = H5T_NO_CLASS;
		dataspace->P_id = 0;
		H5S_class_t space_class = H5Sget_simple_extent_type(dataspace->S_id);
		if (space_class == H5S_SCALAR) {
			_H5DSprint_debug(__FUNCTION__, "Attribute is a scalar");
			dataspace->rank = 0;
			dataspace->dims = NULL;
			dataspace->dimlims = NULL;
			dataspace->dimchunks = NULL;
			dataspace->hyperslab_start = NULL;
		}
		else {
			_H5DSprint_debug(__FUNCTION__, "Attribute is an array (rank=%d)", dataspace->rank);
			dataspace->rank = H5Sget_simple_extent_ndims(dataspace->S_id);
			size_t dim_data_bytes = dataspace->rank*sizeof(hsize_t);
			dataspace->dims = malloc(dim_data_bytes);
			H5Sget_simple_extent_dims(dataspace->S_id, dataspace->dims, NULL);

			dataspace->dimlims = NULL;
			dataspace->dimchunks = NULL;
			dataspace->hyperslab_start = NULL;
		}
		return;
	}
	_H5DSprint_debug(__FUNCTION__, "Dataset: %s", dataspace->name);
	// else object is appropriate for dataset open
    dataspace->P_id = H5Pcopy(P_id);
    dataspace->D_id = H5Dopen(src_id, dataspace->name, dataspace->P_id);
    dataspace->S_id = H5Dget_space(dataspace->D_id);
    dataspace->Tmem_id = H5Dget_type(dataspace->D_id);
    dataspace->Tsto_id = H5T_NO_CLASS;
    H5S_class_t space_class = H5Sget_simple_extent_type(dataspace->S_id);
    if (space_class == H5S_SCALAR) {
		_H5DSprint_debug(__FUNCTION__, "Dataset is a scalar");
        dataspace->rank = 0;
        dataspace->dims = NULL;
        dataspace->dimlims = NULL;
        dataspace->dimchunks = NULL;
        dataspace->hyperslab_start = NULL;
    }
    else {
		dataspace->rank = H5Sget_simple_extent_ndims(dataspace->S_id);
		_H5DSprint_debug(__FUNCTION__, "Dataset is an array (rank=%d)", dataspace->rank);
        size_t dim_data_bytes = dataspace->rank*sizeof(hsize_t);
        dataspace->dims = malloc(dim_data_bytes);
        H5Sget_simple_extent_dims(dataspace->S_id, dataspace->dims, NULL);
        
        if (dataspace->dimchunks != NULL) {
			H5DSaccess_set_chunks(dataspace);
        }
    }
}

void H5DSaccess_set_chunks(H5_open_dataspace_t *dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s (rank=%d)", dataspace->name, dataspace->rank);
	for (int i = 0; i < dataspace->rank; i++) {
		if (dataspace->dimchunks[i] == 0) {
			dataspace->dimchunks[i] = dataspace->dims[i];
		}
		_H5DSprint_debug(__FUNCTION__, "dimchunk[%d] = %d", i, dataspace->dimchunks[i]);
	}
	dataspace->C_id = H5Screate_simple(dataspace->rank, dataspace->dimchunks, NULL);
	if (dataspace->hyperslab_start == NULL) {
		dataspace->hyperslab_start = calloc(dataspace->rank*sizeof(hsize_t), 0);
	}
	H5Sselect_hyperslab(
		dataspace->S_id,
		H5S_SELECT_SET,
		dataspace->hyperslab_start,
		NULL, // stride of 1
		dataspace->dimchunks,
		NULL // blocks of 1
	);
}

herr_t H5DSread(
    H5_open_dataspace_t *dataspace,
    void *data
) {
	if (H5Iget_type(dataspace->D_id) == H5I_ATTR) {
		_H5DSprint_debug(__FUNCTION__, "Attribute: %s", dataspace->name);
		return H5Aread(
			dataspace->D_id,
			dataspace->Tmem_id,
			data
		);
	}
    if (dataspace->rank == 0 || dataspace->dimchunks == NULL) {
		_H5DSprint_debug(__FUNCTION__, "Dataset: %s (rank=%d)", dataspace->name, dataspace->rank);
        return H5Dread(
            dataspace->D_id,
            dataspace->Tmem_id,
            H5S_ALL,
            H5S_ALL,
            H5P_DEFAULT,
            data
        );
    }
    else {
		_H5DSprint_debug(__FUNCTION__, "Dataset: %s (chunked)", dataspace->name);
        herr_t status = H5Dread(
            dataspace->D_id,
            dataspace->Tmem_id,
            dataspace->C_id,
            dataspace->S_id,
            dataspace->P_id,
            data
        );
        
		// increment hyperslab
		for (size_t i = dataspace->rank; i-- > 0; )
		{
			dataspace->hyperslab_start[i] += dataspace->dimchunks[i];
			if(dataspace->hyperslab_start[i] >= dataspace->dims[i]) {
				dataspace->hyperslab_start[i] = 0;
			}
			else {
				// the next hyperslab doesn't require an in a slower dimension
				break;
			}
		}
		status += H5Sselect_hyperslab(dataspace->S_id, H5S_SELECT_SET, dataspace->hyperslab_start, NULL, dataspace->dimchunks, NULL);
        return status;
    }
}

void* H5DSread_all(
	hid_t src_id,
	char *d_name
) {
	H5_open_dataspace_t dataspace = {0};
	dataspace.name = d_name;
	H5DSaccess(src_id, H5P_DEFAULT, &dataspace);
	void *data = H5DSmalloc(&dataspace);
	H5DSread(&dataspace, data);
	H5DSclose(&dataspace);
	return data;
}