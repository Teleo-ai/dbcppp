#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <dirent.h>   // POSIX library for directory iteration
#include <sys/stat.h> // To check if directory or file

#include "dbcppp/Network.h"
#include "dbcppp/Network2Functions.h"

#include "Config.h"

#include "Catch2.h"

std::vector<std::string> get_dbc_files(const std::string &directory_path)
{
    std::vector<std::string> dbc_files;
    DIR *dir = opendir(directory_path.c_str());
    if (dir)
    {
        struct dirent *entity;
        while ((entity = readdir(dir)) != nullptr)
        {
            std::string file_name = entity->d_name;
            std::string full_path = directory_path + "/" + file_name;

            struct stat info;
            if (stat(full_path.c_str(), &info) != 0) // Error getting file info
            {
                continue;
            }
            if (S_ISREG(info.st_mode) && full_path.substr(full_path.find_last_of(".") + 1) == "dbc")
            {
                dbc_files.push_back(full_path);
            }
        }
        closedir(dir);
    }
    return dbc_files;
}

TEST_CASE("DBCParserTest", "[]")
{
    std::size_t i = 0;
    auto dbc_files = get_dbc_files(std::string(TEST_FILES_PATH) + "/dbc");
    for (const auto &dbc_file : dbc_files)
    {
        std::cout << "Testing DBC grammar with file: " << dbc_file << std::endl;
        auto dbc_file_tmp = dbc_file + ".tmp";
        std::unique_ptr<dbcppp::INetwork> spec;
        std::unique_ptr<dbcppp::INetwork> test;
        {
            std::ifstream dbc(dbc_file);
            spec = dbcppp::INetwork::LoadDBCFromIs(dbc);
            std::ofstream tmp_dbc(dbc_file_tmp);
            bool open = tmp_dbc.is_open();
            using namespace dbcppp::Network2DBC;
            tmp_dbc << std::setprecision(10);
            tmp_dbc << *spec << std::endl;
        }
        {
            std::ifstream dbc(dbc_file_tmp);
            test = dbcppp::INetwork::LoadDBCFromIs(dbc);
            REQUIRE(test);
        }
        auto error_msg = "Failed for " + std::to_string(i) + "th file ('" + dbc_file + "')";
        if (*spec != *test)
        {
            std::cout << error_msg << std::endl;
        }
        REQUIRE(*spec == *test);
        std::remove(dbc_file_tmp.c_str());
        i++;
    }
}