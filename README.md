# <img src="http://rrwick.github.io/Bandage/images/logo.png" alt="Bandage" width="115" height="115" align="middle">Bandage

## Table of Contents

* [Intro](https://github.com/rrwick/Bandage#intro)
* [Help](https://github.com/rrwick/Bandage#help)
* [Pre-built binaries](https://github.com/rrwick/Bandage#pre-built-binaries)
* [Building from source](https://github.com/rrwick/Bandage#building-from-source)
  * [Ubuntu](https://github.com/rrwick/Bandage#ubuntu)
  * [Other Linux distributions](https://github.com/rrwick/Bandage#other-linux-distributions)
  * [Mac](https://github.com/rrwick/Bandage#mac)
  * [Using Qt Creator](https://github.com/rrwick/Bandage#using-qt-creator)
* [History](https://github.com/rrwick/Bandage#history)
* [Contributing](https://github.com/rrwick/Bandage#contributing)
* [Citation](https://github.com/rrwick/Bandage#citation)
* [Credits](https://github.com/rrwick/Bandage#credits)
* [License](https://github.com/rrwick/Bandage#license)

## Intro

Bandage is a GUI program that allows users to interact with the assembly graphs made by *de novo* assemblers such as <a href="https://www.ebi.ac.uk/~zerbino/velvet/" target="_blank">Velvet</a>, <a href="http://bioinf.spbau.ru/spades" target="_blank">SPAdes</a>, <a href="https://github.com/voutcn/megahit" target="_blank">MEGAHIT</a> and others.

*De novo* assembly graphs contain not only assembled contigs but also the connections between those contigs, which were previously not easily accessible. Bandage visualises assembly graphs, with connections, using graph layout algorithms. Nodes in the drawn graph, which represent contigs, can be automatically labelled with their ID, length or depth. Users can interact with the graph by moving, labelling and colouring nodes. Sequence information can also be extracted directly from the graph viewer. By displaying connections between contigs, Bandage opens up new possibilities for analysing and improving *de novo* assemblies that are not possible by looking at contigs alone.

More information and download links are on the Bandage website: <a href="http://rrwick.github.io/Bandage/" target="_blank">rrwick.github.io/Bandage</a>

## Help

Bandage documentation is available on the <a href="https://github.com/rrwick/Bandage/wiki" target="_blank">Bandage GitHub wiki</a>.

Bandage help tips are also built into the program. Throughout the UI, you will find these icons next to controls and settings: <img src="http://rrwick.github.io/Bandage/images/helptext.png" alt="help text icon" width="16" height="16">. Click them to see a description of that element of Bandage.

## Pre-built binaries

Compiled 64-bit binaries for Ubuntu Linux, CentOS, OS X and Windows are available in the GitHub <a href="https://github.com/rrwick/Bandage/releases/" target="_blank">'Releases' section</a> and from the <a href="http://rrwick.github.io/Bandage/" target="_blank">Bandage website</a>.

If you download and run the Mac binary, you may receive a warning stating that Bandage 'can't be opened because it is from an unidentified developer.' Right click on the file and select 'Open' to override this warning.

The Linux binaries comes in two varieties: dynamically-linked and statically-linked. The dynamically-linked binary is preferable, but it has more dependencies (like Qt 5). If you experience problems with the dynamically-linked binary, please try the statically-linked version instead.

## Building from source

If the compiled binaries do not work for you, the instructions below should help you build Bandage on most common OSs. If you are having difficulties building Bandage for your OS, feel free to contact me (Ryan) at rrwick@gmail.com and I'll do my best to help you out!

### <img src="http://rrwick.github.io/Bandage/images/OS/ubuntu.png" alt="" width="34" height="40" align="middle"> Ubuntu

The following instructions successfully build Bandage on a fresh installation of Ubuntu 14.04:

1. Ensure the package lists are up-to-date: `sudo apt-get update`
2. Install prerequisite packages: `sudo apt-get install build-essential git qtbase5-dev`
3. Download the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
4. Open a terminal in the Bandage directory.
5. Set the environment variable to specify that you will be using Qt 5, not Qt 4: `export QT_SELECT=5`
6. Run qmake to generate a Makefile: `qmake`
7. Build the program: `make`
8. `Bandage` should now be an executable file.
9. Optionally, copy the program into /usr/local/bin: `sudo make install`. The Bandage build directory can then be deleted.

### <img src="http://rrwick.github.io/Bandage/images/OS/linux.png" alt="" width="34" height="40" align="middle"> Other Linux distributions

If your Linux distribution is sufficiently similar to Ubuntu (e.g. Linux Mint), I recommend trying the Ubuntu build instructions above. Otherwise, try the 'Using Qt Creator' instructions below.

### <img src="http://rrwick.github.io/Bandage/images/OS/apple.png" alt="" width="34" height="40" align="middle"> Mac

The following instructions successfully build Bandage on OS X 10.8 and later:

1. Install Xcode, along with the Xcode Command Line Tools. If you can run the `clang` command from the terminal, you should be good to continue.
2. Install Qt 5. This is most easily done using Homebrew: <a href="http://brew.sh/" target="_blank">brew.sh</a>. Install the Qt 5 package with this command: `brew install qt5`.
3. Download the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
4. Open a terminal in the Bandage directory.
5. Run qmake to generate a Makefile. The location of the `qmake` command may vary, depending on how Qt was installed. If you used Homebrew, it should be here: `/usr/local/opt/qt5/bin/qmake`.
6. Build the program: `make`
7. `Bandage` should now be an application bundle.
8. Optionally, copy the Bandage application bundle into the 'Applications' folder and delete the Bandage build directory.

### <img src="http://rrwick.github.io/Bandage/images/OS/qt.png" alt="" width="34" height="40" align="middle"> Using Qt Creator

For Windows and some Linux distributions, the easiest way to build Bandage from source is by using Qt Creator, the Qt IDE. I successfully used this procedure has been used on Windows and on CentOS 6.7. It should also work for Ubuntu Linux and OS X.

1. Set up a compiler.
  * For Windows, you can use the Microsoft Visual C++ compiler by installing <a href=" https://www.visualstudio.com/" target="_blank">Visual Studio</a>. Alternatively, you can skip this step and the Qt installation will provide the MinGW compiler.
  * For CentOS, this can be done quickly using the yum package manager: `yum install gcc-c++`
  * <a href="http://doc.qt.io/qt-5/supported-platforms.html" target="_blank">Look here</a> for more information on Qt supported compilers.
2. Install the Qt SDK: <a href="http://www.qt.io/download-open-source/" target="_blank">www.qt.io/download-open-source</a>. The disk space required can be reduced by unticking the iOS and Android options in the Qt installer.
3. Download the Bandage code from GitHub:
  * Either clone using: `git clone https://github.com/rrwick/Bandage.git`
  * or download from here: <a href="https://github.com/rrwick/Bandage/archive/master.zip" target="_blank">https://github.com/rrwick/Bandage/archive/master.zip</a>
4. Open the Qt Creator program and load the `Bandage.pro` file which is in the Bandage directory. Since this is the first time the project has been loaded, it will ask you to configure the project. The defaults should be okay, so click 'Configure Project'.
5. Change to the release build configuration by clicking where it says 'Debug' in the bottom left area of Qt Creator and select 'Release'.
6. Build and run Bandage by clicking the green arrow in the bottom left area of Qt Creator. Bandage should open when the build has finished.
7. You will find the executable file in a new folder that begins with 'build-Bandage-Desktop'.

## History

v0.8.0 - 30 April 2016

v0.7.1 - 6 December 2015

v0.7.0 - 16 October 2015

v0.6.4 - 17 September 2015

v0.6.3 - 31 August 2015

v0.6.2 - 29 July 2015

v0.6.1 - 15 July 2015

v0.6.0 - 8 July 2015

v0.5.6 - 26 May 2015

v0.5.5 - 14 May 2015

v0.5.4 - 5 May 2015

v0.5.3 - 30 April 2015

v0.5.2 - 25 April 2015

v0.5.1 - 20 April 2015

v0.5.0 - 9 April 2015

v0.4.2 - 20 March 2015

v0.4.1 - 12 March 2015

v0.4.0 - 10 March 2015

v0.3.0 – 19 February 2015

## Contributing

New contributors are welcome! If you're interested or have ideas, please contact me (Ryan) at rrwick@gmail.com.

## Citation

If you use Bandage in your research, please cite the following publication:

[Wick R.R., Schultz M.B., Zobel J. & Holt K.E. (2015). Bandage: interactive visualisation of de novo genome assemblies. Bioinformatics, 31(20), 3350-3352.](http://bioinformatics.oxfordjournals.org/content/31/20/3350)

## Credits

Bandage makes use of the <a href="http://www.ogdf.net/" target="_blank">OGDF</a> library for performing graph layout algorithms. My thanks goes out to the OGDF developers for their excellent work!

I also thank <a href="https://github.com/rchikhi" target="_blank">Rayan Chikhi</a> and <a href="https://github.com/epruesse" target="_blank">Elmar Pruesse</a> for their contributions to the code.

## License

GNU General Public License, version 3
