# Tyr
A Windows MFC application for handing out awards at WCI. Successor to the _Heimdall_ program. Students get points based on activities and academics, and awards are assigned based on those points.

This app is meant to be run on x86 Windows 7. It requires MySQL server 5.7 and MySQL Connector/C++ 1.1. To set this up, follow these instructions:
1. Download the MySQL 5.7.33 installer. As of right now, it is available here: https://dev.mysql.com/downloads/windows/installer/5.7.html

2. Download and set up the Visual C++ 2015 redistributable. As of right now, it is available here: https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0
3. Run the MySQL installer and choose to add the following products:
    - MySQL Server 5.7.33 x86
    - MySQL Connector/C++ 1.1.9 x86
3. When setting up the server, create a new database user named "points" with the password "points". Use default setup for everything else
4. Ensure the server is installed in `C:\Program Files (x86)\MySQL\MySQL Server 5.7` and the connector is installed in `C:\Program Files (x86)\MySQL\Connector C++ 1.1`. Add `C:\Program Files (x86)\MySQL\MySQL Server 5.7\bin` and `C:\Program Files (x86)\MySQL\Connector C++ 1.1\lib\opt` to the Path system environment variable
5. Run schema.sql to set up the DB
6. Load user data (fill in after app is finished)

## Development
To make changes to the source code of the app, or to rebuild it, use Visual Studio 2019 or later. Ensure all Visual C++ components are downloaded. To set up the development environment, follow these instructions:
1. Download the Boost library (Connector/C++ depends on Boost), version 1.59.0. As of right now, it is available here: https://www.boost.org/users/history/. It's probably necessary to unzip the folder (e.g. `tar -xf` from command line)

2. Ensure the library is at `C:\Program Files\boost_1_59_0`, and move the boost_1_59_0 folder if necessary (if you keep it elsewhere, add that path to **Include Directories** instead)
3. At this point, open Visual Studio. Ensure the project configuration is set to "Release x86"
4. Open project properties and change the following:
    - In **VC++ Directories**, add `C:\Program Files (x86)\MySQL\Connector C++ 1.1\include` and `C:\Program Files\boost_1_59_0` to **Include Directories**
    - In **VC++ Directories**, add `C:\Program Files (x86)\MySQL\Connector C++ 1.1\lib\opt` to **Library Directories**
    - In **Linker**, in **Input**, add `mysqlcppconn.dll` to **Additional Dependencies**
