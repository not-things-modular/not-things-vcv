# TIMESEQ JSON SCRIPT
*Part of the set of [not-things VCV Rack](../README.md) modules.*

## Intro
- Processing in script order
- Triggers collected during run, become active on next cycle
- Variables immediately available after set

## Overview
![TimeSeq JSON Script high level view](./timeseq-json-high-level.png)

From a high-level view, a TimeSeq JSON script contains:
* One or more *Timeline*s. Each *Timeline* can contain a *TimeScale*, which affects how time is calculated within that timeline
* A *Timeline* contains one or more *Lane*s
* A *Lane* can loop or repeat, and may list the IDs of *Trigger*s that start, restart or stop it. A *Lane* can also be configured to auto-start
* Each *Lane* contains one or more *Segment*s
* A *Segment* has a *Duration* (in samples, milliseconds, beats/bars or hertz) and contains a list of *Action*s
* An *Action* can: 
    * Execute at the start or end of the segment or at the end,
    * Glide between a start and end value over the duration of the segment, transitioning smoothly over the duration of the segment,
    * Or generate a gate: outputting 10V during the first part of the segment and 0V in the second part.
* A *Script* can contain a list of *Global Actions*, which are executed when the script starts or resets (e.g. to initialize output polyphony)
* *Input Triggers* define which input ports (and channels) should produce a *Trigger* when transitioning from low to high voltage
* The *Component Pool* allows reusable definitions of objects to be created that can be referenced throughout the script. This avoids duplicating identical objects and can help with structuring complex script through the use of meaningful IDs.

## JSON Format
### Full object hierarchy
The full object hierarchy of the TimeSeq JSON script looks as follows:
* [script](#script)
    * [input-trigger](#input-trigger)
    * [component-pool](#component-pool)
    * [timeline](#timeline)
        * [time-scale](#time-scale)
        * [lane](#lane)
            * [segment](#segment)
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


### script
The root item of the TimeSeq JSON script.
#### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `type` | yes | string | Must be set to `not-things_timeseq_script` |
| `version`| yes | string | Identifies which version of the TimeSeq JSON script format is used. Currently only `1.0.0` is supported |
| `timelines` | no | [timeline](#timeline) list | An list of instances that will drive the sequencer |
| `global-actions` | no | [action](#action) list | A list of actions that will be executed when the script starts or is reset. Only actions which have their `timing` set to `START` are allowed |
| `input-triggers` | no | [input-trigger](#input-trigger) list | A list of input trigger definitions, allowing a trigger on input ports to be translated into internal TimeSeq triggers |
| `component-pool` | no | [component-pool](#component-pool) | A pool of reusable TimeSeq object definitions that can be referenced from elsewhere in the TimeSeq script |

#### Example
```
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


### input-trigger
Identifies an input port (and optionally the channel for polyponic inputs) that will be monitored for trigger inputs.

An input will be considered to have 'triggered' if it went from low voltage (0V) to high voltage (more then 1V). After triggering, the input must first return back to low voltage (0V) before it can be triggered again.

When an input has been triggered, the internal trigger (identified by the `id` property) will be set, which can then influence the running state of a [Lane](#Lane) if it uses that trigger as `start-trigger`, `restart-trigger` or `stop-trigger`.

#### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The id of the trigger that will be set when the an input trigger is detected |
| `input`| yes | [input](#input) | The input that will be monitored for input triggers |

#### Example
```
{
    "id": "start-chord-sequence",
    "input": {
        "index": 5,
        "channel": 2
    }
}
```


### component-pool
A pool of reusable script components. Objects defined in this pool aren't added into the script directly from here, but can instead be referenced throughout the script using the [ref](#referencing) mechanism.

All objects defined in this pool **must** have an `id` property, since this will be used to reference them from within the script. It is allowed to use the same `id` value for different types of objects (e.g. for a `segment` and a `value`), but within one type of object, the `id` must be unique.

#### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `segment-blocks` | no | [segment-block](#segment-block) list | A list of reusable `segment-block` objects. |
| `segments`| no | [segment](#segment) list | A list of reusable `segment` objects. |
| `inputs` | no | [input](#input) list | A list of reusable `input` objects. |
| `outputs` | no | [output](#output) list | A list of reusable `output` objects. |
| `calcs` | no | [calc](#calc) list | A list of reusable `calc` objects. |
| `values` | no | [value](#value) list | A list of reusable `value` objects. |
| `actions` | no | [action](#action) list | A list of reusable `action` objects. |

#### Example
```
{
    ...
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
        ],
		...
    }
    ...
}
```

### timeline
A timeline is the container for the sequencing definitions of a script. It groups together one or more [lane](#lane)s.

An optional [time-scale](#time-scale) property controls the timing calculations that will be performed for all `lane`s (and thus the duration of their `segment`s).

If there are looping `lane`s present in this timeline, the `loop-lock` property will define when the `lane`s loop: if `loop-lock` is enabled, any `lane` that reaches the end of its processing will not restart until all other `lane`s (looping or not-looping) have finished processing. Once all `lane`s have finished, those that should loop will loop together. If `loop-lock` is not enabled, any lane that finishes processing and is set to loop will do so immediately.

#### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `time-scale` | no | [time-scale](#time-scale) | The time scale that should be used when calculating durations in this timeline |
| `loop-lock`| no | boolean | If `true`, `lane`s will only loop once all other `lane`s have completed. If `false`, `lane`s loop immediately when finished. Defaults to `false` |
| `lanes` | yes | [lane](#lane) list | The `lane`s that contain the sequences for this timeline |

#### Example
```
{
    "time-scale": {
		...
	}
    "loop-lock": true,
    "lanes": [
        { ... },
        { ... }
    ]
}
```

## Referencing
- circular references
- segments duration and timeline time-scale
