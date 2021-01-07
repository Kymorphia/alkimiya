# Instrument Browser
The Instrument Browser shows the currently open Instrument Files as a tree, much like a file browser. There are many types of Nodes in the tree with different operations that can be performed on them. Navigating the tree involves expanding parent Nodes to access their children and selecting one or more Nodes to perform actions on. Actions can be performed by drag and drop operations, the right click menu, or keyboard hot keys.

## Selecting Nodes
Selecting nodes in the Instrument Browser works like most other File Browsers in regards to selecting one or more items. The list below outlines these various selection operations.
* Selecting a single item is done by simply clicking on it in the Instrument Browser or pressing the Up or Down arrows on the keyboard to select the previous or next item.
* Selecting multiple individual items can be done by holding down the CTRL key and clicking on each item to add it to the current selection.
* To select a range of items, click on the first one in the range (top or bottom) and then click on the other end of the range while holding the SHIFT key. This can also be done with the keyboard by holding the shift KEY and pressing the Up or Down arrow.

## Expanding and Collapsing Nodes
Nodes can be expanded by clicking on the triangle to the left of it's icon, revealing the Node's children. Clicking on the triangle again collapses the tree branch. This can also be done with the + (plus) and - (minus) keys on the keyboard.

## Drag and Drop
Many reorganizing operations can be performed by drag and drop.

**NOTE:** When dragging Nodes, horizontal lines are drawn above and/or below the destination to indicate whether the drop operation is **on** a Node or **in between** nodes. If a line is drawn above and below a node while dragging it is a "drag into" operation and if it is a single line, it is a "drag between" operation. Dragging Nodes on to another Node will attempt to copy/move them inside the Node, as children. Whereas dropping selected Nodes between other Nodes will attempt to copy/move them in-between those nodes.

Drag and drop operations include the following:
* Instruments can be moved between Instrument Files, by selecting them and dragging them **on** to the desination File Node or **in between** Instruments of another File. Holding down the CTRL key when dropping the Nodes will copy them instead.
* The order of Instruments can be changed by moving them within the same Instrument File.
* Waves, Controls, and Filters can be moved between Instruments, by selecting them and dragging them **on** to the destination Instrument Node or **in between** children Nodes of another Instrument. Holding down the CTRL key when dropping the Nodes will copy them instead.
* Likewise the order of children Nodes of Instruments can be changed by moving them via drag and drop within the same Instrument.

## Node Types
There exist several node types which are found in the Instrument Browser. These are described in the following list.

