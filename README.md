# <img src="http://rrwick.github.io/Bandage/images/logo.png" alt="Bandage" width="115" height="115" align="middle">Bandage

## Table of Contents

* [2022 update](https://github.com/rrwick/Bandage#2022-update)
* [Intro](https://github.com/rrwick/Bandage#intro)
* [Help](https://github.com/rrwick/Bandage#help)
* [Pre-built binaries](https://github.com/rrwick/Bandage#pre-built-binaries)
* [Building from source](https://github.com/rrwick/Bandage#building-from-source)
* [Contributing](https://github.com/rrwick/Bandage#contributing)
* [Citation](https://github.com/rrwick/Bandage#citation)
* [Credits](https://github.com/rrwick/Bandage#credits)
* [License](https://github.com/rrwick/Bandage#license)



## 2022 update

Bandage is now about seven years old! Despite its popularity, I sadly don't have the time to give it the attention it deserves. Also, its codebase is too large and my C++/Qt skills are too rusty for me to make anything more than trivial changes.

I'm not declaring Bandage completely abandoned, because I still think it's a useful tool, but don't expect a lot development or support on this repo. I will, however, strive to ensure that Bandage continues to run well on modern systems. E.g. I released [v0.9.0](https://github.com/rrwick/Bandage/releases/tag/v0.9.0) in January 2022, which compiles using modern versions of [Qt](https://www.qt.io/) and runs natively on Apple Silicon Macs.

A number of enterprising users have [forked Bandage](https://techgaun.github.io/active-forks/index.html#rrwick/Bandage), so if you’re after a specific new feature, you might find it implemented in one of the forks, the most active of which is [BandageNG](https://github.com/asl/BandageNG).



## Intro

Bandage is a GUI program that allows users to interact with the assembly graphs made by *de novo* assemblers such as <a href="https://www.ebi.ac.uk/~zerbino/velvet/" target="_blank">Velvet</a>, <a href="http://bioinf.spbau.ru/spades" target="_blank">SPAdes</a>, <a href="https://github.com/voutcn/megahit" target="_blank">MEGAHIT</a> and others.

*De novo* assembly graphs contain not only assembled contigs but also the connections between those contigs, which were previously not easily accessible. Bandage visualises assembly graphs, with connections, using graph layout algorithms. Nodes in the drawn graph, which represent contigs, can be automatically labelled with their ID, length or depth. Users can interact with the graph by moving, labelling and colouring nodes. Sequence information can also be extracted directly from the graph viewer. By displaying connections between contigs, Bandage opens up new possibilities for analysing and improving *de novo* assemblies that are not possible by looking at contigs alone.

More information and download links are on the Bandage website: <a href="http://rrwick.github.io/Bandage/" target="_blank">rrwick.github.io/Bandage</a>



## Help

Bandage documentation is available on the <a href="https://github.com/rrwick/Bandage/wiki" target="_blank">Bandage GitHub wiki</a>.

Bandage help tips are also built into the program. Throughout the UI, you will find these icons next to controls and settings: <img src="http://rrwick.github.io/Bandage/images/helptext.png" alt="help text icon" width="16" height="16">. Click them to see a description of that element of Bandage.



## Pre-built binaries

Compiled 64-bit binaries for Ubuntu Linux, CentOS, macOS and Windows are available in the GitHub <a href="https://github.com/rrwick/Bandage/releases/" target="_blank">'Releases' section</a> and from the <a href="http://rrwick.github.io/Bandage/" target="_blank">Bandage website</a>.

If you download and run the Mac binary, you may receive a warning stating that Bandage 'can't be opened because it is from an unidentified developer' or 'is damaged and can't be opened'. Right click on the file and select 'Open' to override this warning. If that doesn't work, clear the quarantine extended attribute via the command line with `xattr -cr Bandage.app`.

The Linux binaries come in two varieties: dynamically-linked and statically-linked. The dynamically-linked binary is preferable, but it has more dependencies (like Qt 5). If you experience problems with the dynamically-linked binary, please try the statically-linked version instead.



## Building from source

If the compiled binaries do not work for you, you can build Bandage on most common OSs using [Qt](https://www.qt.io/):

1. Make sure you have a compiler and other common developer tools set up.
  * For macOS, this means Xcode and its command line tools.
  * For Ubuntu: `sudo apt install build-essential git libgl1-mesa-dev libxcb-xinerama0`
2. Install the [Qt SDK](https://www.qt.io/download-open-source) with version 5.15 or later. Double check that you install a version compatible with your OS ([Qt 5 supported platforms](https://doc.qt.io/qt-5/supported-platforms.html) and [Qt 6 supported platforms](https://doc.qt.io/qt-6/supported-platforms.html)).
3. Clone the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
4. Open the Qt Creator program and load the `Bandage.pro` file which is in the Bandage directory. Since this is the first time the project has been loaded, it will ask you to configure the project. The defaults should be okay, so click 'Configure Project'.
5. Change to the release build configuration by clicking where it says 'Debug' in the bottom left area of Qt Creator and select 'Release'.
6. Build and run Bandage by clicking the green arrow in the bottom left area of Qt Creator. Bandage should open when the build has finished.
7. You will find the executable file in a new folder that begins with 'build-Bandage-Desktop'.



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
