#include "pch.h"

#if MAP

int Add5(const int& arg)
{
	return arg + 5;
}

int Ret5()
{
	return 5;
}

TEST(additional_map, simple) {

#if THREAD_POOL
	int result;
	{
		ThreadPool threadPool(8);
		threadPool.execute([&result] {
			Promise< int > promise;
			Future< int > future = promise.GetFuture();

			auto future_mapped = Map(
				std::move(future),
				Add5
			);
			promise.Set(-7);
			result = std::move(future_mapped.Get());
		});
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	ASSERT_EQ(result, -2);
#else
	Promise< int > promise;
	Future< int > future = promise.GetFuture();
	
	auto future_mapped = Map(
		std::move(future), 
		Add5
	);

	promise.Set(-7);
	ASSERT_EQ(future_mapped.Get(), -2);
#endif
}

TEST(additional_map, wrapped) {
	auto future_wrapped = Map(
		Map(
			MakeFuture(Ret5),
			Add5
		),
	
		Add5
	);

	ASSERT_EQ(
		Flatten(
			std::move(future_wrapped)
		).Get(),	
		
		15
	);
}

#endif

