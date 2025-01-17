#include "Pattern.hpp"

using namespace std;
namespace fs = filesystem;

void Pattern::getPathFromPattern(const string& path){

    this->path = path; 
    this->filePattern = path;
    size_t firstBracket = path.find("{"); // find location of first named group
    if(firstBracket == string::npos) return; // return if none found
    
    // find first slash before named group
    while(path[firstBracket] != '/'){
        --firstBracket;
        if(firstBracket == 0) {
            throw invalid_argument("Invalid path. Atleast one directory without a named group must be provided.");
        }
    }
    ++firstBracket;

    this->path = path.substr(0, firstBracket); // piece of path without named groups becomes the path
    this->filePattern = path.substr(firstBracket, path.length()-1); // the rest of the path is the pattern
}

void Pattern::setGroup(const string& group){
    if(find(this->variables.begin(), this->variables.end(), group) != variables.end()) {
        this->group = group;
    } else {
        throw invalid_argument("Group by variable must be contained in the pattern.");
    }
}

vector<string> Pattern::getVariables(){
    return this->variables;
}

void Pattern::filePatternToRegex(){
    tuple vars = getRegex(this->filePattern);
    this->regexFilePattern = get<0>(vars);
    this->variables = get<1>(vars);
    this->namedGroups = get<2>(vars);
}

tuple<string, vector<string>, vector<string>> Pattern::getRegex(string& pattern){
    
    getNewNaming(pattern);

    // regex to match variables
    std::regex e("(\\{(\\w+):([dc+]+)\\})|(\\(P\\?<(\\w+)>(.+)\\))"); // check for bracket expressions or named groups
    std::regex group("\\P\\?<(\\w+)>(.+)"); // check for regex named groups
    std::regex var("\\{(\\w+):([dc+]+)\\}"); // pattern style of groups (e.g {r:ddd})

    map<char, string> patternMap; // map of variable types to regex equivalent 
    patternMap['d'] = "[0-9]"; 
    patternMap['c'] = "[a-zA-Z]";
    patternMap['+'] = "+";
    
    string str, rgx; // temp string and regex
    vector<pair<string,string>> matches; // map between bracket expression and regex
    vector<string> variables; // store variable names
    string patternCopy = pattern; // get a copy of pattern since regex_search is inplace
    std::smatch sm, m; // regex matches

    string temp;
    // extract bracket expressions from pattern and store regex
    while (regex_search(patternCopy, m, e)){
        temp = m[0];
        // find any named groups with regex style naming
        if(temp.rfind("(P?<", 0) == 0) {
           while (regex_search(temp, sm, group)){
                rgx = sm[2]; // store regex value in named group
                rgx.pop_back(); // remove trailing )

                variables.push_back(sm[1]); // store variable name
                str = sm[0]; // store entire group
                str = "(" + str;
                matches.push_back(make_pair(str, rgx));
                temp = sm.suffix().str();
           }
        } else {
            // get named groups with the filepattern style groups (e.g. {r:ddd})
            while (regex_search(temp, sm, var)){
                str = sm[2]; // store variable values (e.g. ddd)
                rgx = ""; // construct regex
                for(const auto c: str){
                    rgx += patternMap[c];
                }
                variables.push_back(sm[1]); // store variable name
                matches.push_back(make_pair(sm[0], rgx));
                temp = sm.suffix().str();
            }
        }

        patternCopy = m.suffix().str();
    }

    string regexFilePattern = pattern;
    vector<string> namedGroups;

    // Replace bracket groups with regex capture groups
    for(const auto& match: matches){
        namedGroups.push_back(match.first);

        // Create capture group
        str = "(" + match.second + ")";
        s::replace(regexFilePattern, match.first, str);
    }

    return make_tuple(regexFilePattern, variables, namedGroups);
}


Tuple Pattern::getVariableMapMultDir(const string& filePath, const smatch& sm){
    Tuple tup;
    
    bool matched = false;
    string basename;
    string file = s::getBaseName(filePath);
    // iterate over matched files, checking if filename already exists
    for(int i = 0; i < validFiles.size(); i++){ 
        basename = s::getBaseName(s::to_string(get<1>(validFiles[i])[0])); // store the basename
        // if the filename is found, add the filepath to the vector in the second member of the tuple 
        if(basename == file){
            matched = true;
            get<1>(validFiles[i]).push_back(filePath); // Add path to existing mapping
            break;
        } 
    }

    // Create new mapping if not already present
    if(!matched){
        tup = getVariableMap(filePath, sm);
    }

    return tup;
}

