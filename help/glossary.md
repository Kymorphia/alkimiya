# Glossary
* **Node** - All items in the instrument tree view (Instrument Files, Instruments, Waves)
* **Property** - Parameter of a Node which can have an assigned value
* **Instrument File** - A Node containing a collection of instruments stored in a file (.kvi or .kvz for uncompressed or compressed formats)
* **Instrument** - An Instrument is a Node described by a tree of Waves and is what is assigned to a channel and played
* **Wave** - The fundamental building block Node used in Instruments. Described by a Curve, several Properties, Morph Curves, and input and output Attribute Ports.
* **Curve** - A cubic Bézier curve
* **Speaker** - An individual audio output. Stereo has 2 Speakers for example.
* **Mix Node** - A Node used in Instruments for mixing signals to Speaker outputs. The Instrument is itself a Mix Node.
* **X Coordinate** - A position value in a Curve along the horizontal axis, from 0.0 to 1.0.
* **Y Coordinate** - A position value in a Curve along the vertical axis, from -1.0 to 1.0 (bipolar) or 0.0 to 1.0 (unipolar).
* **Point** - A point on a Curve described by an X and Y Coordinate (or H and V Coordinate for Morph Curves).
* **Target Point** - Points on a Curve which lie on the curve.
* **Handle Point** - Points used to control the slope of the curve between Target Points. Each Target Point has 2 handles (1 for End Points).
* **End Point** - The first and last Target Points in a Curve.
* **Signal** - A stream of numeric floating point data which passes between synthesis Nodes.
* **Attribute** - An identifier of Signal content. Used for controlling how Waves and other synthesis nodes are interconnected. Examples: Volume, Pitch, Balance, and Effect.
* **Port** - An input or output Signal port of a synthesis Node (such as a Wave). Identified by an Attribute, which determines what other Node it gets connected to.
* **Waveform** - The Curve output by a Wave node
* **Morphing** - The process of transforming (or animating) a Curve between one or more additional Curves
* **Morph Curve** - A Curve used to morph the X or Y Coordinate of a Point in a Waveform
* **H Coordinate** - The horizontal (X) Coordinate of a Morph Curve. Identified separately since Morph Curves can be used to morph either the X or Y coordinates of Points in a Waveform.
* **V Coordinate** - The vertical or value (Y) Coordinate of a Morph Curve. Identified separately since Morph Curves can be used to morph either the X or Y coordinates of Points in a Waveform.
* **Morph Key** - The H Coordinate of a Point on a Morph Curve

