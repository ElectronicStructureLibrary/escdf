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

#include <check.h>

#include "utils.h"

START_TEST(test_set_bool)
{
  _bool_set_t var = _bool_set(false);
  ck_assert(var.is_set == true);
  ck_assert(var.value == false);
}
END_TEST

START_TEST(test_set_uint)
{
    _uint_set_t var = _uint_set(1);
    ck_assert(var.is_set == true);
    ck_assert(var.value == 1);
}
END_TEST

START_TEST(test_set_int)
{
    _int_set_t var = _int_set(2);
    ck_assert(var.is_set == true);
    ck_assert(var.value == 2);
}
END_TEST

START_TEST(test_set_double)
{
    _double_set_t var = _double_set(3.0);
    ck_assert(var.is_set == true);
    ck_assert(var.value == 3.0);
}
END_TEST


Suite * make_utils_suite(void)
{
    Suite *s;
    TCase *tc_set;

    s = suite_create("Utils");

    tc_set = tcase_create("Setters");
    tcase_add_test(tc_set, test_set_bool);
    tcase_add_test(tc_set, test_set_uint);
    tcase_add_test(tc_set, test_set_int);
    tcase_add_test(tc_set, test_set_double);
    suite_add_tcase(s, tc_set);

    return s;
}
