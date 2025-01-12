
#include "argHandler.h"
#include <iostream>

void ArgHandler::defArg(std::string name, std::set<std::string> validValues,
                        std::string shortcut) {
  if (name.empty())
    return;

  nameToValidValues[name] = validValues;
  if (!shortcut.empty())
    shortcutToName[shortcut] = name;
}

std::pair<bool, std::string> ArgHandler::getArg(std::string name) {
  if (nameToValue.find(name) == nameToValue.end())
    return {false, ""};

  auto value = nameToValue[name];
  return {true, value};
}

std::vector<std::string> &ArgHandler::getPosArgs() { return posArgs; }

void ArgHandler::parseError(std::string msg) {
  std::cerr << "arg parsing error: " << msg << "\n";
  std::cerr << "usage: gu [OPTIONS] file\n";
  std::cerr << "Options:\n";
  std::cerr << "\t[--assembly -S] [ obj | asm | basicIR | IR] -> Defines the "
               "output format, the default is obj, asm are the Assembly format "
               "of the target machine, IR is the final generated IR and "
               "basicIR is the non-optimized basic form of IR\n";
  std::cerr
      << "\t--output -o filename -> Uses the specified filename as output\n";
  std::cerr << "\t[--compile -c] -> Instructs to compile the code without main"
               "function validation\n";
  std::cerr << "\t[--opt -O] level -> Specify the optimization level to use "
               "(0, 1, 2, 3). "
               "The default value is 2\n";
  exit(1);
}

void ArgHandler::parseArgs(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg[0] == '-' && arg.length() > 1) {
      bool isShortcut = arg[1] != '-';
      std::string refName = isShortcut ? arg.substr(1) : arg.substr(2);
      if (isShortcut) {
        if (shortcutToName.find(refName) == shortcutToName.end())
          parseError("Invalid arg " + refName);
        refName = shortcutToName[refName];
      }

      if (++i < argc) {
        std::string value(argv[i]);
        if (nameToValidValues.find(refName) != nameToValidValues.end()) {
          bool isValid = nameToValidValues[refName].empty() ||
                         nameToValidValues[refName].find(value) !=
                             nameToValidValues[refName].end();
          if (!isValid)
            parseError("Invalid value for arg " + refName + ": " + value);
        }

        if (value[0] != '-')
          nameToValue[refName] = value;
        else {
          nameToValue[refName] = "";
          i--;
        }
      }
    } else {
      posArgs.push_back(arg);
    }
  }
}