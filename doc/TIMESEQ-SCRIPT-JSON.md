# TIMESEQ JSON SCRIPT FORMAT
*Part of the set of [not-things VCV Rack](../README.md) modules.*

## Full object hierarchy
The full object hierarchy of the TimeSeq JSON script looks as follows:
* [script](#script)
    * [input-triggers](#input-trigger)
    * global-actions ([action](#action))
    * [component-pool](#component-pool)
    * [timeline](#timeline)
        * [time-scales](#time-scale)
        * [lanes](#lane)
            * [segments](#segment)
                * [duration](#duration)
                * [segment-block](#segment-block)
                * [action](#action)
                    * [if](#if)
                        * `eq, ne, lt, lte, gt, gte, and, or`
                    * [set-value](#set-value)
                        * [output](#output)
                        * [value](#value)
                            * [input](#input)
                            * [output](#output)
                            * [rand](#rand)
                            * [calc](#calc)
                    * [set-variable](#set-variable)
                        * [value](#value)
                    * [set-polyphony](#set-polyphony)
                    * [assert](#assert)
                        * [if](#if)
                    * `trigger`


## script
The root item of the TimeSeq JSON script.

Sequencing is done through the *timeline* instances of the `timelines` list. For those cases where execution order might have an impact on script processing: on each processing cycle, the *timeline* instances are executed in the order that they appear in this list.

If there are specific *action*s that should be executed when a script is started or reset (e.g. setting the number of channels on a polyphonic output), these *action*s can be added to the `global-actions` list. Only *action*s that have a `timing` set to `START` can be added to this list.

If there is a need to generate internal TimeSeq triggers based on external trigger sources, the `input-triggers` property allows input ports to set up for receiving trigger signals.

In the `component-pool`, TimeSeq objects (*segment*s, *input*s, *output*s, *value*s, ...) can be defined that can then be referenced from elsewhere in the script. This allows a single object definition to be re-used in multiple parts of the script, and can allow better structuring of complex scripts through the usage of clear/descriptive IDs.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `type` | yes | string | Must be set to `not-things_timeseq_script` |
| `version`| yes | string | Identifies which version of the TimeSeq JSON script format is used. Currently only `1.0.0` is supported. |
| `timelines` | no | [timeline](#timeline) list | A list of instances that will drive the sequencer. |
| `global-actions` | no | [action](#action) list | A list of actions that will be executed when the script starts or is reset. Only actions which have their `timing` set to `START` are allowed. |
| `input-triggers` | no | [input-trigger](#input-trigger) list | A list of input trigger definitions, allowing a trigger on input ports to be translated into internal TimeSeq triggers. |
| `component-pool` | no | [component-pool](#component-pool) | A pool of reusable TimeSeq object definitions that can be referenced from elsewhere in the TimeSeq script. |

### Example
```js
{
    "type": "not-things_timeseq_script",
    "version": "1.0.0",
    "timelines": [
        { ... },
        { ... }
    ],
    "global-actions": [
        { ... },
        { ... }
    ],
    "input-triggers": [
        { ... },
        { ... }
    ],
    "component-pool": {
        ...
    }
}
```


## input-trigger
Identifies the input port that will be monitored for trigger inputs.

An input will be considered to have 'triggered' if it went from low voltage (0V) to high voltage (more then 1V). After triggering, the input must first return back to low voltage (0V) before it can be triggered again.

When an input has been triggered, the internal trigger (identified by the `id` property) will be set, which can then influence the running state of a [Lane](#Lane) if it uses that trigger as `start-trigger`, `restart-trigger` or `stop-trigger`.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The id of the trigger that will be set when the an input trigger is detected. |
| `input`| yes | [input](#input) | The *input* that will be monitored for input triggers. |

### Example
```json
{
    "id": "start-chord-sequence",
    "input": {
        "index": 5,
        "channel": 2
    }
}
```


## component-pool
A pool of reusable script components. Objects defined in this pool aren't added into the script directly from here, but can instead be referenced throughout the script using the [ref](#referencing) mechanism.

All objects defined in this pool **must** have an `id` property, since this will be used to reference them from within the script. It is allowed to use the same `id` value for different types of objects (e.g. for a *segment* and a *value*), but within one type of object, the `id` must be unique.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `segment-blocks` | no | [segment-block](#segment-block) list | A list of reusable *segment-block* objects. |
| `segments`| no | [segment](#segment) list | A list of reusable *segment* objects. |
| `inputs` | no | [input](#input) list | A list of reusable *input* objects. |
| `outputs` | no | [output](#output) list | A list of reusable *output* objects. |
| `calcs` | no | [calc](#calc) list | A list of reusable *calc* objects. |
| `values` | no | [value](#value) list | A list of reusable *value* objects. |
| `actions` | no | [action](#action) list | A list of reusable *action* objects. |

### Example
```json
{
    "component-pool": {
        "inputs": [
            {
                "id": "status-input",
                "index": 3,
                "channel": 5
            }
        ],
        "values": [
            { "id": "one-and-a-half", "voltage": 1.5 },
            { "id": "full", "voltage": 10 }
        ]
    }
}
```


## timeline
A timeline is the container for the sequencing definitions of a script. It groups together one or more [lane](#lane)s.

An optional [time-scale](#time-scale) property controls the timing calculations that will be performed for all *lane*s (and thus the duration of their *segment*s).

If there are looping *lane*s present in this timeline, the `loop-lock` property will define when the *lane*s loop: if `loop-lock` is enabled, any *lane* that reaches the end of its processing will not restart until all other *lane*s (looping or not-looping) have finished processing. Once all *lane*s have finished, those that should loop will loop together. If `loop-lock` is not enabled, any lane that finishes processing and is set to loop will do so immediately.

When running the script, each processing cycle will run through the *lane*s in the order that they appear in the `lanes` list.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `time-scale` | no | [time-scale](#time-scale) | The time scale that should be used when calculating durations in this timeline. |
| `loop-lock`| no | boolean | If `true`, *lane*s will only loop once all other *lane*s have completed. If `false`, *lane*s loop immediately when finished. Defaults to `false`. |
| `lanes` | yes | [lane](#lane) list | The *lane*s that contain the sequences for this timeline. |

### Example
```json
{
    "time-scale": {
        "bpm": 120
    }
    "loop-lock": true,
    "lanes": [
        { "segments": [ { "ref": "segment-1" } ] },
        { "segments": [ { "ref": "segment-2" } ] }
    ]
}
```


## time-scale
The *time-scale* object defines how certain timing calculations should be performed for a [timeline](#timeline). Although the properties of a *time-scale* are all optional, if a *time-scale* is added to a *timeline* then at least
one of `sample-rate` or `bpm` must be provided.

### sample-rate
Using samples is the most fine-grained scale to specify timing within TimeSeq. Since VCV Rack allows the active sample rate to be changed, the exact sample rate at which the script will be running may however not be known in advance. The `sample-rate` property allows you to specify that any *segment*s in this *timeline* that use `samples` for their `duration` have been configured to run at the specified sample rate. When parsing the script, TimeSeq will remap the provided duration of these *segment* so that they will last as long as they would have under the specified *sample-rate* E.g. if `sample-rate` is set to 48000, but VCV Rack is running a 96000 sample rate, a *segment* with a 250 samples duration will instead last 500 samples (since there are double the amount of samples per second).

Note that a *segment* can never be shorter then one sample. Sample rate recalculation can not make a segment shorter then one sample.

## Beats per minute and Beats per bar
In order to facilitate the definition of musical sequences, the *time-scale* allows the beats per minute and beats per bar to be defined for all *segments* in this *timeline* through the `bpm` and `bpb` properties. When a `bpm` value is set, all *segment*s in this *timeline* can now specify their duration using the `beats` property. If the number of beats in a bar has also been specified through the `bpb` property, *segment*s can also specify a duration in `bars`.

A `bpb` value can only be set if there is also a `bpm` value set.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `sample-rate` | no | unsigned number | The sample rate in which the `samples` duration of all *segment*s in the *timeline* are expressed. |
| `bpm`| no | unsigned number | The number of Beats per Minute to use for all *segment*s in the *timeline* that use a *beats* duration. |
| `bpb` | no | unsigned number | How many beats go into one bar for all *segment*s in the *timeline* that use a *bars* duration. Can only be set if `bpm` is also set. |

### Example
```json
"time-scale": {
    "sample-rate": 48000,
    "bpm": 120,
    "bpb": 4
}
```


## lane
A lane provides the core sequencing functionality of TimeSeq. The *lane* will activate the `segments` in the order that they appear in the list. Only one *segment* can be active within a *lane*, and when a *segment* completes, the *lane* will move on to the next *segment*.

Several properties control how a *lane* executes:
* `auto-start` defines if the *lane* should automatically be started when the script is started.
* `loop` defines if the *lane* should loop through the `segments`, activating the first *segment* again when the last *segment* of the list completes if looping is enabled.
* `repeat` defines how many times the `segments` in the *lane* should be repeated. A value of 0 or 1 mean that the list of `segments` will only be executed once. A value of 2 results in running through the list twice, 3 results in three iterations, etc. If `loop` is enabled for the *lane*, the `segments` will be repeated indefinitely, so the `repeat` property will have no impact anymore in that case.

The running state of a *lane* can be controlled using triggers:
* If a trigger matching the `start-trigger` property fires, and the *lane* is not currently running, it will be started. Any previous progress of the *lane* will be reset, and it will start again from the first *segment*. If the *lane* was already running when the trigger was received, the trigger will have no impact on the *lane* status or position.
* If a trigger matching the `restart-trigger` property fires, the *lane* will be restarted from the first *segment*. If the *lane* was paused or hadn't started yet, it will start running from the first *segment*. If the *lane* was already running, its position will be reset to that of the first *segment*.
* If a trigger matching the `stop-trigger` property fires, the *lane* will stop running if it is currently running. If it is not running, the trigger will have no impact.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `segments` | yes | [segment](#segment) list | The sequence of segments that will be executed for this *lane* |
| `auto-start` | no | boolean | If set to `true`, the *lane* will start automatically when the script is loaded. If set to `false` the *lane* will remain stopped when the script is loaded. Defaults to `true` |
| `loop`| no | boolean | If set to `true`, the *lane* will restart from the first segment once the last segment completes. Otherwise the lane will stop once the last segment completes. Defaults to `false` |
| `repeat` | no | unsigned number | Specifies how many times the *lane* should be executed before stopping. Has no impact if `loop` is set to `true`. Defaults to `0` |
| `start-trigger` | no | string | The name of the trigger that will cause this *lane* to start running. A start trigger on an already running *lane* has no impact on the state of the lane. Defaults to empty. |
| `restart-trigger` | no | string | The name of the trigger that will cause this *lane* to restart. A restart trigger on an inactive *lane* will cause it to start running. A restart trigger on a running *lane* will cause it to restart from the first *segment*. Defaults to empty.|
| `stop-trigger` | no | string | The name of the trigger that will cause this *lane* to stop running. A stop trigger on an an incative *lane* has no impact on the state of the lane. Defaults to empty. |
| `disable-ui` | no | boolean | If set to `true`, the *S* LED on the TimeSeq panel will light up when this lane loops. If set to `false`, a loop of this *lane* will not cause the *S* LED on the TimeSeq panel to light up. Defaults to `true`. |

### Example
```json
{
    "auto-start": true,
    "loop": true,
    "restart-trigger": "restart-chord-lane",
    "segments": [
        { "ref": "segment-1" },
        { "ref": "segment-2" }
    ]
}
```
