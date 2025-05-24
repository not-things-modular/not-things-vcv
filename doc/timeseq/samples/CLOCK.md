# TimeSeq CLOCK sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

By combining the [time-scale](../TIMESEQ-SCRIPT-JSON.md#time-scale) of [timeline](../TIMESEQ-SCRIPT-JSON.md#timeline)s with one or more looping [lane](../TIMESEQ-SCRIPT-JSON.md#lane)s and the different time units of [segment](../TIMESEQ-SCRIPT-JSON.md#segment) [duration](../TIMESEQ-SCRIPT-JSON.md#duration)s a complex combination of clock signals can be set up in a TimeSeq script.

This page will give an example of simple script setup using a couple of *bpm*s and beat subdivisions. This sample will output the different clock signals using different channels of the same polyphonic output port. While this is not really needed here since there are only a couple of clock signals, it is likely that real-life scenarios will contain many more output signals next to the clock signals, so combining the clock singals into one output frees up the other outputs for other script outputs.

## Table of Contents

* [Set Up Polyphonic Output](#set-up-polyphonic-output)
* [A 120 BPM Timeline](#a-120-bpm-timeline)
  * [Clock Signal on Every Beat](#clock-signal-on-every-beat)
  * [Clock Signal with Partial Beat](#clock-signal-with-partial-beat)
  * [A Slow Clock Signal](#a-slow-clock-signal)
* [A 90 BPM Timeline](#a-90-bpm-timeline)

## Set Up Polyphonic Output

Since we want to send the clock outputs as polyphonic channels on the first output port, we'll have to set up this polyphony first. We'll do this using using the `global-actions` property of the TimeSeq [script](../TIMESEQ-SCRIPT-JSON.md#script). These actions are executed when a script is loaded or reset, so setting up the polyphony here will make it available throughout the rest of the script processing:

```json
"global-actions": [
    {
        "set-polyphony": {
            "index": 1,
            "channels": 5
        }
    }
],
```

The [set-polyphony](../TIMESEQ-SCRIPT-JSON.md#set-polyphony) action specifies that the first output port should contain five polyphonic channels, which is how many clock signals we'll generate in this sample.

## A 120 BPM Timeline

The first *timeline* we'll set up is one that will use a 120 Beats per Minutes (`bpm`) time scale, with 4 Beats per Bar (`bpb`):

```json
{
    "time-scale": { "bpm": 120, "bpb": 4 },
    "lanes": [
    ]
}
```

### Clock Signal on Every Beat

The first *lane* in this *timeline* will create a clock signal that fires on every beat of the 120 bpm. Just like all the *lane*s in this script, it will be set to `auto-start` and  `loop` so that the clock is always running:

```json
{
    "auto-start": true, "loop": true,
    "segments": [
        {
            "duration": { "beats": 1 },
            "actions": [
                {
                    "timing": "gate",
                    "output": { "index": 1, "channel": 1 } }
            ]
        }
    ]
}
```

The [gate](../TIMESEQ-SCRIPT-JSON.md#gate-actions) action in this segment will generate a clock signal on the first channel of the output port that will be high for the first half of the *segment* duration, and low for the second half.

### Clock Signal with Partial Beat

The `beat` of the clock signal can also use decimal numbers. In the second lane we'll set the `beats` to a value of `1.75`:

```json
{
    "auto-start": true, "loop": true,
    "segments": [
        {
            "duration": { "beats": 1.75 },
            "actions": [
                {
                    "timing": "gate",
                    "gate-high-ratio": 0.66,
                    "output": { "index": 1, "channel": 2 }
                }
            ]
        }
    ]
}
```

This *lane* will create a clock beat that lasts one and three quarters of the 120 bpm that is specified on the *timeline* (so faster then the previous *lane*, but not quite double the speed). The clock signal will be sent to the second channel of the first output port.

The `gate-high-ratio` property will control how long the clock signal will remain high (as a value between `0` and `1`). The `0.66` value will make cause it to remain high for 66% of the segment duration, and low for 44% of the segment duration. This can be usefull in certain scenarios, like when the clock signal is used for generating an ADSR envelope, and the sustain section of the envelope should last longer.

### A Slow Clock Signal

Since both a `bpm` and a `bpb` is specified on the *time-scale* of the *timeline*, a slow clock signal can also be expressed in `bars`:

```json
{
    "auto-start": true, "loop": true,
    "segments": [
        {
            "duration": { "beats": 0, "bars": 4 },
            "actions": [
                {
                    "timing": "gate",
                    "gate-high-ratio": 0.25,
                    "output": { "index": 1, "channel": 5 }
                }
            ]
        }
    ]
}
```

This clock will trigger every 4 bars on channel 5 of the first output port. When left it its default setting, this clock signal would remain high for 2 bars, and low for 2 bars. The `gate-high-ratio` of `0.25` will cause it to only remain high for a quarter for the duration, and low for the remainder of the segment.

## A 90 BPM Timeline

Because a script can have multiple *timeline*s, the clock signals in a script don't all have to use the same `bpm`. This second *timeline* will use 90 Beats per Minute, and contain a segment that triggers the clock ones for each of these 90 `beats` on channel 4 of output port 1:

```json
{
    "time-scale": { "bpm": 90 },
    "lanes": [
        {
            "auto-start": true, "loop": true,
            "segments": [
                {
                    "duration": { "beats": 1 },
                    "actions": [
                        {
                            "timing": "gate",
                            "output": { "index": 1, "channel": 3 }
                        }
                    ]
                }
            ]
        }
    ]
}
```

Since the *segment* in this *timeline* doesn't use a `bars` time unit, we won't specify a `bpb` on the *timeline* *time-scale*.

## A 75 BPM Timeline

The third timeline will be a bit more off-beat compared to the other timelines since those were a multiple of 30, while this final *timeline* will use a `bpm` of 75. It will again trigger on every beat of these 75 Beats per Minute, and send that clock signal to the fourth channel of the first output port.

```json
{
    "time-scale": { "bpm": 75, "bpb": 4 },
    "lanes": [
        {
            "auto-start": true, "loop": true,
            "segments": [
                {
                    "duration": { "beats": 1 },
                    "actions": [
                        {
                            "timing": "gate",
                            "output": { "index": 1, "channel": 4 }
                        }
                    ]
                }
            ]
        }
    ]
}
```

## Full Script and VCV Rack Patch

The full clock script can be found [here](clock/clock.vcv).

The [clock.vcv](clock/clock.vcv) patch will split up the five clock signals from the polyphonic first output port and use each of the clock signals to generate a separate ADSR Envelope. This Envelope will be used to control the volume of five VCOs (each playing a different note).

The clock signals themselves will also be show on oscilloscopes: the four shorter clocks on combined on the Count Modula Oscilloscope, and the slow clock on a VCV Scope.
