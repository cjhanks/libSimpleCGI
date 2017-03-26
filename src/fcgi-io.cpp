#include "fcgi-io.hpp"

#include <dirent.h>
#include <fstream>

#include "logging.hpp"

using std::ifstream;
using std::istreambuf_iterator;
using std::string;

namespace fcgi {
FileAsset::FileAsset(const string& path, struct stat fstat, 
                     CacheMode cacheMode)
    : filePath(path), fileStat(fstat), cacheMode(cacheMode)
{
    if (cacheMode == CacheMode::EAGER) {
        cacheData = loadFromFile();
    }
}

FileAsset::operator string()
{
    switch (cacheMode) {
        case CacheMode::EAGER:
            break;

        case CacheMode::LAZY:
            cacheData = loadFromFile();
            break;

        case CacheMode::NOCACHE:
            return loadFromFile();
    }

    return cacheData;
}

string
FileAsset::loadFromFile()
{   
    ifstream ifs(filePath);
    return string((istreambuf_iterator<char>(ifs)),
                  (istreambuf_iterator<char>()));
}

void
FileAsset::dumpTo(std::ostream& strm) const
{
    strm << "    AbsPath: "  << filePath         << std::endl
         << "    Cached:  "  << cacheData.size() << std::endl 
         << "    Size:    "  << fileStat.st_size << std::endl;
}
    
void
Assets::addSearchPath(const string& path, CacheMode cacheMode)
{
    implAddSearchPath(path, path, cacheMode);
}

void
Assets::implAddSearchPath(const string& origin, const string& path, 
                          CacheMode cacheMode)
{
    struct dirent** nameList;
    int n = scandir(path.c_str(), &nameList, nullptr, alphasort);

    if (n < 0) {
        LOG(WARNING) << "Unable to observe directory: " << path;
        return;
    } 

    for (ssize_t i = 0; i < n; ++i) {
        string pthName(nameList[i]->d_name);
        string newPath = path + "/" + pthName;
        struct stat fstat;

        if (pthName == "." || pthName == "..") {
            continue;
        }

        if (stat(newPath.c_str(), &fstat) < 0) {
            continue;
        }
        
        char*  memPath = realpath(newPath.c_str(), nullptr);
        if (memPath == nullptr) {
            continue;
        }
        string absPath(memPath);
        free(memPath);

        if (S_ISDIR(fstat.st_mode)) {
            implAddSearchPath(origin, newPath, cacheMode);
        } else if (S_ISLNK(fstat.st_mode) || S_ISREG(fstat.st_mode)) {
            FileAsset asset(absPath, fstat, cacheMode);
            fileAssets.emplace(
                    std::make_pair(newPath.substr(origin.size() + 1), asset));
        }

    }

    for (ssize_t i = 0; i < n; ++i) {
        free(nameList[i]);
    }

    free(nameList);
}

void 
Assets::dumpTo(std::ostream& strm) const
{
    strm << string(80, '=') << std::endl;
    strm << "File Assets:" << std::endl;
    for (auto& ref: fileAssets) {
        strm << string(80, '-') << std::endl;
        strm << ref.first << std::endl;
        ref.second.dumpTo(strm);
    }
}
} // ns fcgi
