#include "Progress.h"
#include <TinyXML.h>
#include "Logger.h"
#include <boost/foreach.hpp>

Progress::Progress(void)
{
	Logger::DiagnosticOut() << "Loading progress record\n";
	//Load XML
	TiXmlDocument doc = TiXmlDocument("Progress.xml");
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("Progress");
	if(root)
	{
		TiXmlElement* record = root->FirstChildElement("Record");
		int record_number = 0;
		while(record)
		{
			std::string filename = "";
			bool completed = false;
			bool error = false;
			error |= (record->QueryValueAttribute("Filename", &filename) != TIXML_SUCCESS);
			error |= (record->QueryValueAttribute("Completed", &completed) != TIXML_SUCCESS);
			if(error)
				Logger::DiagnosticOut() << "Error parsing record number: " << record_number << ". Possible data: Filename:" << filename << ", completed: " << completed << "\n";
			else
			{
				ProgressRecord pr;
				pr.completed = completed;
				progress_[filename] = pr;
			}
			record = record->NextSiblingElement("Record");
			record_number++;
		}
	}
}

Progress::~Progress(void)
{
	Logger::DiagnosticOut() << "Saving progress record\n";
	//Save XML
	TiXmlDocument doc = TiXmlDocument("Progress.xml");
	TiXmlDeclaration* decl = new TiXmlDeclaration();
	TiXmlElement* root = new TiXmlElement("Progress");
	doc.LinkEndChild(decl);
	doc.LinkEndChild(root);
	std::pair<std::string, ProgressRecord> item;
	BOOST_FOREACH(item, progress_)
	{
		TiXmlElement* record = new TiXmlElement("Record");
		record->SetAttribute("Filename", item.first);
		record->SetAttribute("Completed", item.second.completed);

		root->LinkEndChild(record);
	}
	doc.SaveFile();
}

Progress& Progress::GetInstance()
{
	static Progress instance;
	return instance;
}

ProgressRecord Progress::GetProgress(std::string _filename)
{
	std::map<std::string, ProgressRecord>& progress = GetInstance().progress_;
	if(progress.find(_filename) != progress.end())
	{
		return progress[_filename];
	} else
	{
		ProgressRecord pr;
		pr.completed = false;
		return pr;
	}
}

void Progress::SetProgress(std::string _filename, ProgressRecord _record)
{
	std::map<std::string, ProgressRecord>& progress = GetInstance().progress_;
	if(progress.find(_filename) != progress.end())
	{
		progress[_filename].completed |= _record.completed;
	} else
	{
		progress[_filename] = _record;
	}
}