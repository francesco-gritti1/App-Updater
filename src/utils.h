
#ifndef UTILS_h
#define UTILS_h


#include <optional>
#include <string>

#define PKG_FILENAME  "packages.json"
#define PKG_DIRECTORY "backup/"
#define TRACKED_PKG_FILEPATH  PKG_DIRECTORY"tracked_packages.json"


int execute_install (const std::string& script_path, const std::string& gitlink, const std::string& repo_name, const std::string package_name);
std::optional<std::string> get_installed_package_version (const std::string& package_name);
std::optional<std::string> git_exec_ls_remote (const std::string& repo_url);
void update_packages ();
void install_missing ();
void check_packages ();
void check_fs ();


#endif