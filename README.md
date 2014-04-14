Datenbanksysteme und moderne CPU-Architekturen SS2014

http://www-db.in.tum.de/teaching/ss14/moderndbs/?lang=de

assignment solutions by:
	Michael Haubenschild (ga29tar@mytum.de)
	Andreas Seibold (ga34daq@mytum.de)

1. Assignment: externalSorting
==============================
	the Makefile is build upon the official googletest example. To make it run, you should only need to specify the variable GTEST_DIR to point to the correct location. We used gtest-1.7.0 for our testing.

	running make will create the binary file 'externalSort_unittest' in the /bin directory.
	It can be run with the following arguments: externalSort_unittest <inputFile> <outputFile> <memoryBufferInMB>

	You need write access in the current directory you are in, since there are several tempfiles created there
