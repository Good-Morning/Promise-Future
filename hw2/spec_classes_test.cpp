#include "pch.h"

#if SPEC_CLASSES 1

TEST(spec_classes, noDefault) 
{
	class NoDefault 
	{
	public:
		NoDefault(int) {}
	};
	
	Promise<NoDefault> promise;
	auto future = promise.GetFuture();
	
	promise.Set(5);
	future.Get();
}

TEST(spec_classes, notCopiable) 
{
	class NotCopiable 
	{
	public:
		NotCopiable(int) {}
		NotCopiable(const NotCopiable&) = delete;
		NotCopiable& operator=(const NotCopiable&) = delete;
		NotCopiable(NotCopiable&&) {}
	};

	[]()->NotCopiable 
	{ 
		NotCopiable a = 5; 
		return a; 
	}();

	Promise<NotCopiable> promise;
	auto future = promise.GetFuture();

	promise.Set(5);
	future.Get();
}

#endif


