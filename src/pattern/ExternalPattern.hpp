/**
 * @file ExternalPattern.hpp
 * @author Jesse McKinzie (Jesse.McKinzie@axleinfo.com)
 * @brief Parent class of ExternalFilePattern and ExternalStringPattern
 * @version 0.1
 * @date 2021-12-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#ifndef ExternalPattern_H
#define ExternalPattern_H
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <regex>
#include <map>
#include <tuple>
#include <chrono>
#include "Pattern.hpp"
#include "util.hpp"
#include "fs_stream.hpp"
#include "sort.hpp"

class ExternalPattern : public Pattern {
    
    protected:
        long blockSize; // Max amount of main memory to use at a time
        std::string matching;
        std::string matchingCopy;
        std::string validFilesPath; // Path to temporary txt file containing valid files
        int mapSize;
        std::ifstream matchingStream;
        std::ifstream groupStream;
        Types currentValue; 
        std::ifstream infile; // Input stream used throughout methods
        std::string fp_tmpdir;
        bool firstCall; // True if first call has not been made to next()
        Tuple temp;
        FilesystemStream stream; // I/O stream from temporary file

        /**
         * @brief Helper function for the getMatching method.
         * 
         * Gets all files where the keyword arguement (variable) is equal to the arguement value.
         * Performs functionality of getMatching after getMatching() parses the input.
         * 
         * 
         * @param variableMap Keyword arguement where the keyword is a variable and the value is a vector of the variable value(s)
         * @param matching Path to file to store matching files
         */
        void getMatchingHelper(const std::tuple<std::string, std::vector<Types>>& variableMap, const std::string& matching);

        /**
         * @brief Main loop of the getMatching function.
         * 
         * Adds file from infile to outfile if the variable matches the value
         * 
         * @param infile File to read file maps from
         * @param outfile File to write matching files to
         * @param variable Variable to get matching of
         * @param values Value of variable(s)
         * @param temp Temporary Tuple structure
         * @param tempMap Temporary Map structure
         */
        void getMatchingLoop(std::ifstream& infile, 
                             std::ofstream& outfile,
                             const std::string& variable, 
                             const std::vector<Types>& values, 
                             Types& temp,
                             Tuple& tempMap);
        
        /**
         * @brief Converts a directory of files that match the pattern to a single name which captures
         * all variable values present
         * 
         * @return std::string A filename which captures all variable values
         */
        std::string externalOutPutName();

    public: 
        std::vector<Tuple> currentBlock; // Store current block of files
        std::vector<std::pair<std::pair<std::string, Types>, std::vector<Tuple>>> currentGroup; //Store current block of grouped files

        /**
         * @brief Construct a new External Pattern object
         * 
         * @param path Path to directory or text file
         * @param blockSize Maximum amount of memory to use
         * @param recursive True to iterate over subdirectories
         */
        ExternalPattern(const std::string& path, const std::string& blockSize, bool recursive);

        /**
         * @brief Returns files that match the value of variable. Needs to be updated to match old version input. 
         * 
         * @param variables Variables with value to match (ex. "x=1, y=2")
         * @return std::vector<Tuple> Vector of files where the variable(s) match the value.
         */
        std::string getMatching(const std::vector<std::tuple<std::string, std::vector<Types>>>& variables);

        /**
         * @brief Returns a block of files after a call to getMatching where the block of files
         * does not use more memory than blockSize.
         * 
         * @return std::vector<Tuple> A block of file that are matched from getMatching
         */
        std::vector<Tuple> getMatchingBlock();

        /**
         * @brief Get the next block of matching files.
         * 
         * Read the next block of matching files that is of size less than or equal to 
         * the block size from the temporary .txt file. 
         */
        void next();

        /**
         * @brief Returns the next group of files when group_by is called.
         * 
         * Returns a tuple of a pair of a string and a type as the first member and 
         * a vector of Tuples which contain the matched and grouped files in the second.
         * The grouped variable, along with the current value, is the first pair.
         * 
         */
        void nextGroup();

        /**
         * @brief Returns the number of files in the current block.
         * 
         * @return int Number of files in the block
         */
        int currentBlockLength();

        /**
         * @brief Returns a block of valid files that consumes at most the user specified
         * amount of memory
         *
         * Reads a block of files from the temporary .txt file into memory that is of size less 
         * than or equal to the block size.  
         *
         * @return std::vector<Tuple> The block of matching files where the first member of the tuple
         * is the map of variables to their value and the second member is a vector containing paths to 
         * matching files.
         */
        std::vector<Tuple> getValidFilesBlock();

        /**
         * @brief Group files by a variable. 
         *
         * Sorts the temporary .txt file by the provided variable uses an 
         * external merge sort algorithm to maintain memory usage of less than the block size.
         * 
         * @param groupBy A variable that is contained in the pattern.
         */
        void groupBy(const std::string& groupBy);

        /**
         * @brief 
         * 
         * @param vec 
         * @return std::string 
         */
        std::string outputName(std::vector<Tuple>& vec);

        /**
         * @brief Guesses the pattern given a directory or text file.
         * 
         * @param path Path to directory or text file
         * @param variables Variable names. Optional
         * @param blockSize Maximum amount of memory to consume
         * @return std::string A guess of the pattern
         */
        static std::string inferPattern(const std::string& path, std::string& variables, const std::string& blockSize);

        /**
         * @brief Returns the length of the current group of files
         * 
         * @return int Length of current block of files
         */
        int getGroupLength();
};

#endif