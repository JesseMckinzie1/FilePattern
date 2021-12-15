#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <filesystem>
#include <map>
#include "file_stream.hpp"
#include "block.hpp"
#include "Variables.hpp"

namespace fs = std::filesystem;

enum Structure {std_map, std_vector};

typedef p::pair variableFileMap; 
typedef std::vector<std::string> vector_string;
class ExternalMergeSort {

    public:
        ExternalMergeSort(const Structure&,
                          const std::string&, 
                          const std::string&, 
                          const std::string&, 
                          const std::string&,
                          const int);

    private:
        vector_string vec;
        std::string inputFileName;
        std::string outputFileName;
        vector_string filesToMerge;
        vector_string newFiles;
        long double blockSize;
        std::string tmpdir;
        std::string sortVariable;
        std::string blockSizeStr;
        int mapSize;

        double currentSize(const vector_string&, const int&, const double&);

        void writeTmpFile(std::string& output);
        
        void sortFile();

        void sortMapFile();

        void mergeMaps();

        void writeFile(const std::string&);

        void merge();

        void writeMapTmpFile(std::ofstream&, 
                             const std::vector<variableFileMap>& vec);

        bool getMapBlock(std::ifstream&, std::vector<variableFileMap>&);

        bool getMap(std::ifstream&, variableFileMap&);

        void writeMap(std::ofstream&, variableFileMap&);

        bool getFilesBlock(std::ifstream&, std::vector<variableFileMap>&);

        //void sort(const std::string&);

};
