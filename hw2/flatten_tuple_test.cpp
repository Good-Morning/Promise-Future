#include "pch.h"

#if FLATTEN_TUPLE

TEST(additional_flatten, tuple) {
	Promise<int> promise0, promise1;
	int x = 0;
	std::tuple<
		int,
		Future<int>,
		int,
		Future<int>
	> tuple(
		5,
		promise0.GetFuture(),
		x,
		promise1.GetFuture()
	);
	std::tuple<int, int, int, int> exceptions(
		5, 2, 0, -7
	);
	promise0.Set(2);
	promise1.Set(-7);
	auto temp = Flatten(std::move(tuple));
	ASSERT_TRUE(exceptions == temp.Get());
}

TEST(additional_flatten, tupleFull) {
	Promise<int> promise0, promise1, promise2, promise3;
	int x = 0;
	std::tuple<
		Future< int >,
		Future< int >,
		Future< int >,
		Future< int >
	> tuple(
		promise0.GetFuture(),
		promise1.GetFuture(),
		promise2.GetFuture(),
		promise3.GetFuture()
	);
	std::tuple<int, int, int, int> exceptions(
		5, 2, 0, -7
	);
	promise0.Set(5);
	promise1.Set(2);
	promise2.Set(0);
	promise3.Set(-7);
	auto temp = Flatten(std::move(tuple));
	ASSERT_TRUE(exceptions == temp.Get());
}

#endif