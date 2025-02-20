# Starttime

## Build/Install Dependencies

Required dependencies:
- [WiringPi](https://github.com/WiringPi/WiringPi)
- [Boost::mySQL](https://github.com/boostorg/mysql), requires [Boost 1.82](https://www.boost.org/doc/libs/1_82_0) or higher

#### WiringPi
- Just follow the instructions in the [Installing from source](https://github.com/WiringPi/WiringPi?tab=readme-ov-file#from-source) section on their Github.
- Use any directory outside the git project folder

#### Boost
- Tested with [Boost 1.87](https://www.boost.org/doc/libs/1_87_0/)
- Based on [Boost installation instructions](https://www.boost.org/doc/libs/1_87_0/more/getting_started/unix-variants.html)
- Use any directory outside the git project folder
- INSTRUCTIONS:
  1) Download [boost_1_87_0.tar.bz2](https://www.boost.org/users/history/version_1_87_0.html) and extract it:
  ```shell
  tar --bzip2 -xf /path/to/boost_1_87_0.tar.bz2
  ```
  2) Change into the directory and run bootstrap script. We are not using an install prefix, so the library will be installed into ``/usr/local/lib``, which requires ``root`` permissions during the install step. Since we are using header-only libraries, we do not need any libraries to be built, therefore the empty ``with-libraries`` parameter:
  ```shell
  ./bootstrap.sh --with-libraries=
  ```
  3) Start the build/install process with:
  ```shell
  sudo ./b2 install
  ```

## Build and Run

#### Using the Command Line
- In the project directory, run:
  ``` shell
  mkdir build
  cmake -B ./build
  cmake --build ./build -j 4
  ```
- To start the program, run:
  ```shell
  ./build/starttime
  ```

#### Using VS Code
- For VS Code, build and launch tasks are provided in the ``.vscode`` folder.

