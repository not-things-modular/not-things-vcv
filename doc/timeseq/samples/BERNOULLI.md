# TimeSeq BERNOULLI sample script

*A Sample script for the not-things [TimeSeq](../../TIMESEQ.md) module.*

It is unlikely that the TimeSeq module will be used as a Bernoulli Gate. After all: Bernoulli Gate module already does an excellent job at providing that functionality. The construction of this script will however show how TimeSeq can be used in other ways than for plain linear sequences. We'll start out constructing a basic version of a Bernoulli Gate, and then move on to a more complete version.

Throughout the script, we'll use short versions of notations where possible, e.g.:

* Use *input*s, *output*s and fixed *value*s shorthand notations instead of the full verbose versions (see the [input](../TIMESEQ-SCRIPT-JSON.md#shorthand-input-notation), [output](../TIMESEQ-SCRIPT-JSON.md#shorthand-output-notation) and [value](../TIMESEQ-SCRIPT-JSON.md#shorthand-value-notation) shorthand sections of the TimeSeq Script JSON sepcification for more details).
* Since an *action* doesn't have a `timing` property will default to a `start` timing, we'll only include the `timing` property on actions if it is different from that `start` timing.

## Table of Contents

* [Basic Bernoulli Gate Version](#basic-bernoulli-gate-version)
  * [Triggering the Gate](#triggering-the-gate)
  * [Reacting to a Trigger](#reacting-to-the-trigger)
  * [Determine Output Chacne](#determine-output-chance)
  * [Set Output Gates](#set-output-gates)
  * [Reset the Outputs](#reset-the-outputs)
  * [Full Script and VCV Rack Patch](#full-script-and-vcv-rack-patch-for-basic-version)

## Basic Bernoulli Gate Version

The basic version of the Bernoulli Gate will accept an input trigger on an input port. Each time a trigger is detected, a 0.5 second gate is sent out on one of two output ports. Which of the two possible output gates is receiving that gate is determined by a fixed 50% chance.

### Triggering the Gate

To detect incoming triggers, an [input-trigger](../TIMESEQ-SCRIPT-JSON.md#input-trigger) must be defined that will be linked to an internal TimeSeq trigger signal:

```json
"input-triggers": [
    {
        "id": "determine-output",
        "input": 1
    }
]
```

The script snipplet above specifies that when a trigger is detected on [input](../TIMESEQ-SCRIPT-JSON.md#input) port one, an internal [trigger](../TIMESEQ-SCRIPT.md#triggers) with the name `determine-output` should be fired.

### Reacting to the Trigger

In order to actually handle the *input-trigger*, we need to define a [lane](../TIMESEQ-SCRIPT-JSON.md#lane) that will start when the trigger is fired:

```json
"lanes": [
    {
        "auto-start": false,
        "loop": false,
        "start-trigger": "determine-output",
        "segments": [
            {
                "duration": { "millis": 500 },
                "actions": [
                ]
            }
        ]
    }
]
```

The `start-trigger` specifies that this lane will start when the previously set up *input-trigger* is received. Since it is defined as a `start-trigger` (and not a `restart-trigger`), this *lane* will only react to the input trigger if the lane isn't running yet. Otherwise, the trigger will be ignored.

Since this *lane* is started based on a trigger, it should not `auto-start`, and to provide the Bernoulli Gate functionality, we also want to stop the lane processing when it reaches the end, so `loop` is set to `false`.

The first [segment](../TIMESEQ-SCRIPT-JSON.md#segment) of the *lane* will last for half a second. This is where the gate output signal will be generated that lasts for that duration. The [action](../TIMESEQ-SCRIPT-JSON.md#action])s of this *segment* will handle the logic that is required for the Bernoulli Gate.

### Determine Output Chance

The first thing to do for a Bernoulli Gate is to determine which of the two outputs will receive the output gate. This will be the first *action* of the *lane*:

```json
{
    "set-variable": {
        "name": "bernoulli-chance",
        "value": {
            "rand": {
                "lower": -5,
                "upper": 5
            }
        }
    },
    {
        "set-value": {
            "output": 3,
            "value": { "variable": "bernoulli-chance" }
        }
    }
}
```

This action will set a `bernoulli-chance` [variable](../TIMESEQ-SCRIPT-JSON.md#set-variable) to a [rand](../TIMESEQ-SCRIPT-JSON.md#rand)om value between -5V and 5V. We can use that to determine the active output in the next steps. To be able to examine and visualize this chance variable, we'll also output it on [output](../TIMESEQ-SCRIPT-JSON.md#output) port 3.

### Set Output Gates

Using the generated `bernoulli-chance` variable, we can now set the gate signal on the appropriate *output* port:

```json
{
    "if": {
        "gt": [
            { "variable": "bernoulli-chance" },
            0
        ]
    },
    "set-value": {
        "output": 1,
        "value": 10
    }
}
```

The [if](../TIMESEQ-SCRIPT-JSON.md#if) condition will check if the `bernoulli-chance` variable is greater than (`gt`) 0V and set the first output to 10V if this is the case. This will result in a 50% chance since the variable is between -5V and 5V.

The second output is set using a similar check:

```json
{
    "if": {
        "lte": [
            { "variable": "bernoulli-chance" },
            0
        ]
    },
    "set-value": {
        "output": 2,
        "value": 10
    }
}
```

This *action* will set the second output to 10V if the variable is less than or equal to (`lte`) 0V. The *equal* will make sure that an exact 0V random value doesn't result in no outputs being triggered, which would happen if a regular `lt` would be used.

Since the *segment* that contains these actions has a *duration* of 500 milliseconds, the currently assigned gate will remain active on either the first or the second gate for half a second

### Reset the Outputs

Once the previous *segment* finishes, a new *segment* will be executed that sets both outputs back to 0V:

```json
{
    "duration": { "samples": 1 },
    "actions": [
        {
            "set-value": {
                "output": 1,
                "value": 0
            }
        },
        {
            "set-value": {
                "output": 2,
                "value": 0
            }
        }
    ]
}
```

This *segment* has a 1 sample duration (the shortest possible segment duration) since its only purpose is to reset both gate outputs back to 0V. When this *segment* finishes, the *lane* will also finish since it is set to not loop. Both gate outputs will be low again now, and the *lane* is ready to receive the next *input-trigger*.

### Full Script and VCV Rack Patch for Basic Version

The full script for the basic Bernoulli Gate can be found [here](bernoulli/bernoulli-basic.json).

The [bernoulli-basic.vcv](bernoulli/bernoulli-basic.vcv) patch shows this script in action:

* A *VCV Push* module is connected to the first *input* to allow it to trigger the start of the *lane* logic
* The first port/gate output is sent to the top (green) Scope, and through an ADSR Envelope will make a VCO sound out that plays a higher note
* The second port/gate output is sent to the bottom (blue) Scope, and through another ADSR Envelope will make a VCO sound out that plays a lower note2
* A third (red) scope will display the value of the `bernoulli-chance` variable that is used by the script to determine which of the two outputs should receive the generated gate signal.
