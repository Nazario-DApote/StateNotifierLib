#include "Utils.h"
//
#include <fstream>
#include <io.h>
#include <string>
#include <direct.h>
#include <list>
#include "Shlwapi.h"
//
#include <boost/date_time.hpp>
//
using namespace std;

bool ExtractSectionID (const string& line, UINT& result)
{
	bool bRes = false;

	result = 0;
	string delimiter = "##";
	string inner_delimiter = "=";

	string tmpString(line);
	string token;

	size_t start = 0U;
	size_t end = tmpString.find(delimiter);

	while (end != string::npos)
	{
		token = tmpString.substr(start, end - start);
		start = end + delimiter.length();
		end = tmpString.find(delimiter, start);

		size_t idPos = token.find(inner_delimiter);
		if ( idPos != string::npos )
		{
			string strID = token.substr(idPos + 1);
			if ( ::Parse<UINT>("SectionID", strID.c_str(), result) )
			{
				bRes = true;
				break; // OK found!
			}
		}
	}

	return bRes;
}

bool fileExists(const char *filename)
{
	ifstream ifile(filename);
	return ifile;
}

bool folderExists(const char* folderName) 
{
	if (_access(folderName, 0) == -1) {
		//File not found
		return false;
	}

	DWORD attr = GetFileAttributes((LPCSTR)folderName);
	if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
		// File is not a directory
		return false;
	}

	return true;
}

bool createFolder(std::string folderName) 
{
	list<std::string> folderLevels;
	char* c_str = (char*)folderName.c_str();

	// Point to end of the string
	char* strPtr = &c_str[strlen(c_str) - 1];

	// Create a list of the folders which do not currently exist
	do {
		if (folderExists(c_str)) {
			break;
		}
		// Break off the last folder name, store in folderLevels list
		do {
			strPtr--;
		} while ((*strPtr != '\\') && (*strPtr != '/') && (strPtr >= c_str));
		folderLevels.push_front(string(strPtr + 1));
		strPtr[1] = 0;
	} while (strPtr >= c_str);

	if (_chdir(c_str)) {
		return false;
	}

	// Create the folders iteratively
	for (list<std::string>::iterator it = folderLevels.begin(); it != folderLevels.end(); ++it) {
		if (CreateDirectory(it->c_str(), nullptr) == 0) {
			return false;
		}
		_chdir(it->c_str());
	}

	return true;
}

std::string getExectablePath() 
{
	std::vector<char> executablePath(MAX_PATH);

	// Try to get the executable path with a buffer of MAX_PATH characters.
	DWORD result = ::GetModuleFileNameA( nullptr, &executablePath[0], static_cast<DWORD>(executablePath.size()) );

	// As long the function returns the buffer size, it is indicating that the buffer
	// was too small. Keep enlarging the buffer by a factor of 2 until it fits.
	while(result == executablePath.size()) 
	{
		executablePath.resize(executablePath.size() * 2);
		result = ::GetModuleFileNameA( nullptr, &executablePath[0], static_cast<DWORD> (executablePath.size()) );
	}

	// If the function returned 0, something went wrong
	if(result == 0) 
	{
		throw std::runtime_error("GetModuleFileName() failed");
	}

	// We've got the path, construct a standard string from it
	return std::string(executablePath.begin(), executablePath.begin() + result);
}

std::vector<std::string> getFileListInFolder(const std::string& folder, const std::string& fileNameMatch)
{
	std::vector<std::string> fileList;
	if( ::folderExists(folder.c_str()) )
	{
		WIN32_FIND_DATA fd;
		std::string searchPath = folder + "\\" + fileNameMatch;
		HANDLE hFind = ::FindFirstFile(searchPath.c_str(), &fd); 
		if(hFind != INVALID_HANDLE_VALUE) 
		{ 
			do 
			{ 
				// read all (real) files in current folder, delete '!' read other 2 default folder . and ..
				if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
				{
					fileList.push_back(fd.cFileName);
				}
			} while( ::FindNextFile(hFind, &fd) ); 

			::FindClose(hFind); 
		} 
	}
	return fileList;
}

bool isRelativePath(const std::string& path)
{
	return ::PathIsRelative(path.c_str()) == TRUE; // Returns TRUE if the path is relative, or FALSE if it is absolute.
}

bool makeAbsolutePath(const std::string& path, std::string& fullPath)
{
	DWORD  retval=0;
	char buffer[MAX_PATH]=""; 
	char buf[MAX_PATH]=""; 
	char** lppPart={nullptr};

	retval = ::GetFullPathName(path.c_str(), MAX_PATH, buffer, lppPart);
	if (retval == 0) 
	{
		// Handle an error condition.
		return false;
	}

	fullPath = buffer;
	return true;
}

std::string getFileName(const std::string fullPath)
{
	// Remove directory if present.
	// Do this before extension removal in case directory has a period character.
	const size_t last_slash_idx = fullPath.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
		return fullPath.substr(last_slash_idx+1);

	return fullPath;
}

std::string getFileNameWithoutExtension(const std::string fileName)
{
	// Remove extension if present.
	const size_t last_period_idx = fileName.find_last_of('.');
	if (std::string::npos != last_period_idx)
		return fileName.substr(0,last_period_idx);

	return fileName;
}

std::string getDirectoryName(const std::string fullPath)
{
	const size_t last_slash_idx = fullPath.find_last_of("/\\");
	if (std::string::npos != last_slash_idx)
		return fullPath.substr(0,last_slash_idx);

	return fullPath;
}

std::string getStringTimeStamp(const char* format_string)
{
	std::stringstream date_stream;
	boost::posix_time::time_facet* facet = new boost::posix_time::time_facet(format_string); // Ti comment: pointer deleted by boost library
	date_stream.imbue(std::locale(date_stream.getloc(), facet));
	date_stream << boost::posix_time::microsec_clock::local_time();
	return date_stream.str();
}

std::string time_t_toString(const time_t time)
{
	std::stringstream ss;
	ss << time;
	return ss.str();
}
