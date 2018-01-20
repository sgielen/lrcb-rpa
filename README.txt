LRCB-RPA
========

This project builds a tool for the Dutch expert centre for screening (LRCB,
Landelijk Referentiecentrum voor Bevolkingsonderzoek). It is intended for
radiologist performance assessments (RPA).

Building
--------

Create a build dir. There, run `cmake`, if necessary with `-DQt5_DIR` pointing at
a directory containing `Qt5Config.cmake`. Use `make` to build the binary.

On Windows, install Visual Studio 2015, a version of CMake that supports it and
a version of Qt5 that supports it. Then, in a build dir, run:

  cmake -G 'Visual Studio 14 2015' -DQt5_DIR='C:\Qt\5.10.0\msvc2015\lib\cmake\Qt5' ..
  cmake --build . --target lrcb-rpa --config RelWithDebInfo

This should produce 'RelWithDebInfo/lrcb-rpa.exe'. It will need some DLLs from
C:\Qt\5.10.0\msvc2015\bin to function well.

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
