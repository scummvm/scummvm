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
"""Script to generate a valid asset pack from a given assets folder.

Instant delivery is not supported.
"""

import argparse
import distutils.dir_util
import os
import shutil
import sys
import tempfile

manifest_template = """<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:dist="http://schemas.android.com/apk/distribution" package="{package_name}" split="{asset_pack_name}">
    <dist:module dist:type="asset-pack">
        <dist:delivery>
            <dist:{delivery_mode}/>
        </dist:delivery>
        <dist:fusing dist:include="true"/>
    </dist:module>
</manifest>
"""


def parse_args() -> argparse.Namespace:
    """Parse input arguments."""
    parser = argparse.ArgumentParser(
        description="Generates a valid asset pack from a given assets folder",
        add_help=True)
    parser.add_argument(
        "--packagename", required=True, help="Package name of the app")
    parser.add_argument(
        "--assetpackname", required=True, help="Name of the asset pack module")
    parser.add_argument(
        "--deliverymode",
        required=True,
        choices=["install-time", "fast-follow", "on-demand"],
        help="Delivery mode of the asset pack module")
    parser.add_argument(
        "--assetsdir", required=True, help="Folder to read assets from")
    parser.add_argument("--outdir", required=True, help="Output folder")
    parser.add_argument(
        "--overwrite",
        required=False,
        action="store_true",
        help="Overwrite existing files")
    return parser.parse_args()


def abs_expand_all(path: str) -> str:
    return os.path.abspath(os.path.expandvars(os.path.expanduser(path)))


def get_assets_dir(args: argparse.Namespace) -> str:
    assets_dir = abs_expand_all(args.assetsdir)
    if (not (os.path.isdir(assets_dir) and os.access(assets_dir, os.X_OK) and
             os.access(assets_dir, os.R_OK))):
        print(
            "Assets folder ({assets_dir}) is not accessible. Check permissions."
            .format(assets_dir=assets_dir),
            file=sys.stderr)
        sys.exit(-1)
    return assets_dir


def create_output_dir(args: argparse.Namespace) -> str:
    """Get the output directory."""
    output_dir = abs_expand_all(args.outdir)
    if not os.path.isdir(output_dir):
        try:
            os.makedirs(output_dir)
        except OSError as e:
            print(e, file=sys.stderr)
            sys.exit(-1)

    if (not (os.path.isdir(output_dir) and os.access(output_dir, os.X_OK) and
             os.access(output_dir, os.W_OK))):
        print(
            "Output folder ({output_dir}) is not accessible. Check permissions."
            .format(output_dir=output_dir),
            file=sys.stderr)
        sys.exit(-1)

    return output_dir


def get_output_file_name(output_dir: str, args: argparse.Namespace) -> str:
    output_file_name = os.path.join(output_dir, args.assetpackname)
    if os.path.exists(output_file_name) and not args.overwrite:
        print(
            "Output file {output_file_name} exists. Specify --overwrite to bypass. Exiting."
            .format(output_file_name=output_file_name))
        sys.exit(-1)
    return output_file_name


def make_manifest(package_name: str, asset_pack_name: str, delivery_mode: str,
                  pack_directory: str) -> None:
    """Generate the Android Manifest file for the pack."""
    manifest = manifest_template.format(
        package_name=package_name,
        asset_pack_name=asset_pack_name,
        delivery_mode=delivery_mode)

    manifest_folder = os.path.join(pack_directory, "manifest")
    try:
        os.makedirs(manifest_folder)
    except OSError as e:
        print("Cannot create manifest folder. {e}".format(
            e=e), file=sys.stderr)
        sys.exit(-1)

    manifest_file_name = os.path.join(manifest_folder, "AndroidManifest.xml")
    manifest_file = open(manifest_file_name, "w")
    print(manifest, file=manifest_file)
    manifest_file.close()
    print("Generated {manifest}".format(manifest=manifest_file_name))


def copy_assets(src: str, dest: str) -> None:
    """Copy assets from one folder to another."""
    assets_folder = os.path.join(dest, "assets")
    try:
        os.makedirs(assets_folder)
    except OSError as e:
        print("Cannot create assets folder. {e}".format(e=e), file=sys.stderr)
        sys.exit(-1)
    try:
        distutils.dir_util.copy_tree(src, assets_folder)
    except FileNotFoundError as e:
        print(
            "Cannot copy assets folder into temporary folder. {e}".format(e=e),
            file=sys.stderr)
        sys.exit(-1)

    print(
        "Copied assets into {assets_folder}".format(assets_folder=assets_folder))


def main():
    args = parse_args()
    assets_dir = get_assets_dir(args)
    output_dir = create_output_dir(args)
    output_file_name = get_output_file_name(output_dir, args)

    with tempfile.TemporaryDirectory(dir=output_dir) as pack_dir:
        print("Created temporary working folder: {pack_dir}".format(
            pack_dir=pack_dir))

        make_manifest(args.packagename, args.assetpackname, args.deliverymode,
                      pack_dir)
        copy_assets(assets_dir, pack_dir)

        output_pack_path = shutil.make_archive(
            os.path.join(output_dir, output_file_name), "zip", pack_dir)
        print("Asset pack is generated at {output_pack_path}.\nDone.".format(
            output_pack_path=output_pack_path))


if __name__ == "__main__":
    main()
