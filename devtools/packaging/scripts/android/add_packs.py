#!/usr/bin/env python3
#
# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Script to add Asset Packs to an already built Android App Bundle."""

import argparse
import fnmatch
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import typing
import xml.dom.minidom
import zipfile

import config_pb2

import google.protobuf.json_format as json_format

FASTFOLLOW = "dist:fast-follow"
ONDEMAND = "dist:on-demand"
UPFRONT = "dist:install-time"


def parse_args() -> argparse.Namespace:
    """Parse input arguments."""
    parser = argparse.ArgumentParser(
        description="Augments an Android App Bundle with given asset packs.",
        add_help=True)
    parser.add_argument(
        "--androidsdk", required=True, help="Android SDK location")
    parser.add_argument("--sdkver", required=True, help="Android SDK version")
    parser.add_argument(
        "--buildtoolsver", required=True, help="Android Build Tools version")
    parser.add_argument(
        "--bundletool", required=True, help="Path to Bundletool jar file")
    parser.add_argument(
        "--inputbundle", required=True, help="App Bundle to augment")
    parser.add_argument(
        "--packdir", required=True, help="Folder to read assets packs from")
    parser.add_argument(
        "--packnames",
        required=True,
        help="Comma separated list of asset pack files")
    parser.add_argument("--output", required=True, help="Output App Bundle")
    parser.add_argument(
        "--overwrite",
        required=False,
        action="store_true",
        help="Overwrite existing files")
    parser.add_argument(
        "--striptcfsuffixes",
        required=False,
        action="store_true",
        help="Enable removal of #tcf_xxx suffixes in asset pack folder names")
    parser.add_argument(
        "--compressinstalltimeassets",
        required=False,
        action="store_true",
        help=("Compress assets within install time asset packs."
              "This will not apply to on demand or fast follow asset packs"
              "Setting is overridden for files matched in the uncompressed glob"))

    return parser.parse_args()


def abs_expand_all(path: str) -> str:
    return os.path.abspath(os.path.expandvars(os.path.expanduser(path)))


def get_aapt2_bin_path(args: argparse.Namespace) -> str:
    """Retrieve the path for the aapt2 binary."""
    android_sdk_path = abs_expand_all(args.androidsdk)
    build_tools_version = args.buildtoolsver

    aapt2_bin_path: str
    if platform.system() == "Windows":
        aapt2_bin_path = os.path.join(android_sdk_path, "build-tools",
                                      build_tools_version, "aapt2.exe")
    else:
        aapt2_bin_path = os.path.join(android_sdk_path, "build-tools",
                                      build_tools_version, "aapt2")

    if not os.path.exists(aapt2_bin_path):
        print(
            "Cannot find AAPT2 at {aapt2_bin_path}".format(
                aapt2_bin_path=aapt2_bin_path),
            file=sys.stderr)
        sys.exit(-1)

    return aapt2_bin_path


def get_sdk_jar_path(args: argparse.Namespace) -> str:
    """Retrieve the path for the android SDK JAR file."""

    android_sdk_path = abs_expand_all(args.androidsdk)
    sdk_ver = args.sdkver

    sdk_jar_path = os.path.join(android_sdk_path, "platforms",
                                "android-" + sdk_ver, "android.jar")
    if not os.path.exists(sdk_jar_path):
        print(
            "Cannot find android.jar at {sdk_jar_path}".format(
                sdk_jar_path=sdk_jar_path),
            file=sys.stderr)
        sys.exit(-1)

    return sdk_jar_path


def get_bundletool_path(args: argparse.Namespace) -> str:
    """Retrieve the path for the BundleTool JAR file."""

    bundletool_path = abs_expand_all(args.bundletool)
    if not os.path.exists(bundletool_path):
        print(
            "Cannot find Bundletool at {bundletool_path}".format(
                bundletool_path=bundletool_path),
            file=sys.stderr)
        sys.exit(-1)
    return bundletool_path


def get_packs(args: argparse.Namespace) -> (str, typing.List[str]):
    """Retrieve the pack directory and pack names from the input flags."""

    pack_dir = abs_expand_all(args.packdir)
    if not os.path.exists(pack_dir):
        print(
            "Cannot find asset pack directory at {pack_dir}".format(
                pack_dir=pack_dir),
            file=sys.stderr)
        sys.exit(-1)

    pack_names = args.packnames.split(",")
    for pack_name in pack_names:
        pack_path = os.path.join(pack_dir, pack_name)
        if not os.path.exists(pack_path):
            print(
                "Cannot find asset pack {pack_name} at {pack_path}".format(
                    pack_name=pack_name, pack_path=pack_path),
                file=sys.stderr)
            sys.exit(-1)

    return (pack_dir, pack_names)