* ![Instrument File icon](icon:alki-file-kvi)  **Instrument File** - These toplevel Nodes are the currently open Kymorphia Vector Instrument files, containing one or more Instruments.
* ![Instrument icon](icon:alki-inst)  **Instrument** - The children of an Instrument File Node, each an individual Instrument which can be assigned to a Channel and synthesized.
* **Wave** - Wave Nodes are used for all Curve Nodes within an Instrument. This includes controls as well. They are depicted with different icons depending on their function.
  * ![Signal icon](icon:alki-signal-sine)  **Signal** - All Waves which affect the Signal attribute are depicted with the same sine wave icon, even though they can be arbitrary waveforms defined by a Curve.
  * ![Volume Attack icon](icon:alki-vol-attack)  **Volume Attack** - Volume Attack Nodes control the note-on portion of the Volume envelope (the Attack, Hold, Decay, and Sustain phases of traditional and less flexible AHDSR envelopes).
  * ![Volume Release icon](icon:alki-vol-release)  **Volume Release** - Volume Release Nodes control the note-off portion of the Volume envelope (Release phase of an AHDSR envelope) and also determine when an Instrument Voice stops synthesizing.
  * ![Volume Modulation icon](icon:alki-vol-mod)  **Volume Modulation** - Volume Modulation Nodes oscillate the Volume by looping the Curve in a Wave Node.
  * ![Pitch Attack icon](icon:alki-pitch-attack)  **Pitch Attack** - Pitch Attack Nodes control the note-on portion of the Pitch envelope.
  * ![Pitch Release icon](icon:alki-pitch-release)  **Pitch Release** - Pitch Release Nodes control the note-off portion of the Pitch envelope.
  * ![Pitch Modulation icon](icon:alki-pitch-mod)  **Pitch Modulation** - Pitch Modulation Nodes oscillate the Pitch by looping the Curve in a Wave Node.
  * ![Balance Attack icon](icon:alki-balance-attack)  **Balance Attack** - Balance Attack Nodes control the stereo Balance during note-on phase of an Instrument.
  * ![Balance Release icon](icon:alki-balance-release)  **Balance Release** - Balance Release Nodes control the stereo Balance during the note-off phase of an Instrument.
  * ![Balance Modulation icon](icon:alki-balance-mod)  **Balance Modulation** - Balance Modulation Nodes oscillate the stereo Balance by looping the Curve in a Wave Node.
  * ![Morph Attack icon](icon:alki-morph-attack)  **Morph Attack** - Morph Attack Nodes control the Morph Attribute during the note-on phase of an Instrument.
  * ![Morph Release icon](icon:alki-morph-release)  **Morph Release** - Morph Release Nodes control the Morph Attribute during the note-off phase of an Instrument.
  * ![Morph Modulation icon](icon:alki-morph-mod)  **Morph Modulation** - Morph Modulation Nodes oscillate the Morph Attribute by looping the Curve in a Wave Node.
  * ![Volume Control icon](icon:alki-ctrl-vol)  **Volume Control** - A Node for controlling Volume
  * ![Pitch Control icon](icon:alki-ctrl-pitch)  **Pitch Control** - A Node for controlling Pitch
  * ![Balance Control icon](icon:alki-ctrl-balance)  **Balance Control** - A Node for controlling stereo Balance
  * ![Morph Control icon](icon:alki-ctrl-morph)  **Morph Control** - A Node for controlling the Morph Attribute
  * ![Time Control icon](icon:alki-ctrl-time)  **Time Control** - A Node for controlling Time (a similar but separate attribute from Pitch)
* ![Filter icon](icon:insert-object)  **Filter** - A Filter Node modifies the signal passing through it. For example, a Low Pass filter decreases the level of higher frequencies.
* ![Generator icon](icon:insert-object)  **Generator** - Generator Nodes create a signal without modifying (or filtering) an existing one.


## Right Click Menu
Right clicking on a Node in the Instrument Browser will show the popup right click menu. The available menu options depend on the type of Node that is selected when the right click menu is activated. These menu actions are described in the following list.

* ![New Instrument icon](icon:alki-inst)  **New Instrument** - Add a new instrument node
* ![New Signal icon](icon:alki-signal-sine)  **New Signal** - Menu for adding Signal Waves with predefined waveforms
  * ![Sine Wave icon](icon:alki-signal-sine)  **Sine Wave** (CTRL-I) - Add a sine Wave signal node
  * ![Triangle Wave icon](icon:alki-signal-triangle)  **Triangle Wave** - Add a triangle Wave signal node
  * ![Square Wave icon](icon:alki-signal-square)  **Square Wave** - Add a square Wave signal node
  * ![Saw Wave Up icon](icon:alki-signal-saw-up)  **Saw Wave Up** - Add a saw wave up Signal node (also known as a ramp up wave)
  * ![Saw Wave Down icon](icon:alki-signal-saw-down)  **Saw Wave Down** - Add a saw wave down Signal node (also known as a ramp down wave)
* ![New Volume Node icon](icon:alki-vol-attack)  **New Volume Node** - Menu for adding new volume Waves with predefined waveforms
  * ![Volume Attack icon](icon:alki-vol-attack)  **Volume Attack** - Add an attack Wave for controlling volume during the note-on phase of the Instrument
  * ![Volume Release icon](icon:alki-vol-release)  **Volume Release** - Add a release Wave for controlling volume during the note-off phase of the Instrument
  * ![Volume Modulation icon](icon:alki-vol-mod)  **Volume Modulation** - Add a modulation (oscillator) Wave for controlling Volume
* ![New Pitch Node icon](icon:alki-pitch-mod)  **New Pitch Node** - Menu for adding Pitch Waves with predefined waveforms
  * ![Pitch Attack icon](icon:alki-pitch-attack)  **Pitch Attack** - Add an attack Wave for controlling Pitch during the note-on phase of an Instrument
  * ![Pitch Release icon](icon:alki-pitch-release)  **Pitch Release** - Add a release Wave for controlling Pitch during the note-off phase of an Instrument
  * ![Pitch Modulation icon](icon:alki-pitch-mod)  **Pitch Modulation** - Add a modulation (oscillator) Wave for controlling Pitch
