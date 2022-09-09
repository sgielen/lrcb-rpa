LRCB-RPA
========

This project builds a tool for the Dutch Expert Centre for Screening (LRCB,
Landelijk Referentiecentrum voor Bevolkingsonderzoek). It is intended for
radiologist performance assessments (RPA). It is provided as-is, with no
explicit or implicit guarantees and its use is purely the responsibility
of the user. See the LICENSE.txt file for more details.

Building
--------

First, run 'git submodule update --init' to retrieve the submodule(s).

Create a build dir. There, run `cmake`, if necessary with `-DQt5_DIR` pointing at
a directory containing `Qt5Config.cmake`. Use `make` to build the binary.

On Windows, install Visual Studio 2015, a version of CMake that supports it and
a version of Qt5 that supports it. Also, install NSIS for building a launcher.
Then, in a build dir, run:

  cmake -G 'Visual Studio 14 2015' -DQt5_DIR='C:\Qt\5.10.0\msvc2015\lib\cmake\Qt5' ..
  cmake --build . --target lrcb-rpa-launcher --config RelWithDebInfo

This should produce 'lrcb-rpa-launcher.exe', which is a launcher for lrcb-rpa.exe which
ensures the Qt DLLs are available as well.

To get a proper setup file, use 'lrcb-rpa-setup' as a target instead of
'lrcb-rpa-launcher'.

Installation
------------

See INSTALL.txt.

Validation
----------

* Settings
  * Able to change storage location?
  * Able to add/remove users?
    * Empty user not allowed?
    * Double users not allowed?
* Log in
  * Must select a user?
* Connect multiple cameras: will the right one be used always?
  * Potentially, configure the 'default camera' (if it is found) in settings
