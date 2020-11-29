package org.scummvm.scummvm;

// Based on code from: https://stackoverflow.com/a/11024200
public class Version implements Comparable<Version> {

	private final String versionOnlyDigits;
	private final String versionDescription;

	public final String getDescription() {
		return this.versionDescription;
	}

	public final String get() {
		return this.versionOnlyDigits;
	}

	public Version(String version) {
		if(version == null) {
			this.versionOnlyDigits = "0";
			this.versionDescription = "0";
		} else {
			this.versionDescription = version;
			// cleanup from any non-digit characters in the version string
			final String strippedVersion = version.replaceAll("[^\\d.]", "");
			if (!strippedVersion.matches("[0-9]+(\\.[0-9]+)*")) {
				this.versionOnlyDigits = "0";
			} else {
				this.versionOnlyDigits = strippedVersion;
			}
		}
	}

	// Here a version is considered "dirty" if it contains other characters in the description string than the expected digits (and dots) of a "clean" proper version
	// eg. 2.3.0pre or 2.3.0git or 2.3.0git9272-gc71ac4748b are dirty
	//     2.3.0 is NOT dirty
	public boolean isDirty() {
		return (versionOnlyDigits.compareTo(versionDescription) != 0);
	}


	@Override public int compareTo(Version that) {
		if(that == null)
			return 1;
		String[] thisParts = this.get().split("\\.");
		String[] thatParts = that.get().split("\\.");
		int length = Math.max(thisParts.length, thatParts.length);
		try {
			for (int i = 0; i < length; i++) {
				int thisPart = i < thisParts.length ?
					Integer.parseInt(thisParts[i]) : 0;
				int thatPart = i < thatParts.length ?
					Integer.parseInt(thatParts[i]) : 0;
				if (thisPart < thatPart)
					return -1;
				if (thisPart > thatPart)
					return 1;
			}
		} catch (NumberFormatException e) {
			return 1;
		}
		return 0;
	}

	@Override public boolean equals(Object that) {
		if(this == that)
			return true;
		if(that == null)
			return false;
		if(this.getClass() != that.getClass())
			return false;
		return this.compareTo((Version) that) == 0;
	}

}