* ![New Balance Node icon](icon:alki-balance-mod)  **New Balance Node** - Menu for adding Balance Waves with predefined waveforms for controlling left/right stereo balance
  * ![Balance Attack icon](icon:alki-balance-attack)  **Balance Attack** - Add an attack Wave for conrolling stereo Balance during the note-on phase of an Instrument
  * ![Balance Release icon](icon:alki-balance-release)  **Balance Release** - Add a release Wave for controlling stereo Balance during the note-off phase of an Instrument
  * ![Balance Modulation icon](icon:alki-balance-mod)  **Balance Modulation** - Add a modulation (oscillator) Wave for controlling stereo Balance
* ![New Morph Node icon](icon:alki-morph-mod)  **New Morph Node** - Menu for adding Morph Waves with predefined waveforms for controlling a Wave Morph effect
  * ![Morph Attack icon](icon:alki-morph-attack)  **Morph Attack** - Add an attack Wave for controlling the Morph effect during the note-on phase of an Instrument
  * ![Morph Release icon](icon:alki-morph-release)  **Morph Release** - Add a release Wave for controlling the Morph effect during the note-off phase of an Instrument
  * ![Morph Modulation icon](icon:alki-morph-mod)  **Morph Modulation** - Add a modulation (oscillator) Wave for controlling the Morph effect
* ![New Control icon](icon:alki-ctrl-vol)  **New Control** - Menu for adding controls, which are used for real-time control of Instrument attributes
  * ![Volume Control icon](icon:alki-ctrl-vol)  **Volume Control** - Add a Volume control
  * ![Pitch Control icon](icon:alki-ctrl-pitch)  **Pitch Control** - Add a Pitch control
  * ![Balance Control icon](icon:alki-ctrl-balance)  **Balance Control** - Add a stereo Balance control
  * ![Morph Control icon](icon:alki-ctrl-morph)  **Morph Control** - Add a Morph effect control
  * ![Time Control icon](icon:alki-ctrl-time)  **Time Control** - Add a Time shift control, for modifying the wall clock Time (affects oscillators and envelope Waves)
* ![New Filter icon](icon:insert-object)  **New Filter** - Menu for adding Filter effect nodes
  * ![RBJ Bandpass icon](icon:insert-object)  **RBJ Bandpass** - Add a Band-pass filter node
  * ![RBJ Lowpass icon](icon:insert-object)  **RBJ Lowpass** - Add a Low-pass filter node
  * ![RBJ Highpass icon](icon:insert-object)  **RBJ Highpass** - Add a High-pass filter node
* ![New Generator icon](icon:insert-object)  **New Generator** - Menu for Signal generator nodes
  * ![White Noise icon](icon:insert-object)  **White Noise** - Add a "White Noise" node, Pseudo-Random Number Generator (PRNG)
* ![Rename icon](icon:insert-text)  **Rename** (F2) - Rename a node in the Instrument Browser (double click will also activate a rename operation)
* ![Move Up icon](icon:go-up)  **Move Up** (ALT-UP) - Move the order of a Node up by one position
* ![Move Down icon](icon:go-down)  **Move Down** (ALT-DOWN) - Move the order of a Node down by one position
* ![Copy icon](icon:edit-copy)  **Copy** (CTRL-C) - Copy one or more Node items to the clipboard
* ![Paste icon](icon:edit-paste)  **Paste** (CTRL-V) - Paste one or more Nodes from the clipboard before the currently selected Node
* ![Paste Within icon](icon:insert-object)  **Paste Within** (CTRL-SHIFT-V) - Paste one or more Nodes from the clipboard as children (inside) of the currently selected Node
* ![Cut icon](icon:edit-cut)  **Cut** (CTRL-X) - Cut (remove) one or more selected Nodes to the clipboard, which can be added back with a copy operation
* ![Delete icon](icon:list-remove)  **Delete** (CTRL-D) - Delete one or more selected Nodes
* ![Mute icon](icon:audio-volume-muted)  **Mute** (CTRL-M) - Mute one or more selected Nodes, which will disable their effect on the synthesis output

