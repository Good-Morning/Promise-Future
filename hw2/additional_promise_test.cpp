#include "pch.h"

#if PROMISE 1

TEST(additional_promise, std_exception) {
	Promise<int> promise;
	auto future = promise.GetFuture();

	try 
	{ 
		throw std::exception("error"); 
	}
	catch (...) 
	{ 
		promise.SetException(std::current_exception()); 
	}

	ASSERT_ANY_THROW(future.Get(););
	ASSERT_NO_THROW(
		try 
		{ 
			future.Get(); 
		}
		catch (const std::exception& e) 
		{
			ASSERT_STREQ(e.what(), "error");
		}
	);
}

TEST(additional_promise, promise_move) {
	Promise<int> promise;
	
	[](Promise<int> promise) 
	{
		ASSERT_NO_THROW(promise.GetFuture(););
		ASSERT_NO_THROW(promise.Set(5););
	} (std::move(promise));
	
	ASSERT_ANY_THROW(promise.GetFuture());
	ASSERT_ANY_THROW(promise.Set(5););
}

TEST(additional_promise, no_hope) {
	Promise<int> promise;
	auto future = promise.GetFuture();

	[](Promise<int> promise) {} (std::move(promise));
	
	ASSERT_ANY_THROW(future.Wait(););
	ASSERT_ANY_THROW(future.Get(););
}

TEST(additional_promise, future_move) {
	Promise<int> promise;
	auto future = promise.GetFuture();
	
	[](Future<int> future, Promise<int> promise) 
	{
		promise.Set(5);
		ASSERT_NO_THROW(future.Get(););
	} (std::move(future), std::move(promise));
	
	ASSERT_ANY_THROW(future.Get());
}

TEST(additional_promise, promise_return_control) {
	Promise<int> promise;

	[](Promise<int> promise, Promise<int>& promise_return) 
	{
		promise_return = std::move(promise);
	} (std::move(promise), promise);
	
	ASSERT_NO_THROW(promise.GetFuture());
	ASSERT_NO_THROW(promise.Set(5););
}

TEST(additional_promise, future_return_control) {
	Promise<int> promise;
	auto future = promise.GetFuture();

	[](
		Future<int> future, 
		Promise<int> promise, 
		Future<int>& future_return, 
		Promise<int>& promise_return
	) 
	{
		promise_return = std::move(promise);
		future_return = std::move(future);
	} (
		std::move(future), 
		std::move(promise), 
		future, promise
	);
	
	ASSERT_NO_THROW(promise.Set(5); future.Get());
}

struct MarkableDestructor
{
	static bool destructured, 
				markIsExcpected;
	
	MarkableDestructor() 
	{}

	~MarkableDestructor() 
	{ 
		destructured = markIsExcpected; 
	}
}; 

bool MarkableDestructor::destructured, 
	 MarkableDestructor::markIsExcpected;

TEST(additional_promise, move_future_operator_destructor) 
{
	Promise<MarkableDestructor> promise0, promise1;
	auto future0 = promise0.GetFuture();
	auto future1 = promise1.GetFuture();
	
	promise1.Set();
	future1 = std::move(future0);
	
	MarkableDestructor::markIsExcpected = true;
	MarkableDestructor::destructured = false;
		
		[](Promise<MarkableDestructor>) 
		{}(std::move(promise1));
		
		ASSERT_TRUE(MarkableDestructor::destructured);
	
	MarkableDestructor::markIsExcpected = false;
}

TEST(additional_promise, move_promise_operator_destructor) 
{
	Promise<MarkableDestructor> promise0, promise1;
	
	auto future0 = promise0.GetFuture();
	auto future1 = promise1.GetFuture();
	
	promise1 = std::move(promise0);
	
	ASSERT_ANY_THROW( future1.Get(); );
}

#endif

