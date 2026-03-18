# not-things TimeSeq JSON Schema

The not-things TimeSeq JSON schema can be used to validate JSON scripts that are used by the [TimeSeq](../../TIMESEQ.md) module.

## Versioning

The TImeSeq JSON schema is versioned. Each time new features are added to the schema (to account for new features in TimeSeq), a new version of the schema must be created. THe version number of the schema increases independently from the version number of the not-things modules, and thus the version number of TimeSeq.

## Using the schema

To use the schema for validating a TimeSeq JSON script, include following at the root level of the JSON script:

```json
{
    "$schema": "https://not-things.com/schemas/timeseq-script-1.2.0.schema.json",
}
```

This would validate the JSON script against version `1.2.0` of the TimeSeq JSON script. Update the schema URI to the desired version when a different JSON script version is desired.

## Validating the schema

The TimeSeq JSON schema itself must pass strict JSON schema validation. This can be done using the included `validate.js`. To validate all `timeseq-script-<version>.schema.json` files in the current folder, run:

```shell
npm install
npm run validate
```

To validate a specific schema file, run:

```shell
npm install
npm run validate -- <schema_filename>
```
