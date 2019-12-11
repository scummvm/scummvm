/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

// This file re-generates 'larryScale_generated.cpp'.
// To run it, install Node 8.0+, then run 'node larryScale_generator.js'.

const fs = require('fs');

// Compass directions
const Direction = {
	W:	0,
	NW:	1,
	N:	2,
	NE:	3,
	E:	4,
	SE:	5,
	S:	6,
	SW:	7,

	sanitize(direction) {
		return ((direction % 8) + 8) % 8;
	}
};

function getVector(direction) {
	switch (direction) {
	case Direction.W:	return [-1, 0];
	case Direction.NW:	return [-1, -1];
	case Direction.N:	return [0, -1];
	case Direction.NE:	return [1, -1];
	case Direction.E:	return [1, 0];
	case Direction.SE:	return [1, 1];
	case Direction.S:	return [0, 1];
	case Direction.SW:	return [-1, 1];
	default:
		throw new Error(`Invalid direction: ${direction}`);
	}
}

// An equality matrix is a combination of eight Boolean flags indicating whether
// each of the surrounding pixels has the same color as the central pixel.
//
// +-----------+-----------+-----------+
// | NW = 0x02 | N  = 0x04 | NE = 0x08 |
// +-----------+-----------+-----------+
// | W  = 0x01 | Reference | E  = 0x10 |
// +-----------+-----------+-----------+
// | SW = 0x80 | S  = 0x40 | SE = 0x20 |
// +-----------+-----------+-----------+
class EqualityMatrix {
	constructor(value) {
		this.value = value;
	}

	get(direction) {
		const mask = 0x01 << Direction.sanitize(direction);
		return (this.value & mask) != 0;
	}

	set(direction, flag) {
		const mask = 0x01 << Direction.sanitize(direction);
		this.value = this.value & ~mask | (flag ? mask : 0x00);
	}

	get w() { return this.get(Direction.W); }
	set w(flag) { this.set(Direction.W, flag); }

	get nw() { return this.get(Direction.NW); }
	set nw(flag) { this.set(Direction.NW, flag); }

	get n() { return this.get(Direction.N); }
	set n(flag) { this.set(Direction.N, flag); }

	get ne() { return this.get(Direction.NE); }
	set ne(flag) { this.set(Direction.NE, flag); }

	get e() { return this.get(Direction.E); }
	set e(flag) { this.set(Direction.E, flag); }

	get se() { return this.get(Direction.SE); }
	set se(flag) { this.set(Direction.SE, flag); }

	get s() { return this.get(Direction.S); }
	set s(flag) { this.set(Direction.S, flag); }

	get sw() { return this.get(Direction.SW); }
	set sw(flag) { this.set(Direction.SW, flag); }

	toBraille() {
		return getBrailleColumn(this.nw, this.w, this.sw)
			+ getBrailleColumn(this.n, true, this.s)
			+ getBrailleColumn(this.ne, this.e, this.se);
	}
}

function getBrailleColumn(top, middle, bottom) {
	const codepoint = 0x2800 | (top ? 1 : 0) | (middle ? 2 : 0) | (bottom ? 4 : 0);
	return String.fromCodePoint(codepoint);
}

function indent(string, tabCount = 1) {
	const indentation = '\t'.repeat(tabCount);
	return string
		.split(/\r?\n/)
		.map(s => indentation + s)
		.join('\n');
}

function toHex(number, minLength = 2) {
	const hex = number.toString(16);
	const padding = '0'.repeat(Math.max(minLength - hex.length, 0));
	return `0x${padding}${hex}`;
}

function generateCaseLabel(matrix) {
	return `case ${toHex(matrix.value)} /*${matrix.toBraille()}*/:`
}

function generateCaseBlock(matrixes, body) {
	const maxLabelsPerLine = 8;
	const labels = matrixes
		.map(generateCaseLabel)
		.reduce((a, b, index) => a + ((index % maxLabelsPerLine === 0) ? '\n' : '\t') + b);
	return `${labels}\n${indent(body)}`;
}

function generateSwitchBlock(variableName, getCaseBody) {
	const matrixesByBody = new Map();
	for (let value = 0; value <= 0xFF; value++) {
		const matrix = new EqualityMatrix(value);
		const body = getCaseBody(matrix);
		if (!matrixesByBody.has(body)) {
			matrixesByBody.set(body, []);
		}
		matrixesByBody.get(body).push(matrix);
	}
	const orderedPairs = [...matrixesByBody.entries()]
		// For readability: order cases by increasing code length
		.sort((a, b) => a[0].length - b[0].length);
	const switchStatements = orderedPairs
		.map(([body, matrixes]) => generateCaseBlock(matrixes, body))
		.join('\n');
	const comment = '// Note: There is a case label for every possible value, so we don\'t need a default label, but one is added to avoid any compiler warnings.';
	return `${comment}\nswitch (${variableName}) {\ndefault: ${switchStatements}\n}`;
}

