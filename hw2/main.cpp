#include "pch.h"

int main(int argc, char *argv[]){
	::testing::InitGoogleTest(&argc, argv);
	for (int c = 1; c--;)
	RUN_ALL_TESTS();
	return 0;
}