def get_input_bundle_path(args: argparse.Namespace) -> str:
    bundle_path = abs_expand_all(args.inputbundle)
    if not os.path.exists(bundle_path):
        print(
            "Cannot find input app bundle_path {bundle_path}".format(
                bundle_path=bundle_path),
            file=sys.stderr)
        sys.exit(-1)
    return bundle_path


def get_output_path(args: argparse.Namespace) -> str:
    """Retrieve the output file name."""
    output_path = abs_expand_all(args.output)
    if os.path.exists(output_path):
        if os.path.isdir(output_path):
            print(
                "Output location {output_path} is a directory. Specify a file path instead."
                .format(output_path=output_path))
            sys.exit(-1)
        if not args.overwrite:
            print(
                "Output file {output_path} exists. Specify --overwrite to bypass. Exiting."
                .format(output_path=output_path))
            sys.exit(-1)
        else:
            os.remove(output_path)
    return output_path


def purge_files(directory: str, pattern: str) -> None:
    """Purge the files inside a directory that match the given pattern."""
    for root_dir, _, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, pattern):
            try:
                os.remove(os.path.join(root_dir, filename))
            except OSError as e:
                print(
                    "Error while deleting {filename}: {e}".format(
                        filename=filename, e=e),
                    file=sys.stderr)


def purge_subdirs(directory: str, pattern: str) -> None:
    """Purge the subdirectories inside a directory that match the given pattern."""
    for root_dir, subdirs, _ in os.walk(directory):
        for filename in fnmatch.filter(subdirs, pattern):
            try:
                shutil.rmtree(os.path.join(root_dir, filename))
            except FileNotFoundError as e:
                print(
                    "Error while deleting {filename}: {e}".format(
                        filename=filename, e=e),
                    file=sys.stderr)


def parse_bundle_metadata(bundle_folder: str) -> typing.List[str]:
    """Parse the Bundle Metadata from the given bundle directory."""
    metadata = []
    metadata_folder = os.path.join(bundle_folder, "BUNDLE-METADATA")
    if not os.path.isdir(metadata_folder):
        return

    for folder in os.listdir(metadata_folder):
        inner_directory = os.path.join(metadata_folder, folder)
        if not os.path.isdir(inner_directory):
            continue

        for file in os.listdir(inner_directory):
            entry = "{path_in_bundle}:{physical_file_path}".format(
                path_in_bundle=os.path.join(folder, file),
                physical_file_path=os.path.join(inner_directory, file))
            metadata.append(entry)
    return metadata


def get_min_sdk_version(bundle_path: str, bundletool: str) -> int:
    """Get the minimum supported SDK version from an App Bundle file."""
    bundletool_cmd = [
        "java", "-jar", bundletool, "dump", "manifest", "--bundle", bundle_path,
        "--xpath", "/manifest/uses-sdk/@android:minSdkVersion"
    ]

    print("Running {bundletool_cmd}".format(bundletool_cmd=bundletool_cmd))
    min_sdk = subprocess.check_output(bundletool_cmd)
    return int(min_sdk.decode("utf-8").rstrip())


def get_strip_tcf_suffixes(args: argparse.Namespace) -> bool:
    return args.striptcfsuffixes


def get_compress_install_time_assets(args: argparse.Namespace) -> bool:
    return args.compressinstalltimeassets


def get_asset_pack_type(path: str) -> str:
    """Retrieve the Asset Pack delivery type from an AndroidManifest.xml file."""
    xmldoc = xml.dom.minidom.parse(path)
    tags = xmldoc.getElementsByTagName(ONDEMAND)
    if tags.length:
        return ONDEMAND

    tags = xmldoc.getElementsByTagName(FASTFOLLOW)
    if tags.length:
        return FASTFOLLOW

    tags = xmldoc.getElementsByTagName(UPFRONT)
    if tags.length:
        return UPFRONT

    return None


