# TimeSeq CLOCK sample script (using clocks)

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

With the introduction of [clocks](../TIMESEQ-SCRIPT-JSON.md#clock) in script version v1.3.0 (TimeSeq v2.0.8), it has become easier to create scripts that set up one or more dedicated clock outputs. While the end result is the same as a clock that was set up using *timelines*, *lanes*, *segments* and *actions* (as demonstrated in the [Creating Clock Signals using Timelines](./CLOCK-USING-TIMELINES.md) sample), the new *clocks* feature will require less scripting setup to get the same clock output signal. This script starts by recreating the same clock signal as the old sample page, but using *clocks* with *clock lanes* and *durations* instead of *timelines* with *lanes*, *segments* and *actions*. It will then add an example of a more complex clock signal, demonstrating its usage as a more complex gate/trigger sequencer.

## Table of Contents

* [Set Up Polyphonic Output](#set-up-polyphonic-output)
* [A 120 BPM Clock](#a-120-bpm-clock)
  * [Clock Signal on Every Beat](#clock-signal-on-every-beat)
  * [Clock Signal with Partial Beat](#clock-signal-with-partial-beat)
  * [A Slow Clock Signal](#a-slow-clock-signal)
* [A 90 BPM Clock](#a-90-bpm-clock)
* [A 70 BPM Clock](#a-70-bpm-clock)
* [A Complex Clock Sequence](#a-complex-clock-sequence)
* [Variable Duration](#variable-duration)

## Set Up Polyphonic Output

Since we want to send the clock outputs as polyphonic channels on the first output port, we'll have to set up this polyphony first. We'll do this using the `global-actions` property of the TimeSeq [script](../TIMESEQ-SCRIPT-JSON.md#script). Actions in the `global-actions` list are executed when a script is loaded or reset, so setting up the polyphony here will initialize that port and make polyphony on it available throughout the rest of the script:

```json
"global-actions": [
    {
        "set-polyphony": {
            "index": 1,
            "channels": 5
        }
    }
]
```

The [set-polyphony](../TIMESEQ-SCRIPT-JSON.md#set-polyphony) action specifies that the first output port should contain five polyphonic channels, which is how many clock signals we'll generate in this sample.

## A 120 BPM Clock

Instead of a *timelines* list, this script will contain a *clocks* list. The different *clock lanes* will be added to this *clocks* list, just like regular *lanes* would get added to a *timeline*.

The first *clock* we'll set up is one that will use a 120 Beats per Minutes (`bpm`) time scale, with 4 Beats per Bar (`bpb`):

```json
{
    "time-scale": { "bpm": 120, "bpb": 4 },
    "lanes": [
    ]
}
```

This *time-scale* definition will set how `beats` and `bars` will be interpreted in all clock *lane*s that are part of that *clock*.

### Clock Signal on Every Beat

For the first *lane* in this *clock*, we'll create a clock signal that fires on every beat of the 120 bpm. Just like all other *lane*s that will be added to this script, it will be set to `auto-start`, and since a clock lane always loops, the clock is always running:

```json
{
    "auto-start": true,
    "durations": [ { "beats": 1 } ],
    "output": { "index": 1, "channel": 1 }
}
```

In the [durations](../TIMESEQ-SCRIPT-JSON.md#duration) property of the [clock lane](../TIMESEQ-SCRIPT-JSON.md#clock-lane), a single 1-beat duration is included. This tells TimeSeq that it should send a looping gate signal that lasts 1 beat to the port specified in the *output* property: the first channel of the first output port. This gate signal will be high for the first half of the beat, and low for the second half of that beat.

### Clock Signal with Partial Beat

The `beat` property of the *clock lane* *duration* can also be set to decimal numbers. In the second lane we'll set the `beats` to a value of `1.75`:

```json
{
    "durations": [ { "beats": 1.75 } ],
    "output": { "index": 1, "channel": 2 },
    "gate-high-ratio": 0.66
}
```

Since the *clock lane* `auto-start` property defaults to `true`, we can omit it from the *lane* and still have the lane start automatically when the script loads.
This *clock lane* will create a clock beat that lasts one and three quarters of the 120 bpm that is specified on the *clock* (so slower then the previous *lane*, but not quite double as long). The clock signal will be sent to the second channel of the first output port.

The `gate-high-ratio` property will control how long the clock signal will remain high (as a value between `0` and `1`). The `0.66` value used here will cause it to remain high for 66% of the segment duration, and low for 34% of the segment duration. This can be useful in certain scenarios, like when the clock signal is used for generating an ADSR envelope, and the sustain section of the envelope should be longer or shorter.

### A Slow Clock Signal

Since both a `bpm` and a `bpb` is specified on the *time-scale* of this *clock*, a slow clock signal can also be expressed in `bars`:

```json
{
    "durations": [ { "beats": 0, "bars": 4 } ],
    "output": { "index": 1, "channel": 5 },
    "gate-high-ratio": 0.25
}
```

This clock will trigger every 4 bars on channel 5 of the first output port. When left on its default setting, this clock signal would remain high for 2 bars, and low for 2 bars. But with the  `gate-high-ratio` of `0.25` that is used here, it will only remain high for a quarter for the duration, and low for the remainder of the clock duration.

## A 90 BPM Clock

Because a script can have multiple *clocks*, the clock signals in a script don't all have to use the same `bpm`. This second *clock* will use 90 Beats per Minute, and contain a clock lane that triggers the clock once for each of these 90 `beats`. The output is sent to third channel of output port 1:

```json
{
    "time-scale": { "bpm": 90 },
    "lanes": [
        {
            "durations": [ { "beats": 1 } ],
            "output": { "index": 1, "channel": 3 }
        }
    ]
}
```

Since the *duration* in this *clock* doesn't use a `bars` time unit, we don't specify a `bpb` on the *clock* *time-scale*.

## A 70 BPM Clock

The third clock will be a bit more off-beat compared to the other clocks: the others were a multiple of 30, while this final *clock* will use a `bpm` of 70. It will again trigger on every beat of these 70 Beats per Minute, and send that clock signal to the fourth channel of the first output port.

```json
{
    "time-scale": { "bpm": 70, "bpb": 4 },
    "lanes": [
        {
            "durations": [ { "beats": 1 } ],
            "output": { "index": 1, "channel": 4 }
        }
    ]
}
```

## Full Simple clocks Script and VCV Rack Patch

The full clock script can be found in [clock.json](clock-using-clocks/clock.json).

The [clock.vcv](clock-using-clocks/clock.vcv) patch will split up the five clock signals from the polyphonic output port and use each of the clock signals to generate a separate ADSR Envelope. This Envelope will be used to control the volume of five VCOs (each playing a different note).

The clock signals themselves will also be shown on oscilloscopes: the four shorter clocks are combined on the Count Modula Oscilloscope, and the slow clock on a VCV Scope.

## A Complex Clock Sequence

So far, all *clock lanes* have had a single [duration](../TIMESEQ-SCRIPT-JSON.md#duration) in the *durations* property. This results in a constant looping clock signal, each gate lasting for the same duration. Since this *durations* property is an array, it is possible to also create more complex looping clock trigger/gate signals using the *clocks* feature. If more *duration*s are added to the list, TimeSeq will step through each of them in order, and create clock signals for each of them, one after the other, resulting in a repeating pattern:

```json
{
    "lanes": [
        {
            "durations": [
                { "millis": 250 },
                { "millis": 125 },
                { "millis": 125 },
                { "millis": 250 },
                { "millis": 500 },
                { "millis": 250 },
                { "millis": 500 }
            ],
            "output": 2
        }
    ]
}
```

This clock signal uses milliseconds instead of beats, so there is no *time-scale* definition needed for the *clock*. The *clock lane* contains a list of millisecond *durations* that will create a recognizable looping pattern when played.

## Full Script and VCV Rack Patch

This clock, combined with the previous clock signals, can be found in the [complex-clock.json](clock-using-clocks/complex-clock.json) script.

The [complex-clock.vcv](clock-using-clocks/complex-clock.vcv) patch adds it as an additional VCO on top of the other ones.

## Variable Duration

Since *duration* in TimeSeq supports [Variable-length durations](../TIMESEQ-SCRIPT-JSON.md#variable-length-durations), it is also possible to bring variation in a clock signal. As an example, we'll start from the clock we defined earlier. The last element in the *durations* array will be replaced with a randomly generated duration instead of the fixed 500 millisecond duration:

```json
{
    "lanes": [
        {
            "durations": [
                { "millis": 250 },
                { "millis": 125 },
                { "millis": 125 },
                { "millis": 250 },
                { "millis": 500 },
                { "millis": 250 },
                { "millis": {
                    "rand": {
                        "lower": { "voltage": 500, "no-limit": true },
                        "upper": { "voltage": 2000, "no-limit": true }
                    }
                } }
            ],
            "output": 2
        }
    ]
}
```

In this clock, the last duration will be a random value between 500 and 2000 (milliseconds), causing the gap between two repetitions of the pattern to change each time.
