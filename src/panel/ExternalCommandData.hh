//
// ExternalCommandData.hh for pekwm
// Copyright (C) 2022-2025 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//
#ifndef _PEKWM_PANEL_EXTERNAL_COMMAND_DATA_HH_
#define _PEKWM_PANEL_EXTERNAL_COMMAND_DATA_HH_

#include <string>
#include <vector>

#include "pekwm_panel.hh"
#include "Observable.hh"
#include "PanelConfig.hh"
#include "VarData.hh"

/**
 * Collection of data from external commands.
 *
 * Commands output is collected in the below format at the specified
 * interval.
 *
 * key data
 *
 */
class ExternalCommandData : public Observable
{
public:
	class CommandProcess
	{
	public:
		CommandProcess(const std::string& command, uint interval_s,
			       const std::string& assign);
		~CommandProcess();

		int getFd() const { return _fd; }
		pid_t getPid() const { return _pid; }
		std::string& getBuf() { return _buf; }
		const std::string& getAssign() const { return _assign; }

		bool start();

		bool checkInterval(struct timespec *now)
		{
			return now->tv_sec >= _next_interval.tv_sec;
		}

		void reset();

	private:
		std::string _command;
		uint _interval_s;
		std::string _assign;
		struct timespec _next_interval;

		pid_t _pid;
		int _fd;
		std::string _buf;
	};

	ExternalCommandData(const PanelConfig& cfg, VarData& var_data);
	virtual ~ExternalCommandData();

	void refresh(fdFun addFd, void *opaque);
	bool input(int fd);
	void done(pid_t pid, fdFun removeFd, void *opaque);

protected:
	void append(std::string &buf, const char *data, size_t size,
		    const std::string &assign);
	void parseOutput(const std::string& buf);
	void parseLine(const std::string& line);

private:
	const PanelConfig& _cfg;
	VarData& _var_data;

	std::vector<CommandProcess> _command_processes;
};


#endif // _PEKWM_PANEL_EXTERNAL_COMMAND_DATA_HH_
