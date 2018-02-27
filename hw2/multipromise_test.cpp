#include "pch.h"

#if MULTIPROMISE 1

TEST(multipromise, Set_int) {
	MultiPromise<int> promise;
	Future<int> f0 = promise.GetFuture();
	Future<int> f1 = promise.GetFuture();
	Future<int> f2 = promise.GetFuture();

	int const x = 10;
	promise.Set(x);

	ASSERT_EQ(f0.Get(), x);
	ASSERT_EQ(f1.Get(), x);
	ASSERT_EQ(f2.Get(), x);
}

TEST(multipromise, Set_int_ampersand) {
	MultiPromise<int &> promise;
	Future<int &> f0 = promise.GetFuture();
	Future<int &> f1 = promise.GetFuture();
	Future<int &> f2 = promise.GetFuture();

	int const y = 100;
	int x = 10;

	promise.Set(x);

	f0.Get() = y; 
	f1.Get() = y; 
	f2.Get() = y;

	ASSERT_EQ(x, y);
}

TEST(multipromise, Set_void) {
	MultiPromise<void> promise;
	Future<void> f0 = promise.GetFuture();
	Future<void> f1 = promise.GetFuture();
	Future<void> f2 = promise.GetFuture();

	ASSERT_FALSE(f0.IsReady());
	ASSERT_FALSE(f1.IsReady());
	ASSERT_FALSE(f2.IsReady());

	promise.Set();
	f0.Get(); 
	f1.Get(); 
	f2.Get();

	ASSERT_TRUE(f0.IsReady());
	ASSERT_TRUE(f1.IsReady());
	ASSERT_TRUE(f2.IsReady());
}

#endif


