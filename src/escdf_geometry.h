#ifndef LIBESCDF_GEOMETRY_H
#define LIBESCDF_GEOMETRY_H

#include <stdbool.h>

#include "escdf_error.h"

#include "utils.h"
#include "handle.h"


/*****************************************************************************
 * Data structures                                                           *
 *****************************************************************************/

/**
 * Geometry data type. This type stores all the metadata that can be found in
 * the ESCDF geometry group. It also stores if any of the other data are
 * present in the group or not.
*/
typedef struct  {
    _handle_t handle; /**< Handle for HDF5 group */

    /* The metadata */
    _uint_set_t number_of_physical_dimensions;
    unsigned int *dimension_types;
    _bool_set_t embedded_system;

    /* Information about which data is present in the group */
    bool magnetic_moment_directions_is_present;
} escdf_geometry_t;


/*****************************************************************************
 * Global functions                                                          *
 *****************************************************************************/

/**
 * Creates a new instance of the geometry data type.
 *
 * @param[in] path: the path to the ESCDF geometry group.
 * @return instance of the geometry data type.
 */
escdf_geometry_t * escdf_geometry_new(const char *path);

/**
 * Free all memory associated with the geometry group.
 *
 * @param[in,out] geometry: the geometry.
 */
void escdf_geometry_free(escdf_geometry_t * geometry);


/*****************************************************************************
 * Metadata functions                                                        *
 *****************************************************************************/

/**
 * Given a path to a ESCDF geometry group, this routines opens the group, reads
 * all the metadata stored in the group, and stores the information in the
 * geometry data type.
 *
 * @param[out] geometry: pointer to instance of the geometry group.
 * @param[in] path: the path to the ESCDF geometry group.
 * @return error code.
 */
escdf_err_t escdf_geometry_read_metadata(escdf_geometry_t **geometry, const char *path);

/**
 * Given a geometry data type, it writes all the metadata stored in it to the
 * corresponding ESCDF geometry group. The variables to be written are the ones
 * that have been set with a set function. If not all the variables that are
 * mandatory according to the ESCDF specifications are set, then nothing is
 * written to the group and the function returns an error.
 *
 * @param[in] geometry: instance of the geometry group.
 * @return error code.
 */
escdf_err_t escdf_geometry_write_metadata(const escdf_geometry_t *geometry);


/**
 * Sets the value of number_of_physical_dimensions in the geometry data type.
 *
 * @param[in,out] geometry: instance of the geometry group.
 * @param[in] number_of_physical_dimensions: the value of the variable to be set.
 * @return error code.
 */
escdf_err_t escdf_geometry_set_number_of_physical_dimensions(escdf_geometry_t *geometry,
                                                     const unsigned int number_of_physical_dimensions);

/**
 * Get the value of number_of_physical_dimensions stored in the geometry data
 * type.
 *
 * @param[in] geometry: instance of the geometry group.
 * @return the value of the variable.
 */
unsigned int escdf_geometry_get_number_of_physical_dimensions(const escdf_geometry_t *geometry);

/**
 * Get a pointer to number_of_physical_dimensions stored in the geometry data
 * type.
 *
 * @param[in] geometry: instance of the geometry group.
 * @return pointer to the variable.
 */
const unsigned int * escdf_geometry_ptr_dimension_types(const escdf_geometry_t *geometry);

/**
 * Returns if number_of_physical_dimensions has been set or not in the geometry
 * data type.
 *
 * @param[in] geometry: instance of the geometry group.
 * @return TRUE if the variable has been set, FALSE otherwise.
 */
bool escdf_geometry_is_set_number_of_physical_dimensions(const escdf_geometry_t *geometry);


/*****************************************************************************
 * Data functions                                                            *
 *****************************************************************************/

/**
 * Writes the magnetic_moment_directions variable to the geometry group.
 *
 * @param[in] geometry: instance of the geometry group.
 * @param[in] buffer: the buffer where the data is stored in memory.
 * @param[in] start:
 * @param[in] count:
 * @param[in] map:
 * @return error code.
 */
escdf_err_t escdf_geometry_write_magnetic_moment_directions(const escdf_geometry_t *geometry, const double *buffer,
                                                            const unsigned int *start, const unsigned int *count,
                                                            const unsigned int *map);
/**
 * Reads the magnetic_moment_directions variable from the geometry group.
 *
 * @param[in] geometry: instance of the geometry group.
 * @param[out] buffer: the buffer where the data is to be stored in memory.
 * @param[in] start: vector of integers specifying the index in the variable where the first of the data values will be read.
 * @param[in] count: vector of integers specifying the edge lengths along each dimension of the block of data values to be read.
 * @param[in] map: vector of integers that specifies the mapping between the dimensions of the variable and the in-memory structure of the internal data array.
 * @return error code.
 */
escdf_err_t escdf_geometry_read_magnetic_moment_directions(const escdf_geometry_t *geometry, double *buffer,
                                                           const unsigned int *start, const unsigned int *count,
                                                           const unsigned int *map);



#endif