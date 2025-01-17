/**
 * @file util.hpp
 * @author Jesse McKinzie (Jesse.McKinzie@axleinfo.com)
 * @brief Utility functions to support filepattern.
 * @version 0.1
 * @date 2022-01-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * Utility functions to operate on std::string, std::map, and Types. Types are defined to be 
 * either a int or string and are used to allow for type preservation when extracting
 * variables from filenames. This file also contains type definitions for nested data structures
 * that are used throughout `filepattern`. 
 * 
 * 
 */

#pragma once
#include <regex>
#include <tuple>
#include <variant>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <chrono>
#include <filesystem>
#include <iostream>

typedef std::variant<int, std::string> Types;
typedef std::map<std::string, Types> Map;
typedef std::tuple<Map , std::vector<std::string>> Tuple;

/**
 * @brief Functions for std::strings and Types.
 * 
 */
namespace s {

        /**
         * @brief Get the value of a Type as a string or int based on the type.
         * 
         */
        struct AnyGet {
                
                /**
                 * @brief Helper method of to_string. Returns an integer as a string
                 * 
                 * @param value Integer to be converted to a string
                 * @return std::string String version of the integer
                 */
                std::string operator()(int value) { return std::to_string(value); }

                /**
                 * @brief Helper method of to_string. Returns string if input is string
                 * 
                 * @param value Value to be converted
                 * @return std::string String of the value
                 */
                std::string operator()(const std::string& value) { return value; }
        };

        /**
         * @brief Convert a Types to a string.
         * 
         * @param input A Type to be converted to a string
         * @return std::string The type value as a string
         */
        inline std::string to_string(const Types& input) {
                return std::visit(AnyGet{}, input);
        }

        /**
         * @brief Returns the amount of memory taken by a Types 
         * 
         * @param input Types to find the memory footprint of
         * @return int How much memory the Types is using
         */
        inline int size(const Types& input){
                return input.index() == 0 ? sizeof(int) : sizeof(std::string) + s::to_string(input).length();
        }       
        
        /**
         * @brief Check if a string is a digit.
         * 
         * @param s String to be checked
         * @return true String is a number
         * @return false String is not a number
         */
        inline bool is_number(const std::string &s) {
                return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
        }

        /**
        * @brief Get the basename of a filepath.
        * 
        * @param filePath Filepath to find the basename of.
        * @return string The basename of the filepath.
        */
        inline std::string getBaseName(const std::string& filePath){
                if(filePath.find('/') == std::string::npos) return filePath;

                int i = filePath.size()-1;
                std::string file;
                while(filePath[i] != '/'){
                        file.insert(0, 1, filePath[i]); 
                        --i;
                }     
                return file;
        }

        /**
         * @brief Split a string on a character
         *
         * Splits a string into a vector of strings along a character. 
         * From https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
         *
         * @param s String to be split
         * @param delimiter Character to split string on
         * @return std::vector<std::string> Vector of split strings
         */
        inline std::vector<std::string> split(std::string& s, const std::string& delimiter) {
                size_t pos_start = 0, pos_end, delim_len = delimiter.length();
                std::string token;
                std::vector<std::string> res;

                while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
                        token = s.substr (pos_start, pos_end - pos_start);
                        pos_start = pos_end + delim_len;
                        res.push_back (token);
                }

                res.push_back (s.substr (pos_start));
                return res;
        }

        /**
         * @brief Replace a part of a string with another string
         * 
         * Replace a part of a string with another string in place. For example,
         * if s::replace(str="here is a string", "string", "sentence") is called, 
         * str will become "here is a sentence".
         * 
         * @param str String to replace part of
         * @param from Section of string to replace
         * @param to What to replace the section of the string with
         * @return true Replace successful
         * @return false Replace unsuccessful
         */
        inline bool replace(std::string& str, const std::string& from, const std::string& to) {
                size_t start_pos = str.find(from);
                if(start_pos == std::string::npos)
                        return false;
                str.replace(start_pos, from.length(), to);
                return true;
        }

        /**
         * @brief Checks if a string ends in another string.
         * 
         * From: https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
         * 
         * @param value String to check
         * @param ending Ending to check for
         * @return true value ends with ending
         * @return false value does not end with ending
         */
        inline bool endsWith(std::string const & value, std::string const & ending) {

                if (ending.size() > value.size()) return false;
                return std::equal(ending.rbegin(), ending.rend(), value.rbegin());

        }

        /**
         * @brief Remove spaces from string inplace
         * 
         * @param str String to remove spaces from
         */
        inline void removeSpaces(std:: string& str){
                str.erase(remove(str.begin(), str.end(), ' '), str.end());
        }

        /**
         * @brief Pad the front of a string with zeros
         * 
         * From https://stackoverflow.com/questions/6143824/add-leading-zeros-to-string-without-sprintf
         *
         * @param s String to pad with zeros
         * @param desiredLength Length of string after padding is added
         */
        inline void padWithZeros(std::string& s, unsigned int desiredLength){
                unsigned int number_of_zeros = desiredLength - s.length(); // add 2 zeros

                s.insert(0, number_of_zeros, '0');
        }

        /**
         * @brief Get the number of times a character appears in a string
         * 
         * @param str String to check 
         * @param c Character to check for
         * @return int Number of occurences of c in str
         */
        inline int getCount(const std::string& str, const char c){
                int count = 0;
                for(const auto& s: str){
                        if(s == c) ++count;
                }
                return count;
        }

        inline bool contains(const std::string& str, const char c){
                for(const auto& s: str){
                        if(s == c) return true;
                }
                return false;
        }
        
        /**
         * @brief Removed substring from string
         * 
         * From: https://thispointer.com/how-to-remove-substrings-from-a-string-in-c/
         * 
         * @param str String to erase from
         * @param toErase Substring to erase from mainStr
         */
        inline std::string eraseSubStr(std::string& str, const std::string& toErase) {
                std::string mainStr = str;
                // Search for the substring in string
                size_t pos = mainStr.find(toErase);
                if (pos != std::string::npos)
                {
                        // If found then erase it from string
                        mainStr.erase(pos, toErase.length());
                }
                return mainStr;
        }

        inline std::string getTimeString(){
                return std::to_string(std::chrono::duration_cast< std::chrono::nanoseconds >(
                        std::chrono::system_clock::now().time_since_epoch()
                ).count());
        }
};

