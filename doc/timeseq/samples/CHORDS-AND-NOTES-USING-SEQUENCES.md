# TimeSeq CHORD PROGRESSION AND NOTE MELODY USING SEQUENCES sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

With the introduction of [sequences](../TIMESEQ-SCRIPT-JSON.md#sequence) in script version v1.2.0 (TimeSeq v2.0.6), it has become easier to create scripts that perform repeated actions using a sequence of values/notes (e.g. chord progressions, note sequences, CV value sequences). To demonstrate some of the features of *sequences*, this sample page will recreate the original [Chord Progression and Note Melody](./CHORDS-AND-NOTES.md) with sequences. We'll follow the same flow to construct a script that will generate the same output, but use *sequences* where applicable.

## Table of Contents

* [A Basic Note Sequence](#a-basic-note-sequence)
* [Adding Arpeggiated Chords](#adding-arpeggiated-chords)
* [Adding Gates for Envelopes](#adding-gates-for-envelopes)
* [Adding in Some Chance](#adding-in-some-chance)
* [Modulating the Chance](#modulating-the-chance)
* [Generating a Random Melody](#generating-a-random-melody)

## A Basic Note Sequence

The first thing we'll add to the script is a sequence that contains our chord notes. Sequences have to be created in the `sequences` property at the root of the [script](../TIMESEQ-SCRIPT-JSON.md#script):

```json
{
    "sequences": [
        {
            "id": "chord-root-notes",
            "values": [ "C3", "F3", "D3", "G3" ]
        }
    ]
}
```

Since this sequence contains note values, we're adding them using [shorthand value](../TIMESEQ-SCRIPT-JSON.md#shorthand-value-notation) instead of writing them as [full values](../TIMESEQ-SCRIPT-JSON.md#value) in the `values` property.

We can now use this sequence by referencing it by its `id`. For this, we'll need a [timeline](../TIMESEQ-SCRIPT-JSON.md#timeline) with a single [lane](../TIMESEQ-SCRIPT-JSON.md#lane). The *lane* is set to `auto-start` and `loop` so that the sequence starts when the script is loaded and keeps repeating.
There will be a single [segment](../TIMESEQ-SCRIPT-JSON.md#segment) in the *lane* that lasts 2000 milliseconds (i.e. 2 seconds) and sets the voltage of [output](../TIMESEQ-SCRIPT-JSON.md#output) port 1 to a value that it retrieves from the sequence using a [sequence-value](../TIMESEQ-SCRIPT-JSON.md#sequence-value):

```json
{
    "duration": { "millis": 2000 },
    "actions": [
        {
            "timing": "start",
            "set-value": {
                "output": { "index": 1 },
                "value": { "sequence": { "id": "chord-root-notes" } }
            }
        }
    ]
}
```

Every 2 seconds, this segment will execute the action, retrieve the next value from the sequence and assign it to the output. There is an opportunity for shortening this notation a bit however:

* For *input*s, *output*s, fixed *value*s and *sequence value*s the less verbose shorthand notations can be used. We'll use them throughout the rest of this sample (see the [input](../TIMESEQ-SCRIPT-JSON.md#shorthand-input-notation), [output](../TIMESEQ-SCRIPT-JSON.md#shorthand-output-notation), [value](../TIMESEQ-SCRIPT-JSON.md#shorthand-value-notation) and [sequence value](../TIMESEQ-SCRIPT-JSON.md#shorthand-sequence-notation) shorthand sections of the TimeSeq Script JSON specification for more details)
* If an *action* doesn't have a `timing` property, it will default to `start`. So it is not needed to explicitly specify the `timing` property for any *action*s that should trigger at the start of a *segment*. In the script, we'll only specify the `timing` property if it is different from the default `start` timing.

The updated *segment* notation will thus become:

```json
{
    "duration": { "millis": 2000 },
    "actions": [
        {
            "set-value": {
                "output": 1,
                "value": { "sequence": "chord-root-notes" }
            }
        }
    ]
}
```

### Full Script and VCV Rack Patch

The full script can be found in [chords-and-notes-seq-root-notes.json](chords-and-notes-seq/chords-and-notes-seq-root-notes.json), and the [chords-and-notes-seq-root-notes.vcv](chords-and-notes-seq/chords-and-notes-seq-root-notes.vcv) patch will show it in action by playing the note sequence through a VCO after showing the current note on the Hot Tuna tuner module.

## Adding Arpeggiated Chords

In the second iteration of this script, we'll first move away from *millis* timing, and instead use the musical `bpm` (beats per minute) and `bpb` (beats per bar) to express timing by adding a [time-scale](../TIMESEQ-SCRIPT-JSON.md#time-scale) to the *timeline*:

```json
{
    "time-scale": {
        "bpm": 120,
        "bpb": 4
    }
}
```

The note sequence *segment* can now express its duration in `beats` and `bars`. We'll change it to two bars, which results in the a total of 4 seconds each time the segment is executed (120bpm = 0.5 secs per beat; 0.5s \* 4 beats per bar \* 2 bars = 4s):

```json
{
    "duration": { "bars": 2, "beats": 0 },
    "actions": [
        {
            "set-value": {
                "output": 1,
                "value": { "sequence": "chord-root-notes" }
            }
        }
    ]
}
```

For the arpeggiated notes, we'll create four sequences: one for each of the chords, containing the four arpeggiated notes of that chord:

```json
{
    "sequences": [
        ...
        {
            "id": "c-arp-notes",
            "values": [ "C4", "E4", "G4", "C5" ]
        },
        {
            "id": "f-arp-notes",
            "values": [ "C4", "F4", "A4", "C5" ]
        },
        {
            "id": "d-arp-notes",
            "values": [ "D4", "F4", "A4", "D5" ]
        },
        {
            "id": "g-arp-notes",
            "values": [ "D4", "G4", "B4", "D5" ]
        }
    ]
}
```

We'll use different [segment-block](../TIMESEQ-SCRIPT-JSON.md#segment-block)s for each of the chords that is to be played arpeggiated. Each of these *segment-block*s will contain the information needed to play that one chord, sending it to the second *output* port. Since we're using *sequences*, the *segment-block* will need to contain only a single `set-value` action that will set the value of the 2nd output to the value of the relevant sequence. Each time this action is executed, the [sequence-value](../TIMESEQ-SCRIPT-JSON.md#sequence-value) will also automatically advance the position of the sequence one step forward. Looping this segment will thus cause the arpeggiated note sequence to be played in order. Since we previously set up each chord to lasts for 2 bars, with 4 beats in each bar, we'll have to repeat the `segment` in each of the `segment-block` 16 times to reach the same 2-bar (or 8-beat) length. This results in a `c-arp` segment-block that plays the C chord arpeggiation that looks as follows:

```json
    "component-pool": {
        "segment-blocks" : [
            {
                "id": "c-arp",
                "x-description": "The segment-block plays the arpeggiated C chord.",
                "repeat": 16,
                "segments": [
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "set-value": { "output": 2, "value": { "sequence": "c-arp-notes" } }
                            }
                        ]
                    }
                ]
            }
        ]
    }
```

Similar *segment-block*s will be defined for the F, G an D chords (using the other `sequence`s we've created), and these segment blocks can then be added to a new *lane* in the original *timeline* so that they play alongside the original chord root note sequence:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        { "segment-block": "c-arp" },
        { "segment-block": "f-arp" },
        { "segment-block": "d-arp" },
        { "segment-block": "g-arp" }
    ]
}
```

### Full Script and VCV Rack Patch

The full script can be found in [chords-and-notes-seq-arp.json](chords-and-notes-seq/chords-and-notes-seq-arp.json), and the [chords-and-notes-seq-arp.vcv](chords-and-notes-seq/chords-and-notes-seq-arp.vcv) patch contains a second VCO to play the arpeggiated chord notes.

## Adding Gates for Envelopes

The next step in this patch is to separate the arpeggiated chord notes using an envelope. For this, we'll need a gate signal (or a trigger when using an AD envelope instead of an ADSR envelope). This can be achieved by adding a [gate](../TIMESEQ-SCRIPT-JSON.md#gate-actions) *action* to each of the arpeggiated note *segment*s. Since this *action* will be used in multiple places, we can add it once to the *component-pool*, from where we can re-use it:

```json
{
    "component-pool": {
        "actions": [
            {
                "id": "arp-gate-action",
                "timing": "gate",
                "output": 3
            }
        ]
    }
}
```

This will cause a *high* (10V) signal to be present on *output* port 3 for the first half of the segment, and a *low* (0V) signal for the rest. While it is possible to move the change from *high* to *low* to a different position in the *segment* using the `gate-high-ratio` property, we'll leave it on the halfway point of the *segment* for this script.

This gate can now be used by [reference](../TIMESEQ-SCRIPT.md#referencing) on each of the arpeggiated chord notes:

```json
{
    "duration": { "beats": 0.5 },
    "actions": [
        {
            "set-value": { "output": 2, "value": { "sequence": "c-arp-notes" } }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
},
```

### Full Script and VCV Rack Patch

A script version with gate actions added to all the arpeggiated chord notes can be found int [chords-and-notes-seq-arp-gate.json](chords-and-notes-seq/chords-and-notes-seq-arp-gate.json), and the [chords-and-notes-seq-arp-gate.vcv](chords-and-notes-seq/chords-and-notes-seq-arp-gate.vcv) patch uses this gate for applying a volume envelope on those notes.

## Adding in Some Chance

To bring some variation in the script output, we'll add some chance to the arpeggiated chord notes: on each *action* that causes a note to play, add a 1-in-4 chance that the note will not be played. While the non-sequence-based version of this tutorial was restructured a bit at this point to facilitate the introduction of this chance, the usage of sequences in this version has kept the script more compact, allowing the new functionality to be introduced more easily.

Adding chance to the execution of an *action* can be done using two steps: first generate a random value, and then make the action conditional based on that random value. To generate the random value that determines if an arpeggiated note *action* will be performed, following *action* is added to the `actions` list of the *component-pool*:

```json
{
    "id": "determine-chance",
    "set-variable": {
        "name": "play-note",
        "value": {
            "rand": {
                "lower": 0,
                "upper": 10
            }
        }
    }
}
```

This action will generate a random voltage between `0` and `10` and assign it to the `play-note` variable. In order to use this variable to make the note *action*s have a 1-in-4 chance of not playing, we'll need an [if](../TIMESEQ-SCRIPT-JSON.md#if). Since this *if* condition will be used multiple times, we'll add it to the `ifs` section of the *component-pool* from where it can be re-used:

```json
{
    "component-pool": {
        "ifs": [
            {
                "id": "should-play-note",
                "gt": [
                    { "variable": "play-note" },
                    2.5
                ]
            }
        ]
    }
}
```

This `should-play-note` conditional will evaluate to `true` if the `play-note` variable (which contains the random value between `0` and `10`) is above `2.5`. When used as *if* condition on an *action*, this will give it a 3-in-4 chance of executing. We can now add this new *action* and the *if* condition to the *segment*s that play the notes:

```json
{
    "duration": { "beats": 0.5 },
    "actions": [
        { "ref": "determine-chance" },
        {
            "if": { "ref": "should-play-note" },
            "set-value": { "output": 2, "value": { "sequence": "c-arp-notes" } }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
}
```

The actions are executed in the order they appear in the script, so first the referenced `determine-chance` action will determine the chance that the note will play, then the second action will only play the note if the referenced `should-play-note` condition evaluates to `true`, and finally the `arp-gate-action` will generate the gate that drives the note volume envelope. We'll also have to make this gate *action* conditional based on the same chance, so that the gate doesn't trigger if no note was played. That action (which already existed in the *component-pool*) now becomes:

```json
{
    "id": "arp-gate-action",
    "timing": "gate",
    "if": { "ref": "should-play-note" },
    "output": 3
},
```

### Full Script and VCV Rack Patch

The full updated script for this step can be found in [chords-and-notes-seq-arp-chance.json](chords-and-notes-seq/chords-and-notes-seq-arp-chance.json), and the [chords-and-notes-seq-arp-chance.vcv](chords-and-notes-seq/chords-and-notes-seq-arp-chance.vcv) is the same patch as before, but now with the new version of the script loaded.

## Modulating the Chance

To bring some more variation in the output, we'll allow the chance that was introduced in the previous step to be controlled by an external voltage. Instead of always having a 75% chance that a note will play, an external voltage should be able to move that chance around, allowing external modulation sources to influence how often the notes are triggered. We'll set it up so that a 0V to 10V signal moves the probability between 25% and 75%. All that is needed for this is to update the `should-play-note` condition to:

```json
"ifs": [
    {
        "id": "should-play-note",
        "gt": [
            { "variable": "play-note" },
            {
                "input": 1,
                "calc": [
                    { "div": 2 },
                    { "add": 2.5 }
                ]
            }
        ]
    }
],
```

Instead of checking that the previously generated `play-note` random value is above 2.5V (=75% chance), this updated condition will (using an [input](../TIMESEQ-SCRIPT-JSON.md#input) and [calc](../TIMESEQ-SCRIPT-JSON.md#calc)ulations):

* Capture the voltage on input port 1 (-> a value between 0V and 10V)
* Divide this by `2` (-> a value between 0V and 5V)
* Add `2.5` to this (-> a value between 2.5V and 7.5V)

When checking that value against the randomly generated `play-note` variable (which will be between 0V and 10V), we get a 25% to 75% chance that a note will trigger, depending on the input voltage.

### Full Script and VCV Rack Patch

The resulting script for this step can be found in [chords-and-notes-seq-arp-chance-mod.json](chords-and-notes-seq/chords-and-notes-seq-arp-chance-mod.json), with the [chords-and-notes-seq-arp-chance-mod.vcv](chords-and-notes-seq/chords-and-notes-seq-arp-chance.vcv) patch using an LFO to generate the input voltage. A scope will also show the LFO output and the resulting gates coming from TimeSeq: as the LFO value is low, there is a higher chance of a note playing and when the LFO is high, there is a lower chance.

## Generating a Random Melody

On top of the current chord progression, we're going to generate a random melody. In the [non-sequence-based version](CHORDS-AND-NOTES.md#generating-a-quantized-random-melody) of this tutorial, this was done by generating random values and quantizing them to a [tuning](../TIMESEQ-SCRIPT-JSON.md#tuning). In this tutorial however, the random melody generation can be done while demonstrating a different feature of sequences: controlling the position within the sequence. When used without any additional parameters, a [sequence value](../TIMESEQ-SCRIPT-JSON.md#sequence-value) will cause the position of the sequence to be moved one position forward (with wrapping) after the value is retrieved. It is however possible to control this movement more directly using the `move-before` and `move-after` properties, allowing the position to be changed before and/or after a value is retrieved from the sequence. Possible movement directions are `forward`, `backward`, `none` (to not change the position) or `random` (to jump to a random entry in the sequence). If needed, it's even possible to move the position in the sequence directly using the dedicated [move-sequence](../TIMESEQ-SCRIPT-JSON.md#move-sequence) action. For the random melody here, we'll define a sequence that contains the notes of the c-major pentatonic scale, and use the `move-after` property of the *sequence value* to "jump around" at random in this sequence instead of always moving to the next item in the list of `values`.

First we need to define the sequence that contains the notes that can be played in the random sequence:

```json
"sequences": [
    ...
    {
        "id": "melody-notes",
        "values": [ "C5", "D5", "E5", "G5", "A5", "C6" ]
    }
],
```

A new lane can then use this sequence to play a random melody:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "beats": 1 },
            "actions": [
                {
                    "set-value": {
                        "output": 4,
                        "value": {
                            "sequence": {
                                "id": "melody-notes",
                                "move-after": "random"
                            }
                        }
                    }
                },
                {
                    "timing": "gate",
                    "output": 5
                }
            ]
        }
    ]
}
```

This *lane* repeats one segment that updates the voltage on the 4th *output* of TimeSeq using a value from the `melody-notes` sequence, after which the position within that sequence moved at random. A second action in that segment also generates a gate signal on *output* 5 that can be used to generate a volume envelope for the note.

### Full Script and VCV Rack Patch

The resulting script for this step can be found in [chords-and-notes-seq-random-melody.json](chords-and-notes-seq/chords-and-notes-seq-random-melody.json), with the [chords-and-notes-seq-random-melody.vcv](chords-and-notes-seq/chords-and-notes-seq-random-melody.vcv) sending the newly generated note sequence through an additional VCO, with the new gate signal driving another ADSR envelope generator.
