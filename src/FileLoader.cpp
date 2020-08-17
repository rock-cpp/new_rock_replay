#include "FileLoader.hpp"

#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>

std::vector<std::string> FileLoader::parseFileNames(int argc, char* argv[], std::vector<std::regex>& regExps, std::map<std::string, std::string>& logfiles2Prefix)
{
    std::vector<std::string> filenames;
    
    for(int i = 1; i < argc; i++)
    {
        std::string argv2String(argv[i]);
        std::string whiteList = "--white-list=";
        std::size_t pos = argv2String.find(whiteList);
        
        if(pos != std::string::npos) 
        {
            std::string params = argv2String.substr(whiteList.length(), argv2String.length());
            boost::split(regExps, params, boost::is_any_of(","));
            continue;
        }
    
        std::string prefix = "--prefix=";
        pos = argv2String.find(prefix);
    
        if(pos != std::string::npos) 
        {
            std::string params = argv2String.substr(prefix.length(), argv2String.length());
            std::vector<std::string> prefixTuples;
            boost::split(prefixTuples, params, boost::is_any_of(","));
            
            for(const std::string pair : prefixTuples)
            {
                std::size_t sep = pair.find(":");
                if(sep != std::string::npos)
                {
                    std::string logfile = pair.substr(0, sep);
                    std::string prefix = pair.substr(sep + 1, pair.length());
                    
                    std::cout << "prefixing: " << logfile << " -> " << prefix << std::endl;
                    logfiles2Prefix.insert(std::make_pair(logfile, prefix));
                }
            }
            
            continue;
        }
        
        struct stat file_stat;
        if(stat(argv[i], &file_stat) == -1)
        {
            std::cerr << "stat error: couldn't open folder" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        if(S_ISDIR(file_stat.st_mode))
        {
            DIR *dir = opendir(argv[i]);
            if(!dir) 
            {
                std::cerr << "directory opening error" << std::endl;
                exit(EXIT_FAILURE);   
            }
            else
            {
                dirent *entry;
                std::string dir_path = std::string(argv[i]);
                if(dir_path.back() != '/')
                    dir_path.append("/");
                
                while((entry = readdir(dir)) != 0)
                {
                    std::string filename = entry->d_name;
                    if(filename.substr(filename.find_last_of(".") + 1) == "log")
                    {
                        if(filename == "orocos.log")
                            continue;
                        
                        filenames.push_back(std::string(dir_path).append(filename));
                    }
                }
            }
            closedir(dir);
        }
        else if(S_ISREG(file_stat.st_mode))
        {
            filenames.push_back(argv[i]);
        }
    }
    
    return filenames;
}