namespace m {

        /**
         * @brief Get a map from a .txt file 
         * 
         * Gets a map from a txt file that contains variables mapped to values.
         *
         * @param infile Input stream
         * @param mapping Map to be modified
         * @return true The end of the file has not been reached and the map is modified
         * @return false The end of the file has been reached and the mao is not modified
         */
        inline bool getMap(std::ifstream& infile, Tuple& member, int mapSize){
                std::string str;
                Map map;

                std::string key, value;
                int valueLength;
                size_t pos;

                std::get<1>(member).clear();
                while(std::getline(infile, str)){

                        // if map is correct size, return 
                        if (map.size() == (mapSize)) {
                        std::get<0>(member) = map;
                        str.pop_back();
                        std::get<1>(member).push_back(str);
                        return true;
                        } 

                        // map variable to value 
                        pos = str.find(":");
                        key = str.substr(0, pos);
                        valueLength = str.length() - pos;
                        value = str.substr(pos+1, valueLength);

                        map[key] = value;
                        //size += valueLength + pos;
                }

                return false;
        }

        /**
         * @brief Returns the amount of memory that a Tuple uses
         * 
         * @param mapping Tuple to calculate the memory footprint of
         * @return long Amount of memory the Tuple uses
         */
        inline long getSize(Tuple& mapping){
                long size = 0;
                for(const auto& mem: std::get<0>(mapping)){
                        size += sizeof(mem.first);
                        if(s::is_number(s::to_string(mem.second))) size += sizeof(int);
                        else size += sizeof(std::string) + s::to_string(mem.second).length();
                }
                size += sizeof(std::vector<std::string>);
                for(const auto& str: std::get<1>(mapping)){
                        size += str.length();
                }
                return size;
        }

        /**
         * @brief Write map to output file. 
         * 
         * @param file Output file 
         * @param mapping Map to write to output file
         */
        inline void writeMap(std::ofstream& file, Tuple& mapping){

                for(const auto& element: std::get<0>(mapping)){
                        file << element.first << ":" << s::to_string(element.second) << '\n';
                }

                for(const auto& element: std::get<1>(mapping)){
                        file << element << "," << '\n';
                } 

        }

        /**
         * @brief Converts the value in a mapping of string to Types to the correct type after reading
         * from a text file.
         * 
         * @param mapping The Tuple to convert the Types to the correct type
         */
        inline void preserveType(Tuple& mapping){
                Types value;
                for(auto& p: std::get<0>(mapping)){
                        value = p.second;
                        if(s::is_number(s::to_string(p.second))) value = std::stoi(s::to_string(p.second));
                        p.second = value;
                }
        }

        /**
         * @brief Get the index of the minimum value of a vector of Tuples at a specific variable
         * 
         * @param vec Vector to find the minimum value of 
         * @param variable Which variable to use to find the minumum value
         * @return int Index of minumum value
         */
        inline int getMinIdx(std::vector<Tuple>& vec, std::string& variable){
                Types min = std::get<0>(vec[0])[variable]; 

                int idx = 0;
                int minIdx = 0;
                for(auto& v : vec){
                        if(std::get<0>(v)[variable] < min){
                                min = std::get<0>(v)[variable];
                                minIdx = idx;
                        }
                        ++idx;
                }
                return minIdx;
        }

        /**
         * @brief Get the index of the maximum value of a vector of Tuples at a specific variable
         * 
         * @param vec Vector to find the max value of 
         * @param variable Which variable to use to find the max value
         * @return int Index of max value
         */
        inline int getMaxIdx(std::vector<Tuple>& vec, std::string& variable){
                Types max = std::get<0>(vec[0])[variable]; 

                int idx = 0;
                int maxIdx = 0;
                for(auto& v : vec){
                        if(std::get<0>(v)[variable] > max){
                                max = std::get<0>(v)[variable];
                                maxIdx = idx;
                        }
                        ++idx;
                }
                return maxIdx;
        }
}

namespace d {
        /*
        inline void deleteDirectoryContents(std::string& dir_path){
                if(s::endsWith(dir_path, ".txt")) dir_path = dir_path.substr(0, dir_path.find_last_of('/'));
                std::cout << "DIR_PATH: " << dir_path << std::endl;
                for (const auto& entry : std::filesystem::directory_iterator(dir_path)) 
                        std::cout << "removing " << entry.path() << std::endl;
                        std::filesystem::remove(entry.path());
        }
        */

        inline void remove_dir(std::string& pathToDir){
                if(s::endsWith(pathToDir, ".txt")) pathToDir = pathToDir.substr(0, pathToDir.find_last_of('/'));
                std::filesystem::path path = pathToDir; 
                uintmax_t n = std::filesystem::remove_all(path);
        }
}