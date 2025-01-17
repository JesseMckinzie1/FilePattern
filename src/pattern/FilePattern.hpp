/**
 * @file FilePattern.hpp
 * @author Jesse McKinzie (Jesse.McKinzie@axleinfo.com)
 * @brief 
 * @version 0.1
 * @date 2021-12-22
 * 
 * 
 */

#pragma once 
#include "InternalPattern.hpp"

namespace fs = std::filesystem;

class FilePattern : public InternalPattern {
    public:
        FilePattern(const std::string&, const std::string&, bool);

        /**
         * @brief Match files in directory to the pattern
         * 
         * Filesystem directory iterator is used to iterate over the supplied directory,
         * comparing each filename to the regex version of the pattern. If recursive if true,
         * a recursive directory iteratory is used. If a file matches the pattern, it is stored 
         * in the validFiles vector.
         */
        void matchFiles();

        /**
         * @brief Print the valid files to the console.
         * 
         */
        void printFiles();
        
    private: 
        //std::string path; // path to directory
        fs::directory_iterator iterator; // File iterator for given path
        fs::recursive_directory_iterator recursiveIterator; // Recursive directory iterator
        bool recursive; // Iterate over subdirectories

        /**
         * @brief Match files to pattern using a directory iterator.
         * 
         * Called from matchFiles() method. Matches files to the pattern from a directory iterator
         * if recrusive is false.
         */
        void matchFilesOneDir();

        /**
         * @brief Match files to pattern using a recusive directory iterator. 
         * If a file has the same name and is in a different subdirectory, the filepath will be 
         * added to the existing tuple in the array stored in the second member of the tuple.
         *
         * Called from matchFiles() method. Matches files to the pattern from a directory iterator
         * if recrusive is false.
         */
        void matchFilesMultDir();

};