def extract_bundle_config(bundle_folder: str, add_standalone_config: bool,
                          strip_tcf_suffixes: bool,
                          compress_install_time_assets: bool) -> str:
    """Extract the BundleConfig contents and optionally add standalone_config."""
    bundle_config = config_pb2.BundleConfig()
    with open(os.path.join(bundle_folder, "BundleConfig.pb"), mode="rb") as f:
        content = f.read()
        bundle_config.ParseFromString(content)

    if compress_install_time_assets:
        json_format.ParseDict(
            {
                "compression": {
                    "install_time_asset_module_default_compression": "COMPRESSED"
                }
            }, bundle_config)

    if add_standalone_config:
        json_format.ParseDict(
            {
                "optimizations": {
                    "standalone_config": {
                        "split_dimension": [{
                            "value": "ABI",
                            "negate": True
                        }, {
                            "value": "TEXTURE_COMPRESSION_FORMAT",
                            "negate": True
                        }, {
                            "value": "LANGUAGE",
                            "negate": True
                        }, {
                            "value": "SCREEN_DENSITY",
                            "negate": True
                        }],
                        "strip_64_bit_libraries": True
                    }
                }
            }, bundle_config)

    # Check if game already defines any split_config dimensions
    dimensions = []
    try:
        dimensions = list(json_format.MessageToDict(bundle_config)["optimizations"]
                          ["splitsConfig"]["splitDimension"])
    except KeyError:
        print("No existing split dimensions")

    tcf_split_dimension = {
        "value": "TEXTURE_COMPRESSION_FORMAT",
        "negate": False,
        "suffix_stripping": {
            "enabled": True,
            "default_suffix": ""
        }
    }

    # Add the TCF split dimension, if needed
    if strip_tcf_suffixes:
        dimensions.append(tcf_split_dimension)

    if strip_tcf_suffixes:
        json_format.ParseDict(
            {
                "optimizations": {
                    "splits_config": {
                        "split_dimension": dimensions
                    }
                }
            }, bundle_config)

    output_path = os.path.join(bundle_folder, "BundleConfig.pb.json")
    with open(output_path, mode="w") as f:
        print(json_format.MessageToJson(bundle_config), file=f)

    return output_path


def aapt_link(input_manifest_path: str, output_manifest_folder: str,
              aapt2_bin_path: str, sdk_jar_path: str):
    """Run aapt link to convert the manifest to proto format."""
    aapt_cmd = [
        aapt2_bin_path, "link", "--proto-format", "--output-to-dir", "-o",
        output_manifest_folder, "--manifest", input_manifest_path, "-I",
        sdk_jar_path
    ]
    print("    Running {aapt_cmd}".format(aapt_cmd=aapt_cmd))
    exit_code = subprocess.call(aapt_cmd)
    if exit_code != 0:
        print(
            "Error executing {aapt_cmd}".format(aapt_cmd=aapt_cmd), file=sys.stderr)
        sys.exit(-1)


def process_packs(packs_folder: str, bundle_folder: str,
                  pack_names: typing.List[str], aapt2_bin_path: str,
                  sdk_jar_path: str) -> bool:
    """Repackage all packs into modules."""
    print("Processing packs...")

    has_upfront_pack = False

    for pack_name in pack_names:
        print("  Pack {pack_name}".format(pack_name=pack_name))
        pack_basename = os.path.splitext(pack_name)[0]
        pack_folder = os.path.join(bundle_folder, pack_basename)
        os.makedirs(pack_folder)

        print("    Extracting pack {pack_name} to {pack_folder}.".format(
            pack_name=pack_name, pack_folder=pack_folder))
        pack_zip_path = zipfile.ZipFile(
            os.path.join(packs_folder, pack_name), "r")
        pack_zip_path.extractall(path=pack_folder)
        pack_zip_path.close()

        print("    Processing manifest.")
        manifest_folder = os.path.join(pack_folder, "manifest")
        original_manifest_path = os.path.join(manifest_folder,
                                              "AndroidManifest.xml")

        has_upfront_pack = has_upfront_pack or (
            get_asset_pack_type(original_manifest_path) == UPFRONT)

        tmp_manifest_path = os.path.join(bundle_folder, "manifest.xml")
        shutil.move(original_manifest_path, tmp_manifest_path)

        aapt_link(tmp_manifest_path, manifest_folder,
                  aapt2_bin_path, sdk_jar_path)

        print("    Cleaning up\n")
        os.remove(os.path.join(manifest_folder, "resources.pb"))
        os.remove(tmp_manifest_path)
    return has_upfront_pack


