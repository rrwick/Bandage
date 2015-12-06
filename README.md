# <img src="http://rrwick.github.io/Bandage/images/logo.png" alt="Bandage" width="115" height="115" align="middle">Bandage

Bandage is a GUI program that allows users to interact with the assembly graphs made by *de novo* assemblers Velvet and SPAdes.

*De novo* assembly graphs contain assembled contigs (nodes) but also the connections between those contigs (edges), which are not easily accessible to users. Bandage visualises assembly graphs, with connections, using graph layout algorithms. Nodes in the drawn graph, which represent contigs, can be automatically labelled with their ID, length or depth. Users can interact with the graph by moving, labelling and colouring nodes. Sequence information can also be extracted directly from the graph viewer. By displaying connections between contigs, Bandage opens up new possibilities for analysing *de novo* assemblies that are not possible by looking at contigs alone.

More information and download links are on the Bandage website: <a href="http://rrwick.github.io/Bandage/" target="_blank">rrwick.github.io/Bandage</a>

## Help

Bandage documentation is available on the <a href="https://github.com/rrwick/Bandage/wiki" target="_blank">Bandage GitHub wiki</a>.

Bandage help is also built into the program. Throughout the UI, you will find these help icons next to controls and settings: <img src="http://rrwick.github.io/Bandage/images/helptext.png" alt="help text icon" width="16" height="16">. Click on these icons to read a description of how to use that element of Bandage.

## Building from source

Compiled 64-bit binaries for Ubuntu Linux, OS X and Windows are available in the GitHub <a href="https://github.com/rrwick/Bandage/releases/" target="_blank">'Releases' section</a> and from the <a href="http://rrwick.github.io/Bandage/" target="_blank">Bandage website</a>.

If the compiled binaries do not work for you, the instructions below should help you build Bandage on most common OSs.  If you are having difficulties building Bandage for your OS, feel free to contact me (Ryan) at rrwick@gmail.com and I'll do my best to help you out!

### <img src="http://rrwick.github.io/Bandage/images/OS/ubuntu.png" alt="" width="34" height="40" align="middle"> Ubuntu Linux

The following instructions successfully build Bandage on a fresh installation of Ubuntu 14.04:

1. Ensure the package lists are up-to-date: `sudo apt-get update`
2. Install prerequisite packages: `sudo apt-get install build-essential git qtbase5-dev`
3. Prepare the OGDF library:
  1. Download the OGDF code (2012.07 Sakura release) from <a href="http://www.ogdf.net/doku.php/tech:versions" target="_blank">www.ogdf.net</a> and unzip.
  2. Open a terminal in the OGDF directory.
  3. Create the Makefile: `./makeMakefile.sh`
  4. Compile the library: `make`
4. Download the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
5. Ensure that the Bandage directory and the OGDF directory are stored in the same parent directory.
6. Open a terminal in the Bandage directory.
7. Set the environment variable to specify that you will be using Qt 5, not Qt 4: `export QT_SELECT=5`
8. Run qmake to generate a Makefile: `qmake`
9. Build the program: `make`
10. `Bandage` should now be an executable file.
11. Optionally, copy the program into /usr/local/bin: `sudo make install`. The Bandage build directory can then be deleted.
12. Optionally, delete the OGDF directory.

### <img src="http://rrwick.github.io/Bandage/images/OS/linux.png" alt="" width="34" height="40" align="middle"> Other Linux distributions

If your Linux distribution is sufficiently similar to Ubuntu (e.g. Linux Mint), I recommend trying the Ubuntu build instructions above.  Otherwise, try the 'Using Qt Creator' instructions below.

### <img src="http://rrwick.github.io/Bandage/images/OS/apple.png" alt="" width="34" height="40" align="middle"> Mac

If you download and run the Mac binary, you may receive a warning stating that Bandage 'can't be opened because it is from an unidentified developer.'  Right click on the file and select 'Open' to override this warning.

The following instructions successfully build Bandage on OS X 10.7 (Lion), 10.8 (Mountain Lion), 10.9 (Mavericks) and 10.10 (Yosemite):

1. Install Xcode, along with the Xcode Command Line Tools.
2. Install Qt 5
  * For OS X 10.8 and later this is most easily done using Homebrew: <a href="http://brew.sh/" target="_blank">brew.sh</a>. Install the Qt 5 package with this command: `brew install qt5`
  * For OS X 10.7 (or if the Homebrew install fails), it will be necessary to instead download and install the Qt SDK: <a href="http://www.qt.io/download-open-source/" target="_blank">www.qt.io/download-open-source</a>. The disk space required can be reduced by unticking the iOS and Android options in the Qt installer.
