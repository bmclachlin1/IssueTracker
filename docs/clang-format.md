# Clang-Format Setup

The [clang-format configuration file](../.clang-format) included with this project is based off the Google Style Guide, so using the clang-format tool will format the code correctly in order to adhere to their guidelines. Yay for tools!

## Installation

Before you can use `clang-format`, it needs to be installed, obviously. This guide assumes you are using a Linux machine, or WSL.

1. Check if `clang-format` is installed already:

   ```
   clang-format --version
   ```

   We need to use version 10, so if you don't have version 10 installed, keep following these steps. Otherwise, skip to the next section

2. If you version is less than version 10, remove it (unless you *reeeeeally* need an old version of the formatter) by running `sudo apt-get update && sudo apt-get purge clang-format`

3. Install the correct version using `sudo apt-get install clang-format-10`

4. *(Optional, but recommended)* Create a symlink to `clang-format-10` by getting the path to where `clang-format-10` is installed using `whereis clang-format-10`, copy the path to the executable, and running `sudo ln -s <path> /usr/bin/clang-format` on that path 

5. Test it out on a file! In the project directory, run `clang-format <filename>.cpp` and it should print out the formatted file to the terminal. For example, running `clang-format test/main.cpp` prints out

   ```bash
   #include <iostream>
   
   int main() {
     std::cout << "test program works" << std::endl;
     return 0;
   }%
   ```

## Usage

### Basic

You can run it on a single file from the command line by running `clang-format <filename>.cpp > <filename>.cpp`, which will dump the results of the format back into the original file. This is slow and tedious, but works

### VSCode

There is a VSCode extension called [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format) which can be configured to format the file on save. Follow the configuration steps in their documentation to get it working for you

### Atom

I (Everett) don't use Atom for development, so I'll leave it as an exercise for the reader to figure this one out. There are lots of [atom packages for clang-format](https://atom.io/packages/search?q=clang-format), [this one ](https://atom.io/packages/clang-format) seems popular