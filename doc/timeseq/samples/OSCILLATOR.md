# TimeSeq NOTE SEQUENCE sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

You wouldn't expect to use a sequencer to generate an oscillator signal, but since it's possible... In this script, we'll set up some simple sequences to generate a square, sawtooth, triangle and sine-like waves, all set up to generate a C3 note (at 130.815Hz). And as a bonus, a noise signal will also be thrown in.

In order to generate these signals, we'll have the opportunity to look at some different [glide](../TIMESEQ-SCRIPT-JSON.md#glide-actions) actions.

Each of these five outputs will be set up in their own [lane](../TIMESEQ-SCRIPT-JSON.md#lane), which will `auto-start` and `loop`

An addition in v1.1.0 of the TimeSeq JSON Script also allows these outputs to be [Tuned Oscillators](#tuned-oscillator) instead of always generating a fixed frequency note.

## Table of Contents

* [Square Wave](#square-wave)
* [Sawtooth Wave](#sawtooth-wave)
* [Triangle Wave](#triangle-wave)
* [Sine Wave](#sine-wave)
* [Noise Output](#noise-output)
* [Full Script and VCV Rack Patch](#full-script-and-vcv-rack-patch)
* [Tuned Oscillator](#tuned-oscillator)

## Square Wave

We can generate a square wave by outputting a high voltage (5V) for half the duration of the wave cycle and a low voltage (-5V) for the rest of the cycle. Since segments allow their duration to be expressed in Hz, we can achieve the square wave by having a *lane* loop two *segment*s with half the duration of the C3 frequency, one setting the output to 5V, the other setting it to -5V:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "hz": 261.63 },
            "actions": [
                {
                    "set-value": {
                        "output": 1,
                        "value": 5
                    }
                }
            ]
        },
        {
            "duration": { "hz": 261.63 },
            "actions": [
                {
                    "set-value": {
                        "output": 1,
                        "value": -5
                    }
                }
            ]
        }
    ]
}
```

## Sawtooth Wave

A sawtooth wave is formed by letting the voltage move from a low value (-5V) to a high value (5V) over the duration of the wave cycle, and then letting it immediately drop back to the low value. This behaviour can be achieved in TimeSeq by using a *glide* action, and since there is only one glide needed (moving from the low voltage to the high voltage), this will also only require one *segment* in the looping *lane*:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "hz": 130.815 },
            "actions": [
                {
                    "timing": "glide",
                    "start-value": -5,
                    "end-value": 5,
                    "output": 2
                }
            ]
        }
    ]
}
```

## Triangle Wave

A triangle wave is similar to a [sawtooth](#sawtooth-wave), but instead of immediately dropping back to the low voltage after gliding to the high voltage, it will glide back to the low value. This means that to generate a triangle wave, we'll also have to use two segments with half the duration of the wave cycle: one to glide from -5V to 5V and another to glide from 5V to -5V:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "hz": 261.63 },
            "actions": [
                {
                    "timing": "glide",
                    "start-value": -5.0,
                    "end-value": 5.0,
                    "output": 3
                }
            ]
        },
        {
            "duration": { "hz": 261.63 },
            "actions": [
                {
                    "timing": "glide",
                    "start-value": 5.0,
                    "end-value": -5.0,
                    "output": 3
                }
            ]
        }
    ]
}
```

## Sine Wave

While TimeSeq can't generate an exact sine wave signal, it is possible to use the *easing* functionality of a *glide* *action* to get close to one. We will have to cut up the wave in to four sections since we'll have to use positive and negative easing factors, dependant on how the curve should be shaped:

* Move from -5V to 0V using a positive easing factor to get a curve that starts slow and then speeds up
* Move from 0V to 5V using a negative easing factor to get a curve that starts fast and then slows down
* Move from 5V back to 0V using a positive easing factor so the curve starts slow again and then speeds up
* Move from 0V to -5V again using a negative easing factor so the curve starts fast and then slows down.

To get a quick view of the different segments, you can comment out the `easing-factor` on each of the segments in turn (by renaming it to `x-easing-factor` so it is ignored by the TimeSeq JSON parser) and loading it into the patch that's included at the end of this page.

To approach a sine wave, we'll also have to the `pow` *ease-algorithm* instead of the default `sig` algorithm.

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "hz": 523.26 },
            "actions": [
                {
                    "timing": "glide",
                    "ease-factor": 0.375,
                    "ease-algorithm": "pow",
                    "start-value": -5,
                    "end-value": 0,
                    "output": 4
                }
            ]
        },
        {
            "duration": { "hz": 523.26 },
            "actions": [
                {
                    "timing": "glide",
                    "ease-factor": -0.375,
                    "ease-algorithm": "pow",
                    "start-value": 0,
                    "end-value": 5,
                    "output": 4
                }
            ]
        },
        {
            "duration": { "hz": 523.26 },
            "actions": [
                {
                    "timing": "glide",
                    "ease-factor": 0.375,
                    "ease-algorithm": "pow",
                    "start-value": 5,
                    "end-value": 0,
                    "output": 4
                }
            ]
        },
        {
            "duration": { "hz": 523.26 },
            "actions": [
                {
                    "timing": "glide",
                    "ease-factor": -0.375,
                    "ease-algorithm": "pow",
                    "start-value": 0,
                    "end-value": -5,
                    "output": 4
                }
            ]
        }
    ]
}
```

## Noise Output

A noise output signal can be generated by random value (between -5V and 5V) for each sample:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "samples": 1 },
            "actions": [
                {
                    "set-value": {
                        "value": {
                            "rand": {
                                "lower": -5.0,
                                "upper": 5.0
                            }
                        },
                        "output": 5
                    }
                }
            ]
        }
    ]
}
```

## Full Script and VCV Rack Patch

The full script can be found [here](oscillator/oscillator.json).

The [oscillator.vcv](oscillator/oscillator.vcv) patch will visualize each of the outputs on a Scope and send all of them (except for the noise) through a switch and into the Audio Output module. Pressing the button on the Push module will make the switch move through the different waveforms.

## Tuned Oscillator

With the introduction of [variable-length segments](../TIMESEQ-SCRIPT-JSON.md#variable-length-durations) in v1.1.0 of the TimeSeq JSON Script, it is possible to let the note generated by the output signals be tuned to a modifiable frequency.

In this sample, we'll use the voltage of the first TimeSeq input, interpret it as a 1V/Oct signal and translate that into the frequency that will be used for the signal generating lanes. While it would be possible to calculate this frequency in every cycle of the wave signals, this would cause unneeded processing overhead, so we'll instead use a lane to do this calculation every 50 milliseconds:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "millis": 50 },
            "actions": [
                {
                    "set-variable": {
                        "name": "frequency",
                        "value": {
                            "input": 1,
                            "calc": [ { "vtof": true } ]
                        }
                    }
                }
            ]
        }
    ]
},
```