const PixelType = {
	// Pixel is part of a line
	LINE: 'line',
	// Pixel is part of a fill
	FILL: 'fill',
	// Pixel is part of a line *or* a fill
	INDETERMINATE: 'indeterminate'
};

function getPixelType(matrix) {
	// Single pixels are fills
	if (matrix.value === 0) return PixelType.FILL;

	// 2x2 blocks are fills
	if (
		(matrix.n && matrix.ne && matrix.e)
		|| (matrix.e && matrix.se && matrix.s)
		|| (matrix.s && matrix.sw && matrix.w)
		|| (matrix.w && matrix.nw && matrix.n)
	) return PixelType.FILL;

	// A pixel adjacent to a 2x2 block is a fill.
	// This requires reading out of the matrix, so we can't be sure.
	if (
		(matrix.n && matrix.ne)
		|| (matrix.ne && matrix.e)
		|| (matrix.e && matrix.se)
		|| (matrix.se && matrix.s)
		|| (matrix.s && matrix.sw)
		|| (matrix.sw && matrix.w)
		|| (matrix.w && matrix.nw)
		|| (matrix.nw && matrix.n)
	) return PixelType.INDETERMINATE;

	// Everything else is part of a line
	return PixelType.LINE;
}

function isPowerOfTwo(number) {
    return Math.log2(number) % 1 === 0;
}

// Upscales a line pixel to 2x2.
// Returns a 4-element array of Booleans in order top-left, top-right, bottom-left, bottom-right.
// Each Boolean indicates whether the upscaled pixel should be filled with the original color.
function getLineUpscaleFlags(matrix) {
	// The rules for upscaling lines are *not* symmetrical but biased toward the left

	// Special rules for upscaling smooth angled lines
	switch (matrix.value) {
	case 0x34 /*⠀⠃⠆*/:
		return [false, true, false, false];	// [ ▀]
	case 0x58 /*⠀⠆⠃*/:
		return [false, false, false, true];	// [ ▄]
	case 0x43 /*⠃⠆⠀*/:
		return [false, false, true, false];	// [▄ ]

	case 0x61 /*⠂⠆⠄*/:
		return [false, false, true, false];	// [▄ ]
	case 0x16 /*⠁⠃⠂*/:
		return [false, true, false, false];	// [ ▀]
	case 0xD0 /*⠄⠆⠂*/:
		return [false, false, false, true];	// [ ▄]

	case 0x24 /*⠀⠃⠄*/:
	case 0x48 /*⠀⠆⠁*/:
		return [false, true, false, true];	// [ █]

	case 0x21 /*⠂⠂⠄*/:
	case 0x90 /*⠄⠂⠂*/:
		return [false, false, true, true];	// [▄▄]

	case 0x50 /*⠀⠆⠂*/:
		return [true, true, true, false];	// [█▀]
	}

	// Generic rules for upscaling lines

	// Ignore diagonals next to fully-adjacent pixels
	matrix = new EqualityMatrix(matrix.value);
	if (matrix.w) {
		matrix.sw = matrix.nw = false;
	}
	if (matrix.n) {
		matrix.nw = matrix.ne = false;
	}
	if (matrix.e) {
		matrix.ne = matrix.se = false;
	}
	if (matrix.s) {
		matrix.se = matrix.sw = false;
	}

	// Mirror single lines
	if (isPowerOfTwo(matrix.value)) {
		matrix.value |= (matrix.value << 4) | (matrix.value >> 4);
	}

	return [
		matrix.w || matrix.nw || matrix.n,
		matrix.ne || matrix.e,
		matrix.s || matrix.sw,
		matrix.se
	];
}

// Upscales a fill pixel to 2x2.
// Same result format as getLineUpscaleFlags.
function getFillUpscaleFlags(matrix) {
	// The rules for upscaling fills are *not* symmetrical but biased toward the top-left

	// Special rules for upscaling cornered fills
	switch (matrix.value) {
	case 0xE1 /*⠆⠆⠄*/:
		return [false, false, true, true];	// [▄▄]
	case 0x0F /*⠃⠃⠁*/:
		return [true, true, false, false];	// [▀▀]
	case 0xC3 /*⠇⠆⠀*/:
	case 0x87 /*⠇⠃⠀*/:
		return [true, false, true, false];	// [█ ]
	}

	// Generic rules for upscaling fills
	if (!matrix.s && !matrix.se && !matrix.e && (matrix.sw || matrix.ne)) {
		return [true, true, true, false];	// [█▀]
	} else if (!matrix.n && !matrix.ne && !matrix.e && (matrix.nw || matrix.se)) {
		return [true, false, true, true];	// [█▄]
	} else {
		return [true, true, true, true];	// [██]
	}
}

function formatOffset(number) {
	if (number < 0) {
		return ` - ${-number}`;
	}
	if (number > 0) {
		return ` + ${number}`;
	}
	return '';
}

