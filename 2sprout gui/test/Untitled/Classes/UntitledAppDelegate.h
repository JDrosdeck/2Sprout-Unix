//
//  UntitledAppDelegate.h
//  Untitled
//
//  Created by Jonathan Drosdeck on 1/12/09.
//  Copyright Southern CT State University 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RootViewController;

@interface UntitledAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    RootViewController *rootViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet RootViewController *rootViewController;

@end

