#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

namespace fs = std::filesystem;

int tagMain(int argc, const char* argv[]);
Command TagCommand{
	tagMain,
	"Create, list or delete a tag object"
};


int tagMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.add("delete", 'd', "delete tags");
	argParser.add("list", 'l', "list tag names");
	argParser.footer("<tagname> ...");
	argParser.parse_check(argc, argv);

	if (!GIT_DIR.has_value()) {
		std::cout << red << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << Reset << std::endl;
		return 1;
	}

	fs::path tagDir = GIT_DIR.value();
	tagDir /= "refs";
	tagDir /= "tags";

	if (!fs::exists(tagDir)) {
		if (!fs::create_directories(tagDir)) {
			std::cerr << red << "Error: unable to create tag directory" << Reset << std::endl;
			return 1;
		}
	}

	if (argParser.exist("list")) {
		for (auto &it : fs::directory_iterator(tagDir)) {
			std::cout << it.path().filename().generic_string() << std::endl;
		}
		return 0;
	}


	if (argParser.rest().size() == 0) {
		std::cout << argParser.usage();
		return 0;
	}

	if (argParser.exist("delete")) {
		bool error = false;
		for (auto tagName : argParser.rest()) {
			fs::path tagPath = tagDir / tagName;
			if (fs::is_regular_file(tagPath)) {
				if (fs::remove(tagPath) != 1) {
					error = true;
				}
				else {
					std::cout << yellow << "Deleted tag '" << tagName << '\'' << Reset << std::endl;
				}
			}
		}
		return error ? 1 : 0;
	}
	else {
		std::string sha1 = readMain();
		commit dummy;
		if (!checkSha1(sha1) || !readCommit(sha1, dummy)) {
			std::cerr << red << "Error: failed to resolve 'HEAD' as a valid commit" << Reset << std::endl;
			return 1;
		}

		bool error = false;
		for (auto tagName : argParser.rest()) {
			fs::path tagPath = tagDir / tagName;
			if (fs::exists(tagPath)) {
				std::cerr << red << "Error: tag '" << tagName << "' already exists" << Reset << std::endl;
				error = true;
			}
			else {
				write_file(tagPath, sha1.data(), sha1.length());
				std::cout << "Added tag '" << blod_blue  << tagName << Reset << '\'' << std::endl;
			}
		}
		return error ? 1 : 0;
	}

	return 0;
}
