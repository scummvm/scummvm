window.addEventListener("load", function(){

    var url = window.location.href;
    var start = url.indexOf("/", 27);
    var end = url.indexOf("/", start+1);
    var endMinus = url.indexOf("-", start+1);
    var versionNumber = url.substring(start+1, endMinus);
    var found = url.substring(start+1, end);
    var version = null;

    switch(found) {
      case 'latest':
        version = "development build"
        break;
      default:
        version = versionNumber
        break;
    }
});
