#include "pch.h"

#if FLATTEN_COLLECTIONS 1

#define addcmpable_collection_multi(collection, PUSH) \
TEST(additional_flatten, collection_##collection##_multi) { \
	std::collection< Future< int > > futures; \
	Promise< int > p[3]; \
	MultiPromise< int > mp; \
	\
	for (int i = 0; i < 3; i++) \
	{ \
		futures.PUSH( \
			std::move(mp.GetFuture()) \
		); \
	} \
	for (int i = 0; i < 3; i++) \
	{ \
		futures.PUSH( \
			std::move(p[i].GetFuture()) \
		); \
	} \
	\
	mp.Set(-7); \
	Future< std::collection< int > > vec( \
		Flatten(std::move(futures)) \
	); \
	\
	std::collection<int> exceptions; \
	for (int i = 0; i < 3; i++) \
	{ \
		exceptions.PUSH(-7); \
	} \
	for (int i = 0; i < 3; i++) \
	{ \
		p[i].Set((i - 1) * 546); \
		exceptions.PUSH((i - 1) * 546); \
	} \
	\
	std::collection<int> \
		col = std::move(vec.Get()); \
	\
	ASSERT_TRUE(exceptions == col); \
} 

#define addcmpable_collection(collection, PUSH) \
TEST(additional_flatten, collection_##collection) { \
	std::collection< Future< int > > futures; \
	Promise<int> p[3]; \
	\
	for (int i = 0; i < 3; i++) \
	{ \
		futures.PUSH( \
			std::move(p[i].GetFuture()) \
		); \
	} \
	\
	Future< std::collection< int > > vec( \
		Flatten(std::move(futures)) \
	); \
	\
	std::collection< int > exceptions; \
	\
	for (int i = 0; i < 3; i++) \
	{ \
		p[i].Set((i - 1) * 546); \
		exceptions.PUSH((i - 1)*546); \
	} \
	\
	std::collection<int> \
		col = std::move(vec.Get()); \
	ASSERT_TRUE(exceptions == col); \
} 

TEST(additional_flatten, collection_map) 
{
	std::map< int, Future< int > > futures;
	Promise< int > p[3];

	for (int i = 0; i < 3; i++) 
	{ 
		futures.emplace(i, std::move(p[i].GetFuture())); 
	}
	Future< std::map< int, int > > fut(
		std::move(
			Flatten(
				std::move(futures)
			)
		)
	);
	std::map< int, int > exceptions;
	
	for (int i = 0; i < 3; i++) 
	{ 
		p[i].Set((i - 1) * 546); 
		exceptions[i] = ((i - 1) * 546); 
	}
	
	std::map< int, int > col(fut.Get());
	ASSERT_TRUE(exceptions == col);
}

//TEST(additional_flatten, collection_array) 
//{
//	Promise< int > p[3];
//	std::array< Future< int >, 3 > futures = {
//		p[0].GetFuture(), 
//		p[1].GetFuture(), 
//		p[2].GetFuture()
//	};
//	
//	Future< std::array< int, 3 > > fut(
//		Flatten(
//			std::move(futures)
//		)
//	);
//	
//	std::array< int, 3 > exceptions = { -546, 0, 546 };
//	for (int i = 0; i < 3; i++) 
//	{ 
//		p[i].Set((i - 1) * 546); 
//	}
//	
//	std::array< int, 3 > col(fut.Get());
//	ASSERT_TRUE(exceptions == col);
//}

#if MULTIPROMISE 1
addcmpable_collection_multi(vector, push_back)
addcmpable_collection_multi(list, push_back)
addcmpable_collection_multi(deque, push_back)
addcmpable_collection_multi(stack, push)
addcmpable_collection_multi(queue, push)
addcmpable_collection_multi(set, insert)
addcmpable_collection_multi(multiset, insert)
#endif

addcmpable_collection(vector, push_back)
addcmpable_collection(list, push_back)
addcmpable_collection(deque, push_back)
addcmpable_collection(stack, push)
addcmpable_collection(queue, push)
addcmpable_collection(set, insert)
addcmpable_collection(multiset, insert)

#endif