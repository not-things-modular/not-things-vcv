# TIMESEQ
*Part of the set of [not-things VCV Rack](../README.md) modules.*

![TimeSeq](./timeseq.png)

TimeSeq is a scripted timeline processor. It uses an internal script engine that lets you define what voltages should appear on the eight polyphonic outputs over time. These can be based on values from the eight polyphonic inputs, combined with logic and calculations defined in a JSON script.

With the JSON script, you can create a wide range of behaviour, including:
* Simple or complex chord and note sequences  
* Clock or trigger signal generation  
* Modulation sources via CV signal generation  
* ...and more

To support this functionality, the scripting format includes features such as:
* Multiple parallel timelines and lanes (each can run once, repeat or loop)
* Timing units in samples, milliseconds, BPM (beats and bars), or Hz
* Voltage values defined from:
    * Fixed values (either by voltage or by note name)
    * Current input voltages (by port and channel)
    * Current output voltages (by port and channel)
    * Random voltage generation (with upper/lower limits)
    * Simple calculations combining values
    * Quantization of values to nearest note voltage
* Read/write voltages as named variables
* Control over polyphony count per output
* Output voltage control (per port and channel)
* Gate generation (e.g. for clocks)
* Conditional execution using `if` statements
* Trigger input handling
* Internal trigger generation
* Start, stop or restart script sections based on input and internal triggers

Full details on the scripting language can be found in the [TimeSeq JSON script](TIMESEQ-SCRIPT.md) documentation, with the [TimeSeq JSON Script Format](TIMESEQ-SCRIPT-FORMAT.md) providing the detailed script syntax. For details on the module's UI and how it reflects script state, see the [TimeSeq UI](TIMESEQ-UI.md) page.
