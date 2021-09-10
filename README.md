# <img src="http://rrwick.github.io/Bandage/images/logo.png" alt="Bandage" width="115" height="115" align="middle">Bandage

## IMPORTANT: This is a fork of original Bandage repo. It may contain new features, new bugs and otherwise be not compatible with the Bandage by Ryan Wick. The documentation, instructions, etc. might be incomplete, missed and inaccurate as well

## Table of Contents

* [2022 update](https://github.com/asl/Bandage#2022-update)
* [Intro](https://github.com/asl/Bandage#intro)
* [Help](https://github.com/asl/Bandage#help)
* [Pre-built binaries](https://github.com/asl/Bandage#pre-built-binaries)
* [Building from source](https://github.com/asl/Bandage#building-from-source)
* [Contributing](https://github.com/asl/Bandage#contributing)
* [Citation](https://github.com/asl/Bandage#citation)
* [Credits](https://github.com/asl/Bandage#credits)
* [License](https://github.com/asl/Bandage#license)



## 2022 update

Bandage is now about seven years old! Despite its popularity, I sadly don't have the time to give it the attention it deserves. Also, its codebase is too large and my C++/Qt skills are too rusty for me to make anything more than trivial changes.

I'm not declaring Bandage completely abandoned, because I still think it's a useful tool, but don't expect a lot development or support on this repo. I will, however, strive to ensure that Bandage continues to run well on modern systems. E.g. I released [v0.9.0](https://github.com/rrwick/Bandage/releases/tag/v0.9.0) in January 2022, which compiles using modern versions of [Qt](https://www.qt.io/) and runs natively on Apple Silicon Macs.



## Intro

Bandage is a GUI program that allows users to interact with the assembly graphs made by *de novo* assemblers such as <a href="https://www.ebi.ac.uk/~zerbino/velvet/" target="_blank">Velvet</a>, <a href="http://bioinf.spbau.ru/spades" target="_blank">SPAdes</a>, <a href="https://github.com/voutcn/megahit" target="_blank">MEGAHIT</a> and others.

*De novo* assembly graphs contain not only assembled contigs but also the connections between those contigs, which were previously not easily accessible. Bandage visualises assembly graphs, with connections, using graph layout algorithms. Nodes in the drawn graph, which represent contigs, can be automatically labelled with their ID, length or depth. Users can interact with the graph by moving, labelling and colouring nodes. Sequence information can also be extracted directly from the graph viewer. By displaying connections between contigs, Bandage opens up new possibilities for analysing and improving *de novo* assemblies that are not possible by looking at contigs alone.

## Help

Bandage documentation is available on the <a href="https://github.com/rrwick/Bandage/wiki" target="_blank">Bandage GitHub wiki</a>.

Bandage help tips are also built into the program. Throughout the UI, you will find these icons next to controls and settings: <img src="http://rrwick.github.io/Bandage/images/helptext.png" alt="help text icon" width="16" height="16">. Click them to see a description of that element of Bandage.

## Prerequisites
  * Qt 6
  * ...

## Pre-built binaries

No pre-built binaries are available at the moment

## Building from source

To be updated...




## Contributing

New contributors are welcome! If you're interested or have ideas, please use Issues section in the repo.



## Citation

If you use Bandage in your research, please cite the following publication:

[Wick R.R., Schultz M.B., Zobel J. & Holt K.E. (2015). Bandage: interactive visualisation of de novo genome assemblies. Bioinformatics, 31(20), 3350-3352.](http://bioinformatics.oxfordjournals.org/content/31/20/3350)



## Credits

Bandage makes use of the <a href="http://www.ogdf.net/" target="_blank">OGDF</a> library for performing graph layout algorithms. Big thanks goes out to the OGDF developers for their excellent work!

I also thank <a href="https://github.com/rchikhi" target="_blank">Rayan Chikhi</a> and <a href="https://github.com/epruesse" target="_blank">Elmar Pruesse</a> for their contributions to the code.



## License

GNU General Public License, version 3
