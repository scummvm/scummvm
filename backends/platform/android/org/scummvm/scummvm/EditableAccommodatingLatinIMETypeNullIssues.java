package org.scummvm.scummvm;

import android.text.SpannableStringBuilder;

public class EditableAccommodatingLatinIMETypeNullIssues extends SpannableStringBuilder {
	EditableAccommodatingLatinIMETypeNullIssues(CharSequence source) {
		super(source);
	  }

	//This character must be ignored by your onKey() code.
	public static final CharSequence ONE_UNPROCESSED_CHARACTER = "\\";

	@Override
	public SpannableStringBuilder replace(final int spannableStringStart, final int spannableStringEnd, CharSequence replacementSequence, int replacementStart, int replacementEnd) {
	if (replacementEnd > replacementStart) {
		//In this case, there is something in the replacementSequence that the IME
		// is attempting to replace part of the editable with.
		//We don't really care about whatever might already be in the editable;
		// we only care about making sure that SOMETHING ends up in it,
		// so that the backspace key will continue to work.
		// So, start by zeroing out whatever is there to begin with.
		super.replace(0, length(), "", 0, 0);

		//We DO care about preserving the new stuff that is replacing the stuff in the
		// editable, because this stuff might be sent to us as a keyDown event.  So, we
		// insert the new stuff (typically, a single character) into the now-empty editable,
		// and return the result to the caller.
		return super.replace(0, 0, replacementSequence, replacementStart, replacementEnd);
	} else if (spannableStringEnd > spannableStringStart) {
		//In this case, there is NOTHING in the replacementSequence, and something is
		// being replaced in the editable.
		// This is characteristic of a DELETION.
		// So, start by zeroing out whatever is being replaced in the editable.
		super.replace(0, length(), "", 0, 0);

		//And now, we will place our ONE_UNPROCESSED_CHARACTER into the editable buffer, and return it.
		return super.replace(0, 0, ONE_UNPROCESSED_CHARACTER, 0, 1);
    }

	// In this case, NOTHING is being replaced in the editable.  This code assumes that there
	// is already something there.  This assumption is probably OK because in our
	// InputConnectionAccommodatingLatinIMETypeNullIssues.getEditable() method
	// we PLACE a ONE_UNPROCESSED_CHARACTER into the newly-created buffer.  So if there
	// is nothing replacing the identified part
	// of the editable, and no part of the editable that is being replaced, then we just
	// leave whatever is in the editable ALONE,
	// and we can be confident that there will be SOMETHING there.  This call to super.replace()
	// in that case will be a no-op, except
	// for the value it returns.
	return super.replace(spannableStringStart, spannableStringEnd,
		replacementSequence, replacementStart, replacementEnd);
	}
}
