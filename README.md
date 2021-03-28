# Abandone aka Archived

Just by looking into the code SQLite wins. Although Level DB is not fully conformant SQL database. Still there is no comparison.

Also Wikipedia [Leveld DB article](https://en.wikipedia.org/wiki/LevelDB) makes me wonder. This looks to me like Jeff and Sanjay "playground project". The worst (for me) is it is C++ and not C code.

Perhaps you need to [read this](https://en.wikipedia.org/wiki/Lightning_Memory-Mapped_Database) before deciding on Level DB. LMDB seems the way to go. Funny thing is I have not found any Jeff or Sanjay comment on LMDB yet?

# LevelDB 1.18 
## VisualStudio Solution

Google code with very minimal "make it run" adjustments.

 For TESTING purposes, this code depends on [UBUT](https://github.com/dbj-data/ubut) part of the [MACHINE_WIDE](https://github.com/dbj-data/machine_wide) dbj suite.

 ## Feasibility

 Basically this is "counter effort" :) I "need" to prove implementing KV storage on top of SQLite as the most feasible solution. Not performant buf feasible. Mind you, currently and right now, I have very minimal knowledge about this API.

 Only C API is used, as defined in [`leveldb/c.h`](leveldb_code/leveldb/c.h)

 `leveldb` is built as static lib. Release build optimizations might come latter.

