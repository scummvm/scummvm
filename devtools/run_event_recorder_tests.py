#!/usr/bin/env python3

# To run this in headless mode, set the following environment variables:
# * SDL_VIDEODRIVER=dummy
# * SDL_AUDIODRIVER=dummy
#
# Also make sure to specify the correct paths to the ScummVM binary, demos, and records if they are not in the default locations.
# You can also change them via environment variables:
# * SCUMMVM_BIN - path to the ScummVM binary
# * GAMES_DIR - path to the games directory that are automatically registered before running tests - defaults to none and is skipped then.
#               it's assumed that you have already registered the games for playing back the event recorder files.
#
# Example usage:
#   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SCUMMVM_BIN=./scummvm \
#   python3 devtools/run_event_recorder_tests.py --xunit-output=event_recorder_tests.xml \
#   --filter="*monkey*"


import os
import sys
import subprocess
import platform
import argparse
import time
import fnmatch
import re
import json
import xml.etree.ElementTree as ET
from pathlib import Path

def generate_xunit_report(results, output_file, total_time):
	testsuites = ET.Element("testsuites")
	testsuite = ET.SubElement(testsuites, "testsuite", name="EventRecorderTest", tests=str(len(results)), time=str(total_time))

	failures = 0
	skipped = 0
	for result in results:
		# XUnit format: classname="TestSuite", name="TestCase"
		testcase = ET.SubElement(testsuite, "testcase", name=result['test_case'], classname="EventRecorderTest", time=str(result['duration']))
		if result.get('skipped'):
			skipped += 1
			ET.SubElement(testcase, "skipped", message=result['message'])
		elif not result['success']:
			failures += 1
			failure = ET.SubElement(testcase, "failure", message=result['message'])
			failure.text = result.get('output', '')

	testsuite.set("failures", str(failures))
	testsuite.set("skipped", str(skipped))

	tree = ET.ElementTree(testsuites)
	try:
		tree.write(output_file, encoding="utf-8", xml_declaration=True)
		# print(f"XUnit report generated at: {output_file}")
	except Exception as e:
		print(f"Error writing XUnit report: {e}")

