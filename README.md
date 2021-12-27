# FilePattern
C++ and Python version of filepattern.

<h2> Installation </h2>
To install filepattern:

<ol>
  <li>Clone repository with --recursive-submodules</li>
  <li>cd to the folder and then "run pip install ." </li>
</ol>
  
After installation, use "import pattern" to import the module into Python. The pattern module contains the following classes: 
  <ul>
    <li>[FilePattern](#FilePattern)</li> 
    <li>[StringPattern](#StringPattern) </li> 
    <li>[ExternalFilePattern](#ExternalFilePattern) </li> 
  </ul>
A test is included in the root directory in the file test.py.


  
## FilePattern
File pattern
  
## StringPattern
StringPattern contains all the functionalility of FilePattern, except it takes in a txt file as an input rather than a directory.
  
## ExternalFilePattern
ExternalFilePattern has the same functionality as FilePattern, however it processes the input in a specified block size for when the input is too large to store in main memory, or for when the memory use of FilePattern must be restricted.
