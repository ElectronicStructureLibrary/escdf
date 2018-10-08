/* Copyright (C) 2016-2017 Damien Caliste <dcaliste@free.fr>
 *                         Micael Oliveira <micael.oliveira@mpsd.mpg.de>
 *                         Yann Pouillon <devops@materialsevolution.es>
 *
 * This file is part of ESCDF.
 *
 * ESCDF is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, version 2.1 of the License, or (at your option) any
 * later version.
 *
 * ESCDF is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESCDF.  If not, see <http://www.gnu.org/licenses/> or write to
 * the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA.
 */

#include <stdbool.h>
#include <hdf5.h>
#include <string.h>

#include "escdf_error.h"
#include "utils_hdf5.h"


/******************************************************************************
 * check methods                                                              *
 ******************************************************************************/

bool utils_hdf5_check_present(hid_t loc_id, const char *name)
{
    htri_t bool_id;

    printf("utils_hdf5_check_present: %s\n", name); fflush(stdout);

    if ((bool_id = H5Lexists(loc_id, name, H5P_DEFAULT)) < 0 || !bool_id) {
        printf("utils_hdf5_check_present: link to %s NOT found\n", name); fflush(stdout);
        return false;
    }
    else {
        printf("utils_hdf5_check_present: link to %s found\n", name); fflush(stdout);
    }

    if ((bool_id = H5Oexists_by_name(loc_id, name, H5P_DEFAULT)) < 0 || !bool_id) {
        printf("utils_hdf5_check_present: object %s NOT found\n", name); fflush(stdout);
        return false;
    } 
    else {
        printf("utils_hdf5_check_present: object %s found\n", name); fflush(stdout);
    }


    /*
    bool_id = H5Oexists_by_name(loc_id, name, H5P_DEFAULT);

    printf("utils_hdf5_check_present: %s %d\n", name, bool_id);
    return (bool_id > 0);

    */
    return true;
}

bool utils_hdf5_check_present_recursive(hid_t loc_id, const char *path)
{
    char lpath[ESCDF_STRLEN_GROUP];
    char *p;

    strcpy(lpath, path);

    for (p = lpath + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (!utils_hdf5_check_present(loc_id, lpath))
                return false;
            *p = '/';
        }
    }
    return utils_hdf5_check_present(loc_id, lpath);
}

bool utils_hdf5_check_present_attr(hid_t loc_id, const char *name)
{
    htri_t bool_id;

    if ((bool_id = H5Aexists(loc_id, name)) < 0 || !bool_id)
        return false;

    return true;
}

escdf_errno_t utils_hdf5_check_shape(hid_t dtspace_id, const hsize_t *dims, unsigned int ndims)
{
    H5S_class_t type_id;
    int ndims_id;
    hsize_t *dims_v, *maxdims_v;
    unsigned int i;

    if ((type_id = H5Sget_simple_extent_type(dtspace_id)) == H5S_NO_CLASS) {
        RETURN_WITH_ERROR(ESCDF_ERROR_ARGS);
    }
    FULFILL_OR_RETURN(type_id == H5S_SCALAR || type_id == H5S_SIMPLE, ESCDF_ERROR_DIM);
    
    switch (type_id) {
    case H5S_SCALAR:
        FULFILL_OR_RETURN(dims == NULL && ndims == 0, ESCDF_ERROR);
        break;
    case H5S_SIMPLE:
        if ((ndims_id = H5Sget_simple_extent_ndims(dtspace_id)) < 0) {
            RETURN_WITH_ERROR(ndims_id);
        }
        dims_v = malloc(sizeof(hsize_t) * ndims_id);
        maxdims_v = malloc(sizeof(hsize_t) * ndims_id);
        if ((ndims_id = H5Sget_simple_extent_dims(dtspace_id, dims_v, maxdims_v)) < 0) {
            DEFER_FUNC_ERROR(ndims_id);
            goto cleanup_dims;
        }
        FULFILL_OR_RETURN((unsigned int)ndims_id == ndims ||
                          (ndims == 0 && ndims_id == 1 &&
                           dims_v[0] == 1), ESCDF_ERROR_DIM);
        for (i = 0; i < ndims; i++) {
            if (dims_v[i] != dims[i] || maxdims_v[i] < dims[i]) {
                DEFER_FUNC_ERROR(ESCDF_ERROR_DIM);
                goto cleanup_dims;
            }
        }
        free(dims_v);
        free(maxdims_v);
        break;
    default:
        RETURN_WITH_ERROR(ESCDF_ERROR);
    }
    return ESCDF_SUCCESS;
    
    cleanup_dims:
    free(dims_v);
    free(maxdims_v);
    return ESCDF_ERROR;
}

