#import <Cocoa/Cocoa.h>

#include <stdio.h>

void AGSMacInitPaths(char gamename[256], char appdata[PATH_MAX])
{
  strcpy(gamename, "game.ags");
    
  @autoreleasepool {
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *resourcedir = [bundle resourcePath];
  [[NSFileManager defaultManager] changeCurrentDirectoryPath:resourcedir];

  NSURL *path = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:NULL];
  
  snprintf(appdata, PATH_MAX, "%s", [[path path] UTF8String]);
  }
}

// e.g. "/Users/<username>/Library/Application Support/Steam/steamapps/common/<gamename>"
void AGSMacGetBundleDir(char gamepath[PATH_MAX])
{
  @autoreleasepool {
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *bundleDir = [bundle bundlePath];

  NSString *parentDir = [bundleDir stringByDeletingLastPathComponent];
  strcpy(gamepath, [parentDir UTF8String]);
  }
}
