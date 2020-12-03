#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

int logMain(int argc, const char* argv[]);
Command LogCommand{
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
		std::cout << yellow << "[commit " << currCommit.sha1 << ']' << Reset << std::endl;
		std::cout << cyan << "mmessage:" << std::endl;
		std::cout << std::endl <<Reset;

		std::cout << currCommit.message;

		std::cout  << std::endl << std::endl;
		sha1 = currCommit.parent;
	}
	return 0;
}