def clear_autogenerated_bundle_files(bundle_folder: str):
    print("Removing old META_INF and BundleConfig.pb")
    shutil.rmtree(os.path.join(bundle_folder, "META-INF"), ignore_errors=True)
    os.remove(os.path.join(bundle_folder, "BundleConfig.pb"))

    print("Removing old __MACOSX folders")
    purge_subdirs(bundle_folder, "__MACOSX")

    print("Removing old .DS_Store files")
    purge_files(bundle_folder, ".DS_Store")


def zip_module(module_folder: str, bundle_folder: str) -> str:

    print("  Module {module_folder}".format(module_folder=module_folder))
    basename = os.path.join(bundle_folder, module_folder)
    module_zip = shutil.make_archive(basename, "zip", root_dir=basename)
    return module_zip


def build_bundle(module_zip_files: typing.List[str], output_path: str,
                 bundle_config_path: str, metadata: typing.List[str],
                 bundletool_path: str):
    """Build the bundle using bundletool build-bundle."""
    bundletool_cmd = [
        "java", "-jar", bundletool_path, "build-bundle", "--modules",
        ",".join(module_zip_files), "--output", output_path, "--config",
        bundle_config_path
    ]

    for entry in metadata:
        bundletool_cmd.append("--metadata-file")
        bundletool_cmd.append(entry)

    print("Running {bundletool_cmd}".format(bundletool_cmd=bundletool_cmd))
    exit_code = subprocess.call(bundletool_cmd)

    if exit_code != 0:
        print(
            "Error executing {bundletool_cmd}".format(
                bundletool_cmd=bundletool_cmd),
            file=sys.stderr)
        sys.exit(-1)


def main() -> None:
    args = parse_args()

    bundle_path = get_input_bundle_path(args)
    output_path = get_output_path(args)

    aapt2_bin_path = get_aapt2_bin_path(args)
    sdk_jar_path = get_sdk_jar_path(args)
    bundletool_path = get_bundletool_path(args)
    (pack_dir, pack_names) = get_packs(args)
    strip_tcf_suffixes = get_strip_tcf_suffixes(args)
    compress_install_time_assets = get_compress_install_time_assets(args)

    with tempfile.TemporaryDirectory() as bundle_folder:
        print("Extracting input app bundle to {bundle_folder}".format(
            bundle_folder=bundle_folder))
        bundle_zip_path = zipfile.ZipFile(bundle_path, "r")
        bundle_zip_path.extractall(path=bundle_folder)
        bundle_zip_path.close()

        has_upfront_pack = process_packs(pack_dir, bundle_folder, pack_names,
                                         aapt2_bin_path, sdk_jar_path)

        uses_upfront_pre_l = has_upfront_pack and get_min_sdk_version(
            bundle_path, bundletool_path) < 21

        bundle_config_path = extract_bundle_config(bundle_folder,
                                                   uses_upfront_pre_l,
                                                   strip_tcf_suffixes,
                                                   compress_install_time_assets)

        clear_autogenerated_bundle_files(bundle_folder)

        print("Parsing bundle metadata...")
        metadata = parse_bundle_metadata(bundle_folder)

        print("Zipping module folders...")
        metadata = []
        module_folders = (
            module_folder for module_folder in os.listdir(bundle_folder)
            if module_folder != "BUNDLE-METADATA" and
            os.path.isdir(os.path.join(bundle_folder, module_folder)))
        module_zip_files = [
            zip_module(module_folder, bundle_folder)
            for module_folder in module_folders
        ]

        bundletool_cmd = [
            "java", "-jar", bundletool_path, "build-bundle", "--modules",
            ",".join(module_zip_files), "--output", output_path, "--config",
            bundle_config_path
        ]

        for entry in metadata:
            bundletool_cmd.append("--metadata-file")
            bundletool_cmd.append(entry)

        print("Running {bundletool_cmd}".format(bundletool_cmd=bundletool_cmd))
        exit_code = subprocess.call(bundletool_cmd)

        if exit_code != 0:
            print(
                "Error executing {bundletool_cmd}".format(
                    bundletool_cmd=bundletool_cmd),
                file=sys.stderr)
            sys.exit(-1)

        print("Augmented app bundle is ready at {output_path}".format(
            output_path=output_path))


if __name__ == "__main__":
    main()
