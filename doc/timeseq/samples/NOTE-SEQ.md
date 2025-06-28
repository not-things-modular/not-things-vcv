# TimeSeq NOTE SEQUENCE sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

One of the basic functionalities of TimeSeq is to create repeating chord and note sequences. On this page, we'll start with a simple note sequence and gradually transform this into a sequence that can drive a bass voice, and an arpeggiated notes voice.

## Table of Contents

* [A Basic Note Sequence](#a-basic-note-sequence)
* [Adding Arpeggiated Chords](#adding-arpeggiated-chords)
* [Adding Gates for Envelopes](#adding-gates-for-envelopes)
* [Using Triplets](#using-triplets)
* [Restructuring the Script](#restructuring-the-script)
* [Adding in Some Chance](#adding-in-some-chance)
* [Modulating the Chance](#modulating-the-chance)
* [Generating a Quantized Random Melody](#generating-a-quantized-random-melody)

## A Basic Note Sequence

To create a basic four note sequence, we'll need a [timeline](../TIMESEQ-SCRIPT-JSON.md#timeline) with a single [lane](../TIMESEQ-SCRIPT-JSON.md#lane). The *lane* is set to `auto-start` and `loop` so that the sequence starts when the script is loaded and keeps repeating.
There will be four [segment](../TIMESEQ-SCRIPT-JSON.md#segment)s in the *lane*, one for each note. Each segment will last 2000 milliseconds (or 2 seconds) and will set the voltage of [output](../TIMESEQ-SCRIPT-JSON.md#output) port 1 to the 1V/Oct value of the current note. For the first *segment*, this will be a `C3`, and that full *segment* will look as follows:

```json
{
    "duration": { "millis": 2000 },
    "actions": [
        {
            "timing": "start",
            "set-value": {
                "output": { "index": 1 },
                "value": { "note": "C3" }
            }
        }
    ]
}
```

There is an opportunity for shortening this a bit however:

* For *input*s, *output*s and fixed *value*s the less verbose shorthand notations can be used. We'll use them throughout the rest of this sample (see the [input](../TIMESEQ-SCRIPT-JSON.md#shorthand-input-notation), [output](../TIMESEQ-SCRIPT-JSON.md#shorthand-output-notation) and [value](../TIMESEQ-SCRIPT-JSON.md#shorthand-value-notation) shorthand sections of the TimeSeq Script JSON specification for more details)
* If an *action* doesn't have a `timing` property, it will default to `start`. So it is not needed to specify the `timing` property for any *action*s that should trigger at the start of a *segment*. In the script, we'll only specify the `timing` property if it is different from the default `start` timing.

The updated *segment* notation will thus become:

```json
{
    "duration": { "millis": 2000 },
    "actions": [
        {
            "set-value": {
                "output": 1,
                "value": "C3"
            }
        }
    ]
}
```

The other three *segment*s will look identical, except that their `value`s will be notes `F3`, `G3` and `D3`.

### Full Script and VCV Rack Patch

The full script can be found in [note-seq-root-notes.json](note-seq/note-seq-root-notes.json), and the [note-seq-root-notes.vcv](note-seq/note-seq-root-notes.vcv) patch will show it in action by playing the note sequence through a VCO after showing the current note on the Hot Tuna tuner module.

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

The note sequence *segment*s can now express its duration in `beats` and `bars`. We'll change them to two bars, which results in the a total of 4 seconds for each (120bpm = 0.5 secs per beat; 0.5s \* 4 beats per bar \* 2 bars = 4s):

```json
{
    "duration": { "bars": 2, "beats": 0 },
    "actions": [
        {
            "set-value": {
                "output": 1,
                "value": "C3"
            }
        }
    ]
}
```

For the arpeggiated notes, we'll use [segment-block](../TIMESEQ-SCRIPT-JSON.md#segment-block)s to keep the script manageable. Each *segment-block* will contain the four notes that make up the arpeggiated chord which will be sent to the second *output* port. Each note chord play for half a beat, and each *segment-block* is repeated four times so we end up with the same two bars duration as in the original note sequence (0.5 beats \* 4 notes \* 4 repeats = 8 beats = 2 bars). The first *segment-block* for the arpeggiated C chord will look as follows inside the [component-pool](../TIMESEQ-SCRIPT-JSON.md#component-pool):

```json
    "component-pool": {
        "segment-blocks" : [
            {
                "id": "c-arp",
                "repeat": 4,
                "segments": [
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "set-value": { "output": 2, "value": "C4" }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "set-value": { "output": 2, "value": "E4" }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "set-value": { "output": 2, "value": "G4" }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "set-value": { "output": 2, "value": "C5" }
                            }
                        ]
                    }
                ]
            }
        ]
    }
```

Similar *segment-block*s will be defined for the F, G an D chords, and these segment blocks can then be added to a new *lane* in the original *timeline* so that they play alongside the original note sequence:

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

The full script can be found in [note-seq-arp.json](note-seq/note-seq-arp.json), and the [note-seq-arp.vcv](note-seq/note-seq-arp.vcv) patch contains a second VCO to play the arpeggiated chord notes.

## Adding Gates for Envelopes

The next step in this patch is to separate the arpeggiated chord notes using an envelope. For this, we'll need a gate signal (or a trigger when using an AD envelope instead of an ADSR envelope). This can be achieved by adding a [gate](../TIMESEQ-SCRIPT-JSON.md#gate-actions) *action* to each of the arpeggiated note *segment*s. Since this *action* will be used in multiple places, it will be added once to the *component-pool*:

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
            "set-value": { "output": 2, "value": "C4" }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
},
```

### Full Script and VCV Rack Patch

A script version with gate actions added to all the arpeggiated chord notes can be found int [note-seq-arp-gate.json](note-seq/note-seq-arp-gate.json), and the [note-seq-arp-gate.vcv](note-seq/note-seq-arp-gate.vcv) patch uses this gate for applying a volume envelope on those notes.

## Using Triplets

When there are multiple looping *lanes* in a *timeline* that are expected to stay in sync, unless the `loop-lock` is activated on the *timeline*, it's important to make sure that all the *lane*s have the same duration. This can be demonstrated by making the *segment-block* play half-note triplets instead of eight notes. A triplet *segment* can be written as:

```json
{
    "duration": { "beats": 0.666 },
    "actions": [
        {
            "set-value": { "output": 2, "value": "D4" }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
}
```

But while three of these triplets are expected to have the same duration as four eighteenth notes, using three `0.666` beats ends up lasting `1.998` beats instead of exactly `2` beats. When repeating this in a loop, this will cause a timing drift. In order to fix this, we can just add the missing duration to the length of the last triplet, resulting in following segment block:

```json
{
    "id": "g-arp",
    "repeat": 4,
    "segments": [
        {
            "duration": { "beats": 0.666 },
            "actions": [
                {
                    "set-value": { "output": 2, "value": "D4" }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        },
        {
            "duration": { "beats": 0.666 },
            "actions": [
                {
                    "set-value": { "output": 2, "value": "G4" }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        },
        {
            "duration": { "beats": 0.668 },
            "actions": [
                {
                    "set-value": { "output": 2, "value": "B4" }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        }
    ]
}
```

The last `0.668` *duration* makes the whole *segment* run in sync with the others again. Note how the gate *action*s of these triplet notes also automatically adjusts to the new *segment* length.

### Full Script and VCV Rack Patch

The updated script can be found in [note-seq-arp-triplets.json](note-seq/note-seq-arp-triplets.json), with the [note-seq-arp-triplets.vcv](note-seq/note-seq-arp-triplets.vcv) patch playing it.

## Restructuring the Script

Before adding more new logic to the script it's helpful to simplify the existing structure. Up until now, the arpeggiated note sequences were hardcoded: each chord had its own *segment-block* with fixed notes and gates. This duplication makes the script harder to maintain and extend. What we'll do instead is restructure the script to:

* Use a single reusable *segment-block* for the arpeggios and root note
* Parameterize the chord notes and root note using variables
* Assign values to these variables for each chord instance using *set-variable* actions

This change makes the script shorter, more readable, and allows features to be applied uniformly.

The new generic `arp` *segment-block* would look as follows:

```json
{
    "id": "arp",
    "repeat": 4,
    "segments": [
        {
            "duration": { "beats": 0.5 },
            "actions": [
                {
                    "set-value": { "output": 1, "value": { "variable": "root-note" } }
                },
                {
                    "set-value": { "output": 2, "value": { "variable": "chord-note-1" } }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        },
        {
            "duration": { "beats": 0.5 },
            "actions": [
                {
                    "set-value": { "output": 2, "value": { "variable": "chord-note-2" } }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        },
        {
            "duration": { "beats": 0.5 },
            "actions": [
                {
                    "set-value": { "output": 2, "value": { "variable": "chord-note-3" } }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        },
        {
            "duration": { "beats": 0.5 },
            "actions": [
                {
                    "set-value": {
                        "output": 2,
                        "value": {
                            "variable": "chord-note-1",
                            "calc": [
                                { "add": 1 }
                            ]
                        }
                    }
                },
                {
                    "ref": "arp-gate-action"
                }
            ]
        }
    ]
}
```

Instead of using fixed note values, this segment-block now uses

* The `root-note` variable to set the root note on output 1 in the first segment,
* The `chord-note-1` variable to set the first arpeggiated note on output 2 in the first segment,
* The `chord-note-2` variable to set the second arpeggiated note on output 2 in the second segment,
* The `chord-note-3` variable to set the third arpeggiated note on output 2 in the third segment,
* Re-uses the `chord-note-1` variable, but adds 1V to it to make it one octave higher in the fourth segment

Instead of using the the different *segment-block*s `c-arp`, `f-arp`, ... for each of the chords, we can now assign values to the different variables and call on the `arp` *segment-block* instead.

To set the variables, we'll leverage the fact that you can add *action*s to a *segment* that points to a *segment-block*. These actions will then be executed before (if they have a `start` `timing`) or after (if they have an `end` `timing`). We'll use the default `start` timing here to perform [set-variable](../TIMESEQ-SCRIPT-JSON.md#set-variable) *action*s:

```json
{
    "actions": [
        {
            "set-variable": {
                "name": "root-note",
                "value": "C3"
            }
        },
        {
            "set-variable": {
                "name": "chord-note-1",
                "value": "C4"
            }
        },
        {
            "set-variable": {
                "name": "chord-note-2",
                "value": "E4"
            }
        },
        {
            "set-variable": {
                "name": "chord-note-3",
                "value": "G4"
            }
        }
    ],
    "segment-block": "arp"
}
```

This first segment contains the C4 chord: it sets the `root-note` variable to a `C3`, the `chord-note-1` to a `C4`, the `chord-note-2` to an `E4`, the `chord-note-3` to a `G4` and points to the previously created `arp` *segment-block* that will use these variables.

By using less newlines in the JSON, this can also be written a bit more compact, as this second segment for the F chord shows:

```json
{
    "actions": [
        { "set-variable": { "name": "root-note", "value": "F3" } },
        { "set-variable": { "name": "chord-note-1", "value": "C4" } },
        { "set-variable": { "name": "chord-note-2", "value": "F4" } },
        { "set-variable": { "name": "chord-note-3", "value": "A4" } }
    ],
    "segment-block": "arp"
}
```

### Full Script and VCV Rack Patch

The full updated script can be found [here](note-seq/note-seq-arp-rework.json), and the [note-seq-arp-rework.vcv](note-seq/note-seq-arp-rework.vcv) patch has it loaded into TimeSeq.

## Adding in Some Chance

To bring some variation in the script output, we'll add some chance to the arpeggiated chord notes: on each *action* that causes a note to play, add a 1-in-4 chance that the note will not be played. This is why the script got reworked in the previous step: there are less places where this chance will have to be added.

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

This action will generate a random voltage between `0` and `10` and assign it to the `play-note` variable. In order to use this variable to make the note *action*s have a 1-in-4 chance of not playing, we'll need an [if](../TIMESEQ-SCRIPT-JSON.md#if). Since this *if* condition will be used multiple times, we'll add it to the `ifs` section of the *component-pool*:

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
            "set-value": { "output": 2, "value": { "variable": "chord-note-2" } }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
}
```

The actions are executed in the order they appear in the script, so first the referenced `determine-chance` action will determine the chance that the note will play, then the second action will only play the note if the referenced `should-play-note` condition evaluates to `true`, and then the `arp-gate-action` will generate the gate that drives the note volume envelope. We'll also have to make this gate *action* conditional based on the same chance, so that action (which already existed in the *component-pool*) now becomes:

```json
{
    "id": "arp-gate-action",
    "timing": "gate",
    "if": { "ref": "should-play-note" },
    "output": 3
},
```

Note that the first *action* of the `arp` *segment-block* is the one that sets the root note on output port 1. This action should not be made conditional, since the root note should always update.

### Full Script and VCV Rack Patch

The full updated script for this step can be found in [note-seq-arp-chance.json](note-seq/note-seq-arp-chance.json), and the [note-seq-arp-chance.vcv](note-seq/note-seq-arp-chance.vcv) is the same patch as before, but now with the new version of the script loaded.

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

The resulting script for this step can be found in [note-seq-arp-chance-mod.json](note-seq/note-seq-arp-chance-mod.json), with the [note-seq-arp-chance-mod.vcv](note-seq/note-seq-arp-chance.vcv) patch using an LFO to generate the input voltage. A scope will also show the LFO output and the resulting gates coming from TimeSeq: as the LFO value is low, there is a higher chance of a note playing and when the LFO is high, there is a lower chance.

## Generating a Quantized Random Melody

A final addition to this script will be to generate a random melody. In order to let this melody fit in with the rest of the patch, it can not just play any note. It will have to be quantized to a scale that fits with the chords. We'll use a C major pentatonic scale for this (which contains the c, d, e, g and a notes). In the *component-pool*, we create a tuning for this:

```json
"tunings": [
    {
        "id": "c-major-pentatonic",
        "notes": [ "c", "d", "e", "g", "a" ]
    }
],
```

A tuning can be used in the [calc](../TIMESEQ-SCRIPT-JSON.md#calc) section of a *value* to `quantize` the voltage of that value to the `notes` in the tuning:

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
                            "rand": {
                                "lower": 1,
                                "upper": 2
                            },
                            "calc": [
                                { "quantize": { "ref": "c-major-pentatonic" } }
                            ]
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

This *lane* repeats one segment that updates the voltage on the 4th *output* of TimeSeq to a value. This value is randomly generated between 1V and 2V, and then gets quantized by the *calc* to the `c-major-pentatonic` tuning. A second action in that segment also generates a gate signal on *output* 5 that can be used to generate a volume envelope for the note.

### Full Script and VCV Rack Patch

The resulting script for this step can be found in [note-seq-quantize-random.json](note-seq/note-seq-quantize-random.json), with the [note-seq-quantize-random.vcv](note-seq/note-seq-quantize-random.vcv) sending the newly generated note sequence through an additional VCO, with the new gate signal driving another ADSR envelope generator.
