//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "rocksdb/options.h"
#include "rocksdb/immutable_options.h"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <limits>

#include "rocksdb/cache.h"
#include "rocksdb/compaction_filter.h"
#include "rocksdb/comparator.h"
#include "rocksdb/env.h"
#include "rocksdb/sst_file_manager.h"
#include "rocksdb/memtablerep.h"
#include "rocksdb/merge_operator.h"
#include "rocksdb/slice.h"
#include "rocksdb/slice_transform.h"
#include "rocksdb/table.h"
#include "rocksdb/table_properties.h"
#include "rocksdb/wal_filter.h"
#include "table/block_based_table_factory.h"
#include "util/compression.h"
#include "util/statistics.h"
#include "util/xfunc.h"

namespace rocksdb {

ImmutableCFOptions::ImmutableCFOptions(const Options& options)
    : compaction_style(options.compaction_style),
      compaction_options_universal(options.compaction_options_universal),
      compaction_options_fifo(options.compaction_options_fifo),
      prefix_extractor(options.prefix_extractor.get()),
      comparator(options.comparator),
      merge_operator(options.merge_operator.get()),
      compaction_filter(options.compaction_filter),
      compaction_filter_factory(options.compaction_filter_factory.get()),
      inplace_update_support(options.inplace_update_support),
      inplace_callback(options.inplace_callback),
      info_log(options.info_log.get()),
      statistics(options.statistics.get()),
      env(options.env),
      delayed_write_rate(options.delayed_write_rate),
      allow_mmap_reads(options.allow_mmap_reads),
      allow_mmap_writes(options.allow_mmap_writes),
      db_paths(options.db_paths),
      memtable_factory(options.memtable_factory.get()),
      table_factory(options.table_factory.get()),
      table_properties_collector_factories(
          options.table_properties_collector_factories),
      advise_random_on_open(options.advise_random_on_open),
      bloom_locality(options.bloom_locality),
      purge_redundant_kvs_while_flush(options.purge_redundant_kvs_while_flush),
      min_partial_merge_operands(options.min_partial_merge_operands),
      disable_data_sync(options.disableDataSync),
      use_fsync(options.use_fsync),
      compression(options.compression),
      compression_per_level(options.compression_per_level),
      compression_opts(options.compression_opts),
      level_compaction_dynamic_level_bytes(
          options.level_compaction_dynamic_level_bytes),
      access_hint_on_compaction_start(options.access_hint_on_compaction_start),
      new_table_reader_for_compaction_inputs(
          options.new_table_reader_for_compaction_inputs),
      compaction_readahead_size(options.compaction_readahead_size),
      num_levels(options.num_levels),
      optimize_filters_for_hits(options.optimize_filters_for_hits),
      listeners(options.listeners),
      row_cache(options.row_cache) {}

ColumnFamilyOptions::ColumnFamilyOptions()
    : comparator(BytewiseComparator()),
      merge_operator(nullptr),
      compaction_filter(nullptr),
      compaction_filter_factory(nullptr),
      write_buffer_size(64 << 20),
      max_write_buffer_number(2),
      min_write_buffer_number_to_merge(1),
      max_write_buffer_number_to_maintain(0),
      compression(Snappy_Supported() ? kSnappyCompression : kNoCompression),
      prefix_extractor(nullptr),
      num_levels(7),
      level0_file_num_compaction_trigger(4),
      level0_slowdown_writes_trigger(20),
      level0_stop_writes_trigger(24),
      target_file_size_base(64 * 1048576),
      target_file_size_multiplier(1),
      max_bytes_for_level_base(256 * 1048576),
      level_compaction_dynamic_level_bytes(false),
      max_bytes_for_level_multiplier(10),
      max_bytes_for_level_multiplier_additional(num_levels, 1),
      expanded_compaction_factor(25),
      source_compaction_factor(1),
      max_grandparent_overlap_factor(10),
      soft_rate_limit(0.0),
      hard_rate_limit(0.0),
      soft_pending_compaction_bytes_limit(64 * 1073741824ul),
      hard_pending_compaction_bytes_limit(256 * 1073741824ul),
      rate_limit_delay_max_milliseconds(1000),
      arena_block_size(0),
      disable_auto_compactions(false),
      purge_redundant_kvs_while_flush(true),
      compaction_style(kCompactionStyleLevel),
      compaction_pri(kByCompensatedSize),
      verify_checksums_in_compaction(true),
      filter_deletes(false),
      max_sequential_skip_in_iterations(8),
      memtable_factory(std::shared_ptr<SkipListFactory>(new SkipListFactory)),
      table_factory(
          std::shared_ptr<TableFactory>(new BlockBasedTableFactory())),
      inplace_update_support(false),
      inplace_update_num_locks(10000),
      inplace_callback(nullptr),
      memtable_prefix_bloom_bits(0),
      memtable_prefix_bloom_probes(6),
      memtable_prefix_bloom_huge_page_tlb_size(0),
      bloom_locality(0),
      max_successive_merges(0),
      min_partial_merge_operands(2),
      optimize_filters_for_hits(false),
      paranoid_file_checks(false),
      compaction_measure_io_stats(false) {
  assert(memtable_factory.get() != nullptr);
}

ColumnFamilyOptions::ColumnFamilyOptions(const Options& options)
    : comparator(options.comparator),
      merge_operator(options.merge_operator),
      compaction_filter(options.compaction_filter),
      compaction_filter_factory(options.compaction_filter_factory),
      write_buffer_size(options.write_buffer_size),
      max_write_buffer_number(options.max_write_buffer_number),
      min_write_buffer_number_to_merge(
          options.min_write_buffer_number_to_merge),
      max_write_buffer_number_to_maintain(
          options.max_write_buffer_number_to_maintain),
      compression(options.compression),
      compression_per_level(options.compression_per_level),
      compression_opts(options.compression_opts),
      prefix_extractor(options.prefix_extractor),
      num_levels(options.num_levels),
      level0_file_num_compaction_trigger(
          options.level0_file_num_compaction_trigger),
      level0_slowdown_writes_trigger(options.level0_slowdown_writes_trigger),
      level0_stop_writes_trigger(options.level0_stop_writes_trigger),
      target_file_size_base(options.target_file_size_base),
      target_file_size_multiplier(options.target_file_size_multiplier),
      max_bytes_for_level_base(options.max_bytes_for_level_base),
      level_compaction_dynamic_level_bytes(
          options.level_compaction_dynamic_level_bytes),
      max_bytes_for_level_multiplier(options.max_bytes_for_level_multiplier),
      max_bytes_for_level_multiplier_additional(
          options.max_bytes_for_level_multiplier_additional),
      expanded_compaction_factor(options.expanded_compaction_factor),
      source_compaction_factor(options.source_compaction_factor),
      max_grandparent_overlap_factor(options.max_grandparent_overlap_factor),
      soft_rate_limit(options.soft_rate_limit),
      soft_pending_compaction_bytes_limit(
          options.soft_pending_compaction_bytes_limit),
      hard_pending_compaction_bytes_limit(
          options.hard_pending_compaction_bytes_limit),
      rate_limit_delay_max_milliseconds(
          options.rate_limit_delay_max_milliseconds),
      arena_block_size(options.arena_block_size),
      disable_auto_compactions(options.disable_auto_compactions),
      purge_redundant_kvs_while_flush(options.purge_redundant_kvs_while_flush),
      compaction_style(options.compaction_style),
      compaction_pri(options.compaction_pri),
      verify_checksums_in_compaction(options.verify_checksums_in_compaction),
      compaction_options_universal(options.compaction_options_universal),
      compaction_options_fifo(options.compaction_options_fifo),
      filter_deletes(options.filter_deletes),
      max_sequential_skip_in_iterations(
          options.max_sequential_skip_in_iterations),
      memtable_factory(options.memtable_factory),
      table_factory(options.table_factory),
      table_properties_collector_factories(
          options.table_properties_collector_factories),
      inplace_update_support(options.inplace_update_support),
      inplace_update_num_locks(options.inplace_update_num_locks),
      inplace_callback(options.inplace_callback),
      memtable_prefix_bloom_bits(options.memtable_prefix_bloom_bits),
      memtable_prefix_bloom_probes(options.memtable_prefix_bloom_probes),
      memtable_prefix_bloom_huge_page_tlb_size(
          options.memtable_prefix_bloom_huge_page_tlb_size),
      bloom_locality(options.bloom_locality),
      max_successive_merges(options.max_successive_merges),
      min_partial_merge_operands(options.min_partial_merge_operands),
      optimize_filters_for_hits(options.optimize_filters_for_hits),
      paranoid_file_checks(options.paranoid_file_checks),
      compaction_measure_io_stats(options.compaction_measure_io_stats) {
  assert(memtable_factory.get() != nullptr);
  if (max_bytes_for_level_multiplier_additional.size() <
      static_cast<unsigned int>(num_levels)) {
    max_bytes_for_level_multiplier_additional.resize(num_levels, 1);
  }
}

ColumnFamilyOptions::ColumnFamilyOptions(const ColumnFamilyOptions& options)
    : comparator(options.comparator),
      merge_operator(options.merge_operator),
      compaction_filter(options.compaction_filter),
      compaction_filter_factory(options.compaction_filter_factory),
      write_buffer_size(options.write_buffer_size),
      max_write_buffer_number(options.max_write_buffer_number),
      min_write_buffer_number_to_merge(
          options.min_write_buffer_number_to_merge),
      max_write_buffer_number_to_maintain(
          options.max_write_buffer_number_to_maintain),
      compression(options.compression),
      compression_per_level(options.compression_per_level),
      compression_opts(options.compression_opts),
      prefix_extractor(options.prefix_extractor),
      num_levels(options.num_levels),
      level0_file_num_compaction_trigger(
          options.level0_file_num_compaction_trigger),
      level0_slowdown_writes_trigger(options.level0_slowdown_writes_trigger),
      level0_stop_writes_trigger(options.level0_stop_writes_trigger),
      target_file_size_base(options.target_file_size_base),
      target_file_size_multiplier(options.target_file_size_multiplier),
      max_bytes_for_level_base(options.max_bytes_for_level_base),
      level_compaction_dynamic_level_bytes(
          options.level_compaction_dynamic_level_bytes),
      max_bytes_for_level_multiplier(options.max_bytes_for_level_multiplier),
      max_bytes_for_level_multiplier_additional(
          options.max_bytes_for_level_multiplier_additional),
      expanded_compaction_factor(options.expanded_compaction_factor),
      source_compaction_factor(options.source_compaction_factor),
      max_grandparent_overlap_factor(options.max_grandparent_overlap_factor),
      soft_rate_limit(options.soft_rate_limit),
      soft_pending_compaction_bytes_limit(
          options.soft_pending_compaction_bytes_limit),
      hard_pending_compaction_bytes_limit(
          options.hard_pending_compaction_bytes_limit),
      rate_limit_delay_max_milliseconds(
          options.rate_limit_delay_max_milliseconds),
      arena_block_size(options.arena_block_size),
      disable_auto_compactions(options.disable_auto_compactions),
      purge_redundant_kvs_while_flush(options.purge_redundant_kvs_while_flush),
      compaction_style(options.compaction_style),
      compaction_pri(options.compaction_pri),
      verify_checksums_in_compaction(options.verify_checksums_in_compaction),
      compaction_options_universal(options.compaction_options_universal),
      compaction_options_fifo(options.compaction_options_fifo),
      filter_deletes(options.filter_deletes),
      max_sequential_skip_in_iterations(
          options.max_sequential_skip_in_iterations),
      memtable_factory(options.memtable_factory),
      table_factory(options.table_factory),
      table_properties_collector_factories(
          options.table_properties_collector_factories),
      inplace_update_support(options.inplace_update_support),
      inplace_update_num_locks(options.inplace_update_num_locks),
      inplace_callback(options.inplace_callback),
      memtable_prefix_bloom_bits(options.memtable_prefix_bloom_bits),
      memtable_prefix_bloom_probes(options.memtable_prefix_bloom_probes),
      memtable_prefix_bloom_huge_page_tlb_size(
          options.memtable_prefix_bloom_huge_page_tlb_size),
      bloom_locality(options.bloom_locality),
      max_successive_merges(options.max_successive_merges),
      min_partial_merge_operands(options.min_partial_merge_operands),
      optimize_filters_for_hits(options.optimize_filters_for_hits),
      paranoid_file_checks(options.paranoid_file_checks),
      compaction_measure_io_stats(options.compaction_measure_io_stats) {
  assert(memtable_factory.get() != nullptr);
  if (max_bytes_for_level_multiplier_additional.size() <
      static_cast<unsigned int>(num_levels)) {
    max_bytes_for_level_multiplier_additional.resize(num_levels, 1);
  }
}

DBOptions::DBOptions()
    : create_if_missing(false),
      create_missing_column_families(false),
      error_if_exists(false),
      paranoid_checks(true),
      env(Env::Default()),
      rate_limiter(nullptr),
      sst_file_manager(nullptr),
      info_log(nullptr),
#ifdef NDEBUG
      info_log_level(INFO_LEVEL),
#else
      info_log_level(DEBUG_LEVEL),
#endif  // NDEBUG
      max_open_files(5000),
      max_file_opening_threads(16),
      max_total_wal_size(0),
      statistics(nullptr),
      disableDataSync(false),
      use_fsync(false),
      db_log_dir(""),
      wal_dir(""),
      delete_obsolete_files_period_micros(6ULL * 60 * 60 * 1000000),
      base_background_compactions(-1),
      max_background_compactions(1),
      max_subcompactions(1),
      max_background_flushes(1),
      max_log_file_size(0),
      log_file_time_to_roll(0),
      keep_log_file_num(1000),
      recycle_log_file_num(0),
      max_manifest_file_size(std::numeric_limits<uint64_t>::max()),
      table_cache_numshardbits(6),
      WAL_ttl_seconds(0),
      WAL_size_limit_MB(0),
      manifest_preallocation_size(4 * 1024 * 1024),
      allow_os_buffer(true),
      allow_mmap_reads(false),
      allow_mmap_writes(false),
      allow_fallocate(true),
      is_fd_close_on_exec(true),
      skip_log_error_on_recovery(false),
      stats_dump_period_sec(600),
      advise_random_on_open(true),
      db_write_buffer_size(0),
      access_hint_on_compaction_start(NORMAL),
      new_table_reader_for_compaction_inputs(false),
      compaction_readahead_size(0),
      random_access_max_buffer_size(1024 * 1024),
      writable_file_max_buffer_size(1024 * 1024),
      use_adaptive_mutex(false),
      bytes_per_sync(0),
      wal_bytes_per_sync(0),
      listeners(),
      enable_thread_tracking(false),
      delayed_write_rate(2 * 1024U * 1024U),
      allow_concurrent_memtable_write(false),
      enable_write_thread_adaptive_yield(false),
      write_thread_max_yield_usec(100),
      write_thread_slow_yield_usec(3),
      skip_stats_update_on_db_open(false),
      wal_recovery_mode(WALRecoveryMode::kTolerateCorruptedTailRecords),
      row_cache(nullptr),
#ifndef ROCKSDB_LITE
      wal_filter(nullptr),
#endif  // ROCKSDB_LITE
      fail_if_options_file_error(false) {
}

DBOptions::DBOptions(const DBOptions& options)
{
    // If true, the database will be created if it is missing.
    // Default: false
    create_if_missing = options.create_if_missing;

    // If true, missing column families will be automatically created.
    // Default: false
    create_missing_column_families = options.create_missing_column_families;

    // If true, an error is raised if the database already exists.
    // Default: false
    error_if_exists = options.error_if_exists;

    // If true, RocksDB will aggressively check consistency of the data.
    // Also, if any of the  writes to the database fails (Put, Delete, Merge,
    // Write), the database will switch to read-only mode and fail all other
    // Write operations.
    // In most cases you want this to be set to true.
    // Default: true
    paranoid_checks = options.paranoid_checks;

    // Use the specified object to interact with the environment,
    // e.g. to read/write files, schedule background work, etc.
    // Default: Env::Default()
    env = options.env;

    // Use to control write rate of flush and compaction. Flush has higher
    // priority than compaction. Rate limiting is disabled if nullptr.
    // If rate limiter is enabled, bytes_per_sync is set to 1MB by default.
    // Default: nullptr
    rate_limiter = options.rate_limiter;

    // Use to track SST files and control their file deletion rate.
    //
    // Features:
    //  - Throttle the deletion rate of the SST files.
    //  - Keep track the total size of all SST files.
    //  - Set a maximum allowed space limit for SST files that when reached
    //    the DB wont do any further flushes or compactions and will set the
    //    background error.
    //  - Can be shared between multiple dbs.
    // Limitations:
    //  - Only track and throttle deletes of SST files in
    //    first db_path (db_name if db_paths is empty).
    //
    // Default: nullptr
    sst_file_manager = options.sst_file_manager;

    // Any internal progress/error information generated by the db will
    // be written to info_log if it is non-nullptr, or to a file stored
    // in the same directory as the DB contents if info_log is nullptr.
    // Default: nullptr
    info_log = options.info_log;

    info_log_level = options.info_log_level;

    // Number of open files that can be used by the DB.  You may need to
    // increase this if your database has a large working set. Value -1 means
    // files opened are always kept open. You can estimate number of files based
    // on target_file_size_base and target_file_size_multiplier for level-based
    // compaction. For universal-style compaction, you can usually set it to -1.
    // Default: 5000 or ulimit value of max open files (whichever is smaller)
    max_open_files = options.max_open_files;

    // If max_open_files is -1, DB will open all files on DB::Open(). You can
    // use this option to increase the number of threads used to open the files.
    // Default: 1
    max_file_opening_threads = options.max_file_opening_threads;

    // Once write-ahead logs exceed this size, we will start forcing the flush of
    // column families whose memtables are backed by the oldest live WAL file
    // (i.e. the ones that are causing all the space amplification). If set to 0
    // (default), we will dynamically choose the WAL size limit to be
    // [sum of all write_buffer_size * max_write_buffer_number] * 4
    // Default: 0
    max_total_wal_size = options.max_total_wal_size;

    // If non-null, then we should collect metrics about database operations
    // Statistics objects should not be shared between DB instances as
    // it does not use any locks to prevent concurrent updates.
    statistics = options.statistics;

    // If true, then the contents of manifest and data files are not synced
    // to stable storage. Their contents remain in the OS buffers till the
    // OS decides to flush them. This option is good for bulk-loading
    // of data. Once the bulk-loading is complete, please issue a
    // sync to the OS to flush all dirty buffesrs to stable storage.
    // Default: false
    disableDataSync = options.disableDataSync;

    // If true, then every store to stable storage will issue a fsync.
    // If false, then every store to stable storage will issue a fdatasync.
    // This parameter should be set to true while storing data to
    // filesystem like ext3 that can lose files after a reboot.
    // Default: false
    use_fsync = options.use_fsync;

    // A list of paths where SST files can be put into, with its target size.
    // Newer data is placed into paths specified earlier in the vector while
    // older data gradually moves to paths specified later in the vector.
    //
    // For example, you have a flash device with 10GB allocated for the DB,
    // as well as a hard drive of 2TB, you should config it to be:
    //   [{"/flash_path", 10GB}, {"/hard_drive", 2TB}]
    //
    // The system will try to guarantee data under each path is close to but
    // not larger than the target size. But current and future file sizes used
    // by determining where to place a file are based on best-effort estimation,
    // which means there is a chance that the actual size under the directory
    // is slightly more than target size under some workloads. User should give
    // some buffer room for those cases.
    //
    // If none of the paths has sufficient room to place a file, the file will
    // be placed to the last path anyway, despite to the target size.
    //
    // Placing newer data to earlier paths is also best-efforts. User should
    // expect user files to be placed in higher levels in some extreme cases.
    //
    // If left empty, only one path will be used, which is db_name passed when
    // opening the DB.
    // Default: empty
    db_paths = options.db_paths;

    // This specifies the info LOG dir.
    // If it is empty, the log files will be in the same dir as data.
    // If it is non empty, the log files will be in the specified dir,
    // and the db data dir's absolute path will be used as the log file
    // name's prefix.
    db_log_dir = options.db_log_dir;

    // This specifies the absolute dir path for write-ahead logs (WAL).
    // If it is empty, the log files will be in the same dir as data,
    //   dbname is used as the data dir by default
    // If it is non empty, the log files will be in kept the specified dir.
    // When destroying the db,
    //   all log files in wal_dir and the dir itself is deleted
    wal_dir = options.wal_dir;

    // The periodicity when obsolete files get deleted. The default
    // value is 6 hours. The files that get out of scope by compaction
    // process will still get automatically delete on every compaction,
    // regardless of this setting
    delete_obsolete_files_period_micros = options.delete_obsolete_files_period_micros;

    // Suggested number of concurrent background compaction jobs, submitted to
    // the default LOW priority thread pool.
    //
    // Default: max_background_compactions
    base_background_compactions = options.base_background_compactions;

    // Maximum number of concurrent background compaction jobs, submitted to
    // the default LOW priority thread pool.
    // We first try to schedule compactions based on
    // `base_background_compactions`. If the compaction cannot catch up , we
    // will increase number of compaction threads up to
    // `max_background_compactions`.
    //
    // If you're increasing this, also consider increasing number of threads in
    // LOW priority thread pool. For more information, see
    // Env::SetBackgroundThreads
    // Default: 1
    max_background_compactions = options.max_background_compactions;

    // This value represents the maximum number of threads that will
    // concurrently perform a compaction job by breaking it into multiple,
    // smaller ones that are run simultaneously.
    // Default: 1 (i.e. no subcompactions)
    max_subcompactions = options.max_subcompactions;

    // Maximum number of concurrent background memtable flush jobs, submitted to
    // the HIGH priority thread pool.
    //
    // By default, all background jobs (major compaction and memtable flush) go
    // to the LOW priority pool. If this option is set to a positive number,
    // memtable flush jobs will be submitted to the HIGH priority pool.
    // It is important when the same Env is shared by multiple db instances.
    // Without a separate pool, long running major compaction jobs could
    // potentially block memtable flush jobs of other db instances, leading to
    // unnecessary Put stalls.
    //
    // If you're increasing this, also consider increasing number of threads in
    // HIGH priority thread pool. For more information, see
    // Env::SetBackgroundThreads
    // Default: 1
    max_background_flushes = options.max_background_flushes;

    // Specify the maximal size of the info log file. If the log file
    // is larger than `max_log_file_size`, a new info log file will
    // be created.
    // If max_log_file_size == 0, all logs will be written to one
    // log file.
    max_log_file_size = options.max_log_file_size;

    // Time for the info log file to roll (in seconds).
    // If specified with non-zero value, log file will be rolled
    // if it has been active longer than `log_file_time_to_roll`.
    // Default: 0 (disabled)
    log_file_time_to_roll = options.log_file_time_to_roll;

    // Maximal info log files to be kept.
    // Default: 1000
    keep_log_file_num = options.keep_log_file_num;

    // Recycle log files.
    // If non-zero, we will reuse previously written log files for new
    // logs, overwriting the old data.  The value indicates how many
    // such files we will keep around at any point in time for later
    // use.  This is more efficient because the blocks are already
    // allocated and fdatasync does not need to update the inode after
    // each write.
    // Default: 0
    recycle_log_file_num = options.recycle_log_file_num;

    // manifest file is rolled over on reaching this limit.
    // The older manifest file be deleted.
    // The default value is MAX_INT so that roll-over does not take place.
    max_manifest_file_size = options.max_manifest_file_size;

    // Number of shards used for table cache.
    table_cache_numshardbits = options.table_cache_numshardbits;

    // DEPRECATED
    // int table_cache_remove_scan_count_limit;

    // The following two fields affect how archived logs will be deleted.
    // 1. If both set to 0, logs will be deleted asap and will not get into
    //    the archive.
    // 2. If WAL_ttl_seconds is 0 and WAL_size_limit_MB is not 0,
    //    WAL files will be checked every 10 min and if total size is greater
    //    then WAL_size_limit_MB, they will be deleted starting with the
    //    earliest until size_limit is met. All empty files will be deleted.
    // 3. If WAL_ttl_seconds is not 0 and WAL_size_limit_MB is 0, then
    //    WAL files will be checked every WAL_ttl_secondsi / 2 and those that
    //    are older than WAL_ttl_seconds will be deleted.
    // 4. If both are not 0, WAL files will be checked every 10 min and both
    //    checks will be performed with ttl being first.
    WAL_ttl_seconds = options.WAL_ttl_seconds;
    WAL_size_limit_MB = options.WAL_size_limit_MB;

    // Number of bytes to preallocate (via fallocate) the manifest
    // files.  Default is 4mb, which is reasonable to reduce random IO
    // as well as prevent overallocation for mounts that preallocate
    // large amounts of data (such as xfs's allocsize option).
    manifest_preallocation_size = options.manifest_preallocation_size;

    // Hint the OS that it should not buffer disk I/O. Enabling this
    // parameter may improve performance but increases pressure on the
    // system cache.
    //
    // The exact behavior of this parameter is platform dependent.
    //
    // On POSIX systems, after RocksDB reads data from disk it will
    // mark the pages as "unneeded". The operating system may - or may not
    // - evict these pages from memory, reducing pressure on the system
    // cache. If the disk block is requested again this can result in
    // additional disk I/O.
    //
    // On WINDOWS system, files will be opened in "unbuffered I/O" mode
    // which means that data read from the disk will not be cached or
    // bufferized. The hardware buffer of the devices may however still
    // be used. Memory mapped files are not impacted by this parameter.
    //
    // Default: true
    allow_os_buffer = options.allow_os_buffer;

    // Allow the OS to mmap file for reading sst tables. Default: false
    allow_mmap_reads = options.allow_mmap_reads;

    // Allow the OS to mmap file for writing.
    // DB::SyncWAL() only works if this is set to false.
    // Default: false
    allow_mmap_writes = options.allow_mmap_writes;

    // If false, fallocate() calls are bypassed
    allow_fallocate = options.allow_fallocate;

    // Disable child process inherit open files. Default: true
    is_fd_close_on_exec = options.is_fd_close_on_exec;

    // DEPRECATED -- this options is no longer used
    skip_log_error_on_recovery = options.skip_log_error_on_recovery;

    // if not zero, dump rocksdb.stats to LOG every stats_dump_period_sec
    // Default: 600 (10 min)
    stats_dump_period_sec = options.stats_dump_period_sec;

    // If set true, will hint the underlying file system that the file
    // access pattern is random, when a sst file is opened.
    // Default: true
    advise_random_on_open = options.advise_random_on_open;

    // Amount of data to build up in memtables across all column
    // families before writing to disk.
    //
    // This is distinct from write_buffer_size, which enforces a limit
    // for a single memtable.
    //
    // This feature is disabled by default. Specify a non-zero value
    // to enable it.
    //
    // Default: 0 (disabled)
    db_write_buffer_size = options.db_write_buffer_size;

    // Specify the file access pattern once a compaction is started.
    // It will be applied to all input files of a compaction.
    // Default: NORMAL
    access_hint_on_compaction_start = options.access_hint_on_compaction_start;

    // If true, always create a new file descriptor and new table reader
    // for compaction inputs. Turn this parameter on may introduce extra
    // memory usage in the table reader, if it allocates extra memory
    // for indexes. This will allow file descriptor prefetch options
    // to be set for compaction input files and not to impact file
    // descriptors for the same file used by user queries.
    // Suggest to enable BlockBasedTableOptions.cache_index_and_filter_blocks
    // for this mode if using block-based table.
    //
    // Default: false
    new_table_reader_for_compaction_inputs = options.new_table_reader_for_compaction_inputs;

    // If non-zero, we perform bigger reads when doing compaction. If you're
    // running RocksDB on spinning disks, you should set this to at least 2MB.
    // That way RocksDB's compaction is doing sequential instead of random reads.
    //
    // When non-zero, we also force new_table_reader_for_compaction_inputs to
    // true.
    //
    // Default: 0
    compaction_readahead_size = options.compaction_readahead_size;

    // This is a maximum buffer size that is used by WinMmapReadableFile in
    // unbuffered disk I/O mode. We need to maintain an aligned buffer for
    // reads. We allow the buffer to grow until the specified value and then
    // for bigger requests allocate one shot buffers. In unbuffered mode we
    // always bypass read-ahead buffer at ReadaheadRandomAccessFile
    // When read-ahead is required we then make use of compaction_readahead_size
    // value and always try to read ahead. With read-ahead we always
    // pre-allocate buffer to the size instead of growing it up to a limit.
    //
    // This option is currently honored only on Windows
    //
    // Default: 1 Mb
    //
    // Special value: 0 - means do not maintain per instance buffer. Allocate
    //                per request buffer and avoid locking.
    random_access_max_buffer_size = options.random_access_max_buffer_size;

    // This is the maximum buffer size that is used by WritableFileWriter.
    // On Windows, we need to maintain an aligned buffer for writes.
    // We allow the buffer to grow until it's size hits the limit.
    //
    // Default: 1024 * 1024 (1 MB)
    writable_file_max_buffer_size = options.writable_file_max_buffer_size;


    // Use adaptive mutex, which spins in the user space before resorting
    // to kernel. This could reduce context switch when the mutex is not
    // heavily contended. However, if the mutex is hot, we could end up
    // wasting spin time.
    // Default: false
    use_adaptive_mutex = options.use_adaptive_mutex;

    // Allows OS to incrementally sync files to disk while they are being
    // written, asynchronously, in the background. This operation can be used
    // to smooth out write I/Os over time. Users shouldn't reply on it for
    // persistency guarantee.
    // Issue one request for every bytes_per_sync written. 0 turns it off.
    // Default: 0
    //
    // You may consider using  to regulate write rate to device.
    // When rate limiter is enabled, it automatically enables bytes_per_sync
    // to 1MB.
    //
    // This option applies to table files
    bytes_per_sync = options.bytes_per_sync;

    // Same as bytes_per_sync, but applies to WAL files
    // Default: 0, turned off
    wal_bytes_per_sync = options.wal_bytes_per_sync;

    // A vector of EventListeners which call-back functions will be called
    // when specific RocksDB event happens.
    listeners = options.listeners;

    // If true, then the status of the threads involved in this DB will
    // be tracked and available via GetThreadList() API.
    //
    // Default: false
    enable_thread_tracking = options.enable_thread_tracking;

    // The limited write rate to DB if soft_pending_compaction_bytes_limit or
    // level0_slowdown_writes_trigger is triggered, or we are writing to the
    // last mem table allowed and we allow more than 3 mem tables. It is
    // calculated using size of user write requests before compression.
    // RocksDB may decide to slow down more if the compaction still
    // gets behind further.
    // Unit: byte per second.
    //
    // Default: 2MB/s
    delayed_write_rate = options.delayed_write_rate;

    // If true, allow multi-writers to update mem tables in parallel.
    // Only some memtable_factory-s support concurrent writes; currently it
    // is implemented only for SkipListFactory.  Concurrent memtable writes
    // are not compatible with inplace_update_support or filter_deletes.
    // It is strongly recommended to set enable_write_thread_adaptive_yield
    // if you are going to use this feature.
    //
    // THIS FEATURE IS NOT STABLE YET.
    //
    // Default: false
    allow_concurrent_memtable_write = options.allow_concurrent_memtable_write;

    // If true, threads synchronizing with the write batch group leader will
    // wait for up to write_thread_max_yield_usec before blocking on a mutex.
    // This can substantially improve throughput for concurrent workloads,
    // regardless of whether allow_concurrent_memtable_write is enabled.
    //
    // THIS FEATURE IS NOT STABLE YET.
    //
    // Default: false
    enable_write_thread_adaptive_yield = options.enable_write_thread_adaptive_yield;

    // The maximum number of microseconds that a write operation will use
    // a yielding spin loop to coordinate with other write threads before
    // blocking on a mutex.  (Assuming write_thread_slow_yield_usec is
    // set properly) increasing this value is likely to increase RocksDB
    // throughput at the expense of increased CPU usage.
    //
    // Default: 100
    write_thread_max_yield_usec = options.write_thread_max_yield_usec;

    // The latency in microseconds after which a std::this_thread::yield
    // call (sched_yield on Linux) is considered to be a signal that
    // other processes or threads would like to use the current core.
    // Increasing this makes writer threads more likely to take CPU
    // by spinning, which will show up as an increase in the number of
    // involuntary context switches.
    //
    // Default: 3
    write_thread_slow_yield_usec = options.write_thread_slow_yield_usec;

    // If true, then DB::Open() will not update the statistics used to optimize
    // compaction decision by loading table properties from many files.
    // Turning off this feature will improve DBOpen time especially in
    // disk environment.
    //
    // Default: false
    skip_stats_update_on_db_open = options.skip_stats_update_on_db_open;

    // Recovery mode to control the consistency while replaying WAL
    // Default: kTolerateCorruptedTailRecords
    wal_recovery_mode = options.wal_recovery_mode;

    // A global cache for table-level rows.
    // Default: nullptr (disabled)
    // Not supported in ROCKSDB_LITE mode!
    row_cache = options.row_cache;

  #ifndef ROCKSDB_LITE
    // A filter object supplied to be invoked while processing write-ahead-logs
    // (WALs) during recovery. The filter provides a way to inspect log
    // records, ignoring a particular record or skipping replay.
    // The filter is invoked at startup and is invoked from a single-thread
    // currently.
    wal_filter = options.wal_filter;
  #endif  // ROCKSDB_LITE

    // If true, then DB::Open / CreateColumnFamily / DropColumnFamily
    // / SetOptions will fail if options file is not detected or properly
    // persisted.
    //
    // DEFAULT: false
    fail_if_options_file_error = options.fail_if_options_file_error;
}

DBOptions::DBOptions(const Options& options)
    : create_if_missing(options.create_if_missing),
      create_missing_column_families(options.create_missing_column_families),
      error_if_exists(options.error_if_exists),
      paranoid_checks(options.paranoid_checks),
      env(options.env),
      rate_limiter(options.rate_limiter),
      sst_file_manager(options.sst_file_manager),
      info_log(options.info_log),
      info_log_level(options.info_log_level),
      max_open_files(options.max_open_files),
      max_file_opening_threads(options.max_file_opening_threads),
      max_total_wal_size(options.max_total_wal_size),
      statistics(options.statistics),
      disableDataSync(options.disableDataSync),
      use_fsync(options.use_fsync),
      db_paths(options.db_paths),
      db_log_dir(options.db_log_dir),
      wal_dir(options.wal_dir),
      delete_obsolete_files_period_micros(
          options.delete_obsolete_files_period_micros),
      base_background_compactions(options.base_background_compactions),
      max_background_compactions(options.max_background_compactions),
      max_subcompactions(options.max_subcompactions),
      max_background_flushes(options.max_background_flushes),
      max_log_file_size(options.max_log_file_size),
      log_file_time_to_roll(options.log_file_time_to_roll),
      keep_log_file_num(options.keep_log_file_num),
      recycle_log_file_num(options.recycle_log_file_num),
      max_manifest_file_size(options.max_manifest_file_size),
      table_cache_numshardbits(options.table_cache_numshardbits),
      WAL_ttl_seconds(options.WAL_ttl_seconds),
      WAL_size_limit_MB(options.WAL_size_limit_MB),
      manifest_preallocation_size(options.manifest_preallocation_size),
      allow_os_buffer(options.allow_os_buffer),
      allow_mmap_reads(options.allow_mmap_reads),
      allow_mmap_writes(options.allow_mmap_writes),
      allow_fallocate(options.allow_fallocate),
      is_fd_close_on_exec(options.is_fd_close_on_exec),
      skip_log_error_on_recovery(options.skip_log_error_on_recovery),
      stats_dump_period_sec(options.stats_dump_period_sec),
      advise_random_on_open(options.advise_random_on_open),
      db_write_buffer_size(options.db_write_buffer_size),
      access_hint_on_compaction_start(options.access_hint_on_compaction_start),
      new_table_reader_for_compaction_inputs(
          options.new_table_reader_for_compaction_inputs),
      compaction_readahead_size(options.compaction_readahead_size),
      random_access_max_buffer_size(options.random_access_max_buffer_size),
      writable_file_max_buffer_size(options.writable_file_max_buffer_size),
      use_adaptive_mutex(options.use_adaptive_mutex),
      bytes_per_sync(options.bytes_per_sync),
      wal_bytes_per_sync(options.wal_bytes_per_sync),
      listeners(options.listeners),
      enable_thread_tracking(options.enable_thread_tracking),
      delayed_write_rate(options.delayed_write_rate),
      allow_concurrent_memtable_write(options.allow_concurrent_memtable_write),
      enable_write_thread_adaptive_yield(
          options.enable_write_thread_adaptive_yield),
      write_thread_max_yield_usec(options.write_thread_max_yield_usec),
      write_thread_slow_yield_usec(options.write_thread_slow_yield_usec),
      skip_stats_update_on_db_open(options.skip_stats_update_on_db_open),
      wal_recovery_mode(options.wal_recovery_mode),
      row_cache(options.row_cache),
#ifndef ROCKSDB_LITE
      wal_filter(options.wal_filter),
#endif  // ROCKSDB_LITE
      fail_if_options_file_error(options.fail_if_options_file_error)
{
}

static const char* const access_hints[] = {
  "NONE", "NORMAL", "SEQUENTIAL", "WILLNEED"
};

void DBOptions::Dump(Logger* log) const {
    Header(log, "         Options.error_if_exists: %d", error_if_exists);
    Header(log, "       Options.create_if_missing: %d", create_if_missing);
    Header(log, "         Options.paranoid_checks: %d", paranoid_checks);
    Header(log, "                     Options.env: %p", env);
    Header(log, "                Options.info_log: %p", info_log.get());
    Header(log, "          Options.max_open_files: %d", max_open_files);
    Header(log,
        "Options.max_file_opening_threads: %d", max_file_opening_threads);
    Header(log,
        "      Options.max_total_wal_size: %" PRIu64, max_total_wal_size);
    Header(log, "       Options.disableDataSync: %d", disableDataSync);
    Header(log, "             Options.use_fsync: %d", use_fsync);
    Header(log, "     Options.max_log_file_size: %" ROCKSDB_PRIszt,
         max_log_file_size);
    Header(log, "Options.max_manifest_file_size: %" PRIu64,
         max_manifest_file_size);
    Header(log, "     Options.log_file_time_to_roll: %" ROCKSDB_PRIszt,
         log_file_time_to_roll);
    Header(log, "     Options.keep_log_file_num: %" ROCKSDB_PRIszt,
         keep_log_file_num);
    Header(log, "  Options.recycle_log_file_num: %" ROCKSDB_PRIszt,
           recycle_log_file_num);
    Header(log, "       Options.allow_os_buffer: %d", allow_os_buffer);
    Header(log, "      Options.allow_mmap_reads: %d", allow_mmap_reads);
    Header(log, "      Options.allow_fallocate: %d", allow_fallocate);
    Header(log, "     Options.allow_mmap_writes: %d", allow_mmap_writes);
    Header(log, "         Options.create_missing_column_families: %d",
        create_missing_column_families);
    Header(log, "                             Options.db_log_dir: %s",
        db_log_dir.c_str());
    Header(log, "                                Options.wal_dir: %s",
        wal_dir.c_str());
    Header(log, "               Options.table_cache_numshardbits: %d",
        table_cache_numshardbits);
    Header(log, "    Options.delete_obsolete_files_period_micros: %" PRIu64,
        delete_obsolete_files_period_micros);
    Header(log, "             Options.base_background_compactions: %d",
           base_background_compactions);
    Header(log, "             Options.max_background_compactions: %d",
        max_background_compactions);
    Header(log, "                     Options.max_subcompactions: %" PRIu32,
        max_subcompactions);
    Header(log, "                 Options.max_background_flushes: %d",
        max_background_flushes);
    Header(log, "                        Options.WAL_ttl_seconds: %" PRIu64,
        WAL_ttl_seconds);
    Header(log, "                      Options.WAL_size_limit_MB: %" PRIu64,
        WAL_size_limit_MB);
    Header(log,
         "            Options.manifest_preallocation_size: %" ROCKSDB_PRIszt,
         manifest_preallocation_size);
    Header(log, "                         Options.allow_os_buffer: %d",
        allow_os_buffer);
    Header(log, "                        Options.allow_mmap_reads: %d",
        allow_mmap_reads);
    Header(log, "                       Options.allow_mmap_writes: %d",
        allow_mmap_writes);
    Header(log, "                     Options.is_fd_close_on_exec: %d",
        is_fd_close_on_exec);
    Header(log, "                   Options.stats_dump_period_sec: %u",
        stats_dump_period_sec);
    Header(log, "                   Options.advise_random_on_open: %d",
        advise_random_on_open);
    Header(log,
         "                    Options.db_write_buffer_size: %" ROCKSDB_PRIszt
         "d",
         db_write_buffer_size);
    Header(log, "         Options.access_hint_on_compaction_start: %s",
        access_hints[access_hint_on_compaction_start]);
    Header(log, "  Options.new_table_reader_for_compaction_inputs: %d",
         new_table_reader_for_compaction_inputs);
    Header(log,
         "               Options.compaction_readahead_size: %" ROCKSDB_PRIszt
         "d",
         compaction_readahead_size);
    Header(
        log,
        "               Options.random_access_max_buffer_size: %" ROCKSDB_PRIszt
        "d",
        random_access_max_buffer_size);
    Header(log,
         "              Options.writable_file_max_buffer_size: %" ROCKSDB_PRIszt
         "d",
         writable_file_max_buffer_size);
    Header(log, "                      Options.use_adaptive_mutex: %d",
        use_adaptive_mutex);
    Header(log, "                            Options.rate_limiter: %p",
        rate_limiter.get());
    Header(
        log, "     Options.sst_file_manager.rate_bytes_per_sec: %" PRIi64,
        sst_file_manager ? sst_file_manager->GetDeleteRateBytesPerSecond() : 0);
    Header(log, "                          Options.bytes_per_sync: %" PRIu64,
        bytes_per_sync);
    Header(log, "                      Options.wal_bytes_per_sync: %" PRIu64,
        wal_bytes_per_sync);
    Header(log, "                       Options.wal_recovery_mode: %d",
        wal_recovery_mode);
    Header(log, "                  Options.enable_thread_tracking: %d",
        enable_thread_tracking);
    Header(log, "         Options.allow_concurrent_memtable_write: %d",
           allow_concurrent_memtable_write);
    Header(log, "      Options.enable_write_thread_adaptive_yield: %d",
           enable_write_thread_adaptive_yield);
    Header(log, "             Options.write_thread_max_yield_usec: %" PRIu64,
           write_thread_max_yield_usec);
    Header(log, "            Options.write_thread_slow_yield_usec: %" PRIu64,
           write_thread_slow_yield_usec);
    if (row_cache) {
      Header(log, "                               Options.row_cache: %" PRIu64,
           row_cache->GetCapacity());
    } else {
      Header(log, "                               Options.row_cache: None");
    }
#ifndef ROCKSDB_LITE
    Header(log, "       Options.wal_filter: %s",
           wal_filter ? wal_filter->Name() : "None");
#endif  // ROCKDB_LITE
}  // DBOptions::Dump

void ColumnFamilyOptions::Dump(Logger* log) const {
  Header(log, "              Options.comparator: %s", comparator->Name());
  Header(log, "          Options.merge_operator: %s",
      merge_operator ? merge_operator->Name() : "None");
  Header(log, "       Options.compaction_filter: %s",
      compaction_filter ? compaction_filter->Name() : "None");
  Header(log, "       Options.compaction_filter_factory: %s",
      compaction_filter_factory ? compaction_filter_factory->Name() : "None");
  Header(log, "        Options.memtable_factory: %s", memtable_factory->Name());
  Header(log, "           Options.table_factory: %s", table_factory->Name());
  Header(log, "           table_factory options: %s",
      table_factory->GetPrintableTableOptions().c_str());
  Header(log, "       Options.write_buffer_size: %" ROCKSDB_PRIszt,
       write_buffer_size);
  Header(log, " Options.max_write_buffer_number: %d", max_write_buffer_number);
    if (!compression_per_level.empty()) {
      for (unsigned int i = 0; i < compression_per_level.size(); i++) {
        Header(log, "       Options.compression[%d]: %s", i,
            CompressionTypeToString(compression_per_level[i]).c_str());
      }
    } else {
      Header(log, "         Options.compression: %s",
          CompressionTypeToString(compression).c_str());
    }
    Header(log, "      Options.prefix_extractor: %s",
        prefix_extractor == nullptr ? "nullptr" : prefix_extractor->Name());
    Header(log, "            Options.num_levels: %d", num_levels);
    Header(log, "       Options.min_write_buffer_number_to_merge: %d",
        min_write_buffer_number_to_merge);
    Header(log, "    Options.max_write_buffer_number_to_maintain: %d",
         max_write_buffer_number_to_maintain);
    Header(log, "           Options.compression_opts.window_bits: %d",
        compression_opts.window_bits);
    Header(log, "                 Options.compression_opts.level: %d",
        compression_opts.level);
    Header(log, "              Options.compression_opts.strategy: %d",
        compression_opts.strategy);
    Header(log, "     Options.level0_file_num_compaction_trigger: %d",
        level0_file_num_compaction_trigger);
    Header(log, "         Options.level0_slowdown_writes_trigger: %d",
        level0_slowdown_writes_trigger);
    Header(log, "             Options.level0_stop_writes_trigger: %d",
        level0_stop_writes_trigger);
    Header(log, "                  Options.target_file_size_base: %" PRIu64,
        target_file_size_base);
    Header(log, "            Options.target_file_size_multiplier: %d",
        target_file_size_multiplier);
    Header(log, "               Options.max_bytes_for_level_base: %" PRIu64,
        max_bytes_for_level_base);
    Header(log, "Options.level_compaction_dynamic_level_bytes: %d",
        level_compaction_dynamic_level_bytes);
    Header(log, "         Options.max_bytes_for_level_multiplier: %d",
        max_bytes_for_level_multiplier);
    for (size_t i = 0; i < max_bytes_for_level_multiplier_additional.size();
         i++) {
      Header(log,
          "Options.max_bytes_for_level_multiplier_addtl[%" ROCKSDB_PRIszt
                "]: %d",
           i, max_bytes_for_level_multiplier_additional[i]);
    }
    Header(log, "      Options.max_sequential_skip_in_iterations: %" PRIu64,
        max_sequential_skip_in_iterations);
    Header(log, "             Options.expanded_compaction_factor: %d",
        expanded_compaction_factor);
    Header(log, "               Options.source_compaction_factor: %d",
        source_compaction_factor);
    Header(log, "         Options.max_grandparent_overlap_factor: %d",
        max_grandparent_overlap_factor);

    Header(log,
         "                       Options.arena_block_size: %" ROCKSDB_PRIszt,
         arena_block_size);
    Header(log, "  Options.soft_pending_compaction_bytes_limit: %" PRIu64,
           soft_pending_compaction_bytes_limit);
    Header(log, "  Options.hard_pending_compaction_bytes_limit: %" PRIu64,
         hard_pending_compaction_bytes_limit);
    Header(log, "      Options.rate_limit_delay_max_milliseconds: %u",
        rate_limit_delay_max_milliseconds);
    Header(log, "               Options.disable_auto_compactions: %d",
        disable_auto_compactions);
    Header(log, "                          Options.filter_deletes: %d",
        filter_deletes);
    Header(log, "          Options.verify_checksums_in_compaction: %d",
        verify_checksums_in_compaction);
    Header(log, "                        Options.compaction_style: %d",
        compaction_style);
    Header(log, "                          Options.compaction_pri: %d",
           compaction_pri);
    Header(log, " Options.compaction_options_universal.size_ratio: %u",
        compaction_options_universal.size_ratio);
    Header(log, "Options.compaction_options_universal.min_merge_width: %u",
        compaction_options_universal.min_merge_width);
    Header(log, "Options.compaction_options_universal.max_merge_width: %u",
        compaction_options_universal.max_merge_width);
    Header(log, "Options.compaction_options_universal."
            "max_size_amplification_percent: %u",
        compaction_options_universal.max_size_amplification_percent);
    Header(log,
        "Options.compaction_options_universal.compression_size_percent: %d",
        compaction_options_universal.compression_size_percent);
    Header(log,
        "Options.compaction_options_fifo.max_table_files_size: %" PRIu64,
        compaction_options_fifo.max_table_files_size);
    std::string collector_names;
    for (const auto& collector_factory : table_properties_collector_factories) {
      collector_names.append(collector_factory->Name());
      collector_names.append("; ");
    }
    Header(log, "                  Options.table_properties_collectors: %s",
        collector_names.c_str());
    Header(log, "                  Options.inplace_update_support: %d",
        inplace_update_support);
    Header(log,
         "                Options.inplace_update_num_locks: %" ROCKSDB_PRIszt,
         inplace_update_num_locks);
    Header(log, "              Options.min_partial_merge_operands: %u",
        min_partial_merge_operands);
    // TODO: easier config for bloom (maybe based on avg key/value size)
    Header(log, "              Options.memtable_prefix_bloom_bits: %d",
        memtable_prefix_bloom_bits);
    Header(log, "            Options.memtable_prefix_bloom_probes: %d",
        memtable_prefix_bloom_probes);

    Header(log,
         "  Options.memtable_prefix_bloom_huge_page_tlb_size: %" ROCKSDB_PRIszt,
         memtable_prefix_bloom_huge_page_tlb_size);
    Header(log, "                          Options.bloom_locality: %d",
        bloom_locality);

    Header(log,
         "                   Options.max_successive_merges: %" ROCKSDB_PRIszt,
         max_successive_merges);
    Header(log, "               Options.optimize_filters_for_hits: %d",
        optimize_filters_for_hits);
    Header(log, "               Options.paranoid_file_checks: %d",
         paranoid_file_checks);
    Header(log, "               Options.compaction_measure_io_stats: %d",
         compaction_measure_io_stats);
}  // ColumnFamilyOptions::Dump

void Options::Dump(Logger* log) const {
  DBOptions::Dump(log);
  ColumnFamilyOptions::Dump(log);
}   // Options::Dump

void Options::DumpCFOptions(Logger* log) const {
  ColumnFamilyOptions::Dump(log);
}  // Options::DumpCFOptions

//
// The goal of this method is to create a configuration that
// allows an application to write all files into L0 and
// then do a single compaction to output all files into L1.
Options*
Options::PrepareForBulkLoad()
{
  // never slowdown ingest.
  level0_file_num_compaction_trigger = (1<<30);
  level0_slowdown_writes_trigger = (1<<30);
  level0_stop_writes_trigger = (1<<30);

  // no auto compactions please. The application should issue a
  // manual compaction after all data is loaded into L0.
  disable_auto_compactions = true;
  disableDataSync = true;

  // A manual compaction run should pick all files in L0 in
  // a single compaction run.
  source_compaction_factor = (1<<30);

  // It is better to have only 2 levels, otherwise a manual
  // compaction would compact at every possible level, thereby
  // increasing the total time needed for compactions.
  num_levels = 2;

  // Need to allow more write buffers to allow more parallism
  // of flushes.
  max_write_buffer_number = 6;
  min_write_buffer_number_to_merge = 1;

  // When compaction is disabled, more parallel flush threads can
  // help with write throughput.
  max_background_flushes = 4;

  // Prevent a memtable flush to automatically promote files
  // to L1. This is helpful so that all files that are
  // input to the manual compaction are all at L0.
  max_background_compactions = 2;
  base_background_compactions = 2;

  // The compaction would create large files in L1.
  target_file_size_base = 256 * 1024 * 1024;
  return this;
}

Options* Options::OldDefaults(int rocksdb_major_version,
                              int rocksdb_minor_version) {
  ColumnFamilyOptions::OldDefaults(rocksdb_major_version,
                                   rocksdb_minor_version);
  DBOptions::OldDefaults(rocksdb_major_version, rocksdb_minor_version);
  return this;
}

DBOptions* DBOptions::OldDefaults(int rocksdb_major_version,
                                  int rocksdb_minor_version) {
  max_file_opening_threads = 1;
  table_cache_numshardbits = 4;
  return this;
}

ColumnFamilyOptions* ColumnFamilyOptions::OldDefaults(
    int rocksdb_major_version, int rocksdb_minor_version) {
  write_buffer_size = 4 << 20;
  target_file_size_base = 2 * 1048576;
  max_bytes_for_level_base = 10 * 1048576;
  soft_pending_compaction_bytes_limit = 0;
  hard_pending_compaction_bytes_limit = 0;
  return this;
}

// Optimization functions
ColumnFamilyOptions* ColumnFamilyOptions::OptimizeForSmallDb() {
  write_buffer_size = 2 << 20;
  target_file_size_base = 2 * 1048576;
  max_bytes_for_level_base = 10 * 1048576;
  soft_pending_compaction_bytes_limit = 256 * 1048576;
  hard_pending_compaction_bytes_limit = 1073741824ul;
  return this;
}

#ifndef ROCKSDB_LITE
ColumnFamilyOptions* ColumnFamilyOptions::OptimizeForPointLookup(
    uint64_t block_cache_size_mb) {
  prefix_extractor.reset(NewNoopTransform());
  BlockBasedTableOptions block_based_options;
  block_based_options.index_type = BlockBasedTableOptions::kHashSearch;
  block_based_options.filter_policy.reset(NewBloomFilterPolicy(10));
  block_based_options.block_cache =
      NewLRUCache(static_cast<size_t>(block_cache_size_mb * 1024 * 1024));
  table_factory.reset(new BlockBasedTableFactory(block_based_options));
  memtable_factory.reset(NewHashLinkListRepFactory());
  return this;
}

ColumnFamilyOptions* ColumnFamilyOptions::OptimizeLevelStyleCompaction(
    uint64_t memtable_memory_budget) {
  write_buffer_size = static_cast<size_t>(memtable_memory_budget / 4);
  // merge two memtables when flushing to L0
  min_write_buffer_number_to_merge = 2;
  // this means we'll use 50% extra memory in the worst case, but will reduce
  // write stalls.
  max_write_buffer_number = 6;
  // start flushing L0->L1 as soon as possible. each file on level0 is
  // (memtable_memory_budget / 2). This will flush level 0 when it's bigger than
  // memtable_memory_budget.
  level0_file_num_compaction_trigger = 2;
  // doesn't really matter much, but we don't want to create too many files
  target_file_size_base = memtable_memory_budget / 8;
  // make Level1 size equal to Level0 size, so that L0->L1 compactions are fast
  max_bytes_for_level_base = memtable_memory_budget;

  // level style compaction
  compaction_style = kCompactionStyleLevel;

  // only compress levels >= 2
  compression_per_level.resize(num_levels);
  for (int i = 0; i < num_levels; ++i) {
    if (i < 2) {
      compression_per_level[i] = kNoCompression;
    } else {
      compression_per_level[i] = kSnappyCompression;
    }
  }
  return this;
}

ColumnFamilyOptions* ColumnFamilyOptions::OptimizeUniversalStyleCompaction(
    uint64_t memtable_memory_budget) {
  write_buffer_size = static_cast<size_t>(memtable_memory_budget / 4);
  // merge two memtables when flushing to L0
  min_write_buffer_number_to_merge = 2;
  // this means we'll use 50% extra memory in the worst case, but will reduce
  // write stalls.
  max_write_buffer_number = 6;
  // universal style compaction
  compaction_style = kCompactionStyleUniversal;
  compaction_options_universal.compression_size_percent = 80;
  return this;
}

DBOptions* DBOptions::IncreaseParallelism(int total_threads) {
  max_background_compactions = total_threads - 1;
  max_background_flushes = 1;
  env->SetBackgroundThreads(total_threads, Env::LOW);
  env->SetBackgroundThreads(1, Env::HIGH);
  return this;
}

#endif  // !ROCKSDB_LITE

ReadOptions::ReadOptions()
    : verify_checksums(true),
      fill_cache(true),
      snapshot(nullptr),
      iterate_upper_bound(nullptr),
      read_tier(kReadAllTier),
      tailing(false),
      managed(false),
      total_order_seek(false),
      prefix_same_as_start(false),
      pin_data(false) {
  XFUNC_TEST("", "managed_options", managed_options, xf_manage_options,
             reinterpret_cast<ReadOptions*>(this));
}

ReadOptions::ReadOptions(bool cksum, bool cache)
    : verify_checksums(cksum),
      fill_cache(cache),
      snapshot(nullptr),
      iterate_upper_bound(nullptr),
      read_tier(kReadAllTier),
      tailing(false),
      managed(false),
      total_order_seek(false),
      prefix_same_as_start(false),
      pin_data(false) {
  XFUNC_TEST("", "managed_options", managed_options, xf_manage_options,
             reinterpret_cast<ReadOptions*>(this));
}

}  // namespace rocksdb
