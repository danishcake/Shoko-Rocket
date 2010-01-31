#pragma once
#include <string>
#include <map>

struct ProgressRecord
{	
	bool completed;
	ProgressRecord()
	{
		completed = true;
	}	
};

class Progress
{
private:
	Progress(void);
	static Progress& GetInstance();
	
	std::map<std::string, ProgressRecord> progress_;
public:
	~Progress(void);
	static ProgressRecord GetProgress(std::string _filename);
	static void SetProgress(std::string _filename, ProgressRecord _record);
};
