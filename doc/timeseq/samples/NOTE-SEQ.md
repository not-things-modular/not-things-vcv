# TIMESEQ NOTE SEQUENCE SAMPLE SCRIPT

*A Sample script for the not-things [TimeSeq](../TIMESEQ.md) module.*

One of the basic functionalities of TimeSeq is to create repeating chord and note sequences. On this page, we'll start with a simple note sequence and gradually transform this into a sequence to a script that can drive a bass voice, chord voice and arpeggiated notes voice.

## Table of Contents

* [A Basic Note Sequence](#a-basic-note-sequence)
* [Adding Arpeggiated Chords](#adding-arpeggiated-chords)
* [Adding Gates for Envelopes](#adding-gates-for-envelopes)
* [Using Triplets](#using-triplets)

## A Basic Note Sequence

To create a basic four note sequence, we'll need a [timeline](../TIMESEQ-SCRIPT-JSON.md#timeline) with a single [lane](../TIMESEQ-SCRIPT-JSON.md#lane). The *lane* is set to `auto-start` and `loop` so that the sequence starts when the script is started, and keeps repeating.
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

The other three *segment*s will look identical, except that the `note` will become `F3`, `G3` and `D3`.

The full script can be found [here](note-seq/note-seq-root-notes.json), and the [note-seq-root-notes.vcv](note-seq/note-seq-root-notes.vcv) patch will show it in action by playing the note sequence through a VCO after showing the current note on the Hot Tuna tuner.

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

The note sequence *segment*s can now express its duration in `beats` and `bars`, so we'll change them to two bars, which results in the same 2 second duration as before:

```json
{
    "duration": { "bars": 2, "beats": 0 },
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

For the arpeggiated notes, we'll use [segment-block](../TIMESEQ-SCRIPT-JSON.md#segment-block)s to keep the script manageable. Each *segment-block* will contain the four notes that make up the arpeggiated chord which will be sent to the second *output* port. Since we'll let each note chord play for half a beat, each *segment-block* is repeated four times so we end up with the same two bars duration as in the original note sequence (0.5 beats \* 4 notes \* 4 repeats = 8 beats = 2 bars). The first *segment-block* for the arpeggiated C chord will look as follows inside the [component-pool](../TIMESEQ-SCRIPT-JSON.md#component-pool):

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
                                "timing": "start",
                                "set-value": { "output": { "index": 2 }, "value": { "note": "C4" } }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "timing": "start",
                                "set-value": { "output": { "index": 2 }, "value": { "note": "E4" } }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "timing": "start",
                                "set-value": { "output": { "index": 2 }, "value": { "note": "G4" } }
                            }
                        ]
                    },
                    {
                        "duration": { "beats": 0.5 },
                        "actions": [
                            {
                                "timing": "start",
                                "set-value": { "output": { "index": 2 }, "value": { "note": "C5" } }
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

The full script can be found [here](note-seq/note-seq-arp.json), and the [note-seq-arp.vcv](note-seq/note-seq-arp.vcv) patch contains a second VCO to play the arpeggiated chord notes.

## Adding Gates for Envelopes

The next step in this patch is to separate the arpeggiated chord notes using an envelope. For this, we'll need a gate signal (or a trigger when using an AD envelope instead of an ADSR envelope). This can be achieved by adding a [gage](../TIMESEQ-SCRIPT-JSON.md#gate-actions) action to each of the arpeggiated note segments. Since this action will be used in multiple places, it will be added once to the *component-pool*:

```json
{
    "component-pool": {
        "actions": [
            {
                "id": "arp-gate-action",
                "timing": "gate",
                "output": { "index": 3 }
            }
        ]
    }
}
```

This will cause a *high* (10V) signal to be present on *output* port 3 for the first half of the segment, and a *low* (0V) signal for the rest. While it is possible to move the change from *high* to *low* to a different position in the *segment* using the `gate-high-ratio` property, we'll leave it on the halfway point of the *segment* for this script.

This gate can now be used by reference on each of the arpeggiated chord notes:

```json
{
    "duration": { "beats": 0.5 },
    "actions": [
        {
            "timing": "start",
            "set-value": { "output": { "index": 2 }, "value": { "note": "C4" } }
        },
        {
            "ref": "arp-gate-action"
        }
    ]
},
```

A script version with gate actions added to all the arpeggiated chord notes can be found [here](note-seq/note-seq-arp-gate.json), and the [note-seq-arp-gate.vcv](note-seq/note-seq-arp-gate.vcv) patch uses this gate for applying a volume envelope on those notes.

## Using Triplets

When there are multiple looping *lanes* in a *timeline* that are expected to stay in sync, unless the `loop-lock` is activated on the *timeline*, it's important to make sure that all the *lane*s have the same duration. This can be demonstrated by making the *segment-block* play half-note triplets instead of eight notes. A tripplet *segment* can be written as:

```json
{
    "duration": { "beats": 0.666 },
    "actions": [
        {
            "timing": "start",
            "set-value": { "output": { "index": 2 }, "value": { "note": "D4" } }
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
                    "timing": "start",
                    "set-value": { "output": { "index": 2 }, "value": { "note": "D4" } }
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
                    "timing": "start",
                    "set-value": { "output": { "index": 2 }, "value": { "note": "G4" } }
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
                    "timing": "start",
                    "set-value": { "output": { "index": 2 }, "value": { "note": "B4" } }
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

The updated script can be found [here](note-seq/note-seq-tripplets.json), with the [note-seq-arp-tripplets.vcv](note-seq/note-seq-arp-tripplets.vcv) patch playing it.
