#include "apple_script.h"
#import <Foundation/Foundation.h>


bool AppleScript::run_apple_script(const std::string &script)
{
    NSAppleScript *appleScript = [[NSAppleScript alloc] initWithSource:[NSString stringWithCString:script.c_str() encoding:NSUTF8StringEncoding]];
    if (appleScript == nil) {
        return false;
    }
    bool result = [appleScript executeAndReturnError:nil];
    [appleScript release];
    return result;
}
