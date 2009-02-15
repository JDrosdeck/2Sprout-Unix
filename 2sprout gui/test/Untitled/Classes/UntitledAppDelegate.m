//
//  UntitledAppDelegate.m
//  Untitled
//
//  Created by Jonathan Drosdeck on 1/12/09.
//  Copyright Southern CT State University 2009. All rights reserved.
//

#import "UntitledAppDelegate.h"
#import "RootViewController.h"

@implementation UntitledAppDelegate


@synthesize window;
@synthesize rootViewController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    [window addSubview:[rootViewController view]];
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [rootViewController release];
    [window release];
    [super dealloc];
}

@end
