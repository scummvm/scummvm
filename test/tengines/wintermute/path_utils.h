#include <cxxtest/TestSuite.h>
#include "engines/wintermute/utils/path_util.h"
/**
 * Test suite for the functions in engines/wintermute/utils/path_util.h
 *
 * NOTE: This is not a prescription;
 *       this was not written by the original engine author;
 *       this was not written by the engine porter.
 *
 * It might, however, help to spot variations in behavior that are introduced by modifications
 */

class PathUtilTestSuite : public CxxTest::TestSuite {
	public:
	const Common::String unixPath;
	const Common::String unixCapPath;
	const Common::String windowsPath;
	const Common::String windowsCapPath;
	const Common::String emptyString;
	const Common::String dualExtPath;
	const Common::String manyExtPath;
	const Common::String mixedSlashesPath1;
	const Common::String mixedSlashesPath2;
	const Common::String unixRelativePath;
	const Common::String windowsRelativePath;
	const Common::String unixDirPath;
	const Common::String windowsDirPath;
	PathUtilTestSuite () :
		unixPath("/some/file.ext"),
		unixCapPath("/SOME/FILE.EXT"),
		windowsPath("C:\\some\\file.ext"),
		windowsCapPath("C:\\SOME\\FILE.EXT"),
		emptyString(""),
		dualExtPath("/some/file.tar.gz"),
		manyExtPath("/some/file.tar.bz2.gz.zip"),
		mixedSlashesPath1("C:\\this/IS_REALLY\\weird.exe"),
		mixedSlashesPath2("/pretty\\weird/indeed.txt"),
		unixRelativePath("some/file.ext"),
		windowsRelativePath("some\\file.ext"),
		unixDirPath("/some/dir/"),
		windowsDirPath("C:\\some\\dir\\")
	{}
	void test_getdirectoryname() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(unixPath),
				Common::String("/some/")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(unixCapPath),
				Common::String("/SOME/")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(windowsPath),
				Common::String("C:\\some\\")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(windowsCapPath),
				Common::String("C:\\SOME\\")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(emptyString),
				Common::String("")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(unixDirPath),
				Common::String("/some/dir/")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(windowsDirPath),
				Common::String("C:\\some\\dir\\")
				);
	}

	void test_getfilename() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(unixPath),
				Common::String("file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(unixCapPath),
				Common::String("FILE.EXT")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(windowsPath),
				Common::String("file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(windowsCapPath),
				Common::String("FILE.EXT")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(emptyString),
				Common::String("")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(unixRelativePath),
				Common::String("file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(windowsRelativePath),
				Common::String("file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(windowsDirPath),
				Common::String("")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileName(unixDirPath),
				Common::String("")
				);
	}

	void test_getextension() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(windowsPath),
				Common::String("ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(windowsCapPath),
				Common::String("EXT")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(emptyString),
				Common::String("")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(dualExtPath),
				Common::String("gz")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(manyExtPath),
				Common::String("zip")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(unixRelativePath),
				Common::String("ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getExtension(windowsRelativePath),
				Common::String("ext")
				);
	}

	void test_getfilenamewithoutextension() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(windowsPath),
				Common::String("file")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(windowsCapPath),
				Common::String("FILE")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(emptyString),
				Common::String("")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(dualExtPath),
				Common::String("file.tar")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(manyExtPath),
				Common::String("file.tar.bz2.gz")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(unixRelativePath),
				Common::String("file")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getFileNameWithoutExtension(windowsRelativePath),
				Common::String("file")
				);
	}

	void test_combine_identity() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(windowsPath) +
				Wintermute::PathUtil::getFileNameWithoutExtension(windowsPath) +
				"." +
				Wintermute::PathUtil::getExtension(windowsPath),
				windowsPath
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(windowsCapPath) +
				Wintermute::PathUtil::getFileNameWithoutExtension(windowsCapPath) +
				"." +
				Wintermute::PathUtil::getExtension(windowsCapPath),
				windowsCapPath
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(unixCapPath) +
				Wintermute::PathUtil::getFileNameWithoutExtension(unixCapPath) +
				"." +
				Wintermute::PathUtil::getExtension(unixCapPath),
				unixCapPath
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::getDirectoryName(manyExtPath) +
				Wintermute::PathUtil::getFileNameWithoutExtension(manyExtPath) +
				"." +
				Wintermute::PathUtil::getExtension(manyExtPath),
				manyExtPath
				);
	}

	void test_normalize() {
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(windowsCapPath),
				Common::String("c:/some/file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(windowsPath),
				Common::String("c:/some/file.ext")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(mixedSlashesPath1),
				Common::String("c:/this/is_really/weird.exe")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(mixedSlashesPath2),
				Common::String("/pretty/weird/indeed.txt")
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(emptyString),
				emptyString
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(unixRelativePath),
				unixRelativePath
				);
		TS_ASSERT_EQUALS(
				Wintermute::PathUtil::normalizeFileName(windowsRelativePath),
				unixRelativePath // NOT windows
				);
	}
};
