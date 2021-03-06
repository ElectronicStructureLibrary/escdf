/* Copyright (C) 2016-2017 Micael Oliveira <micael.oliveira@mpsd.mpg.de>
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

#include "utils.h"


/******************************************************************************
 * Accessors                                                                  *
 ******************************************************************************/

_bool_set_t _bool_set(const bool value)
{
    _bool_set_t result;
    result.value = value;
    result.is_set = true;
    return result;
}

_uint_set_t _uint_set(const unsigned int value)
{
    _uint_set_t result;
    result.value = value;
    result.is_set = true;
    return result;
}

_int_set_t _int_set(const int value)
{
    _int_set_t result;
    result.value = value;
    result.is_set = true;
    return result;
}

_double_set_t _double_set(const double value)
{
    _double_set_t result;
    result.value = value;
    result.is_set = true;
    return result;
}
