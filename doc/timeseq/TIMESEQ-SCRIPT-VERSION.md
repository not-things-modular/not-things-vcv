# TIMESEQ JSON SCRIPT VERSIONS

*Version history of the JSON script schema for the not-things [TimeSeq](../TIMESEQ.md) module.*

## Table of Contents

* [1.0.0](#version-100)
* [1.1.0](#version-110)

## Version 1.0.0

**Supported from**: TimeSeq v2.0.2, **Release date**: 2025-06-06

### Changes

* Initial version of the TimeSeq script

## Version 1.1.0

**Supported from**: TimeSeq v2.0.5, **Release date**: 2025-06-28

### Changes

* Added additional [calc](TIMESEQ-SCRIPT-JSON.md#calc) operations: `max`, `min`, `remain`, `trunc`, `frac`, `round`, `quantize`, `sign` and `vtof`
* Introduced a new [tuning](TIMESEQ-SCRIPT-JSON.md#tuning)s element under the [component-pool](TIMESEQ-SCRIPT-JSON.md#component-pool). Allows [value](TIMESEQ-SCRIPT-JSON.md#value)s to be quantized to a scale/tuning using a `quantize` [calc](TIMESEQ-SCRIPT-JSON.md#calc)
* Added new `no-limit` property on `voltage` [value](TIMESEQ-SCRIPT-JSON.md#value)s. Allows disabling of the `-10` to `10` volt limit check on `voltage` values.
* Enabled `samples`, `millis`, `beats` and `hz` properties of [duration](TIMESEQ-SCRIPT-JSON.md#duration)s to be set using a [value](TIMESEQ-SCRIPT-JSON.md#value) (instead of only through fixed numbers), allowing for variable *segment* durations

### JSON Schema

Add following property at the root of the JSON Script to allow JSON Schema validation:

```json
{
    "$schema": "https://not-things.com/schemas/timeseq-script-1.1.0.schema.json"
}
```
