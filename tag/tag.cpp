#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

namespace fs = std::filesystem;

int tagMain(int argc, const char* argv[]);
extern Command TagCommand{
	tagMain,
	// TODO "Create, list or delete a tag object"
	"Create a tag object"
};


int tagMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.footer("<tagname>");
	argParser.parse_check(argc, argv);
	if (!GIT_DIR.has_value()) {
		std::cout << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << std::endl;
		return 1;
	}

	fs::path tagDir = GIT_DIR.value();
	tagDir /= "refs";
	tagDir /= "tags";

	if (!fs::exists(tagDir)) {
		if (!fs::create_directories(tagDir)) {
			std::cerr << "Error: unable to create tag directory" << std::endl;
			return 1;
		}
	}

	if (argParser.rest().size() > 1) {
		std::cout << "Warning: too many tags. can only add one tag at a time." << std::endl;
		return 1;
	}
	else if (argParser.rest().size() == 0) {
		argParser.usage();
		return 0;
	}

	std::string tagName = argParser.rest()[0];

	fs::path tagPath = tagDir / tagName;
	if (fs::exists(tagPath)) {
		std::cerr << "Error: tag '" << tagName << "' already exists";
		return 1;
	}
	
	std::string sha1 = readMain();
	commit dummy;
	if (!checkSha1(sha1) || !readCommit(sha1, dummy)) {
		std::cerr << "Error: failed to resolve 'HEAD' as a valid commit";
	}

	write_file(tagPath, sha1.data(), sha1.length());
	return 0;
}
