# TimeSeq SAMPLE AND HOLD (S&H) sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

Because voltage values on output ports and variables within TimeSeq are only changed when an action updates them, implementing sample-and-hold-like behaviour can be easily done in TimeSeq. This sample will demonstrate basic S&H behaviour using input sources and triggers, and expand on this by using internally generated voltages, trigger timing and combinations of values.

## Table of Contents

* [Basic Input Source and Trigger S&H](#basic-input-source-and-trigger-sh)
  * [Set Up the Inputs](#set-up-the-inputs)
  * [Set Up the Outputs](#set-up-the-outputs)
  * [Set Up the Input Triggers](#set-up-the-input-triggers)
  * [Perform the Sampling](#perform-the-sampling)
  * [Basic Script and VCV Rack Patch](#basic-script-and-vcv-rack-patch)
* [Internal Noise Source](#internal-noise-source)
* [Combining Sample and Hold Result](#combining-sample-and-hold-result)
* [Generating Internal S&H Triggers](#generating-internal-sh-triggers)
* [Full Script and Patch](#full-script-and-patch)

## Basic Input Source and Trigger S&H

The first S&H aspect that we will set up is the basic S&H behaviour: sample the voltage from an input source every time an external trigger is received, and hold that value on the output port until the next trigger is received. We'll set up four of these, each having their own input source and individual trigger.

In the sample VCV Rack patch that is linked to below, they will be connected to three LFO output signals (a sine, triangle and sawtooth signal) and to a noise output (i.e a random value). So that's what we'll also call them in the script.

### Set Up the Inputs

To keep a better overview in this script, we'll define re-usable [input](../TIMESEQ-SCRIPT-JSON.md#input) and [output](../TIMESEQ-SCRIPT-JSON.md#output) definitions in the [component-pool](../TIMESEQ-SCRIPT-JSON.md#component-pool) that we can then [reference](../TIMESEQ-SCRIPT.md#referencing) by their ID throughout the rest of the script, choosing IDs that specify what the ports are used for.

We'll use four input ports, combining the input voltage (from the LFO and Noise modules) with the corresponding trigger on the same port using different channels: channel 1 will be the input voltage, channel 2 will be input trigger. This results in following *input* definitions:

```json
"component-pool": {
    "inputs": [
        {
            "id": "input-sine-source",
            "index": 1,
            "channel": 1
        },
        {
            "id": "input-sine-trigger",
            "index": 1,
            "channel": 2
        },
        {
            "id": "input-triangle-source",
            "index": 2,
            "channel": 1
        },
        {
            "id": "input-triangle-trigger",
            "index": 2,
            "channel": 2
        },
        {
            "id": "input-sawtooth-source",
            "index": 3,
            "channel": 1
        },
        {
            "id": "input-sawtooth-trigger",
            "index": 3,
            "channel": 2
        },
        {
            "id": "input-noise-source",
            "index": 4,
            "channel": 1
        },
        {
            "id": "input-noise-trigger",
            "index": 4,
            "channel": 2
        }
    ]
}
```

### Set Up the Outputs

Similar to the *input*s, we'll also add *output* definitions to the *component-pool*, one for each of the four signals:

```json
"outputs": [
    {
        "id": "output-sine",
        "index": 1
    },
    {
        "id": "output-triangle",
        "index": 2
    },
    {
        "id": "output-sawtooth",
        "index": 3
    },
    {
        "id": "output-noise",
        "index": 4
    }
]
```

To make it easier in VCV Rack to identify which output contains which output signal, we'll also update the tooltip labels of the outputs using a [set-label](../TIMESEQ-SCRIPT-JSON.md#set-label) action in the *global-actions* property of the [script](../TIMESEQ-SCRIPT-JSON.md#script):

```json
"global-actions": [
    { "set-label": { "index": 1, "label": "Sine S&H"} },
    { "set-label": { "index": 2, "label": "Triangle S&H"} },
    { "set-label": { "index": 3, "label": "Sawtooth S&H"} },
    { "set-label": { "index": 4, "label": "Noise S&H"} }
],
```

### Set Up the Input Triggers

With the *input*s and *output*s set up, we'll have to hook up the [input-trigger](../TIMESEQ-SCRIPT-JSON.md#input-trigger)s so that a trigger received on one of the trigger *input*s can cause the S&H action to be performed:

```json
"input-triggers": [
    { "input": { "ref": "input-sine-trigger" }, "id": "sine-trigger" },
    { "input": { "ref": "input-triangle-trigger" }, "id": "triangle-trigger" },
    { "input": { "ref": "input-sawtooth-trigger" }, "id": "sawtooth-trigger" },
    { "input": { "ref": "input-noise-trigger" }, "id": "noise-trigger" }
]
```

This will cause an internal [trigger](../TIMESEQ-SCRIPT.md#triggers) with the corresponding ID to be fired when the external trigger is detected.

### Perform the Sampling

The configured internal triggers can now be used as `start-trigger` of a [lane](../TIMESEQ-SCRIPT-JSON.md#lane) that will perform the sampling of the input voltage. The sine signal sampler would be:

```json
{
    "start-trigger": "sine-trigger",
    "segments": [
        {
            "duration": { "samples": 1 },
            "actions": [
                {
                    "set-value": {
                        "output": { "ref": "output-sine" },
                        "value": { "input": { "ref": "input-sine-source" } }
                    }
                }
            ]
        }
    ]
}
```

The lane doesn't `auto-start`, but instead is started when the `sine-trigger` is fired by the corresponding *input-trigger* definition. The lane doesn't loop and it contains only one [segment](../TIMESEQ-SCRIPT-JSON.md#segment) that lasts for a single sample and will update `output-sine` *output* voltage using the current voltage of the `input-sine-source` *input*.

A similar lane for each of the three other Sample and Hold targets (triangle, sawtooth and noise) will complete the basic Sample and Hold patch.

### Basic Script and VCV Rack Patch

The full script can be found at [sample-and-hold.json](sample-and-hold/sample-and-hold.json), and the [sample-and-hold.vcv](sample-and-hold/sample-and-hold.vcv) patch contains the full setup:

* One LFO provides the Sine, Triangle and Sawtooth source signals, and a Noise module provides the white noise signal
* A second LFO provides the external trigger source on its Square output. A 1-to-4 Switch module is used to divide this trigger over four trigger signals, each receiving a trigger in turn
* The [PI-PO](../../PIPO.md) modules will combine source signals and trigger signals into 4 polyphonic signals with the source signal on the first channel and the trigger signal on the second channel
* The output is send into two Scope modules for visualization
* Cable and scope colouring is kept consistent to allow each of the processed Sample and Hold signals to be identified throughout the chain

## Internal Noise Source

Instead of using external voltage sources, a Sample and Hold action can also sample internal voltages. And instead of writing to an *output* port, we can also send the voltage to a *variable* that can be used in other places in the script.

In this step, we'll sample from a random value and send that both to output port 5 (so we can visualize it in the patch) and send it to a variable that we'll use later.

Following *input* definition in the *component-pool* will be used to receive the input trigger for the Sample and Hold action on *input* port `5`:

```json
{
    "id": "input-random-variable-trigger",
    "index": 5
}
```

Following *output* definition in the *component-pool* will be used to send the resulting Sample and Hold voltage to *output* port `5`:

```json
{
    "id": "output-random-variable",
    "index": 5
}
```

And following *input-trigger* definition will cause the `random-variable-trigger` internal trigger to be fired when the *input* detects an external trigger:

```json
{ "input": { "ref": "input-random-variable-trigger" }, "id": "random-variable-trigger" }
```

The new *lane* to perform the Sample and Hold action will then look as follows:

```json
{
    "start-trigger": "random-variable-trigger",
    "segments": [
        {
            "duration": { "samples": 1 },
            "actions": [
                {
                    "set-variable": {
                        "name": "random-variable",
                        "value": {
                            "rand": {
                                "lower": -5,
                                "upper": 5
                            }
                        }
                    }
                },
                {
                    "set-value": {
                        "value": { "variable": "random-variable" },
                        "output": { "ref": "output-random-variable" }
                    }
                }
            ]
        }
    ]
}
```

It generates a [rand](../TIMESEQ-SCRIPT-JSON.md#rand)om value between -5 and 5 volts and assigns it to the `random-variable` using a [set-variable](../TIMESEQ-SCRIPT-JSON.md#set-variable) action. This variable is then used to update the `output-random-variable` *output* port voltage using a [set-value](../TIMESEQ-SCRIPT-JSON.md#set-value) action.

### Script and VCV Rack Patch

The full script can be found at [sample-and-hold-internal-random.json](sample-and-hold/sample-and-hold-internal-random.json), and the [sample-and-hold-internal-random.vcv](sample-and-hold/sample-and-hold-internal-random.vcv) patch contains the full setup.

The patch adds another LFO to generate the trigger for the fifth *input* port and visualizes the resulting voltage output on another Scope.

## Combining Sample and Hold Result

The stored `random-variable` Sample and Hold variable from the previous step can also be used by other lanes to generate new signal combinations. In this step, we'll use the raw sine input from the first step, and use the `random-variable` to offset this.

First we need to define the *output* that we will send the result to:

```json
{
    "id": "output-random-variable-plus-sine",
    "index": 6
}
```

To generate an output signal that follows the original Sine signal, we'll need to set up a *lane* that starts automatically and loops:

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
                            "input": { "ref": "input-sine-source" },
                            "calc": [
                                { "add": { "variable": "random-variable" } }
                            ]
                        },
                        "output": { "ref": "output-random-variable-plus-sine" }
                    }
                }
            ]
        }
    ]
}
```

The single *segment* in this *lane* last for only one segment, so combined with the looping of the *lane*, it will be executed for every VCV Rack sample. It will take the current voltage from the sine *input*, and add the current voltage of the `random-variable` variable to it using a [calc](../TIMESEQ-SCRIPT-JSON.md#calc). The resulting voltage is then sent to the new *output* port.

The resulting script can be found at [sample-and-hold-sine-offset.json](sample-and-hold/sample-and-hold-sine-offset.json), and the [sample-and-hold-sine-offset.vcv](sample-and-hold/sample-and-hold-sine-offset.vcv) patch visualizes the result, showing a Sine signal that jumps up or down each time a new internal random voltage is sampled.

## Generating Internal S&H Triggers

The triggers for a Sample and Hold action don't have to come from an external source through an *input-trigger*. A TimeSeq can also perform the action based on an internal event. This can be an internal [trigger](../TIMESEQ-SCRIPT.md#triggers) or based on the timing execution of a *segment*. In this sample, we'll use the last option: a looping *segment* will perform the sampling action, with its [duration](../TIMESEQ-SCRIPT-JSON.md#duration) defining the frequency.

### Generating a new internal sampling source

First we'll create a new internal sampling source that we can use for this part of the script: a triangle signal that moves between -5V and 5V:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "millis": 2333 },
            "actions": [
                {
                    "timing": "glide",
                    "start-value": -5,
                    "end-value": 5,
                    "variable": "internal-triangle"
                }
            ]
        },
        {
            "duration": { "millis": 2333 },
            "actions": [
                {
                    "timing": "glide",
                    "start-value": 5,
                    "end-value": -5,
                    "variable": "internal-triangle"
                }
            ]
        }
    ]
},
```

This automatically started and looping lane uses two [glide](../TIMESEQ-SCRIPT-JSON.md#glide-actions) actions to move up and down between -5V and 5V, taking 2.333 seconds for each movement. The resulting moving voltage is stored in the `internal-triangle` variable.

The following lane can then be used to sample this variable and send it to the `output-internal-triangle` *output*:

```json
{
    "auto-start": true,
    "loop": true,
    "segments": [
        {
            "duration": { "millis": 500 },
            "actions": [
                {
                    "set-value": {
                        "value": {
                            "variable": "internal-triangle"
                        },
                        "output": { "ref": "output-internal-triangle" }
                    }
                }
            ]
        }
    ]
},
```

The `"millis": 500` duration of the *segment* makes this Sample and Hold action be executed every half second.

And since we have one more output port available, we can send a similar signal to the sine-and-random-value output to it, but this time combining the `random-variable` and `internal-triangle` variables. The resulting output signal will be completely based on internally generated voltages and trigger timings:

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
                            "variable": "internal-triangle",
                            "calc": [
                                { "add": { "variable": "random-variable" } }
                            ]
                        },
                        "output": { "ref": "output-internal-triangle-plus-random-value" }
                    }
                }
            ]
        }
    ]
}
```

The two additional *output* definitions that are needed to complete this patch are:

```json
{
    "id": "output-internal-triangle",
    "index": 7
},
{
    "id": "output-internal-triangle-plus-random-value",
    "index": 8
}
```

## Full Script and Patch

The full script can be found at [sample-and-hold-full.json](sample-and-hold/sample-and-hold-full.json), and the [sample-and-hold-full.vcv](sample-and-hold/sample-and-hold-full.vcv) patch contains a full setup with source signals and output visualization.
