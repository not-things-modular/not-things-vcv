# TIMESEQ SCRIPT SAMPLES

*Samples of the JSON script for the not-things [TimeSeq](../TIMESEQ.md) module.*

This page provides a collection of script examples/tutorials and VCV Rack patches that use them to illustrate different aspects of the TimeSeq script and some ways in which they can be used.

Each of the samples will contain a description of how the script is constructed, and include a link to the full JSON script and a VCV Rack patch that uses that script in a TimeSeq module. When loading one of the VCV Rack patches, remember to select your audio device on the Audio Output module.

While each of the samples demonstrates separate pieces of functionality, the fact that a TimeSeq script can contain multiple *timeline*s with multiple *lane*s and has 8 polyphonic input and and output ports means that several types of functionality can be combined into one script as needed, and be executed by a single TimeSeq module instance.

## Table of Contents

* [Building a Chord and Note Sequence](#building-a-chord-and-note-sequence)
* [Creating Clock Signals](#creating-clock-signals)
* [Sample and Hold](#sample-and-hold)
* [A Bernoulli Gate](#a-bernoulli-gate)
* [Oscillator](#oscillator)

## Building a Chord and Note Sequence

The [note sequence](samples/NOTE-SEQ.md) scripts start with a simple note sequence and gradually builds this into a full chord sequence with eight and triplet notes and some chance control. This script demonstrates:

* Building a simple simple sequence
* Using multiple lanes with sequences running in parallel
* Using segment blocks
* Using eight- and triplet timings
* Using variables and random values
* Starting and stopping lanes through triggers
* Using input ports as CV sources
* Using to component-pool to reduce script size using re-usable components
* Using a tuning to quantize values to a scale

## Creating Clock Signals

The [clock](samples/CLOCK.md) script demonstrates the usage of TimeSeq as a clock source. This script demonstrates:

* Using multiple timelines with different time-scales
* Using gate actions as clock signal sources
* Using beats and bars to specify clock durations
* Controlling the length that a clock (i.e. gate) signal remains high

## Sample and Hold

The [sample and hold](samples/SAMPLE-AND-HOLD.md) script demonstrates how S&H functionality can be made in TimeSeq, starting from a basic multi-port Sample and Hold, and extending it with internal voltages, calculations and timings. This script demonstrates:

* Defining reusable input and output definitions
* Using input triggers and input voltages
* Using internal random voltages
* Using variables
* Performing calculations
* Combining multiple voltage sources

## A Bernoulli Gate

The [bernoulli gate](samples/BERNOULLI.md) sample demonstrates how TimeSeq can be used to reproduce the functionality of the Bernoulli Gate (i.e. Mutable Instruments Branches) module. It is not intended to replace the Bernoulli Gate, but instead a demonstration of how the logic processing of TimeSeq can be used for more then just sequence generation.

The sample will start with a simplified version of the Bernoulli Gate, and then extend this functionality to the full version.

## Oscillator

The [oscillator](samples/OSCILLATOR.md) sample will set up a script that causes TimeSeq to generate several types of waveforms (square, sawtooth, triangle and Sine-like). Just like the [bernoulli gate](#a-bernoulli-gate) sample, this sample is not intended to demonstrate how TimeSeq can replace an Oscillator, but instead is intended to show what can be done. In this case, it also provides an opportunity to show different aspects of the *glide* actions.

The sample starts with creating versions of the waveforms that all produce a C3 note, but the last section of the sample page will update this using segments with a variable duration, causing it to become tunable using a 1V/Oct input signal. 

As a bonus, this sample also throws in a random voltage generator (i.e. noise) output.
