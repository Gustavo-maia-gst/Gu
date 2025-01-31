#include "importManager.h"
#include <string>
#include <vector>

namespace fs = std::filesystem;

const char *GU_LIB_ENV_VAR = "GU_LIB_PATH";
const char *homePath = std::getenv("HOME");
const std::string defaultLibPath =
    "/usr/lib:/usr/local/lib:" + std::string(homePath) + "/.local/lib";
const char *envLibPathPtr = std::getenv(GU_LIB_ENV_VAR);
const std::string envLibPath = envLibPathPtr ? envLibPathPtr : "";

ImportManager::ImportManager() {
  std::string allPaths = defaultLibPath + envLibPath + ":";
  std::string currPath = "";

  for (ulint i = 0; i < allPaths.size(); i++) {
    if (allPaths[i] != ':') {
      currPath += allPaths[i];
      continue;
    }

    if (!fs::exists(currPath) || !fs::is_directory(currPath)) {
      currPath = "";
      continue;
    }

    for (auto &entry : fs::directory_iterator(currPath)) {
      if (!entry.is_regular_file())
        continue;

      auto filename = entry.path().filename().string();
      auto extension = entry.path().extension().string();
      if (filename.substr(0, 2) != "GU")
        continue;

      if (extension == ".o")
        libFilesO[filename.substr(2, filename.length() - 4)] = entry.path();
      else if (extension == ".guh")
        libFilesH[filename.substr(2, filename.length() - 6)] = entry.path();
    }

    currPath = "";
  }
}

void ImportManager::processImports(ProgramNode *program) {
  LibCDefiner libcDefiner{};
  std::vector<AstNode *> storedChildren;
  for (auto child : program->_children)
    storedChildren.push_back(child);
  program->_children.clear();

  libcDefiner.addPosixSyscallsDefs(program);
  handleFileImports(program);

  for (auto child : storedChildren)
    program->_children.push_back(child);
}

void ImportManager::handleFileImports(ProgramNode *program) {
  for (auto &import : program->imports) {
    ProgramNode *imported;
    std::string filePath;

    if (import.second)
      filePath = import.first;
    else
      filePath = getPackagePath(import.first);

    if (visitedFiles.find(filePath) != visitedFiles.end()) {
      if (visitedFiles[filePath])
        continue;
      error("Circular import detected");
    }

    visitedFiles[filePath] = false;

    imported = readFile(filePath);
    handleFileImports(imported);
    program->merge(imported);

    visitedFiles[filePath] = true;
  }
}

fs::path ImportManager::getPackagePath(std::string package) {
  if (libFilesO.find(package) == libFilesO.end() ||
      libFilesH.find(package) == libFilesH.end())
    error("Package " + package +
          " was not found, make sure that the package exists and there are "
          "binary and declaration files for the package");

  auto declPath = libFilesH[package];
  auto objPath = libFilesO[package];

  importedObjFiles.insert(objPath);

  auto stream = new std::ifstream(declPath);
  if (!stream->is_open())
    error("Error while opening declaration file for package " + package +
          ", make sure you have the rights to access the package");

  return declPath;
}

ProgramNode *ImportManager::readFile(fs::path path) {
  auto lexer = Lexer::fromFile(path.string());
  AstParser parser(lexer);
  auto program = parser.parseProgram();
  return program;
}

void ImportManager::error(std::string msg) {
  std::cerr << "import error: " + msg << std::endl;
  exit(1);
}