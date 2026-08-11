# Exceptions
When things go wrong, PyCdlib generally throws an exception.  There is a base exception called `PyCdlibException`, which is never itself thrown.  Instead, PyCdlib will throw one of the following exceptions, all of which are subclasses of `PyCdlibException`:

* PyCdlibInvalidISO - Thrown when PyCdlib can't successfully parse an ISO with one of the [open](pycdlib-api.html#PyCdlib-open) methods.  Usually this indicates that the ISO does not follow relevant standards, though it can also sometimes be a bug in PyCdlib itself.
* PyCdlibInvalidInput - Thrown when the user provides invalid input to a PyCdlib API.
* PyCdlibInternalError - Thrown when an unexpected situation happens within PyCdlib itself.  This can happen when there is a bug within PyCdlib itself, or sometimes when an ISO that doesn't conform to standards is parsed.

The arrangement of a base exception of `PyCdlibException` along with subclassed specified errors allows maximum error handling flexibility for client programs.  If a client program wants to handle all PyCdlib errors, it can catch `PyCdlibException`, but if it wants to do something different based on the exception type, it can catch the various exception types.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="tools.html"><-- Tools</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="reporting-issues.html">Reporting issues/bugs --></a>
    </div>
</div>
