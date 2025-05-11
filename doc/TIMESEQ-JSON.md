# TIMESEQ JSON SCRIPT
*Part of the set of [not-things VCV Rack](../README.md) modules.*

## Intro

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


### Script
The root item of the TimeSeq JSON script.
#### Properties
| property | required | type | description |
| --- | --- | --- | --- |
| `type` | yes | string | Must be set to `not-things_timeseq_script` |
| `version`| yes | string | Identifies which version of the TimeSeq JSON script format is used. Currently only `1.0.0` is supported |
| `timelines` | no | [timeline](#timeline) array | An array of instances that will drive the sequencer |
| <no-br>`global-actions`</no-br> | no | [action](#action) array | A list of actions that will be executed when the script starts or is reset. Only actions which have their `timing` set to `START` are allowed |
| <no-br>`input-triggers`</no-br> | no | [input-trigger](#input-trigger) array | A list of input trigger definitions, allowing a trigger on input ports to be translated into internal TimeSeq triggers |
| <no-br>`component-pool`</no-br> | no | [component-pool](#component-pool) | A pool of reusable TimeSeq object definitions that can be referenced from elsewhere in the TimeSeq script |

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