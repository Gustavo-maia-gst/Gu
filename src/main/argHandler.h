#ifndef _argHandler
#define _argHandler

#include <map>
#include <set>
#include <string>
#include <vector>

class ArgHandler {
public:
  void defArg(std::string name, std::set<std::string> validValues = {},
              std::string shortcut = "");
  std::pair<bool, std::string> getArg(std::string name);
  std::vector<std::string> &getPosArgs();

  void parseArgs(int argc, char **argv);
  void parseError(std::string msg);

private:
  std::vector<std::string> posArgs;
  std::map<std::string, std::string> shortcutToName;
  std::map<std::string, std::string> nameToValue;
  std::map<std::string, std::set<std::string>> nameToValidValues;
};

#endif