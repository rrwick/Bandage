# <img src="http://rrwick.github.io/Bandage/images/logo.png" alt="alt text" width="115" height="115" align="middle">Bandage

Bandage is a GUI program that allows users to interact with the assembly graphs made by *de novo* assemblers Velvet and SPAdes.

*De novo* assembly graphs contain assembled contigs (nodes) but also the connections between those contigs (edges), which are not easily accessible to users.  Bandage visualises assembly graphs, with connections, using graph layout algorithms.  Nodes in the drawn graph, which represent contigs, can be automatically labelled with their ID, length or depth.  Users can interact with the graph by moving, labelling and colouring nodes.  Sequence information can also be extracted directly from the graph viewer.  By displaying connections between contigs, Bandage opens up new possibilities for analysing *de novo* assemblies that are not possible by looking at contigs alone.

More information and screenshots are on the Bandage website: <a href="http://rrwick.github.io/Bandage/" target="_blank">rrwick.github.io/Bandage</a>

## Installation

Compiled 64-bit binaries for Linux, OS X and Windows are available in the GitHub <a href="https://github.com/rrwick/Bandage/releases/" target="_blank">'Releases' section</a> and from the <a href="http://rrwick.github.io/Bandage/" target="_blank">Bandage website</a>.

### Linux

The following instructions successfully build Bandage on a fresh installation of Ubuntu 14.04:

1. Ensure the package lists are up-to-date: `sudo apt-get update`
2. Install the prerequisite packages: `sudo apt-get install build-essential git qtbase5-dev`
3. Prepare the OGDF library:
  1. Download the OGDF code: <a href="http://www.ogdf.net/" target="_blank">www.ogdf.net</a>
  2. Open a terminal in the OGDF directory.
  3. Create the Makefile: `./makeMakefile.sh`
  4. Compile the library: `make`
4. Download the Bandage code from GitHub: `git clone https://github.com/rrwick/Bandage.git`
5. Ensure that the Bandage directory and the OGDF directory are stored in the same parent directory
6. Open a terminal in the Bandage directory.
7. Set the environment variable to specify that you will be using Qt 5, not Qt 4: `export QT_SELECT=5`
8. Run qmake to generate a Makefile: `qmake`
9. Build the program: `make`
10. `Bandage` should now be an executable file.
11. Optionally, copy the program into /usr/local/bin: `sudo make install`.  The Bandage build directory can then be deleted.
12. Optionally, delete the OGDF directory.

### Mac

The following instructions successfully build Bandage on OS X 10.7 (Lion), 10.8 (Mountain Lion) 10.9 (Mavericks) and 10.10 (Yosemite):

1. Install Xcode, along with the Xcode Command Line Tools.
2. Install Qt 5
  * For OS X 10.8 and later this is most easily done using Homebrew: <a href="http://brew.sh/" target="_blank">brew.sh</a>.  Install the Qt 5 package with this command: `brew install qt5`
  * For OS X 10.7 (or if the Homebrew install fails), it will be necessary to instead download and install the Qt SDK: <a href="http://www.qt.io/download-open-source/" target="_blank">www.qt.io/download-open-source</a>.  The disk space required can be greatly reduced by unticking the iOS and Android options in the Qt installer.
3. Prepare the OGDF library:
  1. Download the OGDF code (2012.07 Sakura release) from <a href="http://www.ogdf.net/" target="_blank">www.ogdf.net</a> and unzip.
  2. In the OGDF directory, edit the makeMakefile.config file.  In the 'VERSIONS' section, add `-DOGDF_MEMORY_MALLOC_TS` to both the debug and release lines so they look like this:
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

### Windows

Building Bandage in Windows is more challenging than in Linux or OS X, but it can be done.  If you would like to, feel free to contact me (Ryan) at rrwick@gmail.com and I'll do my best to help you out!

## Usage

### Basics

##### Usage

1. Open a Bandage window.
2. Load a graph file using the 'File' menu, either a LastGraph file from Velvet or a FASTG file from SPAdes.  For large graph files, this may take a few seconds or more.
3. Click the 'Draw graph' button now to visualise the entire graph.

##### Drawing options:

* Due to the stochastic nature of the graph layout algorithm, clicking the 'Draw graph' button again will redraw the graph with nodes in different positions.
* By changing the scope from 'Entire graph' to 'Around node(s)', it is possible to draw only a subsection of the graph.  The specified nodes will be drawn, along with all nodes within the given distance.  For example, a distance of zero will draw only the specified nodes, a distance of one will draw the specified nodes and their immediate neighbours, etc.

##### Node style:

