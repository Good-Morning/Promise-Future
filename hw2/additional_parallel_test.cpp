#include "pch.h"

#if PARALLEL

TEST(additional_parallel, vector) {
	std::vector<int> vector = { 1, 2, 3, 4, 5 };
	std::atomic<bool> flag = false;
	Parallel(
		vector.begin(), 
		vector.end(), 
		std::shared_ptr< ThreadPool >(new ThreadPool(4)), 
		
		[&flag](std::vector<int>::iterator it) 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "df";
			std::cout << *it; 
			flag = true;
		}
	);
	std::cout << "end?";
	ASSERT_TRUE(flag);
}

TEST(additional_parallel, set) {
	std::set<int> set = { 1, 2, 3, 4, 5 };
	std::atomic<bool> flag = false;
	Parallel(
		set.begin(),
		set.end(),
		std::shared_ptr< ThreadPool >(new ThreadPool(4)),

		[&flag](std::set<int>::iterator it)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << *it;
			flag = true;
		}
	);
	std::cout << "end?";
	ASSERT_TRUE(flag);
}

#endif

