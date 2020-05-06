// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020, Intel Corporation */

#include <libpmemkv.h>

#include "unittest.h"

#include <stdlib.h>
#include <string.h>

static const int INIT_VAL = 1;
static const int DELETED_VAL = 2;

struct custom_type {
	int a;
	char b;
};

static void deleter(struct custom_type *ct_ptr)
{
	ct_ptr->a = DELETED_VAL;
	ct_ptr->b = DELETED_VAL;
}

static void simple_test()
{
	pmemkv_config *config = pmemkv_config_new();
	UT_ASSERT(config != NULL);

	int ret = pmemkv_config_put_string(config, "string", "abc");
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	ret = pmemkv_config_put_int64(config, "int", 123);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	struct custom_type *ptr = malloc(sizeof(struct custom_type));
	ptr->a = INIT_VAL;
	ptr->b = INIT_VAL;
	ret = pmemkv_config_put_object(config, "object_ptr", ptr, NULL);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	ret = pmemkv_config_put_data(config, "object", ptr, sizeof(*ptr));
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	struct custom_type *ptr_deleter = malloc(sizeof(struct custom_type));
	ptr_deleter->a = INIT_VAL;
	ptr_deleter->b = INIT_VAL;
	ret = pmemkv_config_put_object(config, "object_ptr_with_deleter", ptr_deleter,
				       (void (*)(void *)) & deleter);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	const char *value_string;
	ret = pmemkv_config_get_string(config, "string", &value_string);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERT(strcmp(value_string, "abc") == 0);

	int64_t value_int;
	ret = pmemkv_config_get_int64(config, "int", &value_int);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(value_int, 123);

	struct custom_type *value_custom_ptr;
	ret = pmemkv_config_get_object(config, "object_ptr", (void **)&value_custom_ptr);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(value_custom_ptr->a, INIT_VAL);
	UT_ASSERTeq(value_custom_ptr->b, INIT_VAL);

	struct custom_type *value_custom_ptr_deleter;
	ret = pmemkv_config_get_object(config, "object_ptr_with_deleter",
				       (void **)&value_custom_ptr_deleter);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(value_custom_ptr_deleter->a, INIT_VAL);
	UT_ASSERTeq(value_custom_ptr_deleter->b, INIT_VAL);

	struct custom_type *value_custom;
	size_t value_custom_size;
	ret = pmemkv_config_get_data(config, "object", (const void **)&value_custom,
				     &value_custom_size);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(value_custom_size, sizeof(value_custom));
	UT_ASSERTeq(value_custom->a, INIT_VAL);
	UT_ASSERTeq(value_custom->b, INIT_VAL);

	int64_t none;
	UT_ASSERTeq(pmemkv_config_get_int64(config, "non-existent", &none),
		    PMEMKV_STATUS_NOT_FOUND);

	pmemkv_config_delete(config);
	config = NULL;

	UT_ASSERTeq(value_custom_ptr_deleter->a, DELETED_VAL);
	UT_ASSERTeq(value_custom_ptr_deleter->b, DELETED_VAL);

	/* deleter was nullptr */
	UT_ASSERTeq(ptr, value_custom_ptr);
	UT_ASSERTeq(value_custom_ptr->a, INIT_VAL);
	UT_ASSERTeq(value_custom_ptr->b, INIT_VAL);

	free(ptr);
	free(ptr_deleter);
}