3. Prepare the OGDF library:
  1. Download the OGDF code (2012.07 Sakura release) from <a href="http://www.ogdf.net/doku.php/tech:versions" target="_blank">www.ogdf.net</a> and unzip.
  2. In the OGDF directory, edit the makeMakefile.config file. In the 'VERSIONS' section, add `-DOGDF_MEMORY_MALLOC_TS` to both the debug and release lines so they look like this:
    * `debug = -g3 -O0 -DOGDF_DEBUG -DOGDF_MEMORY_MALLOC_TS`
    * `release = -O2 -DOGDF_MEMORY_MALLOC_TS`
  3. Open a terminal in the OGDF directory.
  4. Create the Makefile: `./makeMakefile.sh`
  5. Compile the library: `make`
4. Download the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
5. Ensure that the Bandage directory and the OGDF directory are stored in the same parent directory.
6. Open a terminal in the Bandage directory.
7. Run qmake to generate a Makefile:
  * On OS X 10.7 and 10.8, it is first necessary to remove this line from the Bandage.pro file, `CONFIG += C++11`.
  * If Qt was install with Homebrew, this command will work: `/usr/local/opt/qt5/bin/qmake`
  * If Qt was installed using the Qt installer, qmake will be located here: `QT_INSTALL_DIRECTORY/QT_VERSION/clang_64/bin/qmake`
8. Build the program: `make`
9. `Bandage` should now be an application bundle.
10. Optionally, copy the Bandage application bundle into the 'Applications' folder and delete the Bandage build directory.
11. Optionally, delete the OGDF directory.

### <img src="http://rrwick.github.io/Bandage/images/OS/qt.png" alt="" width="34" height="40" align="middle"> Using Qt Creator

For Windows and some Linux distributions, the easiest way to build Bandage from source is by using Qt Creator, the Qt IDE.  I successfully used this procedure has been used on Windows and on CentOS 6.7.  It should also work for Ubuntu Linux and OS X.

1. Set up a compiler.
  * For Windows, I recommend installing Visual Studio 2013 (a.k.a. Visual Studio 12.0) which has the MSVC2013 compiler.
  * For CentOS, this can be done quickly using the yum package manager: `yum install gcc-c++`
2. Install the Qt SDK: <a href="http://www.qt.io/download-open-source/" target="_blank">www.qt.io/download-open-source</a>. The disk space required can be reduced by unticking the iOS and Android options in the Qt installer.
3. Prepare the OGDF library:
  1. Download the OGDF code (2012.07 Sakura release) from <a href="http://www.ogdf.net/doku.php/tech:versions" target="_blank">www.ogdf.net</a> and unzip.
  2. In the OGDF directory, edit the makeMakefile.config file. In the 'VERSIONS' section, add `-DOGDF_MEMORY_MALLOC_TS` to both the debug and release lines so they look like this:
    * `debug = -g3 -O0 -DOGDF_DEBUG -DOGDF_MEMORY_MALLOC_TS`
    * `release = -O2 -DOGDF_MEMORY_MALLOC_TS`
  3. Open a terminal in the OGDF directory.
  4. Create the Makefile: `./makeMakefile.sh`
  5. Compile the library: `make`
4. Download the Bandage code from GitHub:
  * Either clone using: `git clone https://github.com/rrwick/Bandage.git`
  * Or download from here: <a href="https://github.com/rrwick/Bandage/archive/master.zip" target="_blank">https://github.com/rrwick/Bandage/archive/master.zip</a>
5. Ensure that the Bandage directory and the OGDF directory are stored in the same parent directory.
6. Open the Qt Creator program and load the `Bandage.pro` file which is in the Bandage directory.  Since this is the first time the project has been loaded, it will ask you to configure the project.  The defaults should be okay, so click 'Configure Project'.
7. Change the build configuration to release by clicking where it says 'Debug' in the bottom left area of Qt Creator and select 'Release'.
8. Build and run Bandage by clicking the green arrow in the bottom left area of Qt Creator.  Bandage should open when the build has finished.
9. You will find the executable file in a new folder that begins with 'build-Bandage-Desktop'.


## History

v0.7.1 - bug fixes

v0.7.0 - graph editing capabilities

v0.6.4 - improvements to BLAST and main GUI

v0.6.3 - improvements to BLAST functionality

v0.6.2 - new features for working with graph paths

v0.6.1 - improvements to BLAST functionality

v0.6.0 - new features, including command line functionality

v0.5.6 - bug fixes

v0.5.5 - bug fixes

v0.5.4 - bug fixes

v0.5.3 - bug fixes

v0.5.2 - performance improvements

v0.5.1 - bug fixes

v0.5.0 - new features, including Trinity support

v0.4.2 - bug fixes

v0.4.1 - bug fixes

v0.4.0 - new features, including BLAST integration

v0.3.0 – initial release on GitHub

## Contributing

New contributors are welcome!  If you're interested or have ideas, please contact me (Ryan) at rrwick@gmail.com.

## License

GNU General Public License, version 3
