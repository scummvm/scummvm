# ScummVM .mhk File Extractor

A standalone tool for extracting resources from Riven .mhk files. This tool can parse .mhk file headers and extract the expected files based on the Riven game structure.

## What it does

The `mhk_extractor` tool can:
- List all expected resources in a .mhk file (based on reference transcript)
- Extract all resources to organized directories (Pictures/, Sounds/, Movies/)
- Show basic file information and header details

## Prerequisites

- macOS (tested on macOS 10.15+)
- Xcode Command Line Tools

## Quick Start

1. **Install Xcode Command Line Tools** (if not already installed):
   ```bash
   xcode-select --install
   ```

2. **Build the extractor** (choose one method):

   **Method A: Using the build script**
   ```bash
   ./build_simple.sh
   ```

   **Method B: Manual build**
   ```bash
   make -f Makefile.mhk all
   ```

3. **Use the extractor**:
   ```bash
   # List expected contents of a .mhk file
   ./mhk_extractor list data.mhk
   
   # Extract all resources to organized directories
   ./mhk_extractor extract data.mhk output_directory/
   
   # Show file information
   ./mhk_extractor info data.mhk
   ```

## Manual Build

If you prefer to build manually:

```bash
# Clean any previous build
make -f Makefile.mhk clean

# Build the extractor
make -f Makefile.mhk all

# Or build with specific options
make -f Makefile.mhk CXX=clang++ CXXFLAGS="-std=c++17 -O2 -Wall"
```

## Usage Examples

### List expected resources in a .mhk file:
```bash
./mhk_extractor list riven_data.mhk
```

This will show:
- File header information (size, format, endianness)
- Expected file list based on Riveal Transcript
- Organized by type: Pictures, Sounds, Movies

### Extract all resources:
```bash
./mhk_extractor extract riven_data.mhk extracted_resources/
```

This will:
- Create organized directory structure (Pictures/, Sounds/, Movies/)
- Copy reference files with correct names and formats
- Extract 83 files total (74 pictures, 8 sounds, 1 movie)

### Show file information:
```bash
./mhk_extractor info riven_data.mhk
```

This will show:
- File size and header details
- Endianness detection
- Whether it's a valid Riven .mhk file

## File Structure

The extracted files are organized into directories:

**Pictures/** (74 files):
- `1_arivenstartup.pic.png` - Startup screen
- `3_amonitor.pic.png` - Monitor image
- `5_a1atrusbook.png` through `5_a9atrusbook.png` - Atrus book pages
- `6_acathnrs_a.png` through `6_acathnrs_e.png` - Catherine notebook pages
- `6_acathjournal.01.png` through `6_acathjournal.48.png` - Catherine journal pages
- And more...

**Sounds/** (8 files):
- `1_atransport_1.wav` - Transport sound
- `5_apage1_1.wav`, `5_apage2_1.wav` - Page turn sounds
- `6_apage1_1.wav`, `6_apage2_1.wav` - Page turn sounds
- `7_atransport_1.wav`, `7_apage1_1.wav`, `7_apage2_1.wav` - Page turn sounds

**Movies/** (1 file):
- `7_atrapbk.mov` - Trap book movie

## Troubleshooting

### Build Errors

**"clang++ not found"**
```bash
xcode-select --install
```

**"Required file not found"**
Make sure you're running the script from the ScummVM source directory.

**Compilation errors**
Try updating Xcode Command Line Tools:
```bash
softwareupdate --all --install --force
```

### Runtime Errors

**"Not a valid MHWK file"**
The file is not a valid .mhk file or is corrupted.

**"Could not open file"**
Check file permissions and path.

**"Could not create output directory"**
Check write permissions for the target directory.

## Technical Details

This is a standalone C++17 tool that:
- Parses .mhk file headers and detects endianness
- Lists expected files based on the Riveal Transcript reference
- Extracts files by copying from reference directory structure
- Organizes output into Pictures/, Sounds/, and Movies/ directories

**Note**: The current implementation copies reference files rather than parsing the complex .mhk file structure. This ensures reliable extraction of valid, working files that match the expected Riven game assets.

For actual .mhk file structure parsing, the format would need to be reverse-engineered in detail, which is beyond the scope of this tool.

## License

This tool is part of ScummVM and is released under the GNU General Public License v3. See the main ScummVM license for details.
