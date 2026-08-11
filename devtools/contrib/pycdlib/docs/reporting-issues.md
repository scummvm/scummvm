# What to do when things go wrong
The PyCdlib library can fail for a number of reasons.  The most common reason for an unexpected failure is when an ISO file doesn't follow the relevant standards. In these cases, PyCdlib will usually throw a `PyCdlibInvalidISO` exception (though it can also throw a `PyCdlibInternalError` in some circumstances).  If this happens, the absolute best thing to do is to open up a new [issue](https://github.com/clalancette/pycdlib/issues), putting a pointer to the problematic ISO in the issue.  If the ISO file cannot be shared, then a new issue should be created anyway, with the relevant backtrace from PyCdlib in it.  From there, additional information can sometimes help to identify the root cause of the issue.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="exceptions.html"><-- Exceptions</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
    </div>
</div>
