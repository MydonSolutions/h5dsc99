#include "h5dsc99/h5_dataspace.h"

// #define DEBUG

void _H5DSprint_debug(const char *name, const char *msg, ...) {
#ifdef DEBUG
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
	if (dataspace->D_id) {
		_H5DSprint_debug(__FUNCTION__, "\tD_id");
		status += H5Dclose(dataspace->D_id);
	}
	if (dataspace->S_id) {
		_H5DSprint_debug(__FUNCTION__, "\tS_id");
		status += H5Sclose(dataspace->S_id);
	}
	if (dataspace->C_id) {
		_H5DSprint_debug(__FUNCTION__, "\tC_id");
		status += H5Sclose(dataspace->C_id);
	}
	return status + H5DSfree(dataspace);
}

herr_t H5DSfree(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	if (dataspace->dims != NULL) {
		free(dataspace->dims);
	}
	if (dataspace->dimlims != NULL) {
		free(dataspace->dimlims);
	}
	if (dataspace->dimchunks != NULL) {
		free(dataspace->dimchunks);
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
	if(rank > 0) {
		dataspace->dims = malloc(rank * sizeof(hsize_t));
		dataspace->dimlims = malloc(rank * sizeof(hsize_t));
		
		if(chunks != NULL) {
			dataspace->dimchunks = malloc(rank * sizeof(hsize_t));
		}

		for (size_t i = 0; i < rank; i++)
		{
			dataspace->dimlims[i] = dimlims[i];
			dataspace->dims[i] = dimlims[i];
			if(dimlims[i] == H5S_UNLIMITED) {
				dataspace->dims[i] = 0;
			}
			_H5DSprint_debug(__FUNCTION__, "\tdim %ld (%llu/%llu)", i, dataspace->dims[i], dataspace->dimlims[i]);
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
 		return H5Dwrite(dataspace->D_id, dataspace->Tsto_id, dataspace->C_id, dataspace->S_id, H5P_DEFAULT, data);
	}
	else {
 		return H5Dwrite(dataspace->D_id, dataspace->Tsto_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	}
}

herr_t H5DSextend(H5_open_dataspace_t* dataspace) {
	_H5DSprint_debug(__FUNCTION__, "%s", dataspace->name);
	// for selecting extension of space
	hsize_t *start = malloc(dataspace->rank * sizeof(hsize_t));
	memset(start, 0, dataspace->rank * sizeof(hsize_t));

	for (size_t i = 0; i < dataspace->rank; i++)
	{
		// TODO assert at least one unlimited dimension
		if(dataspace->dimlims[i] == H5S_UNLIMITED) {
			start[i] = dataspace->dims[i]; // hyperslab starts where the prior chunk ends
			dataspace->dims[i] += dataspace->dimchunks[i];
		}
	}
	
  herr_t status = H5Dset_extent(dataspace->D_id, dataspace->dims);
	status += H5Sclose(dataspace->S_id); // this
	dataspace->S_id = H5Dget_space(dataspace->D_id); // this

	// select extension of space
	status += H5Sselect_hyperslab(dataspace->S_id, H5S_SELECT_SET, start, NULL, dataspace->dimchunks, NULL);

	free(start);
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
