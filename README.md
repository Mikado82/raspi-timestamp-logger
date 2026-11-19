# Reliable Timestamp Logger

## Build/Install Dependencies

#### WiringPi
- Homepage: [WiringPi](https://github.com/WiringPi/WiringPi)
- Just follow the instructions in the [Installing from source](https://github.com/WiringPi/WiringPi?tab=readme-ov-file#from-source) section on their Github.
- Use any directory outside the git project folder

#### Boost.MySQL
- Homepage: [Boost::mySQL](https://github.com/boostorg/mysql)
- Requires [Boost 1.82](https://www.boost.org/doc/libs/1_82_0) or higher, tested with [Boost 1.87](https://www.boost.org/doc/libs/1_87_0/)
- Based on [Boost installation instructions](https://www.boost.org/doc/libs/1_87_0/more/getting_started/unix-variants.html)
- Use any directory outside the git project folder

###### INSTRUCTIONS:
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

#### Run cmake
- In the project directory, run:
  ``` shell
  cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=Release
  ```
- If you want to create a debug build, replace ``Release`` with ``Debug``.

#### Build and Run on the Command Line
- Change into the ``./build`` directory and run:
  ``` shell
  make -j 4
  ```
  - The ``-j 4`` is optional and compiles up to four files in parallel.

- To launch the program, run:
  ```shell
  ./build/raspi-timestamp-logger
  ```
  - This reads the default ``config.ini`` file. To use a different file, pass it as the first command line argument.
  
## Setup Test Database
- Create MariaDB docker container and connect to it:
```
docker run -p 3306:3306  --name mdb -e MARIADB_ROOT_PASSWORD=example-password -d mariadb:latest
docker exec -it mdb mariadb --user root -pexample-password
```
- Create database and table:
```
CREATE DATABASE example-db;
USE example-db;
CREATE TABLE timestamps (
	start_id int NOT NULL AUTO_INCREMENT,
	start_time varchar(255) NOT NULL,
	PRIMARY KEY (start_id));
SELECT * FROM timestamps;
```
- Starting/stopping/removing the container:
```
docker stop mdb
docker start mdb
docker rm mdb
```