def main():
	parser = argparse.ArgumentParser(description="Run ScummVM Event Recorder tests.")
	parser.add_argument("--xunit-output", help="Path to generate XUnit XML report", default=None)
	parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output", default=False)
	parser.add_argument("--filter", help="Filter tests (glob pattern, e.g. *monkey*)", default="*")
	parser.add_argument("--list", action="store_true", help="List tests", default=False)
	args = parser.parse_args()

	# Configuration
	# Determine default binary name based on OS
	bin_name = "scummvm.exe" if platform.system() == "Windows" else "./scummvm"

	# Allow overriding via env vars
	# Default to looking in current directory
	scummvm_bin = os.getenv("SCUMMVM_BIN", bin_name)
	if os.getenv("GAMES_DIR", None):
		games_dir = Path(os.getenv("GAMES_DIR"))
	else:
		games_dir = None

	# Check if ScummVM exists
	if not Path(scummvm_bin).exists():
		print(f"Error: ScummVM binary not found at {scummvm_bin}")
		print("Please run this script from the root of the ScummVM source tree where the binary is built.")
		sys.exit(127)

	# Check if demos directory exists
	if games_dir is not None:
		if not games_dir.exists():
			print(f"Error: Games directory not found at {games_dir}")
			sys.exit(127)

		# Register games
		for game_path in sorted(games_dir.iterdir()):
			if not game_path.is_dir():
				continue

			try:
				detect_cmd = [
					str(scummvm_bin),
					"--add",
					f"--path={game_path}"
				]
				subprocess.run(detect_cmd, capture_output=True)
			except Exception as e:
				print(f"Error detecting games in {game_path}: {e}")
				continue

	# Get available engines
	available_engines = set()
	try:
		list_engines_cmd = [str(scummvm_bin), "--list-engines-json"]
		result = subprocess.run(list_engines_cmd, capture_output=True, text=True)
		if result.returncode == 0:
			try:
				engines_map = json.loads(result.stdout)
				available_engines = set(engines_map.keys())
			except json.JSONDecodeError:
				print(f"Warning: Failed to parse engines JSON. Output was:\n{result.stdout}")
		else:
			print(f"Warning: Failed to list engines: {result.stderr}")
	except Exception as e:
		print(f"Warning: Error executing ScummVM to list engines: {e}")

	# Collect tests
	test_cases = []
	try:
		list_cmd = [str(scummvm_bin), "--list-records-json"]
		result = subprocess.run(list_cmd, capture_output=True, text=True)
		if result.returncode != 0:
			print(f"Error listing records: {result.stderr}")
			sys.exit(1)
	except Exception as e:
		print(f"Error executing ScummVM to list records: {e}")
		sys.exit(1)

	try:
		json_output = result.stdout.strip()
		records_map = json.loads(json_output)
	except json.JSONDecodeError as e:
		print(f"Error parsing records JSON: {e}\nOutput was:\n{result.stdout}")
		sys.exit(1)

	for target, info in records_map.items():
		engine_id = info.get('engine')
		for record_file in info.get('records', []):
			# Test name: EventRecorderTest.<RecordFileName>
			# We use the filename as the test case name
			test_case_name = record_file
			full_test_name = f"EventRecorderTest.{test_case_name}"

			# Check filter
			# Match against full name or just the test case name
			if not fnmatch.fnmatch(full_test_name, args.filter) and not fnmatch.fnmatch(test_case_name, args.filter):
				continue

			test_cases.append({
				'full_name': full_test_name,
				'test_case': test_case_name,
				'record_file': record_file,
				'target': target,
				'engine_id': engine_id
			})

	if args.list:
		print("EventRecorderTest.")
		for test in test_cases:
			print(f"  {test['test_case']}")
		sys.exit(0)

	total_tests = len(test_cases)

	# Googletest compatible header
	print(f"[==========] {total_tests} tests from 1 test suite ran.")
	print(f"[----------] {total_tests} tests from EventRecorderTest")

	start_total_time = time.time()
	passed_tests = []
	failed_tests = []
	skipped_tests = []
	results_for_xunit = []

	for test in test_cases:
		print(f"[ RUN      ] {test['full_name']}")
		sys.stdout.flush()

		test_start_time = time.time()
		success = False
		message = ""
		output = ""

		if available_engines:
			engine_id = test.get('engine_id')
			if engine_id and engine_id not in available_engines:
				print(f"[  SKIPPED ] {test['full_name']} (Engine '{engine_id}' not available)")
				results_for_xunit.append({
					'name': test['full_name'],
					'test_case': test['test_case'],
					'success': False,
					'skipped': True,
					'duration': 0,
					'message': f"Engine '{engine_id}' not available",
					'output': ""
				})
				skipped_tests.append(test['full_name'])
				continue

		try:
			playback_cmd = [
				str(scummvm_bin),
				"--record-mode=fast_playback",
				f"--record-file-name={test['record_file']}",
				test['target']
			]

			# Run and capture output
			if args.verbose:
				print(f"Executing: {' '.join(playback_cmd)}")
			playback_result = subprocess.run(playback_cmd, capture_output=True, text=True)
			output = playback_result.stdout + "\n" + playback_result.stderr

			if args.verbose:
				if playback_result.stdout.strip():
					print(playback_result.stdout)
				if playback_result.stderr.strip():
					print(playback_result.stderr)

			if playback_result.returncode == 0:
				success = True
			else:
				success = False
				message = f"Exit Code: {playback_result.returncode}"

		except Exception as e:
			success = False
			message = str(e)
			output += f"\nException: {e}"

		duration_sec = time.time() - test_start_time
		duration_ms = int(duration_sec * 1000)

		results_for_xunit.append({
			'name': test['full_name'],
			'test_case': test['test_case'],
			'success': success,
			'duration': duration_sec,
			'message': message,
			'output': output
		})

		if success:
			print(f"[       OK ] {test['full_name']} ({duration_ms} ms)")
			passed_tests.append(test['full_name'])
		else:
			print(f"[  FAILED  ] {test['full_name']} ({duration_ms} ms)")
			failed_tests.append(test['full_name'])

	total_duration_sec = time.time() - start_total_time
	total_duration_ms = int(total_duration_sec * 1000)

	print(f"[----------] {total_tests} tests from EventRecorderTest ({total_duration_ms} ms total)")
	print(f"[==========] {total_tests} tests from 1 test suite ran. ({total_duration_ms} ms total)")
	print(f"[  PASSED  ] {len(passed_tests)} tests.")

	if args.xunit_output:
		generate_xunit_report(results_for_xunit, args.xunit_output, total_duration_sec)

	if failed_tests:
		print(f"[  FAILED  ] {len(failed_tests)} tests, listed below:")
		for failed in failed_tests:
			print(f"[  FAILED  ] {failed}")
		sys.exit(1)

	if skipped_tests:
		print(f"[  SKIPPED ] {len(skipped_tests)} tests.")
		for skipped in skipped_tests:
			print(f"[  SKIPPED ] {skipped}")

	if total_tests == 0:
		print("No tests were run.")
		sys.exit(0)

	sys.exit(0)

if __name__ == "__main__":
	main()
