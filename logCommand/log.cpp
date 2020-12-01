#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

int logMain(int argc, const char* argv[]);
extern Command LogCommand{
	logMain,
	"Show commit logs"
};


int logMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.parse_check(argc, argv);

	if (!GIT_DIR.has_value()) {
		std::cout << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << std::endl;
		return 1;
	}

	std::string sha1 = readMain();
	commit currCommit;
	while (readCommit(sha1, currCommit)) {
		std::cout << "\033[33m[commit " << currCommit.sha1 << ']' << std::endl;
		std::cout << "\033[1;36mmessage:\033[m" << std::endl;
		std::cout << std::endl;

		std::cout << currCommit.message;

		std::cout << std::endl << std::endl;
		sha1 = currCommit.parent;
	}
	return 0;
}
