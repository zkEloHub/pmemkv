# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2020, Intel Corporation

include(${SRC_DIR}/../helpers.cmake)
include(${SRC_DIR}/../engines/pmemobj_based/helpers.cmake)

setup()

pmempool_execute(create -l ${LAYOUT} -s ${DB_SIZE} obj ${DIR}/testfile)
execute(${TEST_EXECUTABLE} ${ENGINE} ${DIR}/testfile ${PARAMS})

finish()
