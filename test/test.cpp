#include "../s-git.h"

int testMain(int argc, const char* argv[]);
Command TestCommand{
	testMain,
	"used for internal test."
};

int testMain(int argc, const char* argv[]) {
	return 0;
}
