#ifndef __FCGI_IO_HPP
#define __FCGI_IO_HPP 

////////////////////////////////////////////////////////////////////////////////
// These classes provide a way to make assets stored on disk easier to locate
// Typical usage is as follows:
// 
// find ./path 
// ./path/cat/0
// ./path/cat/1

// find /tmp/path
// /tmp/path/cat/1
//
// Assets assets;
// assets.AddSearchPath("./path/", CacheMode::LAZY);
// assets.AddSearchPath("/tmp/path/", CacheMode::EAGER);
//
// assets.getAsset("cat/1"); 
// Returns ./path/cat/1
//
// assets.getAsset('cat/0");
// Returns /tmp/path/cat/0
//
// When an asset searched for is not discovered an exception
// `MissingAssetException` is thrown.
//
//
// There are three types of cacheing.
// CacheMode::LAZY
//  Don't initially load into memory, but once loaded keep in memory.
//
// CacheMode::EAGER
//  Load it once the CGI application is launched
//
// CacheMode::NOCACHE
//  Load it from disk each time.
//
//  Note that all cache types are "eager" in the sense that files which did not //  exist on application launch are not accessible.  It will scan the filesystem
//  once at load and never again.

#include <sys/stat.h>
#include <string>
#include <unordered_map>


namespace fcgi {
enum class CacheMode {
  LAZY,
  EAGER,
  NOCACHE
};

class FileAsset {
public:
  FileAsset(const std::string& path, struct stat fstat,
        CacheMode cacheMode);
  operator std::string();
  
  void 
  DumpTo(std::ostream& strm) const;

private:
  const std::string filePath;
  const struct stat fileStat;
  const CacheMode cacheMode;
  std::string cacheData;

  std::string
  LoadFromFile();
};

class Assets {
public:
  void
  AddSearchPath(const std::string& path, CacheMode cacheMode);
  
  void 
  DumpTo(std::ostream& strm) const;
private:
  friend class CompiledAssets;
  std::unordered_map<std::string, FileAsset> fileAssets;
  
  void
  ImplAddSearchPath(const std::string& origin, const std::string& path, 
            CacheMode cacheMode);
};
} // ns fcgi

#endif //__FCGI_IO_HPP
