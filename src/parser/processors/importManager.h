#ifndef _importManager
#define _importManager

#include "../parser.h"
#include "libcDefiner.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ImportManager {
public:
  ImportManager();

  void processImports(ProgramNode *program);

private:
  LibCDefiner libcDefiner;

  std::map<std::string, std::filesystem::path> libFilesO;
  std::map<std::string, std::filesystem::path> libFilesH;
  std::map<std::string, bool> visitedFiles;
  std::set<std::string> importedObjFiles;

  void handleFileImports(ProgramNode *program);
  std::vector<std::string> &getObjectFiles();
  ProgramNode *readFile(std::filesystem::path path);
  std::filesystem::path getPackagePath(std::string package);

  void error(std::string mst);
};

#endif