The *action* takes the current voltage of the first *input* and converts that from a 1V/Oct to a frequency using a `vtof` (Voltage to Frequency) [calc](../TIMESEQ-SCRIPT-JSON.md#calc). This variable can then be used in the duration of the sawtooth wave since it contains only one segment:

```json
{
    "duration": { "hz": { "variable": "frequency" } },
    "actions": [
        {
            "timing": "glide",
            "start-value": -5,
            "end-value": 5,
            "output": 2
        }
    ]
}
```

In the other waveforms, multiple segments are used to generate one wave cycle. To end up with a wave cycle that matches the desired frequency, we'll also have to let those segments run at a higher speed (either at double or at four times the rate). We can add to more actions to the frequency calculation lane to determine these rates:

```json
{
    "set-variable": {
        "name": "frequency-2x",
        "value": {
            "variable": "frequency",
            "calc": [ { "mult": 2 } ]
        }
    }
},
{
    "set-variable": {
        "name": "frequency-4x",
        "value": {
            "variable": "frequency",
            "calc": [ { "mult": 4 } ]
        }
    }
}
```

These `frequency-2x` and `frequency-4x` variables can then be used in the other waveform lanes, e.g. in the triangle wave:

```json
"segments": [
    {
        "duration": { "hz": { "variable": "frequency-2x" } },
        "actions": [
            {
                "timing": "glide",
                "start-value": -5.0,
                "end-value": 5.0,
                "output": 3
            }
        ]
    },
    {
        "duration": { "hz": { "variable": "frequency-2x" } },
        "actions": [
            {
                "timing": "glide",
                "start-value": 5.0,
                "end-value": -5.0,
                "output": 3
            }
        ]
    }
]
```

### Full Script and VCV Rack Patch

The full script for the tuned oscillators can be found in [oscillator-tuned](oscillator/oscillator-tuned.json), and the [oscillator-tuned.vcv](oscillator/oscillator-tuned.vcv) patch uses an 8VERT module to generate the 1V/Oct input signal that will be used to determine the current frequency.
