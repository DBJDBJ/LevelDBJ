// dbj_leveldb_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "dbj_leveldb_test.h"

UTEST_STATE;   // <-- note there is no ()! 

int main(int argc, const char* const argv[])
{
	before_testing();

	int retval_ = utest_main(argc, argv);

	after_testing();

	return retval_;
}

//int main(void)
//{
//	/* Return the major version number for this release. */
//	int major = leveldb_major_version();
//
//	/* Return the minor version number for this release. */
//	int minor = leveldb_minor_version();
//
//	printf("\n\nLEVEL DB %d.%d", major, minor);
//
//	leveldb_env_t* ldb_env = leveldb_create_default_env();
//
//	leveldb_options_t* opts = leveldb_options_create();
//
//	leveldb_options_set_env(opts, ldb_env);
//
//	leveldb_options_destroy(opts);
//	leveldb_env_destroy(ldb_env);
//
//}

