# TIMESEQ JSON SCRIPT FORMAT
*Part of the set of [not-things VCV Rack](../README.md) modules.*

## Full object hierarchy
The full object hierarchy of the TimeSeq JSON script looks as follows:
* [script](#script)
    * [input-triggers](#input-trigger)
    * global-[action](#action)s
    * [component-pool](#component-pool)
    * [timeline](#timeline)
        * [time-scale](#time-scale)
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
Lanes provide the core sequencing functionality of TimeSeq. A *lane* will activate the `segments` in the order that they appear in the list. Only one *segment* can be active within a *lane*, and when a *segment* completes, the *lane* will move on to the next *segment*.

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
| `disable-ui` | no | boolean | If set to `true`, the *L* LED on the TimeSeq panel will light up when this lane loops. If set to `false`, a loop of this *lane* will not cause the *L* LED on the TimeSeq panel to light up. Defaults to `true`. |

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


## segment
Segments provide the core timing functionality of TimeSeq. Through its `duration` property, the segment specifies how long it should last. And since [lane](#lane)s execute *segment*s in order one by one, this allows sequences with more complex timings.

The actual output of a *segment* is determined by its list of `actions`. Differnt types of *action*s exist, with their `timing` specifying when they should be executed (See [action](#action) and its sub-types for more details).

If the order that the actions is executed in is of importance (e.g. when writing and subsequently reading values), a segment groups the actions in three sets according to their timing: ***start*** actions, ***ongoing*** actions (with a `glide` or `gate` timing) and ***end*** actions. The processing order of the actions then becomes:
* If the segment is starting, first execute all **start*** actions (in the order that they appear in the list)
* Subsequently, execute all ***ongoing*** actions (in the order that they appear in the list)
* Finally, if the segment is ending, execute all ***end*** actions (in the order that they appear in the list)

The `segment-block` property provides a special version of a *segment*: if present, the `segment-block` must contain the ID of a *segment-block* in the `segment-blocks` section of the [component-pool](#component-pool). TimeSeq will then replace this *segment* instances with the *segment*s of the *segment-block*. The `segment-block` property can not be combined with the `duration` and `actions` properties within the same *segment* instance: either it is a stand-alone *segment* with a `duration` and `actions`, or it is a link to a `segment-block`.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `duration` | yes | [duratino](#duration) | Defines how long this segment will take to complete. |
| `actions`| no | [action](#action) list | The actions that will be executed as part of this segment. See the description above for details about the timings of actions. |
| `disable-ui` | no | boolean | If set to `true`, the *S* LED on the TimeSeq panel will light up when this *segment* starts. If set to `false`, a start of this *segment* will not cause the *S* LED on the TimeSeq panel to light up. Defaults to `true`. |
| `segment-block` | no | string | The ID of a [segment-block](#segment-block) in the [component-pool](#component-pool) that will take the place of this segment. Can not be combined with the `duration`, `actions` and `disable-ui` properties. |

### Example
#### A "regular" action
```json
{
    "duration": { "beats": 2 },
    "actions": [
		{ "timing": "start", "set-variable": { "name": "next-note", "value": { "voltage": 1.333 } } }
		{ "timing": "end", "set-output": { "output": { "index": 1 }, "value": { "voltage": 1.333 } } }
	]
}
```

#### A link to a *segment-block*
```json
{
    "segment-block": "three-notes-and-a-beat"
}
```


## duration
The duration section defines how long a [segment](#segment) will last. TimeSeq allows several units of time specification. Since TimeSeq works based on VCV Rack samples, all of these types of units will be converted into samples when loading the script.

Only one of the units can be used for a *segment* duration, except for `beats` and `bars`, which are related to each other and can thus be used together.

Note that the *duration* of a *segment* can not be shorter then one sample. If any of the unit conversions result in a *duration* that is shorter then one sample, the *segment* will last one sample instead. TimeSeq does allow fractional durations above that however (e.g. a segment can end up lasting 1.2 samples), and TimeSeq will keep track of these fractions to try and void drifts over time between different *lane*s.

### samples
Samples are the smalles time division in VCV Rack and thus in TimeSeq. The duration of a sample depends on the current sample rate of VCV Rack. E.g. if the current sample rate is 48Khz, there will be 48000 samples per second, and each sample will thus last 1/48000th of a second.

Since the sample rate of VCV Rack (and thus the length of a sample) may not be known in advance, TimeSeq allows sample durations to be "re-mapped": if the [time-scale](#time-scale) of the [timeline](#timeline) in which the *segment* appears has a `sample-rate` property, that sample rate will be used instead to calculate the duration of the *segment*.

E.g. if the *timeline* `sample-rate` is set to 48000, but VCV Rack is running at 96Khz, all `sample` values specified in the segment durations in that *timeline* will be multiplied by 2 (96000 / 48000 = 2). The end result will be that the segment will have the same duration independent of the actual VCV Rack sample rate.

### millis
Specifies the duration of a *segment* in milliseconds. Decimal values are allowed.

### beats and bars
If the [time-scale](#time-scale) of the [timeline](#timeline) in which the *segment* appears has a `bpm` configured, the duration of a *segment* can be expressed in `beats` relative to those Beats per Minute. Decimal values are allowed to express partials of beats (e.g. 8ths, 16ths, ...).

If there is also a `bpb` configured in the *time-scale*, an additional `bars` property is available relative to those Beats per Bar. `bars` can not be decimal, and a `bars` property can not be used if no `beats` property is present. The `beats` can be `0` however to specify that the *segment* last exactly the length of a (number of) bar(s).

### hz
A Hertz duration indicates how often the *duration* of the *segment* should fit within one second. E.g. if `hz` is set to 5, the sample will last 1/5th of a second and thus 200 milliseconds. This value can be a decimal.

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `samples` | no | unsigned number | The number of samples that the *segment* will last. Relative to the `sample-rate` of the [time-scale](#time-scale) of the current [timeline](#timeline), or to the active VCV Rack sample rate if none was specified on the *timeline*  |
| `millis`| no | unsigned float | Number of seconds that the *segment* will last |
| `beats` | no | unsigned float | Number of beats that the *segment* will last, Relative to the `bpm` of the [time-scale](#time-scale) of the current [timeline](#timeline) |
| `bars` | no | unsigned number | Number of bars that the *segment* will last, Relative to the `bpb` of the [time-scale](#time-scale) of the current [timeline](#timeline). Can not be used without `beats` |
| `hz` | no | unsigned float | Expresses the duration of the *segment* in Hertz, or fractions of a second |

### Examples
```json
{
    "millis": 1.25
}
```

```json
{
    "hz": 174.61
}
```

```json
{
    "beats": 0.25
}
```

```json
{
    "beats": 0,
	"bars": 2
}
```


## segment-block
A segment-block allows multiple segments to be grouped together so that they can easily be added in other places within the script.

The `segments` in the block will be executed in the order that they appear in the list. The `repeat` property allows the full list to be repeated a number of times.

`segments-blocks` are defined in the [component-pool](#component-pool), and used by referencing them by `id` in the `segment-block` property of a [segment](#segment).

### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `segments` | yes | [segment](#segment) list | The list of segments in this *segment-block* |
| `repeat`| no | unsigned number | The amount of times that the `segments` list should be repeated. |

### Example
```json
{
	"id": "segment-block-1",
    "segments": [
		{ "ref": "segment-1" },
		{ "ref": "segment-2" },
		{ "segment-block": "segment-block-2" },
		{ "ref": "segment-3" }
	],
	"repeat": 3
}
```
