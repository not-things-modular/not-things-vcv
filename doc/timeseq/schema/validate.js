// This script was created by extracting relevant pieces from the schemastore cli.js script

import fs from 'node:fs/promises'
import jsonlint from '@prantlf/jsonlint'

import { Ajv } from 'ajv'
import _addFormats from 'ajv-formats'
import { ajvFormatsDraft2019 } from '@hyperupcall/ajv-formats-draft2019'

const ajv = new Ajv({ strict: true });
_addFormats(ajv);
ajvFormatsDraft2019(ajv)

if (process.argv.length > 2) {
	const filename = process.argv[2];
	console.info(`[Validating supplied schema file ${filename}]`);
	await validateSchemaFile(filename);
} else {
	console.info(`[Validating all schema files in the current directory]`)
	for (const filename of await fs.readdir('.')) {
		if (filename.startsWith('timeseq-script-') && filename.endsWith('.schema.json')) {
			console.info();
			console.info(`[Validating schema file ${filename}]`);
			await validateSchemaFile(filename);
		}
	}
}

async function validateSchemaFile(filename) {
	const buffer = await fs.readFile(filename);
	const schema = buffer.toString();
	const json = JSON.parse(schema);

	try {
		jsonlint.parse(schema, {
			ignoreBOM: false,
			ignoreComments: false,
			ignoreTrailingCommas: false,
			allowSingleQuotedStrings: false,
			allowDuplicateObjectKeys: false
		});
	} catch (err) {
		console.error(`Failed strict jsonlint parse of file "./${filename}"`, err);
		process.exit(0);
	}

	try {
		ajv.compile(json);
	} catch (err) {
		console.error(`Failed to compile schema file "./${filename}"`, err);
		process.exit(0);
	}

	console.info('Validation succeeded');
}