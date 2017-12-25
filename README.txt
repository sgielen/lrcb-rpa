LRCB-RPA
========

This project builds a tool for the Dutch expert centre for screening (LRCB,
Landelijk Referentiecentrum voor Bevolkingsonderzoek). It is intended for
radiologist performance assessments (RPA).

Building
--------

Create a build dir. There, run `cmake`, if necessary with `-DQt5_DIR` pointing at
a directory containing `Qt5Config.cmake`. Use `make` to build the binary.

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
