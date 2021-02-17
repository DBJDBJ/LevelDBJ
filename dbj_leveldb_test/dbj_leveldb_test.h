#pragma once

#include <ubut/utest.h>
#include "../leveldb_code/leveldb/c.h"

static struct {
	const char* db_file_name;
	leveldb_t* db;
	leveldb_options_t* options;
} dbj_testing_metadata = { "test1.db", NULL };

inline static void before_testing(void)
{
	if (dbj_testing_metadata.db != NULL) return;

	dbj_testing_metadata.options = leveldb_options_create();

	leveldb_options_set_create_if_missing(
		dbj_testing_metadata.options, 1 /* why not std bool? */
	);

	char* errptr = NULL;
	dbj_testing_metadata.db = leveldb_open(
		dbj_testing_metadata.options,
		dbj_testing_metadata.db_file_name,
		&errptr);

	if (errptr) {
		perror(errptr);
		exit(0);
	}

}

inline static void after_testing(void)
{
	leveldb_close(dbj_testing_metadata.db);
}


// [TestMethod]
UTEST(dbj_level_db, TestCreateDisposeDatabase)
{
	EXPECT_TRUE(NULL != dbj_testing_metadata.db);
}

inline void write_foo_bar(char** errptr)
{
	leveldb_writeoptions_t* options = leveldb_writeoptions_create();
	leveldb_writeoptions_set_sync(options, 1);

	const char* key = "Foo";
	const char* val = "Bar";

	_ASSERTE(*errptr == NULL);

	/*void */leveldb_put(
		dbj_testing_metadata.db,
		options,
		key, sizeof(key),
		val, sizeof(val),
		errptr);
}
// [TestMethod]
UTEST(dbj_level_db, TestCreatePutDisposeDatabase)
{
	char* errptr = NULL;

	write_foo_bar(&errptr);

	EXPECT_TRUE(errptr == NULL);

	if (errptr) {
		perror(errptr);
		exit(0);
	}
}

// [TestMethod]
UTEST(dbj_level_db, TestReadWriteDatabase)
{
	char* errptr = NULL;
	write_foo_bar(&errptr);
	EXPECT_TRUE(errptr == NULL);

	/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
	size_t vallen = 0U;
	char* rezult_ = leveldb_get(
		dbj_testing_metadata.db,
		leveldb_readoptions_create(),
		"Foo", sizeof("Foo"),
		&vallen,
		&errptr);

	EXPECT_TRUE(rezult_ != NULL);

	if (rezult_) free(rezult_);
}

// [TestMethod]
UTEST(dbj_level_db, TestReadMissingEntryDatabase)
{
	char* errptr = NULL;
	write_foo_bar(&errptr);
	EXPECT_TRUE(errptr == NULL);

	/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
	size_t vallen = 0U;
	char* rezult_ = leveldb_get(
		dbj_testing_metadata.db,
		leveldb_readoptions_create(),
		"Boo", sizeof("Boo"),
		&vallen,
		&errptr);

	EXPECT_TRUE(rezult_ == NULL);

	if (rezult_) free(rezult_);
}

#if 0
// [TestMethod]
UTEST(dbj_level_db, TestReadFromSnapshotOfDatabase)
{
	/* dbj_testing_metadata.db */
	{
		db.Put(new WriteOptions(), Slice.FromString("Foo"), Slice.FromString("Bar"));
		/* defer */ (var snapshot = db.GetSnapshot())
		{
			var opts = new ReadOptions{ Snapshot = snapshot };
			EXPECT_TRUE(NULL != db.Get(opts, Slice.FromString("Foo")));
			EXPECT_TRUE(db.Delete(new WriteOptions(), Slice.FromString("Foo")));
			EXPECT_TRUE(NULL != db.Get(opts, Slice.FromString("Foo")));
		}
	}
}

// [TestMethod]
UTEST(dbj_level_db, TestIteratorWithSnapshot()
{
	/* defer */ (var db = new DB(new Options{ CreateIfMissing = true }, "test2.db"))
{
	/* defer */ (var batch = new WriteBatch())
	{
		for (int i = 0; i < 100; i++)
		{
			batch.Put(Slice.FromString($"key::{i,20:D20}"), Slice.FromString($"{i,32}"));
		}

		db.Write(new WriteOptions{ Sync = true }, batch);
	}

	/* defer */ (var snapshot = db.GetSnapshot())
		/* defer */ (var batch = new WriteBatch())
		/* defer */ (var itr = db.NewIterator(new ReadOptions{ Snapshot = snapshot }))
	{
		itr.Seek(Slice.FromString("key::"));
		EXPECT_TRUE(itr.Valid());
		int entriesDeleted = 0;
		while (itr.Valid())
		{
			batch.Delete(itr.Key());
			itr.Next();
			entriesDeleted++;
		}

		db.Write(new WriteOptions(), batch);
		Assert.AreEqual(100, entriesDeleted);
	}
}
}

// [TestMethod]
UTEST(dbj_level_db, TestDeleteEntryFromDatabase)
{
	/* dbj_testing_metadata.db */
	{
		db.Put(new WriteOptions(), Slice.FromString("Foo"), Slice.FromString("Bar"));
		EXPECT_TRUE(NULL != db.Get(new ReadOptions(), Slice.FromString("Foo")));
		EXPECT_TRUE(db.Delete(new WriteOptions(), Slice.FromString("Foo")));
		EXPECT_TRUE(NULL == db.Get(new ReadOptions(), Slice.FromString("Foo")));
	}
}

#endif // 0

