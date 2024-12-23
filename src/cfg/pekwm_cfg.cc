//
// pekwm_cfg.cc for pekwm
// Copyright (C) 2021-2023 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#include "Compat.hh"
#include "CfgParser.hh"
#include "Util.hh"

#include <map>
#include <string>

extern "C" {
#include <getopt.h>
}


static void usage(const char* name, int ret)
{
	std::cout << "usage: " << name << " [-j]" << std::endl;
	std::cout << "  -j --json file    dump file as JSON" << std::endl;
	exit(ret);
}

static void
jsonDumpSection(CfgParser::Entry *entry)
{
	// map keeping track of seen section names to ensure unique names
	// in the output.
	std::map<std::string, int> sections;

	CfgParser::Entry::entry_cit it = entry->begin();
	for (; it != entry->end(); ++it) {
		if (it != entry->begin()) {
			std::cout << ",";
		}

		if ((*it)->getSection()) {
			std::string name = (*it)->getName();
			if (! (*it)->getValue().empty()) {
				name += "-" + (*it)->getValue();
			}

			std::map<std::string, int>::iterator s_it =
				sections.find(name);
			if (s_it == sections.end()) {
				sections[name] = 0;
			} else {
				name += "-" + std::to_string(s_it->second);
				s_it->second = s_it->second + 1;
			}

			std::cout << "\"" << name << "\": {" << std::endl;
			jsonDumpSection((*it)->getSection());
			std::cout << "}" << std::endl;
		} else {
			std::cout << "\"" << (*it)->getName() << "\"";
			std::cout << ": \"" << (*it)->getValue()
				  << "\"" << std::endl;
		}
	}
}

static void
jsonDump(const std::string& path,
	 const std::map<std::string, std::string> &cfg_env)
{
	CfgParser cfg(CfgParserOpt(""));
	std::map<std::string, std::string>::const_iterator it =
		cfg_env.begin();
	for (; it != cfg_env.end(); ++it) {
		cfg.setVar(it->first, it->second);
	}
	cfg.parse(path);

	std::cout << "{";
	jsonDumpSection(cfg.getEntryRoot());
	std::cout << "}" << std::endl;
}

int main(int argc, char* argv[])
{
	// Limit access, this will not allow execution of commands in
	// configuration files.
	pledge_x("stdio rpath", "");

	std::string cfg_path;
	std::map<std::string, std::string> cfg_env;

	static struct option opts[] = {
		{const_cast<char*>("json"), required_argument, nullptr, 'd'},
		{const_cast<char*>("env"), required_argument, nullptr, 'e'},
		{const_cast<char*>("help"), no_argument, nullptr, 'h'},
		{nullptr, 0, nullptr, 0}
	};

	int ch;
	while ((ch = getopt_long(argc, argv, "e:j:h", opts, nullptr)) != -1) {
		switch (ch) {
		case 'e': {
			std::vector<std::string> vals;
			if (Util::splitString(optarg, vals, "=", 2) == 2) {
				cfg_env["$" + vals[0]] = vals[1];
			} else {
				usage(argv[0], 1);
			}
		}
		case 'j':
			cfg_path = optarg;
			break;
		case 'h':
			usage(argv[0], 0);
			break;
		default:
			usage(argv[0], 1);
			break;
		}
	}

	if (! cfg_path.empty()) {
		jsonDump(cfg_path, cfg_env);
	}

	return 0;
}
