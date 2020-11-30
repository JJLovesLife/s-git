#include "../s-git.h"

int testMain(int argc, const char* argv[]);
extern Command TestCommand{
	testMain,
	"used for internal test."
};

int testMain(int argc, const char* argv[]) {
	return 0;
}