escdf_errno_t utils_hdf5_check_attr(hid_t loc_id, const char *name, const hsize_t *dims, unsigned int ndims, hid_t *attr_pt)
{
    hid_t attr_id, dtspace_id;

    if ((attr_id = H5Aopen(loc_id, name, H5P_DEFAULT)) < 0)
        RETURN_WITH_ERROR(attr_id);

    /* Check space dimensions. */
    if ((dtspace_id = H5Aget_space(attr_id)) < 0) {
        DEFER_FUNC_ERROR(dtspace_id);
        goto cleanup_attr;
    }
    if (utils_hdf5_check_shape(dtspace_id, dims, ndims) != ESCDF_SUCCESS) {
        goto cleanup_dtspace;
    }
    H5Sclose(dtspace_id);
    if (attr_pt)
        *attr_pt = attr_id;
    else
        H5Aclose(attr_id);
    return ESCDF_SUCCESS;

    cleanup_dtspace:
    H5Sclose(dtspace_id);
    cleanup_attr:
    H5Aclose(attr_id);
    return ESCDF_ERROR;
}

escdf_errno_t utils_hdf5_check_dataset(hid_t loc_id, const char *name, const hsize_t *dims, unsigned int ndims, hid_t *dtset_pt)
{
    hid_t dtset_id, dtspace_id;

    if ((dtset_id = H5Dopen(loc_id, name, H5P_DEFAULT)) < 0)
        RETURN_WITH_ERROR(dtset_id);

    /* Check space dimensions. */
    if ((dtspace_id = H5Dget_space(dtset_id)) < 0) {
        DEFER_FUNC_ERROR(dtspace_id);
        goto cleanup_dtset;
    }
    if (utils_hdf5_check_shape(dtspace_id, dims, ndims) != ESCDF_SUCCESS) {
        goto cleanup_dtspace;
    }
    H5Sclose(dtspace_id);
    if (dtset_pt)
        *dtset_pt = dtset_id;
    else
        H5Dclose(dtset_id);
    return ESCDF_SUCCESS;

    cleanup_dtspace:
    H5Sclose(dtspace_id);
    cleanup_dtset:
    H5Dclose(dtset_id);
    return ESCDF_ERROR;
}


/******************************************************************************
 * read methods                                                               *
 ******************************************************************************/

escdf_errno_t utils_hdf5_read_attr(hid_t loc_id, const char *name, hid_t mem_type_id, const hsize_t *dims, unsigned int ndims, void *buf)
{
    escdf_errno_t err;

    hid_t attr_id;
    herr_t err_id;

    if ((err = utils_hdf5_check_attr(loc_id, name,
                                     dims, ndims, &attr_id)) != ESCDF_SUCCESS) {
        return err;
    }

    err = ESCDF_SUCCESS;
    if ((err_id = H5Aread(attr_id, mem_type_id, buf)) < 0) {
        DEFER_FUNC_ERROR(err_id);
        err = ESCDF_ERROR;
    }

    H5Aclose(attr_id);
    return err;
}