static void integral_conversion_test()
{
	pmemkv_config *config = pmemkv_config_new();
	UT_ASSERT(config != NULL);

	int ret = pmemkv_config_put_int64(config, "int", 123);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	ret = pmemkv_config_put_uint64(config, "uint", 123);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	ret = pmemkv_config_put_int64(config, "negative-int", -123);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	ret = pmemkv_config_put_uint64(config, "uint-max", (uint64_t)-1);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);

	int64_t int_s;
	ret = pmemkv_config_get_int64(config, "int", &int_s);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(int_s, 123);

	size_t int_us;
	ret = pmemkv_config_get_uint64(config, "int", &int_us);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(int_us, 123U);

	int64_t uint_s;
	ret = pmemkv_config_get_int64(config, "uint", &uint_s);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(uint_s, 123);

	size_t uint_us;
	ret = pmemkv_config_get_uint64(config, "uint", &uint_us);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(uint_us, 123U);

	int64_t neg_int_s;
	ret = pmemkv_config_get_int64(config, "negative-int", &neg_int_s);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(neg_int_s, -123);

	size_t neg_int_us;
	ret = pmemkv_config_get_uint64(config, "negative-int", &neg_int_us);
	UT_ASSERTeq(ret, PMEMKV_STATUS_CONFIG_TYPE_ERROR);

	int64_t uint_max_s;
	ret = pmemkv_config_get_int64(config, "uint-max", &uint_max_s);
	UT_ASSERTeq(ret, PMEMKV_STATUS_CONFIG_TYPE_ERROR);

	size_t uint_max_us;
	ret = pmemkv_config_get_uint64(config, "uint-max", &uint_max_us);
	UT_ASSERTeq(ret, PMEMKV_STATUS_OK);
	UT_ASSERTeq(uint_max_us, ((uint64_t)-1));

	pmemkv_config_delete(config);
}

static void not_found_test()
{
	pmemkv_config *config = pmemkv_config_new();
	UT_ASSERT(config != NULL);

	/* all gets should return NotFound when looking for non-existing key */
	const char *my_string;
	int ret = pmemkv_config_get_string(config, "non-existent-string", &my_string);
	UT_ASSERTeq(ret, PMEMKV_STATUS_NOT_FOUND);

	int64_t my_int;
	ret = pmemkv_config_get_int64(config, "non-existent-int", &my_int);
	UT_ASSERTeq(ret, PMEMKV_STATUS_NOT_FOUND);

	size_t my_uint;
	ret = pmemkv_config_get_uint64(config, "non-existent-uint", &my_uint);
	UT_ASSERTeq(ret, PMEMKV_STATUS_NOT_FOUND);

	struct custom_type *my_object;
	ret = pmemkv_config_get_object(config, "non-existent-object",
				       (void **)&my_object);
	UT_ASSERTeq(ret, PMEMKV_STATUS_NOT_FOUND);

	size_t my_object_size = 0;
	ret = pmemkv_config_get_data(config, "non-existent-data",
				     (const void **)&my_object, &my_object_size);
	UT_ASSERTeq(ret, PMEMKV_STATUS_NOT_FOUND);
	UT_ASSERTeq(my_object_size, 0U);

	pmemkv_config_delete(config);
}

/* Test if null can be passed as config to pmemkv_config_* functions */
static void null_config_test()
{
	pmemkv_config *config = pmemkv_config_new();
	UT_ASSERT(config != NULL);

	int ret = pmemkv_config_put_string(NULL, "string", "abc");
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	ret = pmemkv_config_put_int64(NULL, "int", 123);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	struct custom_type *ptr = malloc(sizeof(struct custom_type));
	ptr->a = INIT_VAL;
	ptr->b = INIT_VAL;
	ret = pmemkv_config_put_object(NULL, "object_ptr", ptr, NULL);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	ret = pmemkv_config_put_data(NULL, "object", ptr, sizeof(*ptr));
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	const char *value_string;
	ret = pmemkv_config_get_string(NULL, "string", &value_string);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	int64_t value_int;
	ret = pmemkv_config_get_int64(NULL, "int", &value_int);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	struct custom_type *value_custom_ptr;
	ret = pmemkv_config_get_object(NULL, "object_ptr", (void **)&value_custom_ptr);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	struct custom_type *value_custom;
	size_t value_custom_size;
	ret = pmemkv_config_get_data(NULL, "object", (const void **)&value_custom,
				     &value_custom_size);
	UT_ASSERTeq(ret, PMEMKV_STATUS_INVALID_ARGUMENT);

	free(ptr);

	pmemkv_config_delete(config);
}

int main(int argc, char *argv[])
{
	START();

	simple_test();
	integral_conversion_test();
	not_found_test();
	null_config_test();
}