Tuple Pattern::getVariableMap(const string& filePath, const smatch& sm){
    Tuple tup;
    // filename matches the pattern
    std::get<1>(tup).push_back(filePath);

    
    string str;
    // Extract capture groups from filename and store in mapping
    for(int i = 1; i < sm.size(); ++i){
        str = sm[i];
        // conserve variable type
        s::is_number(str) ? get<0>(tup)[variables[i-1]] = stoi(str) : 
                            get<0>(tup)[variables[i-1]] = str;
        this->variableOccurences[variables[i-1]][get<0>(tup)[variables[i-1]]] += 1; // update count of the variable occurence
        this->uniqueValues[variables[i-1]].insert(get<0>(tup)[variables[i-1]]); // update the unique values for the variable
    }
    
    return tup;
}

std::map<string, std::map<Types, int>> Pattern::getOccurences(const vector<tuple<string, vector<Types>>>& mapping){
    // if no variables request, return all variables
    if(mapping.size() == 0){
        return this->variableOccurences;
    }

    std::map<Types, int> temp;
    std::map<string, std::map<Types, int>> occurences;
    string variable;
    // loop over vector passed in that contains the variable mapped to value(s)
    for(const auto& tup: mapping){
        if(get<1>(tup).size() == 0){
            occurences[get<0>(tup)] = this->variableOccurences[get<0>(tup)];
        } else {
            for(const auto& value: get<1>(tup)){
                variable = get<0>(tup);
                temp[value] = this->variableOccurences[get<0>(tup)][value];
            }
            occurences[variable] = temp;
        }
    }

    return occurences;
}

string Pattern::getPattern(){
    return this->filePattern;
}

void Pattern::setPattern(const string& pattern){
    this->filePattern = pattern;
}

string Pattern::getRegexPattern(){
    return this->regexFilePattern;
}

void Pattern::printVariables(){
    int i = 0;
    cout << "The variables are: ";
    int size = this->getVariables().size();
    for(const auto& var: this->getVariables()){
        cout << var;
        if(i<size-1){
            cout << ", ";
        }
        i++;
    }
    cout << endl;
}

map<string, set<Types>> Pattern::getUniqueValues(const vector<string>& vec){
    if(vec.size() == 0) return this->uniqueValues; // if no variables are passed, return all variables

    map<string, set<Types>> temp;
    // return variables that were requested
    for(const auto& str: vec){
        temp[str] = uniqueValues[str];
    }
    return temp;
}


void Pattern::getNewNaming(string& pattern){
    
    string vars = "\\{([rtczyxp+]+)\\}"; // check for old naming style or named grouped

    std::regex e(vars);

    string str; // temp string
    vector<pair<string,string>> matches; // map between bracket expression and regex
    string patternCopy = pattern; // get a copy of pattern since regex_search is inplace
    std::smatch m; // regex matches
    bool replaced = false;
    // extract bracket expressions from pattern and store regex
    while (regex_search(patternCopy, m, e)){
        str = m[1];

        matches.push_back(make_pair(m[0], m[1]));

        //regexMatches.push_back(m);
        patternCopy = m.suffix().str(); 
        
    }

    // Replace bracket groups with regex capture groups
    for(const auto& match: matches){
        // Create capture group

        if(match.first.find(':') == std::string::npos) {

            replaced = true;
            str = "{";
            str += match.second[0]; 
            str += ":";
            for(const auto& c: match.second){
                if(c != '+') str += "d";
                else str += "+";
            }
            str += "}";

            s::replace(pattern, match.first, str);
        } 
    }
    if(replaced){
        cout << "WARNING: The old style of pattern was used. This style may become deprecated in future releases." << endl;
        cout << "The recommended pattern to use is: " << pattern << 
                ". See the documenation for details about the new style." << endl;
    }
}

void Pattern::replaceOutputName(Tuple& min, Tuple& max, const string& var, string& outputName, const int idx, string& temp, const regex& patternRegex){
        string file;
        smatch sm;
        if(get<0>(min)[var] == get<0>(max)[var]){

            file = s::getBaseName(get<1>(min)[0]);
            regex_match(file, sm, patternRegex);
        
            s::replace(outputName, this->namedGroups[idx], sm[idx+1]);

        } else { // if min is different than max, put range in outputname

            temp = "("; 

            file = s::getBaseName(get<1>(min)[0]); // get basename of filepath
            regex_match(file, sm, patternRegex); // find variables

            temp += sm[idx+1];
            temp += "-";

            file = s::getBaseName(get<1>(max)[0]);
            regex_match(file, sm, patternRegex);

            temp += sm[idx+1];
            temp += ")";

            s::replace(outputName, this->namedGroups[idx], temp);
        }
}

