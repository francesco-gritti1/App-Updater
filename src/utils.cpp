#include <filesystem>
#include <iostream>
#include <libconfig.h++>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <array>
#include "utils.h"

#include "json.h"
using json = nlohmann::json;




#define PKG_FILENAME  "packages.json"
#define PKG_DIRECTORY "backup/"
#define TRACKED_PKG_FILEPATH  PKG_DIRECTORY"tracked_packages.json"

namespace fs = std::filesystem;




int execute_install (const std::string& script_path, const std::string& gitlink, const std::string& repo_name, const std::string package_name)
{
    // Costruisci comando shell da eseguire, aggiungendo i parametri
    std::string command = script_path + " " + gitlink + " " + repo_name + " " + package_name;
    std::cout << "install command " << command << std::endl;

    // Esegue il comando e cattura il valore di ritorno (exit status)
    int ret = system(command.c_str());

    if (ret == -1) {
        std::cerr << "Errore esecuzione system()\n";
        return -1;
    } else {
        // Estrai valore ritorno corretto da exit status
        return WEXITSTATUS(ret);
    }
}






std::optional<std::string> get_installed_package_version (const std::string& package_name) {
    // Costruisce comando dpkg -s
    std::string command = "dpkg -s " + package_name + " 2>&1";

    std::array<char, 256> buffer;
    std::string output;

    // Popen esegue il comando e cattura output
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return std::nullopt;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    int retCode = pclose(pipe);
    if (retCode != 0) {
        // Pacchetto non trovato o errore
        return std::nullopt;
    }

    // Cerca la riga "Version: ..."
    std::string versionPrefix = "Version: ";
    std::size_t pos = output.find(versionPrefix);
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    // Estrae la versione dalla riga
    std::size_t start = pos + versionPrefix.size();
    std::size_t end = output.find('\n', start);
    if (end == std::string::npos) {
        end = output.size();
    }

    return output.substr(start, end - start);
}




std::optional<std::string> git_exec_ls_remote (const std::string& repo_url) 
{
    std::string command = "git ls-remote " + repo_url + " HEAD 2>&1";
    std::array<char, 128> buffer;
    std::string result;


    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) 
    {
        std::cout << "Error opening pipe" << std::endl;
        return std::nullopt;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) 
    {
        result += buffer.data();
    }

    int returnCode = pclose(pipe);
    if (returnCode != 0) 
    {
        std::cout << "Error code " << (int) returnCode << std::endl;
        return std::nullopt;
    }

    std::size_t tab_pos = result.find('\t');
    if (tab_pos == std::string::npos) 
    {
        return std::nullopt;
    }
    return result.substr(0, tab_pos);
}



void update_packages ()
{
    std::ifstream tracked_file(TRACKED_PKG_FILEPATH);
    json json_tracked = json::parse(tracked_file);

    for (auto& package: json_tracked["packages"])
    {

        if (package ["update"] == true)
        {
            std::cout << "update package " << package["name"] << std::endl;
            int retcode = execute_install ("./install.sh", package["gitlink"], package["repo_name"], package["name"]);
            std::cout << "retcode " << (int)retcode << std::endl;
            
            if (retcode == 0)
            {
                package ["installed_commit"] = std::string (package["last_available_commit"]);

                // update version
                auto version_opt = get_installed_package_version (package["name"]);
                if (version_opt)
                {
                    package ["installed_version"] = version_opt.value ();
                    package ["installed"] = true;
                    package ["update"] = false;
                }
            }
        }
    }
    // update json file
    std::ofstream out_file (TRACKED_PKG_FILEPATH);
    out_file << std::setw(4) << json_tracked << std::endl;
}




void install_missing ()
{
    std::ifstream tracked_file(TRACKED_PKG_FILEPATH);
    json json_tracked = json::parse(tracked_file);

    for (auto& package: json_tracked["packages"])
    {

        if (package ["installed"] == false || package ["installed_commit"] == "")
        {
            std::cout << "install package " << package["name"] << std::endl;
            int retcode = execute_install ("./install.sh", package["gitlink"], package["repo_name"], package["name"]);
            std::cout << "retcode " << (int)retcode << std::endl;
            
            if (retcode == 0)
            {
                package ["installed_commit"] = std::string (package["last_available_commit"]);

                // update version
                auto version_opt = get_installed_package_version (package["name"]);
                if (version_opt)
                {
                    package ["installed_version"] = version_opt.value ();
                    package ["installed"] = true;
                }
            }
        }
    }
    // update json file
    std::ofstream out_file (TRACKED_PKG_FILEPATH);
    out_file << std::setw(4) << json_tracked << std::endl;
}



void check_packages ()
{
    std::ifstream intput_file(PKG_FILENAME);
    json json_packages = json::parse(intput_file);

    std::ifstream tracked_file(TRACKED_PKG_FILEPATH);
    json json_tracked = json::parse(tracked_file);

    std::vector <std::string> tracked_packages_name;

    for (const auto& package: json_tracked["packages"])
    {
       tracked_packages_name.push_back (package["name"]);
    }


    // add missing packages to tracked
    for (const auto& package: json_packages["packages"])
    {       
        std::string pkg_name = package ["name"];
        auto it = std::find(tracked_packages_name.begin(), tracked_packages_name.end(), pkg_name);

        if (it == tracked_packages_name.end()) 
        {   
            // se il pacchetto non è conosciuto, copia le infomrazioni e determina se è instllato e in caso la versione
            json_tracked ["packages"].push_back (package);
            json& element = json_tracked["packages"].back ();
            element["installed"] = false;
            element["update"] = false;
            element["installed_version"]= "";
            element["last_available_commit"]= "";
            element["installed_commit"]="";
        }
    }

    // check the status of all tracked packages
    for (auto& tracked: json_tracked["packages"])
    {
        std::string name = tracked["name"];
        std::string gitlink = tracked["gitlink"];

        // fetch last commit info
        auto commit_opt = git_exec_ls_remote (gitlink);
        if (commit_opt)
        {
            tracked ["last_available_commit"] = commit_opt.value();
            if (tracked["last_available_commit"] != tracked ["installed_commit"])
            {
                tracked ["update"] = true;
            }
            else tracked ["update"] = false;
        }
        else {
            std::cerr << "Error fetching commit for repo " << gitlink << std::endl;
        }


        auto version_opt = get_installed_package_version (name);
        if (version_opt)
        {
            std::cout << "Package " << name << " installed. Version " << version_opt.value () << std::endl;
            tracked ["installed_version"] = version_opt.value ();
            tracked ["installed"] = true;
        }
        else
        {
            tracked ["installed"] = false;
            tracked ["update"] = false;
            tracked ["installed_version"] = "0";
            std::cout << "Package " << name << " not installed." << std::endl;
        }
    }

    std::ofstream out_file (TRACKED_PKG_FILEPATH);
    out_file << std::setw(4) << json_tracked << std::endl;
}

void load_packages (std::string filename)
{
    std::ifstream intput_file(filename);
    json data = json::parse(intput_file);

    for (const auto& package: data["packages"])
    {
        std::cout << package["name"] << "\n";
    }
}


void check_fs ()
{
    fs::path dir = PKG_DIRECTORY;
    fs::create_directory(dir);

    if (!std::filesystem::exists(TRACKED_PKG_FILEPATH)) 
    {
        std::cout << "create file " << std::string (TRACKED_PKG_FILEPATH) << std::endl;
        // create empty json file list
        json empty_json_list;
        empty_json_list["packages"] = {};
        std::ofstream out_file (TRACKED_PKG_FILEPATH);
        out_file << std::setw(4) << empty_json_list << std::endl;
    }
}