* Single nodes are the default setting.  This is where only positive nodes are displayed, not negative nodes (the reverse complement nodes).
* Double nodes show nodes and their reverse complement as two separate objects with arrow heads to indicate direction.  This style can be useful for deciphering particular complex regions or extracting strand-specific sequences.

### Controls

##### Selection

* Click on any node or edge to select it.
* Holding Ctrl while clicking on nodes or edges adds them to the selection.
* Click and drag to select large numbers of nodes and edges.

##### Moving nodes

* Left click and drag on any node to move it.  For short nodes, this will move the entire node.  For long nodes, this will move the region of the node near the mouse cursor.
* Right click and drag on any node to move it one piece at a time.  This is useful for fine tuning the shape of a graph or for rotating short nodes.
* Selected nodes are moved in their entirety when dragged.  If multiple nodes are selected, they will all be moved together when dragged.

##### Mouse navigation

* Ctrl+mouse wheel will zoom the view in to and out from the location of your mouse cursor.
* Ctrl+click and drag with the left mouse button will pan the view.
* Ctrl+click and drag with the right mouse button will rotate the view.

##### Keyboard navigation controls

* It is first necessary to click in the viewport (or use tab until it is selected) so it will receive keyboard input.
* Plus and minus keys will zoom in and out.
* Arrow keys will pan the viewport horizontally and vertically.
* Shift+plus and shift+minus will rotate the view clockwise and anti-clockwise.

### Customisation

##### Colours

There are three built-in colouring schemes:
* One colour – all nodes are the same colour, which can be set using the 'Select' button
* Coverage – nodes range in colour from black (low coverage) to red (high coverage)
* Random – colours are assigned randomly, to aid in distinguishing one from the next

It is also possible to select the 'Custom' colouring scheme, where the user can specify node colours.  Inititially, all nodes will be grey, but selected nodes can have their colour specified by clicking the 'Set colour' button in the right pane.

##### Labels

Nodes can be labeled with their number, sequence length, coverage, a custom label or any combination of the above.  Custom labels are applied to selected nodes by clicking the 'Set label' button in the right pane.

The font can be set using the 'Font' button.  Turning on 'Text outline' will surround the black text with a white outline, which can make reading the text easier, especially over darker coloured nodes.

##### Graph layout

The 'Settings' dialog (accessible under the 'Tools' menu) exposes more options.

The 'Base pairs per segment' settings controls the length of the drawn nodes.  The number of line segments that make up a drawn node is determined by dividing the sequence length by this value (rounded up to the nearest integer).  Any node with a sequence length of less than or equal to this value will be drawn a single line segment.  Guidelines for this setting:
* Large values will result in shorter nodes.  Very large values will result in all nodes being a similiar size (one line segment).
* Small values will result in longer nodes and a stronger correlation between sequence length and node length.
* Large values make the graph layout calculations easier, due to their being less pieces to position.  Large values are therefore recommended for visualising very big assembly graphs.

The 'Graph layout iterations' setting controls how much time the graph layout algorithm spends on positioning the graph components.  Low settings are faster and therefore recommended for big assembly graphs.  Higher settings may result in smoother, more pleasing layouts.

##### Graph appearance

Additional settings for controling graph appearance are also accessible via the 'Settings' dialog:
* Base node width – the minimum width for each node
* Additional node width from coverage – the degree to which high coverage nodes are wide than the base node width
* Maximum node width – limits node width so very high coverge nodes do not appear excessively wide
* Edge width – the width of the edges that connect nodes
* Outline thickness – the thickness of the black outline drawn around each node
* Text outline thickness – the thickness of the white outline drawn around node labels (when enabled)
* Antialiasing – results in a smoother graph appearance at the cost of performance

### Assembly tips

The structure of the graph is highly dependant on the k-mer size used for assembly.  Small k-mers result in shorter contigs but with lots of connections, while large k-mers can result in longer contigs but with fewer connections.

If your graph consists of many separate disconnected subgraphs (i.e. there are many small groups of contigs that have no connections to the rest of the graph), then your k-mer size may be too large.  Alternatively, if your graph is connected (i.e. all contigs are tied together in a single graph structure) but is very dense and tangled, then your k-mer size may be too small.

When assembling 100 bp reads in Velvet, a k-mer of 51 would be a good starting point, and then adjust up or down as needed.  SPAdes conveniently conducts assembly multiple times using different k-mers, so you can look at the FASTG files for each assembly (in folders named like K21, K33, etc.) to find the best graph for viewing in Bandage.

## Contributing

New contributors are welcome!  If you're interested or have ideas, please contact me (Ryan) at rrwick@gmail.com.

## History

Version 0.4.0 - new features, including BLAST integration

Version 0.3.0 – initial release on GitHub

## License

GNU General Public License, version 3