string Pattern::outputNameHelper(vector<Tuple>& vec){
    if(vec.size() == 0){
        vec = this->validFiles;
    }

    string outputName = this->filePattern;

    int idx = 0;
    int min, max;
    smatch sm;
    string temp, file;
    
    regex patternRegex(this->regexFilePattern);

    for(auto& var: variables){
        
        min = m::getMinIdx(vec, var); 
        max = m::getMaxIdx(vec, var);

        // if min is the same as max, put variable value in output
        this->replaceOutputName(vec[min], vec[max], var, outputName, idx, temp, patternRegex);
        
        ++idx;
    }

    return outputName;

}

string Pattern::inferPatternInternal(vector<string>& files, string& variables, const string& startingPattern){
    variables += "rtczyxp";
    string pattern;
    if(startingPattern == "") pattern = files[0];
    else pattern = startingPattern;

    string regexStr;
    string patternCpy = pattern;
    regex rgx = regex(get<0>(getRegex(patternCpy)));
    for(auto& file : files){

        if(!regex_match(file, rgx)) {
            pattern = swSearch(pattern, file, variables);
            patternCpy = pattern;
            regexStr = get<0>(getRegex(patternCpy));
            rgx = regex(regexStr);

        }
    }

    return pattern;
}

string Pattern::swSearch(string& pattern, string& filename, const string& variables){
    string numbers = "0123456789<>"; // numeric values, '<' -> d and '>' -> d+
    string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$"; // alphabetic values, '@' -> c+ and '$' -> c
    smatch sm; // store regex group matches
    string rgxStr; // string version of rgx
    regex rgx; 
    string temp, smStr;
    bool match;

    //vlist = [(0,0,"", "")]
    for (const auto& v: variables){
        
        // construct named group
        rgxStr = "\\{(";
        rgxStr += v;
        rgxStr += "):([dc+]+)\\}";

        rgx = regex(rgxStr); // regex to match
        match = false; // match if found
        while(regex_search(pattern, sm, rgx)){

            match = true;
            // check if + is in group
            smStr = sm[0];
            if(s::contains(smStr, '+')) {
                if(sm[2] == "d+") s::replace(pattern, sm[0], ">"); 
                else  s::replace(pattern, sm[0], "@");

            } else {
                temp = "";
                smStr = sm[2].str();

                if(s::contains(smStr, 'd')){
                    temp.insert(0, s::getCount(sm[0].str(), 'd'), '<');
                    s::replace(pattern, sm[0], temp);
                } else {
                    temp.insert(0, s::getCount(sm[0].str(), 'c'), '$');
                    s::replace(pattern, sm[0], temp);
                }
            }
        } 
        if(!match) {
            break;
        }
    }

    // scoring function
    map<string, map<string, int>> sab = {
        {"numeric", {
            {"match", 2}, 
            {"penalty", 1}
        }},
        {"alpha", {
            {"match", 2},
            {"penalty", 1}
        }},
        {"char", { // larger penalty for non-numeric, non-alphabetic characters
            {"match", 5},
            {"penalty", 3} 
        }}
    };

    // scoring matrix creation
    int m = pattern.length();
    int n = filename.length();
    int matrix[m+1][n+1];
    fill(matrix[0], matrix[0] + (m+1) * (n+1), 0);


    int p_idx = 0;
    int f_idx = 0;
    bool pIsNumeric, fIsNumeric;
    bool pIsCharacter, fIsCharacter;
    vector<int> scores;
    int s, wi, wj;

    // populate scoring maxtrix
    for(const auto& p: pattern){
        pIsNumeric = count(numbers.begin(), numbers.end(), p);
        pIsCharacter = count(alphabet.begin(), alphabet.end(), p);

        f_idx = 0;
        for(const auto& f: filename){
            
            fIsNumeric = count(numbers.begin(), numbers.end(), f);
            fIsCharacter = count(alphabet.begin(), alphabet.end(), f);
            
            scores.clear();
            scores.push_back(0);

            // generate similaroity score
            if(fIsNumeric){
                if(pIsNumeric){
                    s = matrix[p_idx][f_idx] + sab["numeric"]["match"];
                } else {
                    s = matrix[p_idx][f_idx] - sab["numeric"]["match"];
                }
            } else if(fIsCharacter) {
                if(pIsCharacter){
                    s = matrix[p_idx][f_idx] + sab["alpha"]["match"];
                } else {
                    s = matrix[p_idx][f_idx] - sab["alpha"]["match"];
                }
            } else {
                if (p == f){
                    s = matrix[p_idx][f_idx] + sab["char"]["match"];
                } else {
                    s = matrix[p_idx][f_idx] - sab["char"]["match"];
                }
            }

            scores.push_back(s);

            // calculate gap scores
            if (fIsNumeric){
                if (pIsNumeric) {
                    wi = matrix[p_idx+1][f_idx] - sab["numeric"]["penalty"];
                    wj = matrix[p_idx][f_idx+1] - sab["numeric"]["penalty"];
                } else {
                    wi = matrix[p_idx+1][f_idx] - sab["alpha"]["penalty"];
                    wj = matrix[p_idx][f_idx+1] - sab["alpha"]["penalty"];
                }
            } else if(fIsCharacter) {
                if(pIsCharacter){
                    wi = matrix[p_idx+1][f_idx] - sab["alpha"]["penalty"];
                    wj = matrix[p_idx][f_idx+1] - sab["alpha"]["penalty"];
                } else {
                    wi = matrix[p_idx+1][f_idx] - sab["numeric"]["penalty"];
                    wj = matrix[p_idx][f_idx+1] - sab["numeric"]["penalty"];
                }
            } else {
                wi = matrix[p_idx+1][f_idx] - sab["char"]["penalty"];
                wj = matrix[p_idx][f_idx+1] - sab["char"]["penalty"];
            }

            scores.push_back(wi);
            scores.push_back(wj);

            // assign matrix score
            matrix[p_idx+1][f_idx+1] = *max_element(scores.begin(), scores.end());

            ++f_idx;

        }
        ++p_idx;
    }

    // find best score
    int bestScore = matrix[m][n];
    int row       = m;
    int col       = n;

    for(int r = 1; r < m+1; ++r){
        for(int c = 1; c < n+1; ++c){
            if(matrix[r][c] > bestScore){
                bestScore = matrix[r][c];
                row = r;
                col = c;
            }
        }
    }

    // traceback, building a new pattern template
    string patternTemplate = ""; //(1, filename[col-1]);
    patternTemplate.push_back(filename[col-1]);
    int lastRow = row;
    int lastCol = col;
    int r, c;

    while(true){ // loop until best_score == 0 is reached

        // Default to next set of characters
        r = row - 1;
        c = col - 1;
        bestScore = matrix[r][c];

        if(matrix[row][col-1] > bestScore){
            bestScore = matrix[row][col-1];
            r = row;
            c = col - 1;
        }

        if(matrix[row-1][col] > bestScore){
            bestScore = matrix[row-1][col];
            r = row - 1;
            c = col;
        }

        if (bestScore == 0) break;

        row = r;
        col = c;
        
        // Default to the matching value if available
        if(filename[col-1] == pattern[row-1] && lastCol != col && lastRow != row){
            patternTemplate = filename[col-1] + patternTemplate;
        } else {
            
            // If the values don't match, throw error if non-numeric and non-alphabetic
            if(!count(numbers.begin(), numbers.end(), filename[col-1]) || 
            !count(numbers.begin(), numbers.end(), pattern[row-1])) {
                if(count(alphabet.begin(), alphabet.end(), filename[col-1]) &&
                   count(alphabet.begin(), alphabet.end(), pattern[row-1])){
                    if(lastCol != col && lastRow != row){
                        if (pattern[row-1] == '@'){
                            patternTemplate = '@' + patternTemplate;
                        } else {
                            patternTemplate = '$' + patternTemplate;
                        }
                    } else {
                        patternTemplate = "@" + patternTemplate;
                    }
                } else {
                    throw runtime_error("Non-numeric, non-alphabetic characters found that do not match");
                }

            } else if(lastCol != col && lastRow != row){ // progrsssion was made so add a placeholder
                if(pattern[row-1] == '>'){
                    patternTemplate = '>' + patternTemplate;
                } else {
                    patternTemplate = '<' + patternTemplate;
                }
                
            } else { // Lengths don't match, so add a place holder
                patternTemplate = ">" + patternTemplate;
            }
        }

        lastRow = row;
        lastCol = col;
    }

    pattern = patternTemplate;
    int vi = 0;
    rgx = "[<>\\$@]+";
    string vdef;

    // Construct a new filepattern
    for (auto i = sregex_iterator(patternTemplate.begin(), patternTemplate.end(), rgx); i != sregex_iterator(); ++i) {
        temp = (*i).str();

        if(temp.find('>') != string::npos){
            vdef = "{"; 
            vdef += variables[vi];
            vdef += ":d+}";
        } else if (temp.find('<') != string::npos){
            vdef = "{";
            vdef += variables[vi];
            vdef += ":";
            vdef.insert(vdef.length(), temp.length(), 'd');
            vdef += "}";
        } else if (temp.find('@') != string::npos) {
            vdef = "{"; 
            vdef += variables[vi];
            vdef += ":c+}";
        } else if (temp.find('$') != string::npos){
            vdef = "{";
            vdef += variables[vi];
            vdef += ":";
            vdef.insert(vdef.length(), temp.length(), 'c');
            vdef += "}";
        }

        s::replace(pattern, temp, vdef);

        ++vi;
    }
    
    return pattern;
} 

vector<string> Pattern::getTmpDirs(){
    return this->tmpDirectories;
}
