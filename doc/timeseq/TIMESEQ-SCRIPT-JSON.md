# TIMESEQ JSON SCRIPT FORMAT

*Specification of the JSON script schema for the not-things [TimeSeq](../TIMESEQ.md) module.*

## Table of Contents

Since the TimeSeq JSON schema uses a nested object structure, following hierarchical object overview can be used as TOC for navigating around:

* [script](#script) - The root TimeSeq script object
  * [timeline](#timeline) - Container for grouping *lanes* that use the same *time-scale*
    * [time-scale](#time-scale) - Contains timing related settings
    * [lanes](#lane) - The core sequencing object of TimeSeq
      * [segments](#segment) - The core timing object of TimeSeq
        * [duration](#duration) - Identify the length of a *segment*
          * [variable-length duration](#variable-length-durations) - duration lengths based on *value*s
        * [segment-block](#segment-block) - Allow grouping of several *segment*s
        * [action](#action) - The core functional component of TimeSeq
          * [if](#if) - Allow conditional executing of *action*s
            * `eq`, `ne`, `lt`, `lte`, `gt`, `gte`, `and`, `or`
          * [set-value](#set-value) - Allow *action*s to set output voltages
            * [output](#output) - Identifies an output port and channel
            * [value](#value) - Evaluates to a voltage
              * [input](#input) - Identifies an input port and channel
              * [output](#output) - Identifies an output port and channel
              * [rand](#rand) - Generates a random voltage
              * [sequence value](#sequence-value) - Retrieves a value from a sequence and optionally moves the sequence position
              * [calc](#calc) - Allows mathematical calculations with *value*s
                * [tuning](#tuning)s - Quantization tunings
          * [set-variable](#set-variable) - Set an internal variable
            * [value](#value) - Evaluates to a voltage
          * [set-polyphony](#set-polyphony) - Sets the number of channels on an output port
          * [set-label](#set-label) - Sets the tooltip label of an output port
          * [assert](#assert) - Allows TimeSeq to be used as a test tool for other modules
            * expect ([if](#if)) - A condition that can trigger an assert
          * `trigger` - Fire an internal trigger
          * [move-sequence](#move-sequence) - Moves the position of a sequence
          * [add-to-sequence](#add-to-sequence) - Adds a value to a sequence
          * [remove-from-sequence](#remove-from-sequence) - Removes a value from a sequence
          * `clear-sequence` - Clears all values from a sequence
  * [input-triggers](#input-trigger) - Fire internal triggers based on external trigger signals
  * global-[action](#action) - *Action*s to perform during script start
  * [component-pool](#component-pool) - A pool of reusable JSON objects
    * [sequences](#sequence) - Sequences of values

## Versions

As new features are added, the `version` of the script is updated. The [script version](TIMESEQ-SCRIPT-VERSION.md) page gives an overview of these changes.

## JSON Schema

To facilitate easier editing of a TimeSeq JSON script, a JSON Schema definition is available for it. see the [Script JSON Reference](TIMESEQ-SCRIPT-JSON.md) page for the schema definition links for the different script versions.

## JSON Property types

Next to the JSON objects that are defined in this document, following property type definitions are used throughout the JSON format specification:

* **string**: a sequence of characters
* **boolean**: a value that can be either set to `true` or `false`
* **unsigned number**: A non-decimal number that is either 0 or positive
* **float**: A decimal number that can be negative, 0 or positive
* **unsigned float**: A decimal number that is either 0 or positive
* **list**: When combined with the name of a JSON object, defines that a list of one or more of those JSON objects is expected (e.g. a list of *segment*s). A list JSON is written as comma-separated list surrounded by square brackets (`[]`). E.g. a list of three strings is written as: `[ "string one", "string two", "string three" ]`

## script

The root item of the TimeSeq JSON script.

Sequencing is done by adding one or more *timeline* objects to the `timelines` list property. Each processing cycle of the script will go through the *timeline* objects in the order that they appear in this list (in case execution order is important for the processing of the sequences, e.g. when setting and reading variables).

The `global-actions` property allows specific *action*s to be executed when a script is loaded or reset (e.g. setting the number of channels on a polyphonic output). Only *action*s that have a `timing` set to `start` can be added to this list.

If there is a need to generate internal TimeSeq [triggers](TIMESEQ-SCRIPT.md#triggers) based on external trigger sources, the `input-triggers` property allows input ports to be set up for receiving external trigger signals.

In the `component-pool`, TimeSeq objects (*segment*s, *input*s, *output*s, *value*s, ...) can be defined that can then be referenced from elsewhere in the script. This allows a single object definition to be re-used in multiple parts of the script, and can allow better structuring of complex scripts through the usage of clear/descriptive IDs. See the [referencing](TIMESEQ-SCRIPT.md#referencing) section of the main TimeSeq script documentation file for more details.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `type` | yes | string | Must be set to `not-things_timeseq_script` |
| `version` | yes | string | Identifies which version of the TimeSeq JSON script format is used. Currently versions `1.0.0` and `1.1.0` are supported (see [this](TIMESEQ-SCRIPT-VERSION.md) page for features included in each version). |
| `$schema` | no | uri string | Allows JSON schema validation to be performed by schema-aware JSON editors. See the [script version](TIMESEQ-SCRIPT-VERSION.md) page for the schema URIs that can be used. The value given to this property will not influence TimeSeq parsing or processing itself. |
| `timelines` | no | [timeline](#timeline) list | A list of *timeline*s that will drive the sequencer. |
| `global-actions` | no | [action](#action) list | A list of *action*s that will be executed when the script loaded or is reset. Only *action*s which have their `timing` set to `start` are allowed in this list. |
| `input-triggers` | no | [input-trigger](#input-trigger) list | A list of input trigger definitions, allowing gate/trigger signals on input ports to be translated into internal TimeSeq [triggers](TIMESEQ-SCRIPT.md#triggers). |
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

## timeline

A timeline is a container for the sequencing definitions of a script. It groups together one or more [lane](#lane)s.

An optional [time-scale](#time-scale) property controls the timing calculations that will be performed for all *lane*s (and thus the duration of their *segment*s).

If there are looping *lane*s present in this timeline, the `loop-lock` property will define when the *lane*s loop: if `loop-lock` is enabled, any *lane* that reaches the end of its processing will not restart until all other *lane*s (looping or non-looping) have finished processing. Once all *lane*s have finished, those that should loop will loop together. If `loop-lock` is not enabled, any *lane* that finishes processing and is set to loop will do so immediately.

When running the script, each processing cycle will run through the *lane*s in the order that they appear in the `lanes` list.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `time-scale` | no | [time-scale](#time-scale) | The time scale that should be used when calculating durations of *segment*s in this timeline. |
| `loop-lock` | no | boolean | If `true`, *lane*s will only loop once all other *lane*s have completed. If `false`, *lane*s loop immediately when finished. Defaults to `false` if not set. |
| `lanes` | yes | [lane](#lane) list | The *lane*s that contain the *segment* sequences for this timeline. |

### Example

```json
{
    "time-scale": {
        "bpm": 120
    },
    "loop-lock": true,
    "lanes": [
        { "segments": [ { "ref": "segment-1" } ] },
        { "segments": [ { "ref": "segment-2" } ] }
    ]
}
```

## time-scale

The *time-scale* object defines how certain timing calculations should be performed for a [timeline](#timeline). Although the properties of a *time-scale* are all optional, if a *time-scale* is added to a *timeline* then at least one of `sample-rate` or `bpm` must be present.

### sample-rate

Using samples is the most fine-grained scale to specify timing within TimeSeq. However, since VCV Rack allows the active sample rate to be changed, the exact sample rate at which the script will be running may not be known in advance. The `sample-rate` property allows you to specify that any *segment*s in this *timeline* that use `samples` for their `duration` have been configured assuming that they are running at the specified sample rate. When parsing the script, TimeSeq will remap the provided duration of these *segment*s so that they will last as long as they would have under the specified *sample-rate* E.g. if `sample-rate` is set to 48000, but VCV Rack is running a 96000 (96Khz) sample rate, a *segment* with a 250 samples duration will instead last 500 samples (since there are double the amount of samples per second at 96Khz when compared to the expected 48Khz).

Note that sample rate recalculation can not make a segment shorter then one sample. If the sample duration recalculation of a *segment* goes below one sample, it will instead last one sample.

## Beats per Minute and Beats per Bar

In order to facilitate the definition of musical sequences, the *time-scale* allows the Beats per Minute and Beats per Bar to be defined for all *segment*s in this *timeline* through the `bpm` and `bpb` properties. When a `bpm` value is set, all *segment*s in this *timeline* can specify their duration using the `beats` property. If the number of beats in a bar has also been specified through the `bpb` property, *segment*s can also specify a duration in `bars`.

A `bpb` value can only be set if there is also a `bpm` value set.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `sample-rate` | no | unsigned number | The sample rate in which the `samples` duration of all *segment*s in the *timeline* are expressed. |
| `bpm` | no | unsigned number | The number of Beats per Minute to use for all *segment*s in the *timeline* that specify their duration using `beats`. |
| `bpb` | no | unsigned number | How many beats go into one bar for all *segment*s in the *timeline* that use a `bars` duration. `bpb` can only be set if `bpm` is also set. |

### Example

```json
"time-scale": {
    "sample-rate": 48000,
    "bpm": 120,
    "bpb": 4
}
```

## lane

Lanes provide the core sequencing functionality of TimeSeq. A lane will activate the `segments` that are assigned to it in the order that they appear in the list. Only one *segment* can be active at a time within a lane. When a *segment* completes, the lane will move on to the next *segment* in the list.

Several properties control how a lane executes:

* `auto-start` defines if the lane should automatically be started when the script is started.
* `loop` defines if the lane should keep looping the `segments`: if enabled, the first *segment* will be activated again when the last *segment* of the list completes.
* `repeat` defines how many times the `segments` in the lane should be repeated. A value of 0 or 1 mean that the list of `segments` will only be executed once. A value of 2 results in running through the list twice, 3 results in three iterations, etc. If `loop` is enabled for the lane, the `segments` will be repeated indefinitely, so the `repeat` property will have no impact anymore in that case.

The running state of a lane can be controlled using triggers:

* If a trigger matching the `start-trigger` property fires and the lane is not currently running, that lane will be started. Any previous progress of the lane will be reset, and it will start from its first *segment*. If the lane was already running when the trigger was received, the trigger will have no impact on the lane status or position.
* If a trigger matching the `restart-trigger` property fires, the lane will be restarted from its first *segment*. If the lane was paused or hadn't started yet, it will start running from the first *segment*. If the lane was already running, its position will be reset to that of the first *segment*.
* If a trigger matching the `stop-trigger` property fires and the lane is currently running, that lane will stop running. If it is not running, the trigger will have no impact.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `segments` | yes | [segment](#segment) list | The sequence of *segment*s that will be executed for this lane |
| `auto-start` | no | boolean | If set to `true`, the lane will start automatically when the script is loaded. If set to `false` the lane will remain stopped when the script is loaded. Defaults to `true` |
| `loop` | no | boolean | If set to `true`, the lane will restart from its first segment once its last segment has completed. Otherwise the lane will stop once its last segment completes. Defaults to `false` |
| `repeat` | no | unsigned number | Specifies how many times the *segment*s in the lane should be repeated before stopping the lane. Both values `0` and `1` mean that the segments are executed once. This property has no impact if `loop` is set to `true`. Defaults to `0` |
| `start-trigger` | no | string | The id of the internal trigger that will cause this lane to start running from its first segment. A start trigger on an already running lane has no impact on the state of that lane. Defaults to empty. |
| `restart-trigger` | no | string | The id of the internal trigger that will cause this lane to restart. A restart trigger on an inactive lane will cause it to start running. A restart trigger on a running lane will cause it to restart from the first *segment*. Defaults to empty. |
| `stop-trigger` | no | string | The id of the internal trigger that will cause this lane to stop running. A stop trigger on an an inactive lane has no impact on the state of that lane. Defaults to empty. |
| `disable-ui` | no | boolean | If set to `true`, the *L* LED on the TimeSeq panel will light up when this lane loops. If set to `false`, a loop of this lane will not cause the *L* LED on the TimeSeq panel to light up. Defaults to `true`. |

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

## input-trigger

Identifies that an input port that will be monitored for trigger input signals.

An input will be considered to have 'triggered' if it went from low voltage (0V) to high voltage (more then 1V). After being triggered, the input signal must first return back to low voltage (0V) before it can be triggered again.

When an input has been triggered, the internal trigger (identified by the `id` property) will be set, which can then influence the running state of a [Lane](#lane) if it uses that trigger id as `start-trigger`, `restart-trigger` or `stop-trigger`.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The id of the trigger that will be set when an input trigger is detected. |
| `input` | yes | [input](#input) | The *input* that will be monitored for input triggers. |

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

A pool of reusable script components. Objects defined in this pool aren't added into the script directly from here, but can instead be referenced throughout the script using the [ref](TIMESEQ-SCRIPT.md#referencing) mechanism.

All objects defined in this pool **must** have an additional `id` property, since this will be used to reference them from within the script. It is allowed to use the same `id` value for different types of objects (e.g. for a *segment* and a *value*), but within one type of object, the `id` must be unique.

See [referencing](TIMESEQ-SCRIPT.md#referencing) in the script overview page for more information about using re-usable components.

### Properties

| property | required | type | since | description |
| --- | --- | --- | --- | --- |
| `segment-blocks` | no | [segment-block](#segment-block) list | | A list of reusable *segment-block* objects. |
| `segments` | no | [segment](#segment) list | | A list of reusable *segment* objects. |
| `inputs` | no | [input](#input) list | | A list of reusable *input* objects. |
| `outputs` | no | [output](#output) list | | A list of reusable *output* objects. |
| `calcs` | no | [calc](#calc) list | | A list of reusable *calc* objects. |
| `values` | no | [value](#value) list | | A list of reusable *value* objects. |
| `actions` | no | [action](#action) list | | A list of reusable *action* objects. |
| `ifs` | no | [if](#if) list | | A list of reusable *if* objects. |
| `tunings` | no | [tuning](#tuning) list | *1.1.0* | A list of *tuning* objects that can be used in *quantize* *calc*s |
| `sequences` | no | [sequence](#sequence) list | *1.2.0* | A list of *sequence*s that can be used in [sequence values](#sequence-value) |

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
        ],
        "sequences": [
            {
                "id": "c-major-pentatonic",
                "values": [ "c4", "d4", "e4", "g4", "a4" ]
            }
        ]
    }
}
```

## segment

Segments provide the core timing functionality of TimeSeq. Through its `duration` property, a segment specifies how long it should last. And since [lane](#lane)s execute their *segment*s in order one by one, this allows sequences with more complex timings to be created.

The actual output of a *segment* is determined by its list of `actions`. Different types of *action*s exist, with their `timing` specifying when they should be executed (See [action](#action) and its sub-types for more details).

Depending on the script, the order that the actions are executed in can be of importance (e.g. when writing and subsequently reading variables). This execution order follows a predefined logic. A segment first groups the *action*s in three sets according to their timing: ***start*** actions, ***ongoing*** actions (with a `glide` or `gate` timing) and ***end*** actions. In each processing cycle, the processing order of these actions then becomes:

* If the segment is starting, first execute all **start** actions (in the order that they appear in the original action list)
* Subsequently, execute all ***ongoing*** actions (in the order that they appear in the original action list)
* Finally, if the segment is ending, execute all ***end*** actions (in the order that they appear in the original action list)

### segment-block segments

The `segment-block` property allows for a special version of a *segment*: if present, the `segment-block` property must contain the ID of a *segment-block* in the `segment-blocks` section of the [component-pool](#component-pool). The *segment*s of that *segment-block* will then take the place of the original *segment*, as if they were added inline. The `segment-block` property can not be combined with the `duration` property within the same *segment* instance: either it is a stand-alone *segment* with a `duration` and `actions`, or it is a link to a `segment-block`.

The `actions` property can still be used together with the `segment-block` property, but in that case, it can only contain *action*s with a `start` or the `end` `timing`. In this case, the `start` actions will be executed before the first segment of the *segment-block* starts. The `end` actions will be executed when the last segment of the *segment-block* has completed. If the *segment-block* has a `repeat` value configured, the *action*s will not be executed each time the *segment-block* repeats. They will only execute at the start of the first repeat, and after the last repeat has completed.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `duration` | yes | [duration](#duration) | Defines how long this segment will take to complete. |
| `actions` | no | [action](#action) list | The actions that will be executed as part of this segment. See the description above for details about the timings of actions. |
| `disable-ui` | no | boolean | If set to `true`, the *S* LED on the TimeSeq panel will light up when this *segment* starts. If set to `false`, a start of this *segment* will not cause the *S* LED on the TimeSeq panel to light up. Defaults to `true`. |
| `segment-block` | no | string | The ID of a [segment-block](#segment-block) in the [component-pool](#component-pool) that will take the place of this segment. Can not be combined with the `duration`, and `disable-ui` properties. |

### Example

#### A "regular" action

```json
{
    "duration": { "beats": 2 },
    "actions": [
        { "timing": "start", "set-variable": { "name": "next-note", "value": { "voltage": 1.333 } } },
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

The duration section defines how long a [segment](#segment) will last. TimeSeq allows the duration to be expressed in several types of units: samples, milliseconds, beats & bars or Hertz. Only one of these units can be used at a time for a *segment* duration, except for `beats` and `bars`, which can be used together.

Since TimeSeq works based on VCV Rack samples, all of these types of units will be converted into samples once the script is loaded. The final converted *duration* of a *segment* can not be shorter then one sample. If any of the unit conversions result in a *duration* that is shorter then one sample, the *segment* will last one sample instead. TimeSeq does allow fractional durations above that however (e.g. a segment can end up lasting 1.2 samples), and TimeSeq will keep track of these fractions to try and avoid drifts over time between different *lane*s.

### samples

Samples are the smallest time division in VCV Rack and thus in TimeSeq. The duration of a sample depends on the current sample rate of VCV Rack. E.g. if the current sample rate is 48Khz, there will be 48000 samples per second, and each sample will thus last 1/48000th of a second.

Since the sample rate of VCV Rack (and thus the length of a sample) may not be known in advance, TimeSeq allows sample durations to be "re-mapped": if the [time-scale](#time-scale) of the [timeline](#timeline) in which the *segment* appears has a `sample-rate` property, that sample rate will be used instead to calculate the duration of the *segment*.

E.g. if the *timeline* `sample-rate` is set to 48000, but VCV Rack is running at 96Khz, all *segment*s in that *timeline* that express their *duration* with a `sample` value will have this value multiplied by 2 (96000 / 48000 = 2). The end result will be that the segment will have the same duration (in absolute time) independent of the actual VCV Rack sample rate.

### millis

Specifies the duration of a *segment* in milliseconds. Decimal values are allowed.

### beats and bars

If the [time-scale](#time-scale) of the [timeline](#timeline) in which the *segment* appears has a `bpm` configured, the duration of a *segment* can be expressed in the number of `beats` relative to those Beats per Minute. Decimal values are allowed so that partials of beats (e.g. 8ths, 16ths, ...) can be expressed.

If there is also a `bpb` configured in the *time-scale*, an additional `bars` property is available relative to those Beats per Bar. `bars` can not be decimal, and a `bars` property can not be used if no `beats` property is present, though the `beats` property can be set to `0` to specify that the *segment* last exactly the length of a (number of) bar(s).

### hz

A Hertz duration indicates how often the *duration* of the *segment* should fit within one second. E.g. if `hz` is set to 5, the sample will last 1/5th of a second and thus 200 milliseconds. This value can be a decimal.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `samples` | no | unsigned number | The number of samples that the *segment* will last. Relative to the `sample-rate` of the [time-scale](#time-scale) of the current [timeline](#timeline), or to the active VCV Rack sample rate if none was specified on the *timeline* |
| `millis` | no | unsigned float | Number of milliseconds that the *segment* will last |
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

### Variable-length durations

Instead of using a fixed numeric value for the duration (e.g. `"beats": 0.25`), it is also possible to use a [value](#value) for the `samples`, `millis`, `beats` or `hz` properties. This allows the duration of the segment to vary based on the evaluated voltage of the *variable*. Each time a *segment* starts, if the duration is set to a *value* object instead of a fixed numeric, that *value* will be evaluated and the result will be used as duration *segment*. The next time that *segment* is executed, the *value* will be re-evaluated and may result in a different duration.

Since the [action](#action)s of a *segment* can have an impact on the resulting voltage of a *value*, a fixed order of operations is followed:

* First all actions of the *segment* with a `start` timing will be executed
* Then the *duration* of the *segment* is calculated
* Then the `glide` and `gate` actions are executed
* Finally, when the segment ends, the `end` actions are executed

This means that `start` actions can be used in a segment to influence the *duration* of that segment. However, once this *length* has been determined, the remaining actions (`glide`, `gate` or `end`) will not influence it anymore, even if they modify properties that the original *duration* calculation was based on.

Just like constant-length durations, a variable-length durations can not be made shorter then one sample. If the *duration* evaluates to a length shorter then one sample, it will be made to last one sample instead. Fractional sample durations (longer then 1 sample) are supported however, resulting in drift compensation by TimeSeq over time.

When using a *value*-based `beats` duration, it is not possible to also specify a `bars` property.

*Variable-length durations were introduced in TimeSeq script version 1.1.0.*

### Examples of Variable-length Durations

The duration is expressed in `beats`, its length is determined by the voltage of input 2:

```json
{
    "beats": { "input": 2 }
}
```

The duration is expressed in `millis`, which is set to the current value of the `my-segment-duration` variable that gets truncated (i.e. the decimal part removed, only the whole value is used)

```json
{
    "millis": {
        "variable": "my-segment-duration",
        "calc": { "trunc": true }
    }
}
```

## segment-block

A segment-block allows multiple segments to be grouped together so that they can easily be added in other places within the script. The *segment*s in a segment-block can be full inline *segment*s, references to re-usable *segment*s (using the `ref` property) or references to other segment-block instances.

The `segments` in the block will be executed in the order that they appear in the list. The `repeat` property allows the full list to be repeated a number of times.

`segments-blocks` are defined in the [component-pool](#component-pool), and used by referencing them by `id` in the `segment-block` property of a [segment](#segment).

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `segments` | yes | [segment](#segment) list | The list of segments in this *segment-block* |
| `repeat` | no | unsigned number | The amount of times that the `segments` list should be repeated. |

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

## action

Actions provide the functional core of a TimeSeq script. When executed in a *segment*, actions can change output voltages, change output polyphony, set variables or fire internal triggers.

An optional `if` property is available on actions that allows their execution to be made optional. If the condition specified by the [if](#if) is met, the action will be executed. If not, the action will be skipped.

The `timing` property of an *action* identifies when the *action* will be executed by the *segment* that contains it. Based on the `timing` property, three major types of actions can be identified:

* Actions that execute once, either at the start of the segment (`start` timing), or when the segment ends (`end` timing),
* Actions that glide from a start value to an end value for the full duration of the segment (`glide` timing).
* Actions that generate a gate signal while the segment is running (`gate` timing)

The order in which the actions of a *segment* are executed is described in the [segment](#segment) section.

### Start and End actions

Actions that have a `start` or an `end` `timing` will be executed once at the appropriate time in the *segment* (i.e. when it starts or ends). If an `if` property is present on the action, the condition of that [if](#if) will be evaluated at the time that the *segment* tries to execute the action. If the condition of the *if* is not met, the action will not be executed.

`start` and `end` actions can perform a number of operations, each with their own appropriate properties:

* Set a voltage on an output port
* Set the polyphony of an output port
* Set the label of an output port
* Set a variable
* Perform an assert
* Fire an internal trigger (see the [triggers](TIMESEQ-SCRIPT.md#triggers) section on the script overview page for more details)
* Move the current position of a shared [sequence](#sequence)
* Add a [value](#value) to a sequence
* Remove a value from a sequence
* Clear (remove) all values from a sequence

Each action must contain exactly one of these operation. If multiple operations need to be performed, separate actions will have to be created for each of them.

Except for the `trigger` action, all the action operations have their own *action* property that will contain a sub-object with the appropriate parameters for that operation. Since `trigger` action only needs to identify the ID of the trigger that has to be fired, a plain string that identifies that trigger ID will be sufficient.

#### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `timing` | no | string | Identifies the timing when this action will be executed. Can be either `start` or `end`. Defaults to `start`. |
| `set-value` | no | [set-value](#set-value) | Sets a voltage on an output port. |
| `set-polyphony` | no | [set-polyphony](#set-polyphony) | Sets the polyphony of an output port. |
| `set-label` | no | [set-label](#set-label) | Sets the tooltip label of an output port. |
| `set-variable` | no | [set-variable](#set-variable) | Sets a variable. |
| `assert` | no | [assert](#assert) | Performs an assert. |
| `trigger` | no | string | Fires an internal trigger with the specified id. |
| `move-sequence` | no | [move-sequence](#move-sequence) | Moves the current position of a sequence. |
| `add-to-sequence` | no | [add-to-sequence](#add-to-sequence) | Adds a value to a sequence. |
| `remove-from-sequence` | no | [remove-from-sequence](#remove-from-sequence) | Removes a value from a sequence. |
| `clear-sequence` | no | string | Removes all `values` from the sequence with the specified identifier. |
| `if` | no | [if](#if) | A condition that must be met in order for the action to be executed. |

#### Examples

```json
{
    "timing": "start",
    "set-value": {
      "value": { "voltage": 6.9 },
      "output": { "index": 4, "channel": 2 }
    }
}
```

```json
{
    "timing": "end",
    "set-variable": {
      "name": "my-first-variable",
      "value": { "voltage": 1.23 }
    }
}
```

```json
{
    "timing": "end",
    "trigger": "trigger-next-sequence"
}
```

```json
{
    "timing": "start",
    "clear-sequence": "my-sequence"
}
```

### Glide actions

Actions with a `glide` timing gradually move from one value to another over the duration of a *segment*. The `start` *value* of the action will define at which voltage the glide starts, and the `end` *value* identifies the voltage the action will reach at the end of the *segment*.

Using the `ease-factor` property, it is possible to influence the rate at which the glide moves from the `start` *value* to the `end` *value*. A positive `ease-factor` will cause the change to start slow and speed up towards the end, while a negative one will cause it to start changing quickly and ease out towards the end. If no `ease-factor` is specified (or it is set to zero), the glide will be executed in a linear fashion.
The calculation of the easing factor supports two algorithms: using sigmoid function (`sig`) or based on power calculations (`pow`). The arc resulting from these algorithms differs slightly. By default, the `sig` algorithm will be used since it is less CPU intensive.

Just like with other actions, a glide action can be made conditional by including an `if` property so that the action will only be executed if the [if](#if) condition is met.

The exact values used for the `start`, `end` and `if` properties will be calculated when the *segment* that contains the glide action is started. They will not be re-evaluated while the *segment* is running, so any change that could influence these value calculations that happens while the *segment* is running will not be taken into account anymore. For example, if the action was determined to be disabled due to its `if` condition when the *segment* started, the action will not become active afterwards while the *segment* is running parameter values for that `if` condition change in between. If the *segment* is started again at a later time however (e.g. due to a looping *lane*), all values for the action will be re-evaluated when the *segment* restarts.

A glide action has two possible targets to send its generated voltages to: either change an [output](#output) voltage or set a variable that can be used in other areas of the script. Only one of these targets can be used per glide action. A glide action that is executed will update the voltage of its target (the *output* or the *variable*) in each processing cycle (i.e. at the active sample rate of VCV Rack).

#### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `start-value` | yes | [value](#value) | The *value* that the action should start the glide from. |
| `end-value` | yes | [value](#value) | The *value* that the action should glide towards. |
| `ease-factor` | no | float | Controls the rate at which the action will move from the `start` value to the `end` value. Must be between -5 and 5. Defaults to 0 |
| `ease-algorithm` | no | string | The algorithm to use for easing calculations. Can be either `sig` or `pow`. Defaults to `sig` |
| `output` | no | [output](#output) | The output port to which the calculated value should be sent |
| `variable` | no | string | The name of the variable that should be set based on the calculated value of the action. |
| `if` | no | [if](#if) | A condition that must be met in order for the action to be executed. |

#### Example

```json
{
    "timing": "glide",
    "start-value": { "voltage": -3 },
    "end-value": { "variable": "glide-end-voltage" },
    "output": { "index": 9, "port": 6 },
    "if": { "ne": [
        { "voltage": 0 },
        { "variable": "glide-condition" }
    ] }
}
```

### Gate actions

An action with the `gate` timing can be used to generate a gate signal on one of the output ports: it will set the output port voltage to 10v when the action starts, and will change it to 0v as the action progresses. By default, the change to 0v will be done when the *segment* that contains the action has completed half of its *duration*. The `gate-high-ratio` property can be used to change this position, with `0` moving it to the start of the *segment*, `1` moving it to the end of the *segment* and `0.5` matching the halfway point of the *segment* *duration*.

The voltage of a gate action must always be sent to an *output* port.

Just like the other action types, a gate action can be made conditional using an [if](#if) property.

#### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `output` | yes | [output](#output) | The output port that will receive the gate signal |
| `gate-high-ratio` | no | unsigned float | The position when the gate signal should go from high to low. Must be a value between `0` and `1`, with `0.5` aligning with half of the *segment* duration. Defaults to `0.5` |
| `if` | no | [if](#if) | A condition that must be met in order for the action to be executed. |

#### Example

```json
{
    "timing": "gate",
    "gate-high-ratio": 0.75,
    "if": { "ne": [
        { "voltage": 0 },
        { "variable": "gate-condition" }
    ] }
}
```

## if

The *if* object provides conditional functionality for *action*s and *assert*s either by comparing two [value](#value)s with each other or by checking the result of two child if conditionals using logical operators. A condition can either evaluate to *true* or *false*. The result of the condition can then enable or disable an [action](#action), or trigger the [assert](#assert) that the *if* was used in.

Each *if* object must contain exactly one comparison or logical operator.

### Comparison operators

Using one of the following properties in the *if* object will cause the matching comparison to be performed:

* `eq`: returns *true* if two *value*s are equal (with an optional `tolerance`),
* `ne`: returns *true* if two *value*s are not equal (with an optional `tolerance`),
* `lt`: returns *true* if the first provided *value* is less than the second provided *value*,
* `lte`: returns *true* if the first provided *value* is less than or equal to the second provided *value*,
* `gt`: returns *true* if the first provided *value* is greater than the second provided *value*,
* `gte`: returns *true* if the first provided *value* is greater than or equal to the second provided *value*.

The value of the comparison property must be set to an array of exactly two [value](#value) objects. When the *if* is triggered, the current voltage of the provided *value*s will be calculated, and the results will be compared with each other.

For the `eq` and the `ne` comparisons, an optional `tolerance` can be can be provided. If provided, the two *value*s will be considered equal (or not equal) if the difference between the two *value*s falls within that tolerance. Since voltage calculations within VCV Rack can result in small rounding errors, providing a small `tolerance` value may be needed when comparing two *value*s for (in)equality.

#### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `eq` | no | [value](#value) array with 2 *value*s | Checks that the two provided *value*s are equal, with an optional `tolerance` |
| `ne` | no | [value](#value) array with 2 *value*s | Checks that the two provided *value*s are not equal, with an optional `tolerance` |
| `lt` | no | [value](#value) array with 2 *value*s | Checks that the first provided *value* is less than the second. |
| `lte` | no | [value](#value) array with 2 *value*s | Checks that the first provided *value* is less than or equal to the second. |
| `gt` | no | [value](#value) array with 2 *value*s | Checks that the first provided *value* is greater than the second. |
| `gte` | no | [value](#value) array with 2 *value*s | Checks that the first provided *value* is greater than or equal to the second. |
| `tolerance` | no | unsigned float | Specifies how much the two *value*s are allowed to differ while still being considered equal (for an `eq` operator) or not equal (for an `ne` operator). |

#### Examples

Some examples of *if* usage with comparison operators within an action:

```json
{
    "if": {
        "eq": [
            { "voltage": 4.2 },
            { "variable": "my-input-variable"}
        ],
        "tolerance": 0.00001
    },
    "set-variable": { "my-output-variable": 6.9 }
}
```

```json
{
    "if": {
        "lt": [
              { "voltage": 4.2 },
              { "variable": "my-input-variable"}
        ]
    },
    "set-variable": { "my-output-variable": 6.9 }
}
```

### Logical operators

A logical operator allows two child *if*s to be combined. Following logical operators can be used:

* `and`: returns *true* if the two child *if*s both evaluate to *true*,
* `or`: returns *true* if at least one of the child *if*s evaluates to *true*

The value of the logical operator property must be set to an array of exactly two child *if* objects. More complex conditional can be constructed by nesting multiple levels of logical *if* operators.

#### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `and` | no | [if](#if) array with 2 *if*s | Checks that the two provided *if*s both evaluate to *true* |
| `or` | no | [if](#if) array with 2 *if*s | Checks that at least one of the provided *if*s evaluates to *true* |

#### Examples

A single-level logical *if* in an action:

```json
{
    "if": {
        "and": [
            {
                "eq": [
                    { "voltage": 4.2 },
                    { "variable": "my-input-variable-1" }
                ]
            },
            {
                "gt": [
                    { "voltage": 3.45 },
                    { "input": { "index": 6 } }
                ]
            }
        ]
    },
    "set-variable": { "my-output-variable": 9.9 }
}
```

An `and` logical operator with a child `or` logical operator as first child conditional

```json
{
    "if": {
        "and": [
            {
                "or": [
                    {
                        "eq": [
                            { "voltage": 2.1 },
                            { "variable": "my-input-variable-1" }
                        ]
                    },
                    {
                        "eq": [
                            { "voltage": 4.2 },
                            { "variable": "my-input-variable-1" }
                        ]
                    }
                ]
            },
            {
                "gt": [
                    { "voltage": 3.45 },
                    { "input": { "index": 6 } }
                ]
            }
        ]
    },
    "set-variable": { "my-output-variable": 9.9 }
}
```

## set-value

The *set-value* is used within an [action](#action) to update the voltage of one of the TimeSeq outputs.

The voltage to use is determined by the `value` property, while the port (and channel) on which the voltage should be updated is determined by the `output` property.

The voltage will be immediately assigned to the [output](#output) as part of the executed action, so any subsequent [value](#value) in the script that references that *output* will receive the updated voltage once the set-value was performed.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `value` | yes | [value](#value) | The value that will determine the voltage to use. |
| `output` | yes | [output](#output) | The output port (and channel) to which the voltage should be applied. |

### Example

An example of a set-value within an action:

```json
{
    "timing": "end",
    "set-value": {
        "value": { "voltage": 5.6 },
        "output": { "index": 7, "channel": 8 }
    }
}
```

## set-variable

The *set-variable* is used within an [action](#action) to update an internal TimeSeq variable that can then be referenced by other [value](#value)s within the script.

The voltage to use is determined by the `value` property, while the `name` property determines the name of the variable that should be updated.

When a variable is set to a voltage using a *set-variable* *action*, that variable will keep that voltage value as long as the script keeps running. Pausing and resuming a script will not clear existing variables. Resetting a script, loading a new script or restarting VCV Rack will cause existing variables to be removed/cleared.

Since unknown variables will default to 0V, setting a variable to 0V will be the same as removing that variable from the list of currently known variables.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `value` | yes | [value](#value) | The value that will determine the voltage to use. |
| `name` | yes | string | The name of the variable to which the voltage should be assigned. |

### Example

An example of a set-variable within an action:

```json
{
    "timing": "start",
    "set-variable": {
        "value": { "voltage": 3.14 },
        "name": "a-piece-of-pi"
    }
}
```

## set-polyphony

The *set-polyphony* is used within an [action](#action) to update the number of polyphonic channels on an output port.

The port on which the number of channels should be updated is determined by the `index` property. The number of channels that the port should have is determined by the `channels` property. Setting the number of channels to `1` will make the port monophonic. A port can have up to `16` channels.

The output ports can be addressed by their number label as it is visible on the UI, so the `index` property can go from `1` up to (and including) `8`

When changing the number of channels on an output port, the voltages that were previously assigned to channels of that output will be remembered. Lowering the number of channels of a port and subsequently increasing the count again will not clear previously assigned voltages.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `index` | yes | unsigned number [1-8] | The output port on which the number of channels should be updated. |
| `channels` | yes | unsigned number [1-16] | The number of channels that should be available on the output port. |

### Example

An example of a set-polyphony within an action:

```json
{
    "timing": "start",
    "set-polyphony": {
        "index": 3,
        "channels": 12
    }
}
```

## set-label

The *set-label* is used within an [action](#action) to update text label of the output port in the tooltip when moving the mouse over the port. This is a purely aesthetic action to allow easier identification of output ports in the VCV Rack UI. Since setting the label of an output port in VCV Rack requires memory allocations (i.e. has a minor performance overhead), this action should not be executed repeatedly in a script. This action is intended to be used during script setup (e.g. in the *global-actions* of the [script](#script) element).

The port on which the label should be updated is determined by the `index` property. The `label` property should contain the text to use.

The output ports can be addressed by their number label as it is visible on the UI, so the `index` property can go from `1` up to (and including) `8`

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `index` | yes | unsigned number [1-8] | The output port on which the label should be updated. |
| `label` | yes | string | The label to assign to the output port. |

### Example

An example of a set-label within an action:

```json
{
    "timing": "start",
    "set-label": {
        "index": 5,
        "label": "My Fifth Script Output"
    }
}
```

## assert

Asserts allow TimeSeq to be used as a module to test the behaviour of other modules. The assert action allows a check to be performed on an [if](#if) condition, and if that condition is not met, it will result in an assert warning becoming active on the module UI. By writing a script that sends varying voltages through the TimeSeq [output](#output)s to another module, and then sending the output of that other module back into the [input](#input)s of TimeSeq, the assert action can subsequently verify that the other module behaved as expected.

The `expect` property will identify the [if](#if) condition that will be checked. If this condition evaluates to *false*, an assert with the specified `name` will be triggered on TimeSeq. The `stop-on-fail` property specifies if the occurrence of a failed assert should also pause TimeSeq, or if the script should continue running.

While the assert is mainly intended to be used to verify voltages on the input ports of TimeSeq, the `expect` condition can be used to compare any types of *value*s.

See [assert](TIMESEQ-UI-PANEL.md#asserts) for more details on how they are handled in the UI.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `expect` | yes | [if](#if) | The condition that must evaluate to *true*. If it evaluates to *false*, an assert will be triggered on TimeSeq. |
| `name` | yes | string | The name of the assert that will be triggered on TimeSeq if the condition evaluates to *false*. |
| `stop-on-fail` | no | boolean | When set to `true`, TimeSeq will pause the script execution if this assert is fired. Defaults to `true`. |

### Example

An example of a set-value within an action:

```json
{
    "timing": "start",
    "assert": {
        "expect": {
            "lt": [
                { "voltage": 4.2 },
                { "input": { "index": 3 }}
            ],
            "name": "input 3 to high"
        }
    }
}
```

## move-sequence

The *move-sequence* action allows the position of a [sequence](#sequence) to be moved, changing the active item within the sequence. This action can only be used with `shared` sequences since non-shared sequences have a different active position for each [sequence value](#sequence-value) that uses the sequence.

When using the `direction` property, the position of the sequence can be moved relatively to the current position: `forward`, `backward` or `random`. The `wrap` property can be used to specify if wraparound behaviour should be used when reaching the start or end of the sequence.

Absolute movements in the sequence can be done using the `position` property, which allows the active *value* to be set by index. Note that the `position` property uses zero-based indexing of the elements, so position `0` is the first value, position `1` is the second value, position `2` is the third value, etc. Using a negative `position` or a value that's bigger then the number of values in the sequence will cause the last value in the sequence to become the active value.

One of `direction` or `position` must be specified for a *move-sequence* and they can't be used together within the same *move-sequence*.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The identifier of the shared [sequence](#sequence) that should be moved. |
| `direction` | no | string | The direction in which to move the sequence. Can be either `forward`, `backward` or `random`. Either this property or `position` must be specified. |
| `wrap` | no | boolean | Indicates if wraparound behaviour should be used when performing a `direction` move. Defaults to `true`. |
| `position` | no | number | The zero-based index of the item to move the sequence to or a negative value to activate the last item. Either this property or `direction` must be specified. |

### Example

Moving by direction without wraparound behaviour:

```json
{
    "timing": "start",
    "move-sequence": {
        "id": "my-sequence",
        "direction": "backward",
        "wrap": false
    }
}
```

Moving by absolute position:

```json
{
    "timing": "start",
    "move-sequence": {
        "id": "my-sequence",
        "position": 3
    }
}
```

Moving to the last item of the sequence:

```json
{
    "timing": "start",
    "move-sequence": {
        "id": "my-sequence",
        "position": -1
    }
}
```

## add-to-sequence

The *add-to-sequence* action allows values to be added to the list of values of a [sequence](#sequence).

The `value` property identifies which [value](#value) to add to the sequence. Any of the available value types (*constant voltage*, *input*, *output*, ...) can be used, either inline or by reference.

The `position` property identifies where to add the `value` in to the sequence using a zero-based index: `0` inserts the `value` as first element of the sequence `values` list, `1` as second, `2` as third, etc. Any values that are in the sequence after that position will be moved one position towards the back. If no `position` is supplied, a negative `position` or a `position` that is bigger then the number of current values in the sequence, the new value will be added to the end of the list of `values`.

By default, the new `value` will be added as-is to the sequence: each time the value is used, it's voltage will be re-evaluated (e.g. retrieve the voltage of an input port for an [input](#input) value or generate a new random value for a [rand](#rand) value). However, if the `as-constant-voltage` property is set to `true`, the current voltage of the `value` will be determined when it is added the the sequence, and that pre-determined voltage will be used each time when this value becomes active within the sequence instead of re-evaluating the voltage each time.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The identifier of the [sequence](#sequence) to which a *value* should be added. |
| `value` | yes | [value](#value) | The value to add to the *sequence*. |
| `position` | no | number | The zero-based index of the position at which the item should be added, or a negative number to add it to the end of the *sequence*. Defaults to `-1`. |
| `as-constant-voltage` | no | boolean | Allows the current value to be inserted as a constant voltage, instead of having the voltage of the `value` be re-determined each time it is used. Defaults to `false`. |

### Example

An example of a set-label within an action:

```json
{
    "timing": "start",
    "set-label": {
        "index": 5,
        "label": "My Fifth Script Output"
    }
}
```

## remove-from-sequence

Removes a value from a [sequence](#sequence).

The value to remove from the *sequence* is identified by it's zero-based `position`. A value of `0` will remove the first value, `1` will remove the second, `2` will remove the third, etc. If the position is outside of the bounds of the list of values for the *sequence*, nothing will happen.

If `position` is set to a negative value, the last value of the *sequence* will be removed. If the *sequence* is already empty, nothing will happen.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The identifier of the [sequence](#sequence) from which a *value* should be removed. |
| `position` | no | number | The zero-based position index of the value that should be removed, or a negative number to remove the last value from the sequence. Defaults to `-1`. |

### Example

```json
{
    "timing": "start",
    "remove-from-sequence": {
        "id": "my-sequence",
        "position": 0
    }
}
```

## value

Throughout the TimeSeq script, whenever a voltage is needed, a value is used to provide different ways to determine that voltage value:

* A constant voltage, either using an exact voltage number or using a note value that will be translated into its corresponding 1V/Oct voltage
* Using a variable that was previously set using a [set-variable](#set-variable) *action*
* Reading the current voltage from an [input](#input) port
* Reading the current voltage from an [output](#output) port
* Using a [rand](#rand)om voltage generator
* Retrieving a [sequence value](#sequence-value) from a [sequence](#sequence)

Since `voltage` values are usually expected to be between -10V and 10V, the constant `voltage` value will by default be limited to this range. In some scenarios (e.g. when specifying *segment* lengths in variable-length [duration](#duration)s), there may be a need to specify a constant value outside this range. The range check on a `voltage` value can be disabled by setting the `no-limit` property of a value to `true`.

When the `note` property is used, it must be a 2 or 3 character string, where the first character specifies the note name (A-G), the second specifies the octave (0-9) and the third (optional) character can either use `+` to indicate a sharp, or `-` to indicate a flat. E.g: `C4+` will result in the 1V/Oct value of a middle C#, while a `A3-` will result in an A flat below middle C.

If a `variable` property is used and no variable with a matching name was previously set using a [set-variable](#set-variable) *action, 0V will be used instead.

When a `sequence` property is used, the retrieval of the value will also move the position within that sequence (see [sequence value](#sequence-value) for more details).

Additional mathematical operations are possible on a value using the `calc` property. This allows simple [calc](#calc)ulations to be performed by either adding, subtracting, dividing or multiplying this value with another value. The `calc` property expects a list of [calc](#calc) objects. Even if only one calculation is to be performed, it should still be supplied as a list (with one element). The calculations will then be executed in the order that they appear in the list. While voltage values are usually expected to fall into the -10V to 10V range in VCV Rack, calculations will not enforce this limit, and the result of the calculation can fall outside of that range.

Using the `quantize` property, a value can optionally be set to quantize to the nearest 1V/Oct note value. If enabled, quantization of the voltage value will be done **after** the *calc* operations have been applied to the voltage value. If more control is needed over the quantization , such as quantizing to a specific scale or a set of custom voltages, a [calc](#calc) with a `quantize` operation should be used instead.

Exactly one of the `voltage`, `note`, `variable`, `input`, `output`, `rand` or `sequence` properties must be specified for a value.

Note: values always resolve into a voltage, which is then used by the object that contains the value. The source of the value voltage will not be tied to the target of that value. E.g. if an action sets the voltage of an *output* port using a value that is based on the voltage of an *input* port, the voltage to use will be determined when the action is executed. If the voltage on the *input* port changes afterwards, the voltage of the *output* port will **not** be changes automatically to the updated voltage of the *input* port. The *set-value* action will have to be re-executed in order for the *output* port to update again.

### Properties

| property | required | type | since | description |
| --- | --- | --- | --- | --- |
| `voltage` | no | float | | An exact constant voltage value between `-10` and `10`. See also [Shorthand Value Notation](#shorthand-value-notation) for a shortened version for voltage values. |
| `no-limit` | no | boolean | *1.1.0* | Can only be used in combination with `voltage`. When set to `true`, the default check that enforces a voltage value between `-10` and `10` will be disabled. |
| `note` | no | string | | A note that will be translated in the corresponding 1V/Oct voltage. See the description above for the format. See also [Shorthand Value Notation](#shorthand-value-notation) for a shortened version for note values |
| `variable` | no | string | | The name of the variable to use. |
| `input` | no | [input](#input) | | Reads the current voltage from one of the TimeSeq inputs. |
| `output` | no | [output](#output) | | Reads the current voltage from one of the TimeSeq outputs. |
| `rand` | no | [rand](#rand) | | Uses a random voltage value (within a specified voltage range). |
| `sequence` | no | [sequence value](#sequence-value) | *1.2.0* | Uses the value from a sequence and (optionally) moves the position in the sequence. |
| `calc` | no | [calc](#calc) list | | Allows mathematical operations to be applied to the voltage of this value, using the voltage of another value. |
| `quantize` | no | boolean | | If set to `true`, the voltage of this value will be quantized to the nearest 1V/Oct note value **after** any optional calculations have been performed. If set to `false`, the voltage value will be used as-is after any optional calculations have been performed. Defaults to `false`. |

### Examples

A constant voltage value:

```json
{ "voltage": 3.14 }
```

A constant voltage value expressed as a note:

```json
{ "note": "D5+" }
```

The voltage of channel 4 on input port 3, multiplied by 2:

```json
{
    "input": { "index": 3, "channel": 4 },
    "calc": [
        { "mult": { "voltage": 2 } }
    ]
}
```

The voltage of channel 8 on output port 5, with a random value between 0.5 and 1 added to it, and subsequently multiplied by 2:

```json
{
    "output": { "index": 5, "channel": 8 },
    "calc": [
        {
            "add": {
                "rand": {
                    "lower": 0.5,
                    "upper": 1
                }
            }
        },
        { "mult": { "voltage": 2 } }
    ]
}
```

### Shorthand Value Notation

To allow easier writing of fixed values, the TimeSeq JSON script allows `voltage` and `note` values to be written using a shorthand notation:

* A voltage value like `{ "voltage": 6.9 }` can be shortened to its float value instead: `6.9`
* A note value like `{ "note": "E4" }` can be shortened to its string note value instead: `"E4"`

This way, following [set-value](#set-value) actions:

```json
{
    "set-value": {
        "value": { "voltage": 3.14 },
        "output": { "index": 6 }
    }
},
{
    "set-value": {
        "value": { "note": "F4" },
        "output": { "index": 7 }
    }
}
```

Can be shortened using both shorthand value and [shorthand output](#shorthand-output-notation) notation as:

```json
{
    "set-value": {
        "value": 3.14,
        "output": 6
    }
},
{
    "set-value": {
        "value": "F4",
        "output": 7
    }
}
```

This shorthand notation can be used in all places where `voltage` or `note` values are used, except when declaring a re-usable variable as a direct child of the [component-pool](#component-pool) `values` list (where the full value notation must be used since an `id` property must also be specified).

## input

An input identifies a channel on one of the input ports of TimeSeq, either to read a voltage from it in a [value](#value) or to monitor it for [input-trigger](#input-trigger)s.

The input port is identified by the `index` property, using a number from `1` to `8`. The `channel` property identifies which (polyphonic) channel to use. Ports can have up to 16 channels. If no `channel` property is specified, the first channel of the port will be used. When working with monophonic input signals, either the `channel` property can be omitted, or it can be set to `1` (since in VCV Rack, a monophonic signal is considered to be a signal containing one channel).

Note that TimeSeq will not validate how many channels are present on the input port. If a channel is requested that is outside of the current polyphonic channel range of the input signal, TimeSeq will use whatever value VCV Rack returns for that channel (usually 0V).

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `index` | yes | unsigned number | The index of the port from which to retrieve a voltage. Must be between `1` and `8`. See [Shorthand Input Notation](#shorthand-input-notation) for a shortened way to write inputs with only an `index` property. |
| `channel` | no | unsigned number | The channel on the input port from which to retrieve the voltage, as a number between `1` and `16`. Defaults to `1` |

### Examples

The third input port, using channel 1 since no `channel` property is specified:

```json
{
    "index": 3
}
```

The fifth input port, using channel 15:

```json
{
    "index": 5,
    "channel": 15
}
```

### Shorthand Input Notation

To allow easer writing of inputs, the TimeSeq JSON script allows inputs for which no `channel` is specified (i.e. monophonic inputs or channel `1` on polyphonic inputs) to be written using a shorthand notation: a full `{ "index": 5 }` or `{ "index": 5, "channel": 1 }` notation can be shortened to `5` (i.e. just the index value).

This way, following [set-value](#set-value) action:

```json
{
    "set-value": {
        "value": { "input": { "index": 3 } },
        "output": { "index": 6 }
    }
}
```

Can be shortened using both shorthand input and [shorthand output](#shorthand-output-notation) notation as:

```json
{
    "set-value": {
        "value": { "input": 3 },
        "output": 6
    }
}
```

This shorthand notation can be used in all places where inputs are used, except when declaring a re-usable input as a direct child of the [component-pool](#component-pool) `inputs` list (where the full input notation must be used since an `id` property must also be specified).

## output

An output identifies a channel on one of the output ports of TimeSeq, either to assign a voltage to it through an [action](#action), or to read a voltage from it using a [value](#value).

The output port is identified by the `index` property, using a number from `1` to `8`. The `channel` property identifies which (polyphonic) channel to use. Ports can have up to 16 channels. If no `channel` property is specified, the first channel of the port will be used. When working with monophonic output signals, either the `channel` property can be omitted, or it can be set to `1` (since in VCV Rack, a monophonic signal is considered to be a signal containing one channel).

Note that TimeSeq will not validate how many channels are present on the output port. Assigning a value to a channel that is outside the current polyphonic channel count will not result in more channels becoming active on that output. TimeSeq will however remember any voltage updates done on all channels (even if they are outside of the current polyphonic channel count), and will assign those voltages to the channels if the channel count is changed afterwards using a [set-polyphony](#set-polyphony) *action*. Similarly, if a voltage is assigned to a channel that is outside of the current polyphonic channel count, a [value](#value) that references that channel on the output port will still return the value that was assigned to it.

When a script is loaded or reset, all output ports of TimeSeq will be set to monophonic mode (i.e. have 1 channel), and all voltages on the output ports will be set to 0 volts.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `index` | yes | unsigned number | The index of the port from which to retrieve or on which to set a voltage. Must be between `1` and `8`. See [Shorthand Output Notation](#shorthand-output-notation) for a shortened way to write outputs with only an `index` property. |
| `channel` | no | unsigned number | The channel on the output port from which to retrieve or on which to set the voltage, as a number between `1` and `16`. Defaults to `1` |

### Examples

The third output port, using channel 1 since no `channel` property is specified:

```json
{
    "index": 3
}
```

The fifth output port, using channel 15:

```json
{
    "index": 5,
    "channel": 15
}
```

### Shorthand Output Notation

To allow easer writing of outputs, the TimeSeq JSON script allows outputs for which no `channel` is specified (i.e. monophonic outputs or channel `1` on polyphonic outputs) to be written using a shorthand notation: a full `{ "index": 5 }` or `{ "index": 5, "channel": 1 }` notation can be shortened to `5` (i.e. just the index value).

This way, following [set-value](#set-value) action:

```json
{
    "set-value": {
        "value": { "voltage": 3.14 },
        "output": { "index": 6 }
    }
}
```

Can be shortened using both shorthand output and [shorthand value](#shorthand-value-notation) notation as:

```json
{
    "set-value": {
        "value": 3.14,
        "output": 6
    }
}
```

This shorthand notation can be used in all places where outputs are used, except when declaring a re-usable output as a direct child of the [component-pool](#component-pool) `outputs` list (where the full output notation must be used since an `id` property must also be specified).

## rand

The rand allows random voltages to be generated for usage in a [value](#value).

The generated random value will be between the `lower` and `upper` [value](#value)s. If the runtime calculation of the *value*s results in a `upper` value that is below the `lower` value, the rand will swap their meaning for the random voltage generation.

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `lower` | yes | [value](#value) | The lowest voltage that can be generated. |
| `upper` | yes | [value](#value) | The generated random value will be below this voltage. |

### Example

```json
{
    "rand": {
        "lower": { "voltage": -5 },
        "upper": { "variable": "the-upper-bounds" }
    }
}
```

## sequence value

A sequence value retrieves the value at the current position of the [sequence](#sequence) with the specified `id` and optionally moves the position of that sequence.

The default behaviour (if no additional properties are specified) is that each time that a value is retrieved from a sequence, the position moved ahead by one after the value is retrieved, with a wraparound if the position reached the end of the sequence. This means that the default behaviour allows continuous looping to be performed over the sequence values.

Using the `move-before` and/or `move-after` properties it is possible to modify the behaviour and specify which movement actions should be done before the value is retrieved from the sequence, and which should be done afterwards. The possible movements are `forward` (to move one position forward in the sequence), `backward` (to move the position backward in the sequence), `random` (to move to a random position in the sequence) and `none` (to leave the position as-is).

The optional `wrap` property specifies if the position should move to the start of the sequence when the end is reached using `forward` movement and to the back of the sequence when the start of the sequence is reached using `backward` movement (i.e if the position should wrap around), or if it should stay in place at the sequence boundaries.

Note that the `shared` property of the `sequence` identifies if the position of that sequence is shared throughout the whole script, or separate for each individual sequence value (see [sequence](#sequence) for more details).

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The id of the [sequence](#sequence) from the `component-pool`. |
| `move-before` | no | string | How the sequence position should be moved before the value is retrieved from the sequence. Can be `forward`, `backward`, `random` or `none`. Defaults to `none` |
| `move-after` | no | string | How the sequence position should be moved after the value has been retrieved from the sequence. Can be `forward`, `backward`, `random` or `none`. Defaults to `forward` |
| `wrap` | no | boolean | Specifies if the position in the sequence should wrap around when the last (or first) element of the sequence has been reached. Defaults to `true` |

### Example

```json
{
  "set-value" {
    "sequence": {
      "id": "my-sequence-id",
      "move-before": "random"
    },
    "output": { "index": 4, "channel": 2 }
  }
}
```

### Shorthand Sequence Notation

A *sequence value* that only specifies the `id` property and not the other properties (i.e. no `move-before`, `move-after` or `wrap` properties) can also be specified with a shorthand notation: the sequence `id` string can be used as value instead of a full object. This means that following *sequence value*

```json
{
    "value": { "sequence": { "id": "my-sequence-id" } }
}
```

can also be written as follows:

```json
{
    "value": { "sequence": "my-sequence-id" }
}
```

This will cause no move to happen before the value is retrieved, and a `forward` move with wraparound behaviour will be performed after the value was retrieved from the sequence with id `my-sequence-id`.

## calc

Allows calculations to be performed on [value](#value)s. A *value* can contain a list of calculations. First the voltage of the value itself will be determined. Subsequently, each calculation modifies the value (e.g. adds or subtracts another value from the current voltage).

To safeguard against calculation errors, a division by zero will result in 0V and a remainder after division by zero will also result in 0V.

The possible operations that are available are:

* `add` for adding a value to the current voltage,
* `sub` for subtracting a value from the current voltage,
* `mult` for multiplying the current voltage with a value,
* `div` for dividing the current voltage by a value,
* `max` for determining the highest of two values,
* `min` for determining the lowest of two values,
* `remain` for calculating the remainder after division by another value,
* `trunc` for getting the non-decimal part of a value,
* `frac` for getting the decimal part of a value,
* `round` for rounding the value up, down or to the nearest non-decimal number,
* `quantize` for quantizing to a [tuning](#tuning)
* `sign` for enforcing that the sign of a value is either positive or negative.
* `vtof` for converting a 1V/Oct value into a frequency

While multiple calcs can be added to the calculation list of a *value*, each calc within that list must specify exactly one mathematical operation.

### Properties

| property | required | type | since | description |
| --- | --- | --- | --- | --- |
| `add` | no | [value](#value) | | Adds a value to the current voltage. |
| `sub` | no | [value](#value) | | Subtracts a value from the current voltage. |
| `mult` | no | [value](#value) | | Multiplies the current voltage with a value. |
| `div` | no | [value](#value) | | Divides the current voltage by a value. |
| `max` | no | [value](#value) | *1.1.0* | Compares the current voltage with the supplied value and uses the higher of the two. |
| `min` | no | [value](#value) | *1.1.0* | Compares the current voltage with the supplied value and uses the lower of the two. |
| `remain` | no | [value](#value) | *1.1.0* | Divides the current voltage by a value and uses the remainder after division. |
| `trunc` | no | boolean | *1.1.0* | Removes the decimal part of the current voltage, keeping only the whole number. The result keeps the same sign (positive or negative) as the original voltage. Must be set to `true`. |
| `frac` | no | boolean | *1.1.0* | Removes the whole number part of the current voltage, keeping only the decimal part. The result keeps the same sign (positive or negative) as the original voltage. Must be set to `true`. |
| `round` | no | string | *1.1.0* | Can be either `up` to round up, `down`to round down or `near` to round to the nearest whole number. |
| `quantize` | no | [tuning](#tuning) | *1.1.0* | Quantizes the current voltage to the nearest voltage in the specified tuning's `notes`. |
| `sign` | no | string | *1.1.0* | Can be set to either `pos` or `neg`. Keeps the current voltage's value but forces it to be positive or negative, depending on which one is specified. |
| `vtof` | no | boolean | *1.1.0* | Interprets the current voltage as a 1V/Oct value and returns the corresponding audio frequency value. Must be set to `true`. |

### Examples

The voltage of channel 4 on input port 3, multiplied by 2:

```json
{
    "input": { "index": 3, "channel": 4 },
    "calc": [
        { "mult": { "voltage": 2 } }
    ]
}
```

The voltage of channel 8 on output port 5, with a random value between 0.5 and 1 added to it, and subsequently multiplied by 2:

```json
{
    "output": { "index": 5, "channel": 8 },
    "calc": [
        {
            "add": {
                "rand": {
                    "lower": 0.5,
                    "upper": 1
                }
            }
        },
        { "mult": { "voltage": 2 } }
    ]
}
```

The maximum of either channel 4 on input 3, or channel 5 on input 4:

```json
{
    "input": { "index": 3, "channel": 4 },
    "calc": [
        { "max": { "input": { "index": 4, "cannel": 5 } } }
    ]
}
```

The voltage on input 6, quantized to the *tuning* with id `c-maj-pent`, and subsequently 1V (i.e. 1 octave) added to it:

```json
{
    "input": 6,
    "calc": [
        { "quantize": { "ref": "c-maj-pent" } },
        { "add": 1 }
    ]
}
```

The decimal part of the voltage that is currently stored in variable `my-voltage`:

```json
{
    "variable": "my-voltage",
    "calc": [
        { "frac": true }
    ]
}
```

## tuning

While a [value](#value) can be quantized to the nearest semitone using its `quantize` property, tunings allow values to be quantized to a scale. The `notes` property of a tuning contains the list of notes that should be quantized towards. Notes can be expressed in two ways:

* A string value where the first character specifies the note name (from A-G), and the optional second character specifies the accidental: `+` for a sharp and `-` for a flat. E.g.: `A-`, `F+`, `C+`, `B-`, `G`
* A float value that specifies the note value as a 1V/Oct value.

Quantization of values is always done in the octave range of the original value. As such, which supplying a tuning note as a float value, only the decimal part of the quantization note will be taken into account.

To quantize a value to a tuning, a [calc](#calc) must be added to the [value](#value), using the tuning as *calc* `quantize` property, either directly inline or through a `ref`. The value will then be quantized up or down towards the nearest `notes` entry in the tuning (ignoring the octave information of the value).

*Tunings were introduced in TimeSeq script version 1.1.0.*

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The identifier of the tuning. |
| `notes` | yes | string/float list | The list of notes to quantize to, either as a float or a string as described above. |

### Examples

A tuning for a C minor pentatonic scale, with the notes specified using their note names:

```json
{
    "id": "c-minor-pentatonic",
    "notes": [ "c", "e-", "f", "g", "b-" ]
}
```

The same C minor pentatonic scale, but this time using a combination of note names and 1V/Oct float values:

```json
{
    "id": "c-minor-pentatonic",
    "notes": [ 0, "e-", "f", 0.5833, "b-" ]
}
```

A tuning that uses 1V/Oct values that don't follow the usual semitone notes:

```json
{
    "id": "non-semitone-tuning",
    "notes": [ 0, 0.1579, 0.3158, 0.4211, 0.5789, 0.7368, 0.8947 ]
}
```

## sequence

A sequence contains a list of values and can be used to allow the same action (or set of actions) to be performed repeatedly with a different value on each iteration by using a [sequence value](#sequence-value). Some (simple) possible scenarios in which sequences can be used are chord progressions, note chains or a list of modulation CV values.

The `values` property specifies the list of values in the sequence. Any type of value can be used for this: constant values, [input](#input) values, [output](#output) values, etc.

Each time a *sequence value* retrieves a voltage from the sequence, the voltage of the *value* at the current position will be returned. The position of the sequence is updated by this *sequence value* usage (by default the position will automatically be moved one forward after the voltage is retrieved, but this behaviour can be modified using the `move-before` and `move-after` properties of the [sequence value](#sequence-value) element).

The `shared` property of a sequence identifies if the position within the sequence is globally shared throughout the script (`shared=true`) or if each *sequence value* has it's own position within the sequence (`shared=false`). If the sequence is shared, any change of the position made by one element of the script will also apply for all other elements within the script that use that sequence (all other *sequence values* and any action that works on that sequence). If the sequence is not shared, the position can/will be different for each *value* that makes use of that sequence, each *value* moving independently through the items of the sequence.

Besides retrieving a voltage from a sequence using a *sequence value*, there are also a number of actions that can be performed on a sequence:

* [move-sequence](#move-sequence): moves the current position of the sequence, either in a direction or to a specific index (can only be used on `shared` sequences),
* [add-to-sequence](#add-to-sequence): adds a [value](#value) to the list of `values` of a sequence,
* [remove-from-sequence](#remove-from-sequence): removes an item from the list of `values` of a sequence,
* `clear-sequence`: removes all items from the `values` of the sequence with the specified identifier.

The currently active value in a sequence is tracked as a position index within that sequence. This active position index is not influenced by the *add-to-sequence*, *remove-from-sequence* or *clear-sequence* actions. If any of those actions cause another value to be at that position, that value will now be considered the active value instead of the one that was in that position before. If an action causes the position to go out of the range of values in the sequence, the last value of the sequence will be considered the active value for the sequence.

Through the `move-before` and `move-after` properties of the [sequence value](#sequence-value), it is possible to retrieve the current value of a sequence multiple times without moving the position of the sequence. When this occurs, the `retrieve-voltage-once` property of the sequence will influence how the voltage to return is determined. If this property is set to `false`, the voltage of the currently active value is determined each and every time again (e.g. retrieve the current voltage of an [input](#input) port). If `retrieve-voltage-once` is set to `true`, the voltage of the value will only be determined the first time that value is used. Each subsequent time that the value is used, that same voltage will be used instead of re-evaluating the value. As soon as any move is performed (through a *sequence value* or using a [move-sequence](#move-sequence) action), this stored voltage is cleared and the next usage of the sequence will trigger a full value determination again.

It is possible for a sequence to contain no `values`. If such a sequence is used by a *sequence value*, a value of `0` volts will be used.

*Sequences were introduced in TimeSeq script version 1.2.0.*

### Properties

| property | required | type | description |
| --- | --- | --- | --- |
| `id` | yes | string | The identifier of the sequence. |
| `values` | yes | [value](#value) list | The list of values in this sequence. |
| `shared` | no | boolean | `true` if the position of the sequence is shared throughout the script or `false` if the position is separate for each [sequence value](#sequence-value) that uses the sequence. Defaults to `true`. |
| `retrieve-voltage-once` | no | boolean | If the value of a sequence is retrieved multiple times without a change in the sequence position, this property specifies if the voltage of the active value is retrieved only once (the first time) and then re-used after that, or if the voltage of the active value will be fully retrieved each time (e.g. read the voltage of an input port each time). Defaults to `true`. |

### Examples

A sequence that contains the notes of a C minor pentatonic (in the 4th octave):

```json
{
    "id": "c-minor-pentatonic",
    "values": [ "c4", "e-4", "f4", "g4", "b-4" ]
}
```

A non-shared sequence that iterates over the 8 input ports:

```json
{
    "id": "input-port-voltages",
    "values": [
        { "input": 1 },
        { "input": 2 },
        { "input": 3 },
        { "input": 4 },
        { "input": 5 },
        { "input": 6 },
        { "input": 7 },
        { "input": 8 },
    ],
    "shared": false
}
```
