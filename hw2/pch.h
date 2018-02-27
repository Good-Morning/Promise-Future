#pragma once

#include "gtest/gtest.h"
#include <mutex>
#include <atomic>
#include <memory>
#include <exception>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
#include <tuple>

#include <array>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <list>
#include <forward_list>
#include <map>
#include <set>

#include "ThreadPool.h"

#pragma hdrstop

#define PROMISE 1
#define MULTIPROMISE 1
#define SPEC_CLASSES 1
#define FLATTEN_UNWRAP 1
#define FLATTEN_COLLECTIONS 1
#define MAP 1
#define FLATTEN_TUPLE 1
#define PARALLEL 1

#define THREAD_POOL 1

extern std::mutex cout_mutex;

#include "subfuture.h"
#include "future.h"
#include "flatten.h"
#include "map.h"
#include "ThreadPool.h"
#include "Parallel.h"








