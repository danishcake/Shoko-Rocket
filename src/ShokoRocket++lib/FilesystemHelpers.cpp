#include "FilesystemHelpers.h"
#include <boost/filesystem.hpp>
#include "Logger.h"

namespace FilesystemHelpers
{
	void EnsureFolderPresent(std::string _path)
	{
		boost::filesystem::path path(_path);
		path = path.parent_path();
		boost::filesystem::path built_path;

		for(boost::filesystem::path::iterator it = path.begin(); it != path.end(); ++it)
		{
			built_path /= *it;
			if(!boost::filesystem::exists(built_path))
			{
				boost::filesystem::create_directory(built_path);
				Logger::DiagnosticOut() << "Folder didn't exist, creating: " << built_path.directory_string() << "\n";
			}
		}
	}
}