escdf_errno_t utils_hdf5_read_attr_string(hid_t loc_id, const char *name, hsize_t len, const hsize_t *dims,
                                          unsigned int ndims, void *buf)
{
    escdf_errno_t err;

    hid_t str_id;

    str_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_id, len);
    H5Tset_strpad(str_id, H5T_STR_NULLTERM);

    if ((err = utils_hdf5_read_attr(loc_id, name, str_id, dims, ndims, buf)) != ESCDF_SUCCESS) {
        H5Tclose(str_id);
        return err;
    };

    H5Tclose(str_id);
    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_attr_bool(hid_t loc_id, const char *name, const hsize_t *dims, unsigned int ndims,
                                        void *buf)
{
    escdf_errno_t err;
    unsigned int i;
    char (*char_values)[4];
    hsize_t len;
    bool *values = (bool *)buf;

    len = 1;
    for (i = 0; i < ndims; i++)
        len *= dims[i];
    char_values = malloc(sizeof(char[4]) * len);

    if ((err = utils_hdf5_read_attr_string(loc_id, name, 4, dims, ndims, char_values)) != ESCDF_SUCCESS) {
        return err;
    }
    for (i = 0; i < len; i++)
        values[i] = char_values[i][0] == 'y';

    free(char_values);
    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_bool_old(hid_t loc_id, const char *name, _bool_set_t *scalar)
{
    escdf_errno_t err;
    char value[4];

    if ((err = utils_hdf5_read_attr_string(loc_id, name, 4, NULL, 0, value)) != ESCDF_SUCCESS) {
        return err;
    }
    *scalar = _bool_set((bool)(value[0] == 'y'));

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_uint(hid_t loc_id, const char *name, _uint_set_t *scalar, unsigned int range[2])
{
    escdf_errno_t err;
    unsigned int value;

    if ((err = utils_hdf5_read_attr(loc_id, name, H5T_NATIVE_UINT,
                                    NULL, 0, &value)) != ESCDF_SUCCESS) {
        return err;
    }
    if (value < range[0] || value > range[1]) {
        RETURN_WITH_ERROR(ESCDF_ERANGE);
    }
    *scalar = _uint_set(value);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_int(hid_t loc_id, const char *name, _int_set_t *scalar, int range[2])
{
    escdf_errno_t err;
    int value;

    if ((err = utils_hdf5_read_attr(loc_id, name, H5T_NATIVE_INT,
                                    NULL, 0, &value)) != ESCDF_SUCCESS) {
        return err;
    }
    if (value < range[0] || value > range[1]) {
        RETURN_WITH_ERROR(ESCDF_ERANGE);
    }
    *scalar = _int_set(value);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_uint_array(hid_t loc_id, const char *name, unsigned int **array, const hsize_t *dims, unsigned int ndims, unsigned int range[2])
{
    escdf_errno_t err;
    unsigned int i;
    hsize_t len;

    len = 1;
    for (i = 0; i < ndims; i++) {
        len *= dims[i];
    }
    *array = malloc(sizeof(unsigned int) * len);

    if ((err = utils_hdf5_read_attr(loc_id, name, H5T_NATIVE_UINT, dims, ndims,
                                    (void*)*array)) != ESCDF_SUCCESS) {
        free(*array);
        return err;
    }
    for (i = 0; i < len; i++) {
        if ((*array)[i] < range[0] || (*array)[i] > range[1]) {
            free(*array);
            RETURN_WITH_ERROR(ESCDF_ERANGE);
        }
    }
    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_int_array(hid_t loc_id, const char *name, int **array, const hsize_t *dims, unsigned int ndims, int range[2])
{
    escdf_errno_t err;
    unsigned int i;
    hsize_t len;

    len = 1;
    for (i = 0; i < ndims; i++) {
        len *= dims[i];
    }
    *array = malloc(sizeof(int) * len);

    if ((err = utils_hdf5_read_attr(loc_id, name, H5T_NATIVE_INT, dims, ndims,
                                    (void*)*array)) != ESCDF_SUCCESS) {
        free(*array);
        return err;
    }
    for (i = 0; i < len; i++) {
        if ((*array)[i] < range[0] || (*array)[i] > range[1]) {
            free(*array);
            RETURN_WITH_ERROR(ESCDF_ERANGE);
        }
    }
    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_dbl_array(hid_t loc_id, const char *name, double **array, const hsize_t *dims, unsigned int ndims, double range[2])
{
    escdf_errno_t err;
    unsigned int i;
    hsize_t len;

    len = 1;
    for (i = 0; i < ndims; i++) {
        len *= dims[i];
    }
    *array = malloc(sizeof(double) * len);

    if ((err = utils_hdf5_read_attr(loc_id, name, H5T_NATIVE_DOUBLE, dims, ndims,
                                    (void*)*array)) != ESCDF_SUCCESS) {
        free(*array);
        return err;
    }
    for (i = 0; i < len; i++) {
        if ((*array)[i] < range[0] || (*array)[i] > range[1]) {
            free(*array);
            RETURN_WITH_ERROR(ESCDF_ERANGE);
        }
    }
    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_dataset(hid_t dtset_id, hid_t xfer_id, void *buf, hid_t mem_type_id, const hsize_t *start, const hsize_t *count, const hsize_t *stride)
{
    escdf_errno_t err;
    hid_t memspace_id, diskspace_id;
    herr_t err_id;

    hid_t xfer_plist;

    if ((err = utils_hdf5_select_slice(dtset_id, &diskspace_id, &memspace_id,
                                       start, count, stride)) != ESCDF_SUCCESS) {
        return err;
    }

    if(xfer_id != ESCDF_UNDEFINED_ID)
        xfer_plist = xfer_id;
    else
        xfer_plist = H5P_DEFAULT;


    /* Read */
    if ((err_id = H5Dread(dtset_id, mem_type_id, memspace_id,
                          diskspace_id, xfer_plist, buf)) < 0) {
        H5Sclose(diskspace_id);
        H5Sclose(memspace_id);
        RETURN_WITH_ERROR(err_id);
    }

    H5Sclose(diskspace_id);
    H5Sclose(memspace_id);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_read_dataset_at(hid_t dtset_id, hid_t xfer_id, void *buf, hid_t mem_type_id, hsize_t num_points, const hsize_t *coord)
{
    hid_t memspace_id, diskspace_id;
    herr_t err_id;
    hsize_t len;

    /* disk use the start, count and stride. */
    if ((diskspace_id = H5Dget_space(dtset_id)) < 0) {
        RETURN_WITH_ERROR(diskspace_id);
    }

    if (num_points) {
        if ((err_id = H5Sselect_elements(diskspace_id, H5S_SELECT_SET,
                                         num_points, coord)) < 0) {
            H5Sclose(diskspace_id);
            RETURN_WITH_ERROR(err_id);
        }
        /* memory is a flat array with the size on the slice. */
        len = (hsize_t)num_points;
        memspace_id = H5Screate_simple(1, &len, NULL);
    } else {
        if ((err_id = H5Sselect_none(diskspace_id)) < 0) {
            H5Sclose(diskspace_id);
            RETURN_WITH_ERROR(err_id);
        }
        memspace_id = H5Screate(H5S_NULL);
    }
    if (memspace_id < 0) {
        H5Sclose(diskspace_id);
        RETURN_WITH_ERROR(memspace_id);
    }

    /* Read */
    if ((err_id = H5Dread(dtset_id, mem_type_id, memspace_id,
                          diskspace_id, xfer_id, buf)) < 0) {
        H5Sclose(diskspace_id);
        H5Sclose(memspace_id);
        RETURN_WITH_ERROR(err_id);
    }

    H5Sclose(diskspace_id);
    H5Sclose(memspace_id);

    return ESCDF_SUCCESS;
}


/******************************************************************************
 * create methods                                                             *
 ******************************************************************************/

escdf_errno_t utils_hdf5_create_group(hid_t loc_id, const char *path, hid_t *group_pt)
{
    escdf_errno_t err;
    hid_t group_id, lcpl_id;

    if (utils_hdf5_check_present_recursive(loc_id, path))
        return ESCDF_ERROR_ARGS;

    if ((lcpl_id = H5Pcreate(H5P_LINK_CREATE)) < 0) {
        RETURN_WITH_ERROR(lcpl_id);
    }
    if ((err = H5Pset_create_intermediate_group(lcpl_id, 1)) < 0) {
        DEFER_FUNC_ERROR(err);
        goto cleanup_prop;
    }

    if ((group_id = H5Gcreate(loc_id, path, lcpl_id, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        DEFER_FUNC_ERROR(group_id);
        goto cleanup_prop;
    }

    H5Pclose(lcpl_id);
    if (group_pt != NULL)
        *group_pt = group_id;
    else
        H5Gclose(group_id);

    return ESCDF_SUCCESS;

    cleanup_prop:
    H5Pclose(lcpl_id);
    return ESCDF_ERROR;
}

escdf_errno_t utils_hdf5_create_attr(hid_t loc_id, const char *name, hid_t type_id, const hsize_t *dims, unsigned int ndims, hid_t *attr_pt)
{
    hid_t attr_id, dtspace_id;

    /* Create space dimensions. */
    if (!dims || !ndims) {
        if ((dtspace_id = H5Screate(H5S_SCALAR)) < 0) {
            RETURN_WITH_ERROR(dtspace_id);
        }
    } else {
        if ((dtspace_id = H5Screate_simple(ndims, dims, NULL)) < 0) {
            RETURN_WITH_ERROR(dtspace_id);
        }
    }

    if ((attr_id = H5Acreate(loc_id, name, type_id, dtspace_id,
                             H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        DEFER_FUNC_ERROR(attr_id);
        goto cleanup_dtspace;
    }

    H5Sclose(dtspace_id);
    if (attr_pt)
        *attr_pt = attr_id;
    else
        H5Aclose(attr_id);
    return ESCDF_SUCCESS;

    cleanup_dtspace:
    H5Sclose(dtspace_id);
    return ESCDF_ERROR;
}

escdf_errno_t utils_hdf5_create_dataset(hid_t loc_id, const char *name, hid_t type_id, const hsize_t *dims, unsigned int ndims, hid_t *dtset_pt)
{
    int i;
    hid_t dtset_id, dtspace_id;
    herr_t error;

    hsize_t ndims_check;
    hsize_t *dims_check, *maxdims_check;


    /* Create space dimensions. */

    
    printf("utils_hdf5_create_datasets: ndims = %d \n", ndims);
    for(i=0; i<ndims; i++) {
        printf("utils_hdf5_create_datasets: dims[%d]= %lld \n", i, dims[i]);
    }
    fflush(stdout);

 
    if ((dtspace_id = H5Screate_simple(ndims, dims, NULL)) < 0) {
        printf("utils_hdf5_create_datasets: H5Screate_simple failed. dtspace_id = %d.\n",dtspace_id);fflush(stdout);
        RETURN_WITH_ERROR(dtspace_id);
    }
    printf("utils_hdf5_create_datasets: H5Screate_simple successful.\n"); fflush(stdout);
    

    ndims_check = H5Sget_simple_extent_ndims(dtspace_id);

    dims_check = malloc(ndims_check * sizeof(hsize_t));
    maxdims_check = malloc(ndims_check * sizeof(hsize_t));

    H5Sget_simple_extent_dims(dtspace_id, dims_check, maxdims_check);

    for(i=0; i<ndims_check; i++) {
        printf("utils_hdf5_create_datasets: check  dims[%lld] = %lld %lld \n",i, dims_check[i], maxdims_check[i]);
    }

    printf("utils_hdf5_create_datasets: Attempt H5Dcreate with: \n"); fflush(stdout);
    printf("  loc_id = %lld,\n  name = %s,\n  type_id = %lld,\n  dtspace_id = %lld. \n", loc_id, name, type_id, dtspace_id); 
    fflush(stdout);

    if ((dtset_id = H5Dcreate(loc_id, name, type_id, dtspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        printf("utils_hdf5_create_datasets: H5Dcreate failed.\n"); fflush(stdout);
        DEFER_FUNC_ERROR(dtset_id);
        goto cleanup_dtspace;
    }
    printf("utils_hdf5_create_datasets: H5Dcreate sucessful.\n"); fflush(stdout);

    error = H5Sclose(dtspace_id);
    if(error) {
        printf("utils_hdf5_create_datasets: H5Sclose failed. Returning ESCDF_ERROR.\n"); fflush(stdout);
        return ESCDF_ERROR;
    }

    if (dtset_pt)
        *dtset_pt = dtset_id;
    else {
        error = H5Dclose(dtset_id);
        if(error) {
            printf("utils_hdf5_create_datasets: H5Dclose failed. Returning ESCDF_ERROR.\n"); fflush(stdout);
            return ESCDF_ERROR;
        }

    }
    printf("utils_hdf5_create_datasets: returning SUCCESS.\n"); fflush(stdout);
    return ESCDF_SUCCESS;

    cleanup_dtspace:
    H5Sclose(dtspace_id);
    printf("utils_hdf5_create_datasets: returning ESCDF_ERROR.\n"); fflush(stdout);
    return ESCDF_ERROR;
}


/******************************************************************************
 * write methods                                                              *
 ******************************************************************************/

escdf_errno_t utils_hdf5_write_attr(hid_t loc_id, const char *name, hid_t disk_type_id, const hsize_t *dims, unsigned int ndims, hid_t mem_type_id, const void *buf)
{
    escdf_errno_t err;

    htri_t bool_id;
    hid_t attr_id;
    herr_t err_id;

    if ( !((bool_id = H5Aexists(loc_id, name)) < 0 || !bool_id) ) {
      if ((err = H5Adelete (loc_id, name)) != ESCDF_SUCCESS) {
	return err;
      }
    }

    if ((err = utils_hdf5_create_attr(loc_id, name, disk_type_id,
                                      dims, ndims, &attr_id)) != ESCDF_SUCCESS) {
        return err;
    }

    err = ESCDF_SUCCESS;
    if ((err_id = H5Awrite(attr_id, mem_type_id, buf)) < 0) {
        DEFER_FUNC_ERROR(err_id);
        err = ESCDF_ERROR;
    }

    H5Aclose(attr_id);
    return err;
}

escdf_errno_t utils_hdf5_write_attr_string(hid_t loc_id, const char *name, hsize_t len, const hsize_t *dims,
                                           unsigned int ndims, const void *buf)
{
    escdf_errno_t err;
    hid_t str_id;

    str_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_id, len);
    H5Tset_strpad(str_id, H5T_STR_NULLTERM);
    err = utils_hdf5_write_attr(loc_id, name, str_id, dims, ndims, str_id, buf);

    H5Tclose(str_id);
    return err;
}

escdf_errno_t utils_hdf5_write_attr_bool(hid_t loc_id, const char *name, const hsize_t *dims, unsigned int ndims,
                                         const void *buf)
{
    escdf_errno_t err;
    unsigned int i;
    char (*char_values)[4];
    hsize_t len;
    bool *values = (bool *)buf;

    len = 1;
    for (i = 0; i < ndims; i++)
        len *= dims[i];
    char_values = malloc(sizeof(char[4]) * len);

    for (i = 0; i < len; i++)
        strcpy(char_values[i], values[i] ? "yes" : "no");

    err = utils_hdf5_write_attr_string(loc_id, name, 4, dims, ndims, char_values);

    free(char_values);
    return err;
}

escdf_errno_t utils_hdf5_write_dataset(hid_t dtset_id, hid_t xfer_id, const void *buf, hid_t mem_type_id, const hsize_t *start, const hsize_t *count, const hsize_t *stride)
{
    escdf_errno_t err;
    hid_t memspace_id, diskspace_id, xfer_plist;
    herr_t err_id;

    hsize_t ndims, *dims, *maxdims, i;

    err = utils_hdf5_select_slice(dtset_id, &diskspace_id, &memspace_id,
                                  start, count, stride);
    FULFILL_OR_RETURN(err == ESCDF_SUCCESS, err);
    
    if( xfer_id != ESCDF_UNDEFINED_ID ) 
        xfer_plist = xfer_id;
    else
        xfer_plist = H5P_DEFAULT;

    ndims = H5Sget_simple_extent_ndims(diskspace_id);

    dims = malloc(sizeof(hsize_t) * ndims);
    maxdims = malloc(sizeof(hsize_t) * ndims);
    

    ndims = H5Sget_simple_extent_dims(diskspace_id, dims, maxdims);

    /*
    printf("utils_hdf5_write_dataset: diskspace ndims = %d.\n", ndims);
    for(i=0; i<ndims; i++) {
        printf("utils_hdf5_write_dataset: diskspace dims[%d] = %d.\n", i, dims[i]);
    }
    */
   
    free(dims);
    free(maxdims);

    /* Write */
    if ((err_id = H5Dwrite(dtset_id, mem_type_id, memspace_id,
                           diskspace_id, xfer_plist, buf)) < 0) {
        H5Sclose(diskspace_id);
        H5Sclose(memspace_id);
        RETURN_WITH_ERROR(err_id);
    }

    H5Sclose(diskspace_id);
    H5Sclose(memspace_id);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_write_bool_old(hid_t loc_id, const char *name, const bool value)
{
    return utils_hdf5_write_string_old(loc_id, name, (value ? "yes" : "no"), 4);
}

escdf_errno_t utils_hdf5_write_string_old(hid_t loc_id, const char *name, const char *string, hsize_t len)
{
    escdf_errno_t err;
    hid_t str_id;

    str_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_id, len);
    H5Tset_strpad(str_id, H5T_STR_NULLTERM);
    err = utils_hdf5_write_attr(loc_id, name, str_id, NULL, 0, str_id, string);

    H5Tclose(str_id);
    return err;
}


/******************************************************************************
 * dataset open methods                                                       *
 ******************************************************************************/

escdf_errno_t utils_hdf5_open_dataset(hid_t loc_id, const char *name, hid_t *dtset_pt )
{
    /* printf("utils_hdf5_open_dataset: attempting to open %s, %d. \n", name, loc_id); */

    *dtset_pt = H5Dopen1(loc_id, name);

    /* printf("utils_hdf5_open_dataset: %s opened with %d. \n", name, *dtset_pt); */


    if(*dtset_pt < 0) RETURN_WITH_ERROR(ESCDF_ERROR);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_close_dataset(hid_t dtset_id)
{
    herr_t err;

    err = H5Dclose(dtset_id);

    if(err<0) RETURN_WITH_ERROR(ESCDF_ERROR);
    return ESCDF_SUCCESS;   
}

escdf_errno_t utils_hdf5_open_group(hid_t parent_id, const char *location_path, hid_t *group_id)
{
 
    *group_id = H5Gopen2(parent_id, location_path, H5P_DEFAULT);

    if(*group_id < 0) RETURN_WITH_ERROR(ESCDF_ERROR);

    return ESCDF_SUCCESS;
}

escdf_errno_t utils_hdf5_close_group(hid_t group_id)
{
    herr_t err;
    err = H5Gclose(group_id);

    if(err<0) RETURN_WITH_ERROR(ESCDF_ERROR);
    return ESCDF_SUCCESS;
}



/******************************************************************************
 * misc methods                                                               *
 ******************************************************************************/

escdf_errno_t utils_hdf5_select_slice(hid_t dtset_id, hid_t *diskspace_id, hid_t *memspace_id, const hsize_t *start, const hsize_t *count, const hsize_t *stride)
{
    herr_t err_id;
    hssize_t len;
    hsize_t len_;

    /* disk use the start, count and stride. */
    if ((*diskspace_id = H5Dget_space(dtset_id)) < 0) {
        RETURN_WITH_ERROR(*diskspace_id);
    }

    /* create dataspace for memory and disk. */
    if (start && count) {
        if ((err_id = H5Sselect_hyperslab(*diskspace_id, H5S_SELECT_SET,
                                          start, stride, count, NULL)) < 0) {
            H5Sclose(*diskspace_id);
            RETURN_WITH_ERROR(err_id);
        }
    } else {
        if ((err_id = H5Sselect_all(*diskspace_id)) < 0) {
            H5Sclose(*diskspace_id);
            RETURN_WITH_ERROR(err_id);
        }
    }
    if ((len = H5Sget_select_npoints(*diskspace_id)) < 0) {
        H5Sclose(*diskspace_id);
        RETURN_WITH_ERROR(len);
    }
    if (!len) {
        if ((err_id = H5Sselect_none(*diskspace_id)) < 0) {
            H5Sclose(*diskspace_id);
            RETURN_WITH_ERROR(err_id);
        }
    }

    if (len > 0) {
        len_ = (hsize_t)len;
        /* memory is a flat array with the size on the slice. */
        *memspace_id = H5Screate_simple(1, &len_, NULL);
    } else {
        *memspace_id = H5Screate(H5S_NULL);
    }
    if (*memspace_id < 0) {
        H5Sclose(*diskspace_id);
        RETURN_WITH_ERROR(*memspace_id);
    }

    return ESCDF_SUCCESS;
}

hid_t utils_hdf5_mem_type(int datatype)
{
    switch (datatype) {
    case ESCDF_DT_BOOL:
        return H5T_C_S1;
    case ESCDF_DT_UINT:
        return H5T_NATIVE_UINT;
    case ESCDF_DT_INT:
        return H5T_NATIVE_INT;
    case ESCDF_DT_DOUBLE:
        return H5T_NATIVE_DOUBLE;
    case ESCDF_DT_STRING:
        return H5T_C_S1;
    default:
        return 0;
    }
}


hid_t utils_hdf5_disk_type(int datatype)
{
    switch (datatype) {
    case ESCDF_DT_BOOL:
        return H5T_C_S1;
    case ESCDF_DT_UINT:
        return H5T_NATIVE_UINT;
    case ESCDF_DT_INT:
        return H5T_NATIVE_INT;
    case ESCDF_DT_DOUBLE:
        return H5T_NATIVE_DOUBLE;
    case ESCDF_DT_STRING:
        return H5T_C_S1;
    default:
        return 0;
    }
}


#if H5_VERS_MINOR < 8 || H5_VERS_RELEASE < 5
htri_t H5Oexists_by_name(hid_t loc_id, const char *name, hid_t lapl_id)
{
  return 1;
}
#endif
