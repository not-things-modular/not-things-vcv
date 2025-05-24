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
* [Full Bernoulli Gate Version](#full-bernoulli-gate-version)
  * [Variable Probability](#variable-probability)
  * [Output Gate Duration](#output-gate-duration)
    * [Update the Output Chance Segment](#update-the-output-chance-segment)
    * [Wait for Low Input Gate](#wait-for-low-input-gate)
  * [Full Script and VCV Rack Patch](#full-script-and-vcv-rack-patch-for-full-version)

## Basic Bernoulli Gate Version

The basic version of the Bernoulli Gate will accept an input trigger on an input port. Each time a trigger is detected, a 0.5 second gate is sent out on one of two output ports. Which of the two possible output gates is receiving that gate is determined by a fixed 50% probability.

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

The [if](../TIMESEQ-SCRIPT-JSON.md#if) condition will check if the `bernoulli-chance` variable is greater than (`gt`) 0V and set the first output to 10V if this is the case. This will result in a 50% probability since the variable is between -5V and 5V.

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

## Full Bernoulli Gate Version

For the full Bernoulli Gate functionality, we need to extend the basic version with two features:

* It should be possible to control the the likeliness of the output gate going to output 1 or output 2 instead of always having a 50% probability
* Instead of sending a fixed 0.5 second gate to the output, the output gate should remain high as long as the input gate that started it is high.

### Variable Probability

To achieve the variable probability, we'll use a voltage from an [input](../TIMESEQ-SCRIPT-JSON.md#input) port.  We'll define that this input voltage is expected to be between 0V and 10V: 0V means that all gates should go to the first output, 10V means that all gates should go to the second output and any value in between should cause the probability to lean more towards the one or the other output, with 5V giving a 50% probability.

An update of the *if* conditions on the *action*s that were previously created in the [Set Output Gates](#set-output-gates) section will give us this desired functionality:

```json
{
    "if": {
        "gt": [
            { "variable": "bernoulli-chance" },
            {
                "input": 2,
                "calc": [
                    { "sub": 5 }
                ]
            }
        ]
    },
    "set-value": {
        "output": 1,
        "value": 10
    }
}
```

Instead of comparing the `bernoulli-chance` variable (which is between -5V and 5V) with a fixed 0V value (Which resulted in a 50% probability), we compare it with the current voltage on *input* port 2 (which is expected to be between 0V and 10V). By subtracting 5V from this input value using a [calc](../TIMESEQ-SCRIPT-JSON.md#calc)ulation, it is moved into the same -5V to 5V range as the `bernoulli-chance` variable, so that a simple `gt` (greater than) compare is sufficient to determine if the output should be set to 10V.

The same logic applies to the *if* condition of the second output, but with a `lte` (less than or equal) instead of a `gt` compare.

## Output Gate Duration

To get the output gate to last as long as the input gate that triggered it, some timing changes will have to be done, combined with an additional lane. The logic to implement in the script will be:

* Remove the second *segment* that was used in the *basic* version to [Reset the Outputs](#reset-the-outputs)
* Perform the output gate determination as before in the first *segment*, but let it last 1 sample (i.e. the minimum duration) instead of 0.5 seconds
* At the end of that segment, fire an internal trigger (called `reset-output`)
* Create another lane loops, and is started by the `reset-output` trigger
* In this new lane, check if the input voltage has gone back below 1V (i.e. the input gate is no longer high), and reset both outputs if that is the case and stop the second lane from running

### Update the Output Chance Segment

Timing-wise, the first *segment* from the *basic* script will become (ommitting the existing actions for brevity) will become

```json
{
    "duration": { "samples": 1 },
    "actions": [
        {
            "set-variable": {
                "name": "bernoulli-chance",
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
                "output": 3,
                "value": { "variable": "bernoulli-chance" }
            }
        },
        {
            "if": {
                "gt": [
                    { "variable": "bernoulli-chance" },
                    {
                        "input": 2,
                        "calc": [
                            { "sub": 5 }
                        ]
                    }
                ]
            },
            "set-value": {
                "output": 1,
                "value": 10
            }
        },
        {
            "if": {
                "lte": [
                    { "variable": "bernoulli-chance" },
                    {
                        "input": 2,
                        "calc": [
                            { "sub": 5 }
                        ]
                    }
                ]
            },
            "set-value": {
                "output": 2,
                "value": 10
            }
        },
        {
            "timing": "end",
            "trigger": "reset-output"
        }
    ]
}
```

Next to the previously explained changes to the *if* conditions of the *set-value* *action*s, the *duration* of the segment has been changed to 1 sample, and an additional action has been added at the end of the action list. This action has an `end` timing (so it will be executed when the segment completes) and will cause an internal trigger to be fired with the `reset-output` name.

Once this new version of the segment has completed, the relevant output port will be set to 10V (i.e. a high gate signal), and the `reset-output` trigger can be used to start waiting for the input gate to go low again.

### Wait for Low Input Gate

To wait for the input gate to go low again, we'll create a new lane that is started by the previously fired `reset-output` trigger and will loop a single 1-sample length segment that checks if the input voltage has gone below 1V (i.e. the gate went low again):

```json
{
    "auto-start": false,
    "loop": true,
    "start-trigger": "reset-output",
    "stop-trigger": "stop-reset-output",
    "segments": [
        {
            "duration": { "samples": 1 },
            "actions": [
                {
                    "if": {
                        "lt": [
                            { "input": 1 },
                            1
                        ]
                    },
                    "set-value": {
                        "output": 1,
                        "value": 0
                    }
                },
                {
                    "if": {
                        "lt": [
                            { "input": 1 },
                            1
                        ]
                    },
                    "set-value": {
                        "output": 2,
                        "value": 0
                    }
                },
                {
                    "if": {
                        "lt": [
                            { "input": 1 },
                            1
                        ]
                    },
                    "trigger": "stop-reset-output"
                }
            ]
        }
    ]
}
```

This lane will not `auto-start` since it is expected to start based on the `reset-output` internal trigger, as specified by the `start-trigger` property. Sine the lane has to wait for the input gate to go low again, it will `loop`.

The *segment* in this *lane* contains three actions that are all using the same *if* conditional: the voltage on *input* port 1 has to be below 1V again. When this happens, three actions will be performed:

* Change the voltage on output 1 to 0V (in case it was previously set to 10V)
* Change the voltage on output 2 to 0V (in case it was previously set to 10V)
* Fire an internal `stop-reset-output` trigger that will stop the lane from running: since set both outputs to a low gate again, we no longer need to check for a low input gate anymore until a new input gate triggers the first *lane*.

The `stop-reset-output` trigger will stop the second *lane* from looping since it is used in its `stop-trigger` property.

### Full Script and VCV Rack Patch for Full Version

The full script for the full Bernoulli Gate can be found [here](bernoulli/bernoulli-full.json).

The [bernoulli-basic.vcv](bernoulli/bernoulli-full.vcv) patch shows this script in action. It adds an additional 8VERT module, on which the probability of the output gate selection can be set using a 0V-10V value.