function generatePixelUpscaleCode(matrix, flags, pixelRecords, { generateBreak = true } = {}) {
	const targetsByValue = new Map();
	function addAssignment(param, value) {
		if (targetsByValue.has(value)) {
			targetsByValue.get(value).push(param);
		} else {
			targetsByValue.set(value, [param]);
		}
	}
	for (const pixelRecord of pixelRecords) {
		const param = pixelRecord.param;
		const useSourceColor = flags
			.filter((flag, index) => pixelRecord.flagIndexes.includes(index))
			.some(flag => flag);
		if (useSourceColor) {
			addAssignment(param, 'pixel');
		} else {
			const sourceDirections = pixelRecord.sourceDirections
				.filter(d => !matrix.get(d));
			const value = sourceDirections
				.filter(d => !matrix.get(d)) // We don't want to get our own color
				.map(d => {
					const vector = getVector(d);
					const otherValueCode = `src.get(x${formatOffset(vector[0])}, y${formatOffset(vector[1])})`;
					return `!linePixels.get(x${formatOffset(vector[0])}, y${formatOffset(vector[1])}) ? ${otherValueCode} : `;
				})
				.join('') + 'pixel';
			addAssignment(param, value);
		}
	}

	return [...targetsByValue.entries()]
		.map(([value, targets]) => [...targets, value].join(' = ') + ';')
		.concat(generateBreak ? ['break;'] : [])
		.join('\n');
}

function generateScalePixelFunction(width, height, pixelRecords) {
	const params = pixelRecords
		.map((pixelRecord, index) => `Color &${pixelRecord.param}`)
		.join(', ');
	const header =
		`inline void scalePixelTo${width}x${height}(\n\tconst MarginedBitmap<Color> &src,\n\tconst MarginedBitmap<bool> &linePixels,\n\tint x, int y,\n\t// Out parameters\n\t${params}\n)`;
	const prefix =
		'const Color pixel = src.get(x, y);\n'
		+ 'const EqualityMatrix matrix = getEqualityMatrix(src.getPointerTo(x, y), src.getStride());';
	const switchBlock = generateSwitchBlock('matrix', matrix => {
		const pixelType = getPixelType(matrix);
		switch (pixelType) {
		case PixelType.LINE:
			return generatePixelUpscaleCode(matrix, getLineUpscaleFlags(matrix), pixelRecords);
		case PixelType.FILL:
			return generatePixelUpscaleCode(matrix, getFillUpscaleFlags(matrix), pixelRecords);
		case PixelType.INDETERMINATE:
			const lineUpscaleCode = generatePixelUpscaleCode(matrix, getLineUpscaleFlags(matrix), pixelRecords, { generateBreak: false });
			const fillUpscaleCode = generatePixelUpscaleCode(matrix, getFillUpscaleFlags(matrix), pixelRecords, { generateBreak: false });
			return `if (linePixels.get(x, y)) {\n${indent(lineUpscaleCode)}\n} else {\n${indent(fillUpscaleCode)}\n}\nbreak;`;
		}
	});
	return `${header} {\n${indent(prefix)}\n\n${indent(switchBlock)}\n}`;
}

function generateScalePixelTo2x2() {
	const pixelRecords = [
		{ param: 'topLeft',		flagIndexes: [0], sourceDirections: [Direction.N, Direction.W] },
		{ param: 'topRight',	flagIndexes: [1], sourceDirections: [Direction.N, Direction.E] },
		{ param: 'bottomLeft',	flagIndexes: [2], sourceDirections: [Direction.S, Direction.W] },
		{ param: 'bottomRight',	flagIndexes: [3], sourceDirections: [Direction.S, Direction.E] }
	];
	return generateScalePixelFunction(2, 2, pixelRecords);
}

function generateScalePixelTo2x1() {
	const pixelRecords = [
		{ param: 'left',	flagIndexes: [0, 2], sourceDirections: [Direction.N, Direction.W, Direction.S] },
		{ param: 'right',	flagIndexes: [1, 3], sourceDirections: [Direction.N, Direction.E, Direction.S] }
	];
	return generateScalePixelFunction(2, 1, pixelRecords);
}

function generateScalePixelTo1x2() {
	const pixelRecords = [
		{ param: 'top',		flagIndexes: [0, 1], sourceDirections: [Direction.N, Direction.W, Direction.E] },
		{ param: 'bottom',	flagIndexes: [2, 3], sourceDirections: [Direction.S, Direction.W, Direction.E] }
	];
	return generateScalePixelFunction(1, 2, pixelRecords);
}

const generators = [generateScalePixelTo2x2, generateScalePixelTo2x1, generateScalePixelTo1x2];
const generatedFunctions = generators
	.map(generator => generator())
	.join('\n\n');
const legalese = fs.readFileSync(__filename, 'utf8').match(/\/\*[\s\S]*?\*\//)[0];
const headerComment = '// This file was generated by larryScale_generator.js.\n// Do not edit directly! Instead, edit the generator script and run it.'
fs.writeFileSync('./larryScale_generated.cpp', `${legalese}\n\n${headerComment}\n\n${generatedFunctions}\n`);
