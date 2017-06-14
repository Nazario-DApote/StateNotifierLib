#ifndef Utils_h__
#define Utils_h__

#include <string>
#include <vector>
#include <Windows.h>
#include <boost/lexical_cast.hpp>
#include "LogMgr.h"

//************************************
// Method:    fileExists
// FullName:  fileExists
// Access:    public 
// Returns:   TRUE if exists otherwise FALSE
// Qualifier: Check if filename exists
// Parameter: const char * filename ( file to check )
//************************************
bool fileExists(const char *filename);

//************************************
// Method:    ExtractSectionID
// FullName:  ExtractSectionID
// Access:    public 
// Returns:   bool
// Qualifier: This function extract the ID from the EPS transaction marker ##TICKET##START##ID=0000000002##
// Parameter: const string & line
// Parameter: UINT result
//************************************
bool ExtractSectionID (const std::string& line, UINT& result);

//************************************
// Method:    Parse
// FullName:  Parse
// Access:    public static 
// Returns:   (TRUE if parse success)
// Qualifier:
// Parameter: const string & paramName (The name of parameter to retries: its only for log information)
// Parameter: const string & strValue (The string variable to parse)
// Parameter: T & res (res contains the parsed value if Parse return TRUE otherwise its value is not changed)
//************************************
template <typename T>
bool Parse(const std::string& paramName, const std::string& strValue, T& res)
{
	T vl;
	try
	{
		vl = boost::lexical_cast<T>(strValue);
		res = vl;
		return true;
	}
	catch(const boost::bad_lexical_cast &)
	{
		LogMgr::Error("[Utils::Parse] Bad %s format", paramName.c_str());
		return false;
	}
}


//************************************
// Method:    createFolder
// FullName:  createFolder
// Access:    public 
// Returns:   bool
// Qualifier: Returns false on success, true on error
// Parameter: std::string folderName
//************************************
bool createFolder(std::string folderName);

//************************************
// Method:    folderExists
// FullName:  folderExists
// Access:    public 
// Returns:   bool
// Qualifier: Return true if the folder exists, false otherwise 
// Parameter: const char * folderName
//************************************
bool folderExists(const char* folderName);


//************************************
// Method:    getExectablePath
// FullName:  getExectablePath
// Access:    public 
// Returns:   std::string
// Qualifier: Get the full path of the current executable module
//************************************
std::string getExectablePath();

//************************************
// Method:    getFileListInFolder
// FullName:  getFileListInFolder
// Access:    public 
// Returns:   a vector of file that matches
// Qualifier: Retrieve the list of file in specified folder
// Parameter: const std::string & folder
// Parameter: const std::string & fileNameMatch
//************************************
std::vector<std::string> getFileListInFolder(const std::string& folder, const std::string& fileNameMatch);

bool isRelativePath(const std::string& path);

bool makeAbsolutePath(const std::string& path, std::string& fullPath);

std::string getFileName(const std::string fullPath);

std::string getFileNameWithoutExtension(const std::string fileName);

std::string getDirectoryName(const std::string fullPath);

std::string getStringTimeStamp(const char* format_string);

std::string time_t_toString(const time_t);

#endif // Utils_h__
