#include "pch.h"

#if FLATTEN_UNWRAP 1

TEST(flatten_unwrap, promise_caused) {
	Promise< 
		Future< Future< Future< int > > > 
	> promise3;
	
	Promise< 
		Future< Future< int > > 
	> promise2;
	
	Promise< 
		Future< int > 
	> promise1;
	
	Promise< int > promise0;

	Future< Future< Future< Future< int > > > > 
		future = promise3.GetFuture();
	
	promise3.Set(promise2.GetFuture());
	promise2.Set(promise1.GetFuture());
	promise1.Set(promise0.GetFuture());
	
	promise0.Set(-27);
	
	Future< int > normalized(
		Flatten(std::move(future))
	);

	std::this_thread::sleep_for(
		std::chrono::milliseconds(500)
	);
	
	ASSERT_EQ(normalized.Get(), -27);
}

TEST(flatten_unwrap, promise_caused_exception) {
	Promise<
		Future< Future< Future< int > > >
	> promise3;

	Promise<
		Future< Future< int > >
	> promise2;
	
	Promise<
		Future< int >
	> promise1;
	
	Promise<int> promise0;

	Future<Future<Future<Future<int>>>> 
		future = promise3.GetFuture();
	
	promise3.Set(promise2.GetFuture());
	promise2.Set(promise1.GetFuture());
	promise1.Set(promise0.GetFuture());
	
	promise0.SetException(
		std::make_exception_ptr(std::exception("error"))
	);
	
	Future<int> normalized(
		Flatten(
			std::move(future)
		)
	);
	ASSERT_NO_THROW(
		try
		{ 
			normalized.Get(); 
		}
		catch (const std::exception& e) 
		{ 
			ASSERT_STREQ(e.what(), "error"); 
		}
	);
}


TEST(flatten_unwrap, function_caused) {
	auto future = MakeFuture(
		[] 
		{
			std::this_thread::sleep_for(
				std::chrono::milliseconds(300)
			);

			return 5;
		}
	);
	
	ASSERT_FALSE(future.IsReady());
	ASSERT_EQ(future.Get(), 5);
}

TEST(flatten_unwrap, function_caused_void) {
	auto future = MakeFuture(
		[] 
		{
			std::this_thread::sleep_for(
				std::chrono::milliseconds(300)
			);

			return;
		}
	);
	
	ASSERT_FALSE(future.IsReady());
	future.Get();
}

//TEST(flatten_unwrap, function_caused_wrappered) {
//	auto future = MakeFuture([] {
//		return MakeFuture([] {
//			return MakeFuture([] {
//				return 5;
//			}); 
//		});
//	});
//		
//	ASSERT_FALSE(future.IsReady());
//	ASSERT_EQ(
//		Flatten(
//			std::move(future)
//		).Get(),
//			
//		5
//	);
//}


TEST(flatten_unwrap, function_caused_wrappered_delay) {
	for (int k = 0; k < 100; k++) 
	{
		auto future = MakeFuture([] {
			std::this_thread::sleep_for(
				std::chrono::milliseconds(3)
			);

			return MakeFuture([] {
				return MakeFuture([] {
					return MakeFuture([] { 
						return 5; 
					});
				});
			});
		});

		ASSERT_FALSE(future.IsReady());
		ASSERT_EQ(
			Flatten(
				std::move(future)
			).Get(), 
			
			5
		);
	}
}

TEST(flatten_unwrap, flatten_delays) {
	auto future = MakeFuture([] {
		std::this_thread::sleep_for(
			std::chrono::milliseconds(300)
		);

		return MakeFuture([] { 
			return 5; 
		});
	});

	auto newFuture = Flatten(
		std::move(future)
	);
	
	ASSERT_FALSE(newFuture.IsReady());
	ASSERT_EQ(newFuture.Get(), 5);
}


#endif