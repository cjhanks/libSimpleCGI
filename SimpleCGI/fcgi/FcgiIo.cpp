#include "FcgiIo.hpp"

#include <dirent.h>
#include <fstream>

#include "SimpleCGI/common/Logging.hpp"

using std::ifstream;
using std::istreambuf_iterator;
using std::string;

namespace fcgi {
FileAsset::FileAsset(const string& path, struct stat fstat,
           CacheMode cacheMode)
  : filePath(path), fileStat(fstat), cacheMode(cacheMode)
{
  if (cacheMode == CacheMode::EAGER) {
    cacheData = LoadFromFile();
  }
}

FileAsset::operator string()
{
  switch (cacheMode) {
    case CacheMode::EAGER:
      break;

    case CacheMode::LAZY:
      cacheData = LoadFromFile();
      break;

    case CacheMode::NOCACHE:
      return LoadFromFile();
  }

  return cacheData;
}

string
FileAsset::LoadFromFile()
{
  ifstream ifs(filePath);
  return string((istreambuf_iterator<char>(ifs)),
                (istreambuf_iterator<char>()));
}

void
Assets::AddSearchPath(const string& path, CacheMode cacheMode)
{
  ImplAddSearchPath(path, path, cacheMode);
}

void
Assets::ImplAddSearchPath(const string& origin, const string& path,
              CacheMode cacheMode)
{
  struct dirent** nameList;
  int n = scandir(path.c_str(), &nameList, nullptr, alphasort);

  if (n < 0) {
    LOG(ERROR) << "Unable to observe directory: " << path;
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
      ImplAddSearchPath(origin, newPath, cacheMode);
    } else if (S_ISLNK(fstat.st_mode) || S_ISREG(fstat.st_mode)) {
      FileAsset asset(absPath, fstat, cacheMode);
      fileAssets.emplace(
          std::make_pair(newPath.substr(origin.size() + 1), asset));
    }

  }

  for (ssize_t i = 0; i < n; ++i)
    free(nameList[i]);

  free(nameList);
}
} // ns fcgi
